// Headless replay runner for Verdant Emerald.
//
// Executes a compiled plan (produced by replay.py) against a ROM using libmgba:
// button input steps that run until a memory predicate holds, assertions,
// watches and save export. It knows nothing about the game: all game
// knowledge (symbols, struct offsets, flag layout) is compiled into the plan's
// RPN memory expressions by replay.py.
//
// Plan format: one op per line, "OP key=value ... | rpn tokens".
// Exit code: 0 = pass, 1 = fail, 2 = usage/setup error.

#include <mgba/core/core.h>
#include <mgba/core/log.h>
#include <mgba-util/vfs.h>
#include <png.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_OPS       4096
#define MAX_RPN       1024
#define MAX_SLOTS     64
#define MAX_WATCHES   64
#define MAX_DIAGS     32
#define MAX_SYMS      262144
#define STACK_DEPTH   64

enum RpnKind { R_NUM, R_READ, R_BINOP, R_NOT, R_BIT, R_CAP };
enum Mode { MODE_NONE, MODE_TAP, MODE_HOLD };
enum OpKind { OP_RUN, OP_UNTIL, OP_EXPECT, OP_CAPTURE, OP_WATCH, OP_SEEN, OP_EXPORTSAVE, OP_SCREENSHOT, OP_STATE,
              OP_TRACK, OP_RECORD, OP_LOGGED, OP_TRANSCRIPT };
enum TrackWhen { WHEN_CHANGE, WHEN_RISE, WHEN_FALL };

struct RpnTok { enum RpnKind kind; int64_t num; int size; int sign; char op[3]; };
struct Rpn { int n; struct RpnTok t[MAX_RPN]; };

struct Op {
    enum OpKind kind;
    int line;
    enum Mode mode;
    uint32_t buttons;
    int period, frames, timeout, slot, watch, stable, hasGuard, soft;
    int negate, update, start, initial, section, skipzero;
    enum TrackWhen when;
    char vkind[16], ckind[16], c2kind[16];
    struct Rpn extra[4];  // TRACK: turn, ctx, ctx2, key
    int numExtra;
    char text[256];
    char path[512];
    struct Rpn rpn;
    struct Rpn guard;  // optional: buttons are only pressed on frames where this holds
};

struct Watch { struct Rpn rpn; int active, seen; char name[64]; };
struct Diag { char name[64]; char kind[16]; struct Rpn rpn; };
struct Sym { char kind[16]; uint32_t value; char *name; };

static struct Op sOps[MAX_OPS];
static int sNumOps;
static struct Watch sWatches[MAX_WATCHES];
static struct Diag sDiags[MAX_DIAGS];
static int sNumDiags;
static struct Sym sSyms[MAX_SYMS];
static int sNumSyms;
static int64_t sSlots[MAX_SLOTS];
static char sRom[512], sSav[512], sArtifacts[512];
// ---- battle transcript recording ----
#define MAX_TRACKS   64
#define MAX_LINES    4096
struct Track { const struct Op *op; int64_t last, lastKey; int primed; };
static struct Track sTracks[MAX_TRACKS];
static int sNumTracks, sRecording;
static char *sLines[2][MAX_LINES];  // section 0 = battle events, 1 = on-screen text
static int sNumLines[2];
static char sTranscriptPath[512];
static uint32_t sTextAddr;
static int sTextLen;
static char *sCharmap[256];
static int sCtrlLen[256];          // EXT_CTRL_CODE_BEGIN code -> length including the code byte
struct TextSkip                    // ignore captured text starting (or ending) with a ROM string
{
    uint32_t addr;
    int suffix;
    int loaded;
    int len;
    uint8_t bytes[64];
};
static struct TextSkip sTextSkips[8];
static int sTextSkipCount;
static uint8_t sLastText[1024];

static struct mCore *sCore;
static color_t *sVideo;
static unsigned sWidth, sHeight;
static long sFrame;

static void quiet(struct mLogger *l, int c, enum mLogLevel lv, const char *f, va_list a) { (void)l; (void)c; (void)lv; (void)f; (void)a; }
static struct mLogger sQuietLogger = { .log = quiet };

static void die(const char *fmt, ...)
{
    va_list a;
    va_start(a, fmt);
    fprintf(stderr, "replay-runner: ");
    vfprintf(stderr, fmt, a);
    fprintf(stderr, "\n");
    va_end(a);
    exit(2);
}

// ---- plan parsing ----

static void parseRpn(struct Rpn *rpn, char *s)
{
    rpn->n = 0;
    for (char *tok = strtok(s, " \t\n"); tok; tok = strtok(NULL, " \t\n"))
    {
        if (rpn->n >= MAX_RPN)
            die("expression too long");
        struct RpnTok *t = &rpn->t[rpn->n++];
        memset(t, 0, sizeof(*t));
        if ((tok[0] >= '0' && tok[0] <= '9') || (tok[0] == '-' && tok[1] >= '0' && tok[1] <= '9'))
        {
            t->kind = R_NUM;
            t->num = strtoll(tok, NULL, 0);
        }
        else if (tok[0] == 'r' && (tok[1] == '8' || tok[1] == '1' || tok[1] == '3'))
        {
            t->kind = R_READ;
            t->size = atoi(tok + 1) / 8;
        }
        else if (tok[0] == 'r' && tok[1] == 's')
        {
            t->kind = R_READ;
            t->size = atoi(tok + 2) / 8;
            t->sign = 1;
        }
        else if (!strncmp(tok, "cap:", 4))
        {
            t->kind = R_CAP;
            t->num = atoi(tok + 4);
        }
        else if (!strcmp(tok, "!"))
            t->kind = R_NOT;
        else if (!strcmp(tok, "bit"))
            t->kind = R_BIT;
        else
        {
            t->kind = R_BINOP;
            strncpy(t->op, tok, 2);
        }
    }
}

static const char *getKey(const char *s, const char *key, char *out, size_t outSize)
{
    char pat[64];
    snprintf(pat, sizeof(pat), " %s=", key);
    const char *p = strstr(s, pat);
    if (!p)
        return NULL;
    p += strlen(pat);
    size_t i = 0;
    while (*p && *p != ' ' && *p != '|' && *p != '\n' && i + 1 < outSize)
        out[i++] = *p++;
    out[i] = 0;
    return out;
}

static int getInt(const char *s, const char *key, int def)
{
    char buf[64];
    return getKey(s, key, buf, sizeof(buf)) ? (int)strtol(buf, NULL, 0) : def;
}

// Name tables: lines of "<kind> <hex value> <name>". Kind "sym" values are
// code/data addresses (thumb bit ignored); other kinds (e.g. "map") are exact.
static void loadNames(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f)
        die("cannot open name table %s", path);
    char line[512];
    while (fgets(line, sizeof(line), f) && sNumSyms < MAX_SYMS)
    {
        char kind[16], name[400];
        unsigned value;
        if (sscanf(line, "%15s %x %399s", kind, &value, name) == 3)
        {
            struct Sym *s = &sSyms[sNumSyms++];
            snprintf(s->kind, sizeof(s->kind), "%s", kind);
            s->value = !strcmp(kind, "sym") ? (value & ~1u) : value;
            s->name = strdup(name);
        }
    }
    fclose(f);
}

static const char *lookupName(const char *kind, uint32_t value)
{
    if (!strcmp(kind, "sym"))
        value &= ~1u;
    for (int i = 0; i < sNumSyms; i++)
        if (sSyms[i].value == value && !strcmp(sSyms[i].kind, kind))
            return sSyms[i].name;
    return NULL;
}

static void loadPlan(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f)
        die("cannot open plan %s", path);
    char line[8192];
    while (fgets(line, sizeof(line), f))
    {
        char head[8200], *bar = strchr(line, '|');
        char rpnText[8192] = "";
        if (bar)
        {
            strcpy(rpnText, bar + 1);
            *bar = 0;
        }
        snprintf(head, sizeof(head), " %s", line);
        char word[32] = "";
        sscanf(line, "%31s", word);
        if (!word[0] || word[0] == '#')
            continue;

        char buf[512];
        if (!strcmp(word, "ROM"))
        {
            getKey(head, "path", sRom, sizeof(sRom));
            continue;
        }
        if (!strcmp(word, "SAV"))
        {
            getKey(head, "path", sSav, sizeof(sSav));
            continue;
        }
        if (!strcmp(word, "NAMES"))
        {
            if (getKey(head, "path", buf, sizeof(buf)))
                loadNames(buf);
            continue;
        }
        if (!strcmp(word, "ARTIFACTS"))
        {
            getKey(head, "path", sArtifacts, sizeof(sArtifacts));
            continue;
        }
        if (!strcmp(word, "TEXTSRC"))
        {
            sTextAddr = (uint32_t)strtoul(getKey(head, "addr", buf, sizeof(buf)) ? buf : "0", NULL, 0);
            sTextLen = getInt(head, "len", 0);
            if (sTextLen > (int)sizeof(sLastText))
                sTextLen = sizeof(sLastText);
            continue;
        }
        if (!strcmp(word, "TEXTSKIP"))
        {
            // TEXTSKIP addr=<string> [at=end]
            if (sTextSkipCount < (int)(sizeof(sTextSkips) / sizeof(sTextSkips[0])))
            {
                struct TextSkip *skip = &sTextSkips[sTextSkipCount++];
                skip->addr = (uint32_t)strtoul(getKey(head, "addr", buf, sizeof(buf)) ? buf : "0", NULL, 0);
                skip->suffix = getKey(head, "at", buf, sizeof(buf)) && !strcmp(buf, "end");
            }
            continue;
        }
        if (!strcmp(word, "CHARMAP"))
        {
            // lines: "<hex byte>\t<text>"
            if (!getKey(head, "path", buf, sizeof(buf)))
                continue;
            FILE *cf = fopen(buf, "r");
            if (!cf)
                die("cannot open charmap %s", buf);
            char cl[256];
            while (fgets(cl, sizeof(cl), cf))
            {
                char *tab = strchr(cl, '\t');
                if (!tab)
                    continue;
                *tab = 0;
                tab[1 + strcspn(tab + 1, "\n")] = 0;
                if (!strncmp(cl, "ctrl ", 5))
                {
                    unsigned code = (unsigned)strtoul(cl + 5, NULL, 16);
                    if (code < 256)
                        sCtrlLen[code] = atoi(tab + 1);
                    continue;
                }
                unsigned b = (unsigned)strtoul(cl, NULL, 16);
                if (b < 256 && !sCharmap[b])
                    sCharmap[b] = strdup(tab + 1);
            }
            fclose(cf);
            continue;
        }
        if (!strcmp(word, "DIAG"))
        {
            if (sNumDiags >= MAX_DIAGS)
                die("too many diagnostics");
            struct Diag *d = &sDiags[sNumDiags++];
            getKey(head, "name", d->name, sizeof(d->name));
            getKey(head, "kind", d->kind, sizeof(d->kind));
            parseRpn(&d->rpn, rpnText);
            continue;
        }

        if (sNumOps >= MAX_OPS)
            die("plan too long");
        struct Op *op = &sOps[sNumOps++];
        memset(op, 0, sizeof(*op));
        op->line = getInt(head, "line", 0);
        op->buttons = (uint32_t)getInt(head, "buttons", 0);
        op->period = getInt(head, "period", 8);
        op->frames = getInt(head, "frames", 0);
        op->timeout = getInt(head, "timeout", 600);
        op->slot = getInt(head, "slot", 0);
        op->watch = getInt(head, "watch", 0);
        op->stable = getInt(head, "stable", 1);
        op->soft = getInt(head, "soft", 0);  // soft: a timeout just moves on (used for probing)
        if (getKey(head, "mode", buf, sizeof(buf)))
            op->mode = !strcmp(buf, "tap") ? MODE_TAP : !strcmp(buf, "hold") ? MODE_HOLD : MODE_NONE;
        getKey(head, "path", op->path, sizeof(op->path));
        const char *txt = strstr(head, " text=");
        if (txt)
        {
            // text runs to the end of the head (it is always the last key)
            snprintf(op->text, sizeof(op->text), "%s", txt + 6);
            op->text[strcspn(op->text, "\n")] = 0;
            for (size_t l = strlen(op->text); l && op->text[l - 1] == ' '; l--)
                op->text[l - 1] = 0;
        }
        op->negate = getInt(head, "negate", 0);
        op->update = getInt(head, "update", 0);
        op->start = getInt(head, "start", 0);
        op->initial = getInt(head, "initial", 0);
        op->section = getInt(head, "section", 0);
        op->skipzero = getInt(head, "skipzero", 0);  // TRACK: don't log changes to 0 (teardown)
        if (getKey(head, "when", buf, sizeof(buf)))
            op->when = !strcmp(buf, "rise") ? WHEN_RISE : !strcmp(buf, "fall") ? WHEN_FALL : WHEN_CHANGE;
        getKey(head, "vkind", op->vkind, sizeof(op->vkind));
        getKey(head, "ckind", op->ckind, sizeof(op->ckind));
        getKey(head, "c2kind", op->c2kind, sizeof(op->c2kind));
        if (!strcmp(word, "LOGGED"))
        {
            // the part after '|' is the literal text to search for
            char *n = rpnText;
            while (*n == ' ')
                n++;
            snprintf(op->path, sizeof(op->path), "%s", n);
            op->path[strcspn(op->path, "\n")] = 0;
            op->kind = OP_LOGGED;
            continue;
        }
        // RPN segments separated by ";;". UNTIL: cond ;; guard. TRACK: value ;; turn ;; ctx ;; ctx2 ;; key.
        char *segs[6] = { rpnText };
        int numSegs = 1;
        for (char *p = strstr(rpnText, ";;"); p && numSegs < 6; p = strstr(p, ";;"))
        {
            *p = 0;
            p += 2;
            segs[numSegs++] = p;
        }
        if (!strcmp(word, "TRACK"))
        {
            for (int k = 1; k < numSegs; k++)
                parseRpn(&op->extra[k - 1], segs[k]);
            op->numExtra = numSegs - 1;
        }
        else if (numSegs > 1)
        {
            parseRpn(&op->guard, segs[1]);
            op->hasGuard = 1;
        }
        parseRpn(&op->rpn, segs[0]);

        if (!strcmp(word, "RUN"))            op->kind = OP_RUN;
        else if (!strcmp(word, "UNTIL"))     op->kind = OP_UNTIL;
        else if (!strcmp(word, "EXPECT"))    op->kind = OP_EXPECT;
        else if (!strcmp(word, "CAPTURE"))   op->kind = OP_CAPTURE;
        else if (!strcmp(word, "WATCH"))     op->kind = OP_WATCH;
        else if (!strcmp(word, "SEEN"))      op->kind = OP_SEEN;
        else if (!strcmp(word, "EXPORTSAVE")) op->kind = OP_EXPORTSAVE;
        else if (!strcmp(word, "SCREENSHOT")) op->kind = OP_SCREENSHOT;
        else if (!strcmp(word, "STATE"))     op->kind = OP_STATE;
        else if (!strcmp(word, "TRACK"))     op->kind = OP_TRACK;
        else if (!strcmp(word, "RECORD"))    op->kind = OP_RECORD;
        else if (!strcmp(word, "LOGGED"))    op->kind = OP_LOGGED;
        else if (!strcmp(word, "TRANSCRIPT")) op->kind = OP_TRANSCRIPT;
        else
            die("unknown plan op '%s'", word);
    }
    fclose(f);
}

// ---- evaluation ----

static int64_t readMem(uint32_t addr, int size, int sign)
{
    switch (size)
    {
    case 1: { uint32_t v = sCore->rawRead8(sCore, addr, -1); return sign ? (int8_t)v : (int64_t)v; }
    case 2: { uint32_t v = sCore->rawRead16(sCore, addr, -1); return sign ? (int16_t)v : (int64_t)v; }
    default: { uint32_t v = sCore->rawRead32(sCore, addr, -1); return sign ? (int32_t)v : (int64_t)v; }
    }
}

static int64_t eval(const struct Rpn *rpn)
{
    int64_t st[STACK_DEPTH];
    int sp = 0;
    for (int i = 0; i < rpn->n; i++)
    {
        const struct RpnTok *t = &rpn->t[i];
        switch (t->kind)
        {
        case R_NUM: st[sp++] = t->num; break;
        case R_CAP: st[sp++] = sSlots[t->num]; break;
        case R_READ: st[sp - 1] = readMem((uint32_t)st[sp - 1], t->size, t->sign); break;
        case R_NOT: st[sp - 1] = !st[sp - 1]; break;
        case R_BIT: { int64_t b = st[--sp]; st[sp - 1] = (st[sp - 1] >> b) & 1; break; }
        case R_BINOP:
        {
            int64_t b = st[--sp], a = st[sp - 1], r = 0;
            const char *o = t->op;
            if (!strcmp(o, "+")) r = a + b;
            else if (!strcmp(o, "-")) r = a - b;
            else if (!strcmp(o, "*")) r = a * b;
            else if (!strcmp(o, "/")) r = b ? a / b : 0;
            else if (!strcmp(o, "%")) r = b ? a % b : 0;
            else if (!strcmp(o, "&")) r = a & b;
            else if (!strcmp(o, "|")) r = a | b;
            else if (!strcmp(o, "^")) r = a ^ b;
            else if (!strcmp(o, "<<")) r = a << b;
            else if (!strcmp(o, ">>")) r = a >> b;
            else if (!strcmp(o, "==")) r = a == b;
            else if (!strcmp(o, "!=")) r = a != b;
            else if (!strcmp(o, "<")) r = a < b;
            else if (!strcmp(o, "<=")) r = a <= b;
            else if (!strcmp(o, ">")) r = a > b;
            else if (!strcmp(o, ">=")) r = a >= b;
            else if (!strcmp(o, "&&")) r = a && b;
            else if (!strcmp(o, "||")) r = a || b;
            else die("bad operator '%s'", o);
            st[sp - 1] = r;
            break;
        }
        }
        if (sp <= 0 || sp >= STACK_DEPTH)
            die("expression stack error");
    }
    return sp ? st[sp - 1] : 0;
}

// ---- artifacts ----

static void writePng(const char *path)
{
    FILE *f = fopen(path, "wb");
    if (!f)
        return;
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);
    if (setjmp(png_jmpbuf(png)))
    {
        png_destroy_write_struct(&png, &info);
        fclose(f);
        return;
    }
    png_init_io(png, f);
    png_set_IHDR(png, info, sWidth, sHeight, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);
    uint8_t *row = malloc(sWidth * 3);
    for (unsigned y = 0; y < sHeight; y++)
    {
        for (unsigned x = 0; x < sWidth; x++)
        {
            color_t c = sVideo[y * sWidth + x];  // 32-bit native: red in the low byte
            row[x * 3 + 0] = c & 0xFF;
            row[x * 3 + 1] = (c >> 8) & 0xFF;
            row[x * 3 + 2] = (c >> 16) & 0xFF;
        }
        png_write_row(png, row);
    }
    free(row);
    png_write_end(png, NULL);
    png_destroy_write_struct(&png, &info);
    fclose(f);
}

static int exportSave(const char *path)
{
    void *data = NULL;
    size_t size = sCore->savedataClone(sCore, &data);
    if (!size || !data)
        return 0;
    FILE *f = fopen(path, "wb");
    if (!f)
    {
        free(data);
        return 0;
    }
    fwrite(data, 1, size, f);
    fclose(f);
    free(data);
    return 1;
}

static void printState(void)
{
    printf("    state at frame %ld:\n", sFrame);
    for (int i = 0; i < sNumDiags; i++)
    {
        int64_t v = eval(&sDiags[i].rpn);
        const char *n = sDiags[i].kind[0] ? lookupName(sDiags[i].kind, (uint32_t)v) : NULL;
        if (n)
            printf("      %-16s %s\n", sDiags[i].name, n);
        else
            printf("      %-16s %lld (0x%llx)\n", sDiags[i].name, (long long)v, (unsigned long long)v);
    }
}

static void printDiagnostics(void)
{
    printState();
    if (sArtifacts[0])
    {
        char p[700];
        snprintf(p, sizeof(p), "%s/failure.png", sArtifacts);
        writePng(p);
        printf("    screenshot: %s\n", p);
    }
}

// ---- transcript ----

static void addLine(int section, const char *line)
{
    if (sNumLines[section] < MAX_LINES)
        sLines[section][sNumLines[section]++] = strdup(line);
}

static void formatName(char *out, size_t size, const char *kind, int64_t v)
{
    const char *n = kind[0] ? lookupName(kind, (uint32_t)v) : NULL;
    if (n)
        snprintf(out, size, "%s", n);
    else
        snprintf(out, size, "%lld", (long long)v);
}

// Expand a track template: {turn} {old} {new} {ctx} {ctx2}
static void emitTrack(const struct Op *op, int64_t oldV, int64_t newV)
{
    char out[512] = "", tmp[128];
    const char *t = op->text;
    while (*t && strlen(out) < sizeof(out) - 64)
    {
        if (*t == '{')
        {
            const char *end = strchr(t, '}');
            if (!end)
                break;
            size_t n = end - t - 1;
            #define IS(name) (n == strlen(name) && !strncmp(t + 1, name, n))
            tmp[0] = 0;
            if (IS("turn") && op->numExtra >= 1)
                snprintf(tmp, sizeof(tmp), "%lld", (long long)eval(&op->extra[0]));
            else if (IS("old"))
                formatName(tmp, sizeof(tmp), op->vkind, oldV);
            else if (IS("new"))
                formatName(tmp, sizeof(tmp), op->vkind, newV);
            else if (IS("ctx2") && op->numExtra >= 3)
                formatName(tmp, sizeof(tmp), op->c2kind, eval(&op->extra[2]));
            else if (IS("ctx") && op->numExtra >= 2)
                formatName(tmp, sizeof(tmp), op->ckind, eval(&op->extra[1]));
            #undef IS
            strcat(out, tmp);
            t = end + 1;
        }
        else
        {
            size_t l = strlen(out);
            out[l] = *t++;
            out[l + 1] = 0;
        }
    }
    addLine(0, out);
}

static void sampleTracks(int force)
{
    for (int i = 0; i < sNumTracks; i++)
    {
        struct Track *tr = &sTracks[i];
        int64_t v = eval(&tr->op->rpn);
        int64_t key = tr->op->numExtra >= 4 && tr->op->extra[3].n ? eval(&tr->op->extra[3]) : 0;
        if (tr->primed && key != tr->lastKey)
        {
            // the thing being tracked was replaced (e.g. a switch-in): resync silently
            tr->lastKey = key;
            tr->last = v;
            continue;
        }
        if (!tr->primed)
        {
            tr->primed = 1;
            tr->last = v;
            tr->lastKey = key;
            if (force && tr->op->initial)
                emitTrack(tr->op, v, v);
            continue;
        }
        if (v == tr->last)
            continue;
        int fire = tr->op->when == WHEN_CHANGE || (tr->op->when == WHEN_RISE && v > tr->last)
                || (tr->op->when == WHEN_FALL && v < tr->last);
        if (fire && !(tr->op->skipzero && v == 0))
            emitTrack(tr->op, tr->last, v);
        tr->last = v;
    }
}

// On-screen battle text: decode the message buffer when it changes.
static void sampleText(void)
{
    if (!sTextAddr || sTextLen <= 0)
        return;
    uint8_t cur[1024];
    for (int i = 0; i < sTextLen; i++)
        cur[i] = sCore->rawRead8(sCore, sTextAddr + i, -1);
    if (!memcmp(cur, sLastText, sTextLen))
        return;
    memcpy(sLastText, cur, sTextLen);
    int eos = 0;
    while (eos < sTextLen && cur[eos] != 0xFF)
        eos++;
    for (int k = 0; k < sTextSkipCount; k++)
    {
        struct TextSkip *skip = &sTextSkips[k];
        if (!skip->loaded)
        {
            // read the string from ROM once (up to EOS)
            skip->loaded = 1;
            for (int i = 0; i < (int)sizeof(skip->bytes); i++)
            {
                uint8_t b = sCore->rawRead8(sCore, skip->addr + i, -1);
                if (b == 0xFF)
                    break;
                skip->bytes[skip->len++] = b;
            }
        }
        if (!skip->len || skip->len > eos)
            continue;
        if (!memcmp(skip->suffix ? cur + eos - skip->len : cur, skip->bytes, skip->len))
            return;
    }
    char out[1024] = "";
    for (int i = 0; i < sTextLen && cur[i] != 0xFF && strlen(out) < sizeof(out) - 16; i++)
    {
        uint8_t b = cur[i];
        if (b == 0xFC && i + 1 < sTextLen)  // control code: skip it and its arguments
            i += sCtrlLen[cur[i + 1]] ? sCtrlLen[cur[i + 1]] : 1;
        else if (b == 0xFA || b == 0xFB || b == 0xFE)
            strcat(out, " ");   // scroll / paragraph / newline
        else if (sCharmap[b])
            strcat(out, sCharmap[b]);
    }
    // collapse repeated spaces, trim
    char clean[1024];
    int j = 0;
    for (int i = 0; out[i]; i++)
        if (!(out[i] == ' ' && (j == 0 || clean[j - 1] == ' ')))
            clean[j++] = out[i];
    while (j && clean[j - 1] == ' ')
        j--;
    clean[j] = 0;
    if (j && (!sNumLines[1] || strcmp(sLines[1][sNumLines[1] - 1], clean)))
        addLine(1, clean);
}

static void writeTranscript(const char *path)
{
    FILE *f = fopen(path, "w");
    if (!f)
        return;
    fprintf(f, "# battle\n");
    for (int i = 0; i < sNumLines[0]; i++)
        fprintf(f, "%s\n", sLines[0][i]);
    fprintf(f, "# text\n");
    for (int i = 0; i < sNumLines[1]; i++)
        fprintf(f, "%s\n", sLines[1][i]);
    fclose(f);
}

// Compare the recording to a golden file section by section.
// Returns 0 if both match, else prints the first difference of each section that differs.
static int compareTranscript(const char *golden, int line)
{
    FILE *f = fopen(golden, "r");
    if (!f)
    {
        printf("FAIL line %d: no golden transcript at %s (run with --update-golden to create it)\n", line, golden);
        return 1;
    }
    char *glines[2][MAX_LINES];
    int gn[2] = { 0, 0 }, section = 0;
    char buf[1024];
    while (fgets(buf, sizeof(buf), f))
    {
        buf[strcspn(buf, "\n")] = 0;
        if (!strcmp(buf, "# battle")) { section = 0; continue; }
        if (!strcmp(buf, "# text"))   { section = 1; continue; }
        if (gn[section] < MAX_LINES)
            glines[section][gn[section]++] = strdup(buf);
    }
    fclose(f);
    static const char *names[2] = { "battle events", "on-screen text" };
    int bad[2] = { 0, 0 };
    for (int s = 0; s < 2; s++)
    {
        int n = gn[s] > sNumLines[s] ? gn[s] : sNumLines[s];
        for (int i = 0; i < n; i++)
        {
            const char *g = i < gn[s] ? glines[s][i] : "(nothing)";
            const char *r = i < sNumLines[s] ? sLines[s][i] : "(nothing)";
            if (strcmp(g, r))
            {
                printf("FAIL line %d: transcript %s differ from %s at entry %d\n", line, names[s], golden, i + 1);
                printf("    expected: %s\n    got:      %s\n", g, r);
                bad[s] = 1;
                break;
            }
        }
    }
    if (bad[1] && !bad[0])
        printf("    (only the wording changed: the battle itself played out as expected)\n");
    return bad[0] || bad[1];
}

// ---- execution ----

static void runFrame(uint32_t keys)
{
    sCore->setKeys(sCore, keys);
    sCore->runFrame(sCore);
    sFrame++;
    if (sRecording)
    {
        sampleTracks(0);
        sampleText();
    }
    for (int i = 0; i < MAX_WATCHES; i++)
        if (sWatches[i].active && !sWatches[i].seen && eval(&sWatches[i].rpn))
            sWatches[i].seen = 1;
}

static uint32_t keysFor(const struct Op *op, int i)
{
    if (op->hasGuard && !eval(&op->guard))
        return 0;
    if (op->mode == MODE_HOLD)
        return op->buttons;
    if (op->mode == MODE_TAP)
        return (i % op->period) < 2 ? op->buttons : 0;
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "usage: %s plan.txt\n", argv[0]);
        return 2;
    }
    loadPlan(argv[1]);
    if (!sRom[0])
        die("plan has no ROM");

    mLogSetDefaultLogger(&sQuietLogger);
    sCore = mCoreFind(sRom);
    if (!sCore || !sCore->init(sCore))
        die("no emulator core for %s", sRom);
    sCore->desiredVideoDimensions(sCore, &sWidth, &sHeight);
    sVideo = calloc(sWidth * sHeight, sizeof(color_t));
    sCore->setVideoBuffer(sCore, sVideo, sWidth);
    mCoreInitConfig(sCore, NULL);
    if (!mCoreLoadFile(sCore, sRom))
        die("cannot load ROM %s", sRom);
    if (sSav[0])
    {
        // Load the save into memory only (no write-back): runs never modify the depot.
        struct VFile *vf = VFileOpen(sSav, O_RDONLY);
        if (!vf)
            die("cannot open save %s", sSav);
        size_t size = vf->size(vf);
        void *buf = malloc(size);
        vf->read(vf, buf, size);
        vf->close(vf);
        if (!sCore->savedataRestore(sCore, buf, size, false))
            die("cannot restore save %s", sSav);
        free(buf);
    }
    sCore->reset(sCore);

    int failed = 0;
    for (int o = 0; o < sNumOps && !failed; o++)
    {
        struct Op *op = &sOps[o];
        long start = sFrame;
        switch (op->kind)
        {
        case OP_RUN:
            for (int i = 0; i < op->frames; i++)
                runFrame(keysFor(op, i));
            break;
        case OP_UNTIL:
        {
            int i = 0, held = 0;
            for (;;)
            {
                held = eval(&op->rpn) ? held + 1 : 0;
                if (held >= op->stable)
                    break;
                if (i >= op->timeout && op->soft)
                    break;
                if (i >= op->timeout)
                {
                    printf("FAIL line %d: timed out after %d frames: %s\n", op->line, op->timeout, op->text);
                    failed = 1;
                    break;
                }
                runFrame(keysFor(op, i));
                i++;
            }
            break;
        }
        case OP_EXPECT:
            if (!eval(&op->rpn))
            {
                printf("FAIL line %d: expectation not met: %s\n", op->line, op->text);
                failed = 1;
            }
            break;
        case OP_CAPTURE:
            sSlots[op->slot] = eval(&op->rpn);
            break;
        case OP_WATCH:
            sWatches[op->watch].rpn = op->rpn;
            sWatches[op->watch].active = 1;
            sWatches[op->watch].seen = eval(&op->rpn) ? 1 : 0;
            snprintf(sWatches[op->watch].name, sizeof(sWatches[op->watch].name), "%s", op->text);
            break;
        case OP_SEEN:
            if (!sWatches[op->watch].seen)
            {
                printf("FAIL line %d: never observed: %s\n", op->line, op->text);
                failed = 1;
            }
            break;
        case OP_EXPORTSAVE:
            if (!exportSave(op->path))
            {
                printf("FAIL line %d: could not export save to %s\n", op->line, op->path);
                failed = 1;
            }
            break;
        case OP_SCREENSHOT:
            writePng(op->path);
            break;
        case OP_STATE:
            printf("info line %-4d %s\n", op->line, op->text);
            printState();
            break;
        case OP_TRACK:
            if (sNumTracks < MAX_TRACKS)
                sTracks[sNumTracks++].op = op;
            break;
        case OP_RECORD:
            if (op->start)
            {
                snprintf(sTranscriptPath, sizeof(sTranscriptPath), "%s", op->path);
                for (int i = 0; i < sNumTracks; i++)
                    sTracks[i].primed = 0;
                memset(sLastText, 0, sizeof(sLastText));
                sRecording = 1;
                sampleTracks(1);
                sampleText();
            }
            else
            {
                sRecording = 0;
                writeTranscript(sTranscriptPath);
            }
            break;
        case OP_LOGGED:
        {
            int found = 0;
            for (int i = 0; i < sNumLines[op->section] && !found; i++)
                found = strstr(sLines[op->section][i], op->path) != NULL;
            if (found == op->negate)
            {
                printf("FAIL line %d: %s: %s\n", op->line, op->negate ? "unexpectedly recorded" : "never recorded", op->text);
                failed = 1;
            }
            break;
        }
        case OP_TRANSCRIPT:
            if (op->update)
            {
                writeTranscript(op->path);
                printf("info line %-4d golden transcript updated: %s\n", op->line, op->path);
            }
            else if (compareTranscript(op->path, op->line))
                failed = 1;
            break;
        }
        if (!failed && op->text[0] && op->kind != OP_CAPTURE && op->kind != OP_WATCH && op->kind != OP_STATE
            && op->kind != OP_TRACK && op->kind != OP_RECORD)
            printf("ok   line %-4d %6ld frames  %s\n", op->line, sFrame - start, op->text);
    }

    if (failed)
        printDiagnostics();
    printf("%s after %ld frames (%.1f s of game time)\n", failed ? "FAILED" : "PASSED", sFrame, sFrame / 59.73);
    sCore->deinit(sCore);
    free(sVideo);
    return failed ? 1 : 0;
}
