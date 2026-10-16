#!/usr/bin/env python3
"""Replay harness driver for Verdant Emerald.

Compiles a .replay scenario into a plan for replay-runner (headless libmgba),
resolving every name against the current build:
  - symbols (functions, globals, statics) from pokeemerald.elf via nm
  - C constants, enum values and struct layouts by compiling a small file with
    the ARM toolchain against the project's own headers

so scenarios survive upstream renumbering and struct changes.

Usage:
  tools/replay/replay.py run SCENARIO [--param k=v ...] [--rom ROM] [--elf ELF]
  tools/replay/replay.py names NAME...      # debug: show what names resolve to

See .claude/skills/testing/SKILL.md for the scenario language.
"""

import argparse
import hashlib
import os
import re
import shlex
import struct
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, '..', '..'))
CACHE = os.path.join(HERE, 'build')
RUNNER = os.path.join(HERE, 'replay-runner')

FACING = {'UP': 'DIR_NORTH', 'DOWN': 'DIR_SOUTH', 'LEFT': 'DIR_WEST', 'RIGHT': 'DIR_EAST'}

BUTTONS = {'A': 1, 'B': 2, 'SELECT': 4, 'START': 8, 'RIGHT': 16, 'LEFT': 32,
           'UP': 64, 'DOWN': 128, 'R': 256, 'L': 512}

# Headers every constant lookup can see. A scenario can add more with "header".
BASE_HEADERS = [
    'global.h', 'main.h', 'task.h', 'palette.h', 'script.h', 'event_data.h', 'field_message_box.h',
    'field_player_avatar.h', 'event_object_movement.h', 'fieldmap.h', 'battle.h',
    'constants/battle.h', 'constants/flags.h', 'constants/vars.h', 'constants/maps.h',
    'constants/species.h', 'constants/items.h', 'constants/moves.h',
]

ARM_CFLAGS = ['-mthumb', '-mthumb-interwork', '-O0', '-mabi=apcs-gnu', '-mtune=arm7tdmi',
              '-march=armv4t', '-std=gnu17', '-Wno-trigraphs', '-DMODERN=1', '-DTESTING=0',
              '-DEMERALD', '-iquote', os.path.join(ROOT, 'include')]


class ReplayError(Exception):
    pass


# ---------------------------------------------------------------------------
# Name resolution
# ---------------------------------------------------------------------------

class Resolver:
    def __init__(self, elf):
        self.elf = elf
        self.syms = {}           # name -> [addresses]
        self.constants = {}      # expression -> int
        self.probes = {}         # (struct, field) -> (byte offset, bit)
        self.headers = list(BASE_HEADERS)
        self._load_syms()

    def _load_syms(self):
        out = subprocess.run(['arm-none-eabi-nm', self.elf], capture_output=True, text=True, check=True).stdout
        for line in out.splitlines():
            parts = line.split()
            if len(parts) == 3:
                self.syms.setdefault(parts[2], []).append(int(parts[0], 16))

    def symbol(self, name):
        addrs = self.syms.get(name)
        if not addrs:
            raise ReplayError(f"unknown symbol '{name}' (not in {os.path.basename(self.elf)})")
        if len(set(addrs)) > 1:
            raise ReplayError(f"ambiguous symbol '{name}': {len(addrs)} statics share this name")
        return addrs[0]

    def name_tables(self):
        lines = [f"map {value:x} {name}" for name, value in self.map_constants().items()]
        for name, addrs in self.syms.items():
            # Only real memory addresses (EWRAM/IWRAM/ROM); skips absolute constants like ABILITY_NONE = 0.
            if len(addrs) == 1 and addrs[0] >= 0x02000000 and not name.startswith('$'):
                lines.append(f"sym {addrs[0]:x} {name}")
        return '\n'.join(lines) + '\n'

    def map_constants(self):
        if not hasattr(self, '_maps'):
            self._maps = {}
            path = os.path.join(ROOT, 'include', 'constants', 'map_groups.h')
            pat = re.compile(r'^\s*(MAP_\w+)\s*=\s*\((\d+)\s*\|\s*\((\d+)\s*<<\s*8\)\)')
            for line in open(path):
                m = pat.match(line)
                if m:
                    self._maps[m.group(1)] = int(m.group(2)) | (int(m.group(3)) << 8)
        return self._maps

    def source_enum(self, path, name):
        """Value of an enum constant declared inside a .c file (not visible to headers)."""
        text = open(os.path.join(ROOT, path)).read()
        for body in re.findall(r'enum\s*\w*\s*\{(.*?)\}', text, re.S):
            value = -1
            for item in body.split(','):
                item = re.sub(r'//.*|/\*.*?\*/', '', item, flags=re.S).strip()
                if not item:
                    continue
                if '=' in item:
                    key, expr = item.split('=', 1)
                    value = int(expr.strip(), 0)
                    key = key.strip()
                else:
                    key = item
                    value += 1
                if key == name:
                    return value
        raise ReplayError(f"enum value {name} not found in {path}")

    def resolve_all(self, exprs, probes):
        """Evaluate C constant expressions and bitfield probes with the ARM compiler (cached)."""
        exprs = sorted(set(exprs) - set(self.constants))
        probes = sorted(set(probes) - set(self.probes))
        if not exprs and not probes:
            return
        src = [f'#include "{h}"' for h in self.headers]
        for i, e in enumerate(exprs):
            src.append(f'const long long rr_c{i} = (long long)({e});')
        for i, (st, field) in enumerate(probes):
            # .field = -1 sets every bit of the bitfield: gives its byte offset, low bit and width
            src.append(f'const struct {st} rr_p{i} = {{ .{field} = -1 }};')
        text = '\n'.join(src) + '\n'
        os.makedirs(CACHE, exist_ok=True)
        key = hashlib.sha1((text + str(self._header_stamp())).encode()).hexdigest()[:16]
        obj = os.path.join(CACHE, f'names-{key}.o')
        if not os.path.exists(obj):
            with tempfile.NamedTemporaryFile('w', suffix='.c', dir=CACHE, delete=False) as f:
                f.write(text)
                cfile = f.name
            res = subprocess.run(['arm-none-eabi-gcc', *ARM_CFLAGS, '-c', cfile, '-o', obj],
                                 capture_output=True, text=True)
            os.unlink(cfile)
            if res.returncode != 0:
                raise ReplayError('could not resolve names against the project headers:\n' +
                                  '\n'.join(l for l in res.stderr.splitlines() if 'error' in l)[:4000])
        data = read_object_symbols(obj)
        for i, e in enumerate(exprs):
            self.constants[e] = struct.unpack('<q', data[f'rr_c{i}'][:8])[0]
        for i, p in enumerate(probes):
            value = int.from_bytes(data[f'rr_p{i}'], 'little')
            if not value:
                raise ReplayError(f'bitfield probe {p} found no set bit')
            low = (value & -value).bit_length() - 1
            width = bin(value >> low).count('1')
            if low // 8 != (low + width - 1) // 8:
                raise ReplayError(f'bitfield {p} straddles a byte boundary; not supported yet')
            self.probes[p] = (low // 8, low % 8, width)

    def _header_stamp(self):
        newest = 0
        for dirpath, _, files in os.walk(os.path.join(ROOT, 'include')):
            for fn in files:
                newest = max(newest, os.path.getmtime(os.path.join(dirpath, fn)))
        return newest

    def const(self, expr):
        if expr not in self.constants:
            self.resolve_all([expr], [])
        return self.constants[expr]


def read_object_symbols(path):
    """Minimal ELF32 reader: symbol name -> bytes of its initial value."""
    blob = open(path, 'rb').read()
    shoff, = struct.unpack_from('<I', blob, 0x20)
    shentsize, shnum, shstrndx = struct.unpack_from('<HHH', blob, 0x2E)
    sections = [struct.unpack_from('<IIIIIIIIII', blob, shoff + i * shentsize) for i in range(shnum)]
    symtab = next(s for s in sections if s[1] == 2)          # SHT_SYMTAB
    strtab = sections[symtab[6]]                             # sh_link
    out = {}
    for off in range(symtab[4], symtab[4] + symtab[5], 16):
        st_name, st_value, st_size, _, _, st_shndx = struct.unpack_from('<IIIBBH', blob, off)
        if not st_size or st_shndx == 0 or st_shndx >= shnum:
            continue
        end = blob.index(b'\0', strtab[4] + st_name)
        name = blob[strtab[4] + st_name:end].decode()
        sec = sections[st_shndx]
        if sec[1] == 8:   # SHT_NOBITS (.bss): all zero
            out[name] = bytes(st_size)
        else:
            out[name] = blob[sec[4] + st_value: sec[4] + st_value + st_size]
    return out


# ---------------------------------------------------------------------------
# Condition compiler: conditions -> RPN memory expressions
# ---------------------------------------------------------------------------

CMP_OPS = ('==', '!=', '<=', '>=', '<', '>')


class Compiler:
    def __init__(self, resolver):
        self.r = resolver
        self.slots = 0

    # --- game knowledge (layouts come from the build, not from here) ---
    def off(self, st, field):
        return self.r.const(f'__builtin_offsetof(struct {st}, {field})')

    def sb1(self):
        return f"{self.r.symbol('gSaveBlock1Ptr')} r32"

    def player_object(self):
        """RPN that pushes the address of the player's ObjectEvent."""
        oe = self.r.symbol('gObjectEvents')
        size = self.r.const('sizeof(struct ObjectEvent)')
        pid = self.r.symbol('gPlayerAvatar') + self.off('PlayerAvatar', 'objectEventId')
        return f"{oe} {pid} r8 {size} * +"

    def player_coord(self, axis):
        coord = self.off('ObjectEvent', f'currentCoords.{axis}')
        return f"{self.player_object()} {coord} + rs16 {self.r.const('MAP_OFFSET')} -"

    def bitfield(self, base, st, field, rpn_base=False):
        """RPN reading a bitfield; base is an address, or an RPN fragment when rpn_base."""
        off, low, width = self.r.probes[(st, field)]
        return f"{base} {off} + r8 {low} >> {(1 << width) - 1} &"

    def number(self, tok):
        if re.fullmatch(r'-?(0x[0-9a-fA-F]+|\d+)', tok):
            return int(tok, 0)
        return self.r.const(tok)

    def callback(self, which, op, target):
        addr = self.r.symbol('gMain') + self.off('Main', which)
        return f"{addr} r32 -2 & {self.r.symbol(target) & ~1} {op}"

    # --- parsing ---
    def condition(self, tokens, captures):
        """tokens: list of words. Returns RPN string. Grammar: atom ((and|or) atom)*, 'not' prefix."""
        terms, ops, cur = [], [], []
        for t in tokens:
            if t in ('and', 'or'):
                terms.append(cur)
                ops.append(t)
                cur = []
            else:
                cur.append(t)
        terms.append(cur)
        rpn = [self.atom(terms[0], captures)]
        # 'and' binds tighter than 'or'
        groups, group = [], [rpn[0]]
        for op, term in zip(ops, terms[1:]):
            a = self.atom(term, captures)
            if op == 'and':
                group.append(a)
            else:
                groups.append(group)
                group = [a]
        groups.append(group)
        ands = [' '.join(g) + ' &&' * (len(g) - 1) for g in groups]
        return ' '.join(ands) + ' ||' * (len(ands) - 1)

    def atom(self, t, captures):
        if not t:
            raise ReplayError('empty condition')
        if t[0] in ('not', '!'):
            return self.atom(t[1:], captures) + ' !'
        if t[0].startswith('!') and len(t[0]) > 1:
            return self.atom([t[0][1:]] + t[1:], captures) + ' !'
        w = t[0]
        if w in ('callback1', 'callback2') and len(t) == 3 and t[1] in ('==', '!='):
            return self.callback(w, t[1], t[2])
        if w == 'in-battle' and len(t) == 1:
            return self.bitfield(self.r.symbol('gMain'), 'Main', 'inBattle')
        if w == 'battle-type' and len(t) == 3 and t[1] == 'has':
            return f"{self.r.symbol('gBattleTypeFlags')} r32 {self.number(t[2])} & 0 !="
        if w == 'flag' and len(t) == 2:
            fid = self.number(t[1])
            if fid < self.r.const('SPECIAL_FLAGS_START'):
                base = f"{self.sb1()} {self.off('SaveBlock1', 'flags') + fid // 8} +"
            else:
                base = f"{self.r.symbol('sSpecialFlags') + (fid - self.r.const('SPECIAL_FLAGS_START')) // 8}"
            return f"{base} r8 {fid % 8} bit"
        if w == 'var' and len(t) == 4 and t[2] in CMP_OPS:
            return f"{self.var_read(self.number(t[1]))} {self.number(t[3])} {t[2]}"
        if w == 'map' and len(t) == 3 and t[1] in ('==', '!='):
            value = self.r.map_constants().get(t[2])
            if value is None:
                raise ReplayError(f"unknown map '{t[2]}'")
            grp = self.off('SaveBlock1', 'location.mapGroup')
            num = self.off('SaveBlock1', 'location.mapNum')
            return f"{self.sb1()} {grp} + r8 8 << {self.sb1()} {num} + r8 | {value} {t[1]}"
        if w == 'player-at' and len(t) == 3:
            return (f"{self.player_coord('x')} {self.number(t[1])} == "
                    f"{self.player_coord('y')} {self.number(t[2])} == &&")
        if w == 'player-moved' and len(t) == 1:
            sx, sy = self.capture(captures, self.player_coord('x')), self.capture(captures, self.player_coord('y'))
            return f"{self.player_coord('x')} cap:{sx} != {self.player_coord('y')} cap:{sy} != ||"
        if w == 'script-running' and len(t) == 1:
            shutdown = self.r.source_enum('src/script.c', 'CONTEXT_SHUTDOWN')
            return f"{self.r.symbol('sGlobalScriptContextStatus')} r8 {shutdown} !="
        if w == 'task-active' and len(t) == 2:
            func = self.r.symbol(t[1]) & ~1
            tasks, size = self.r.symbol('gTasks'), self.r.const('sizeof(struct Task)')
            f_off, a_off = self.off('Task', 'func'), self.off('Task', 'isActive')
            terms = [f"{tasks + i * size + f_off} r32 -2 & {func} == {tasks + i * size + a_off} r8 0 != &&"
                     for i in range(self.r.const('NUM_TASKS'))]
            return ' '.join(terms) + ' ||' * (len(terms) - 1)
        if w == 'field-idle' and len(t) == 1:
            return self.condition(['callback2', '==', 'CB2_Overworld', 'and', '!script-running', 'and',
                                   '!controls-locked', 'and', '!fading'], captures)
        if w == 'controls-locked' and len(t) == 1:
            return f"{self.r.symbol('sLockFieldControls')} r8 0 !="
        if w == 'text-printing' and len(t) == 1:
            # The field message box mode returns to HIDDEN once printing ends, even while the
            # box stays on screen waiting for A, so this means "text is still printing".
            return f"{self.r.symbol('sFieldMessageBoxMode')} r8 {self.r.const('FIELD_MESSAGE_BOX_HIDDEN')} !="
        if w == 'fading' and len(t) == 1:
            return self.bitfield(self.r.symbol('gPaletteFade'), 'PaletteFadeControl', 'active')
        if w == 'player-stopped' and len(t) == 1:
            addr = self.r.symbol('gPlayerAvatar') + self.off('PlayerAvatar', 'tileTransitionState')
            return f"{addr} r8 {self.r.const('T_NOT_MOVING')} =="
        if w == 'player-facing' and len(t) == 2:
            return f"{self.bitfield(self.player_object() , 'ObjectEvent', 'facingDirection', rpn_base=True)} {self.number(t[1])} =="
        if w == 'party-count' and len(t) == 3 and t[1] in CMP_OPS:
            return f"{self.r.symbol('gPlayerPartyCount')} r8 {self.number(t[2])} {t[1]}"
        if w in ('mem8', 'mem16', 'mem32') and len(t) == 4 and t[2] in CMP_OPS:
            m = re.fullmatch(r'(\w+)(?:\+(\w+))?', t[1])
            addr = self.r.symbol(m.group(1)) + (self.number(m.group(2)) if m.group(2) else 0)
            return f"{addr} r{w[3:]} {self.number(t[3])} {t[2]}"
        raise ReplayError(f"unknown condition: {' '.join(t)}")

    def var_read(self, vid):
        if self.r.const('VARS_START') <= vid <= self.r.const('VARS_END'):
            return f"{self.sb1()} {self.off('SaveBlock1', 'vars') + (vid - self.r.const('VARS_START')) * 2} + r16"
        if vid >= self.r.const('SPECIAL_VARS_START'):
            return f"{self.r.symbol('gSpecialVars') + (vid - self.r.const('SPECIAL_VARS_START')) * 4} r32 r16"
        raise ReplayError(f'var 0x{vid:x} is outside the var ranges')

    def capture(self, captures, rpn):
        slot = self.slots
        self.slots += 1
        captures.append((slot, rpn))
        return slot

    def diagnostics(self):
        gmain = self.r.symbol('gMain')
        d = [
            ('callback1', 'sym', f"{gmain + self.off('Main', 'callback1')} r32"),
            ('callback2', 'sym', f"{gmain + self.off('Main', 'callback2')} r32"),
            ('map', 'map', f"{self.sb1()} {self.off('SaveBlock1', 'location.mapGroup')} + r8 8 << "
                           f"{self.sb1()} {self.off('SaveBlock1', 'location.mapNum')} + r8 |"),
            ('player-x', '', self.player_coord('x')),
            ('player-y', '', self.player_coord('y')),
            ('script-status', '', f"{self.r.symbol('sGlobalScriptContextStatus')} r8"),
            ('controls-locked', '', f"{self.r.symbol('sLockFieldControls')} r8"),
            ('textbox-mode', '', f"{self.r.symbol('sFieldMessageBoxMode')} r8"),
            ('fading', '', self.atom(['fading'], [])),
            ('player-facing', '', self.bitfield(self.player_object(), 'ObjectEvent', 'facingDirection', rpn_base=True)),
            ('player-stopped', '', self.atom(['player-stopped'], [])),
            ('battle-type', '', f"{self.r.symbol('gBattleTypeFlags')} r32"),
            ('in-battle', '', self.atom(['in-battle'], [])),
            ('party-count', '', f"{self.r.symbol('gPlayerPartyCount')} r8"),
        ]
        return [f"DIAG name={n}" + (f" kind={k}" if k else '') + f" | {rpn}" for n, k, rpn in d]


# ---------------------------------------------------------------------------
# Scenario parsing
# ---------------------------------------------------------------------------

def parse_buttons(word):
    mask = 0
    for b in word.upper().split('+'):
        if b not in BUTTONS:
            raise ReplayError(f"unknown button '{b}'")
        mask |= BUTTONS[b]
    return mask


def split_options(words):
    """Pull 'timeout N', 'every N' and 'for N' out of a step.
    Returns (rest, timeout, every, stable)."""
    rest, opts = [], {}
    i = 0
    while i < len(words):
        if words[i] in ('timeout', 'every', 'for') and i + 1 < len(words) and words[i + 1].isdigit():
            opts[words[i]] = int(words[i + 1])
            i += 2
        else:
            rest.append(words[i])
            i += 1
    return rest, opts.get('timeout'), opts.get('every'), opts.get('for', 1)


def split_guard(words):
    """'BUTTON when COND until ...' -> (guard words or None, remaining words)."""
    if len(words) > 1 and words[1] == 'when':
        end = words.index('until') if 'until' in words else len(words)
        return words[2:end], [words[0]] + words[end:]
    return None, words


def load_lines(path, params, stack=()):
    """Expand 'use' includes and $params. Yields (file, line number, words, original text)."""
    path = os.path.normpath(path)
    if path in stack:
        raise ReplayError(f'recursive use of {path}')
    local = dict(params)
    for n, raw in enumerate(open(path), 1):
        text = raw.split('#', 1)[0].strip()
        if not text:
            continue
        words = shlex.split(text)
        if words[0] == 'variant':
            continue
        if words[0] == 'param':
            for kv in words[1:]:
                k, v = kv.split('=', 1)
                local.setdefault(k, v)
            continue
        words = [re.sub(r'\$(\w+)', lambda m: subst(m, local, path, n), w) for w in words]
        if words[0] == 'use':
            sub = dict(local)
            for kv in words[2:]:
                k, v = kv.split('=', 1)
                sub[k] = v
            target = os.path.join(HERE, words[1]) if not os.path.isabs(words[1]) else words[1]
            yield from load_lines(target, sub, stack + (path,))
            continue
        yield path, n, words, ' '.join(words)


def subst(m, params, path, n):
    if m.group(1) not in params:
        raise ReplayError(f'{os.path.relpath(path, ROOT)}:{n}: parameter ${m.group(1)} has no value')
    return params[m.group(1)]


def collect_names(lines):
    """Constants and bitfield probes a scenario will need, resolved in one compile."""
    exprs = {'MAP_OFFSET', 'SPECIAL_FLAGS_START', 'VARS_START', 'VARS_END', 'SPECIAL_VARS_START',
             'FIELD_MESSAGE_BOX_HIDDEN', 'sizeof(struct ObjectEvent)', 'NUM_TASKS', 'sizeof(struct Task)'}
    for st, fields in (('Main', ['callback1', 'callback2']),
                       ('SaveBlock1', ['flags', 'vars', 'location.mapGroup', 'location.mapNum']),
                       ('ObjectEvent', ['currentCoords.x', 'currentCoords.y']),
                       ('PlayerAvatar', ['objectEventId']),
                       ('Task', ['func', 'isActive'])):
        for f in fields:
            exprs.add(f'__builtin_offsetof(struct {st}, {f})')
    for _, _, words, _ in lines:
        for w in words:
            if re.fullmatch(r'[A-Z][A-Z0-9_]{2,}', w) and w not in BUTTONS and not w.startswith('MAP_'):
                exprs.add(w)
    exprs |= set(FACING.values())
    exprs |= {'T_NOT_MOVING', '__builtin_offsetof(struct PlayerAvatar, tileTransitionState)'}
    return exprs, {('Main', 'inBattle'), ('PaletteFadeControl', 'active'), ('ObjectEvent', 'facingDirection')}


def compile_scenario(path, params, resolver):
    lines = list(load_lines(path, params))
    for _, _, words, _ in lines:
        if words[0] == 'header':
            resolver.headers.extend(words[1:])
    exprs, probes = collect_names(lines)
    resolver.resolve_all(exprs, probes)
    comp = Compiler(resolver)
    plan, watches = [], {}

    for fpath, n, words, text in lines:
        where = f"{os.path.relpath(fpath, HERE)}:{n}"
        label = f"line={n} text={where}: {text}"
        try:
            verb = words[0]
            if verb == 'header':
                continue
            captures = []
            if verb in ('tap', 'hold'):
                rest, timeout, every, stable = split_options(words[1:])
                guard_words, rest = split_guard(rest)
                guard = f" ;; {comp.condition(guard_words, captures)}" if guard_words else ''
                buttons = parse_buttons(rest[0])
                mode = verb
                period = every or 8
                if len(rest) >= 2 and rest[1] == 'until':
                    rpn = comp.condition(rest[2:], captures)
                    plan += [f"CAPTURE slot={s} | {r}" for s, r in captures]
                    plan.append(f"UNTIL mode={mode} buttons={buttons} period={period} timeout={timeout or 600} "
                                f"stable={stable} {label} | {rpn}{guard}")
                elif len(rest) == 2 and rest[1].isdigit():
                    frames = int(rest[1]) * period if verb == 'tap' else int(rest[1])
                    plan.append(f"RUN mode={mode} buttons={buttons} period={period} frames={frames} {label}")
                elif len(rest) == 1 and verb == 'tap':
                    plan.append(f"RUN mode=tap buttons={buttons} period={period} frames={period} {label}")
                else:
                    raise ReplayError('expected: tap|hold BUTTON [COUNT|until CONDITION] [timeout N] [every N]')
            elif verb == 'wait':
                rest, timeout, _, stable = split_options(words[1:])
                if rest and rest[0] == 'until':
                    rpn = comp.condition(rest[1:], captures)
                    plan += [f"CAPTURE slot={s} | {r}" for s, r in captures]
                    plan.append(f"UNTIL mode=none timeout={timeout or 600} stable={stable} {label} | {rpn}")
                elif len(rest) == 1 and rest[0].isdigit():
                    plan.append(f"RUN mode=none frames={rest[0]} {label}")
                else:
                    raise ReplayError('expected: wait FRAMES | wait until CONDITION [timeout N]')
            elif verb == 'walk':
                # walk DIR to X Y [timeout N]: hold DIR until on (X, Y), then let the step finish
                rest, timeout, _, _ = split_options(words[1:])
                if len(rest) != 4 or rest[1] != 'to':
                    raise ReplayError('expected: walk DIR to X Y [timeout N]')
                at = comp.condition(['player-at', rest[2], rest[3]], captures)
                plan.append(f"UNTIL mode=hold buttons={parse_buttons(rest[0])} timeout={timeout or 600} {label} | {at}")
                plan.append(f"UNTIL mode=none timeout=120 stable=2 | {comp.condition(['player-stopped'], captures)}")
            elif verb == 'face':
                # face DIR: wait for the player to stop, then turn in place with short taps
                if len(words) != 2 or words[1].upper() not in FACING:
                    raise ReplayError('expected: face UP|DOWN|LEFT|RIGHT')
                stopped = comp.condition(['player-stopped'], captures)
                facing = comp.condition(['player-facing', FACING[words[1].upper()]], captures)
                plan.append(f"UNTIL mode=none timeout=120 stable=2 | {stopped}")
                plan.append(f"UNTIL mode=tap buttons={parse_buttons(words[1])} period=6 timeout=60 {label} | {facing}")
            elif verb == 'expect-control':
                # The player can move again: try each direction until one of them moves the player.
                sx = comp.capture(captures, comp.player_coord('x'))
                sy = comp.capture(captures, comp.player_coord('y'))
                moved = f"{comp.player_coord('x')} cap:{sx} != {comp.player_coord('y')} cap:{sy} != ||"
                plan += [f"CAPTURE slot={s} | {r}" for s, r in captures]
                for d in ('RIGHT', 'LEFT', 'DOWN', 'UP'):
                    plan.append(f"UNTIL mode=hold buttons={BUTTONS[d]} timeout=30 soft=1 | {moved}")
                plan.append(f"EXPECT {label} | {moved}")
                captures = []
            elif verb == 'expect':
                plan.append(f"EXPECT {label} | {comp.condition(words[1:], captures)}")
            elif verb == 'watch':
                wid = watches.setdefault(words[1], len(watches))
                plan.append(f"WATCH watch={wid} {label} | {comp.condition(words[2:], captures)}")
            elif verb == 'expect-seen':
                if words[1] not in watches:
                    raise ReplayError(f"no watch named '{words[1]}'")
                plan.append(f"SEEN watch={watches[words[1]]} {label}")
            elif verb == 'export-save':
                out = os.path.join(ROOT, words[1])
                os.makedirs(os.path.dirname(out), exist_ok=True)
                plan.append(f"EXPORTSAVE path={out} {label}")
            elif verb == 'state':
                plan.append(f"STATE {label}")
            elif verb == 'screenshot':
                plan.append(f"SCREENSHOT path={os.path.join(ROOT, words[1])} {label}")
            else:
                raise ReplayError(f"unknown step '{verb}'")
            if captures and verb not in ('tap', 'hold', 'wait'):
                raise ReplayError('player-moved only works in tap/hold/wait steps')
        except ReplayError as e:
            raise ReplayError(f"{where}: {e}") from None
    return plan, comp.diagnostics()


# ---------------------------------------------------------------------------
# Commands
# ---------------------------------------------------------------------------

def build_runner():
    res = subprocess.run(['make', '-s', '-C', HERE, 'replay-runner'], capture_output=True, text=True)
    if res.returncode != 0:
        raise ReplayError('could not build replay-runner (needs libmgba-dev and libpng-dev):\n' + res.stderr)


def scenario_variants(path):
    """'variant k=v ...' lines in a scenario file (not its includes)."""
    out = []
    for raw in open(path):
        words = shlex.split(raw.split('#', 1)[0])
        if words and words[0] == 'variant':
            out.append(dict(kv.split('=', 1) for kv in words[1:]))
    return out or [{}]


def cmd_run(args):
    scenarios = []
    for target in args.scenarios:
        if os.path.isdir(target):
            scenarios += sorted(os.path.join(target, f) for f in os.listdir(target) if f.endswith('.replay'))
        else:
            scenarios.append(target)
    resolver = Resolver(args.elf)
    results = []
    for scenario in scenarios:
        variants = [dict(kv.split('=', 1) for kv in args.param)] if args.param else scenario_variants(scenario)
        for params in variants:
            code = run_one(args, scenario, params, resolver)
            results.append((scenario, params, code))
    if len(results) > 1:
        print('\nsummary:')
        for scenario, params, code in results:
            tag = ' '.join(f'{k}={v}' for k, v in params.items())
            print(f"  {'PASS' if code == 0 else 'FAIL'}  {os.path.relpath(scenario)} {tag}".rstrip())
    return max(code for _, _, code in results)


def run_one(args, scenario, params, resolver):
    plan, diags = compile_scenario(os.path.abspath(scenario), params, resolver)
    build_runner()
    name = os.path.splitext(os.path.basename(scenario))[0]
    tag = '-'.join(f'{k}_{v}' for k, v in sorted(params.items()))
    artifacts = os.path.join(CACHE, 'artifacts', name + (f'-{tag}' if tag else ''))
    os.makedirs(artifacts, exist_ok=True)
    names_path = os.path.join(artifacts, 'names.txt')
    with open(names_path, 'w') as f:
        f.write(resolver.name_tables())
    header = [f"ROM path={os.path.abspath(args.rom)}", f"NAMES path={names_path}", f"ARTIFACTS path={artifacts}"]
    if args.save:
        header.append(f"SAV path={os.path.abspath(args.save)}")
    plan_path = os.path.join(artifacts, 'plan.txt')
    with open(plan_path, 'w') as f:
        f.write('\n'.join(header + diags + plan) + '\n')
    print(f"== {os.path.relpath(scenario)} {' '.join(f'{k}={v}' for k, v in params.items())}".rstrip())
    return subprocess.run([RUNNER, plan_path]).returncode


def cmd_names(args):
    resolver = Resolver(args.elf)
    for n in args.names:
        if n in resolver.syms:
            print(f"{n:40} symbol 0x{resolver.symbol(n):08x}")
        elif n in resolver.map_constants():
            print(f"{n:40} map    0x{resolver.map_constants()[n]:04x}")
        else:
            print(f"{n:40} const  {resolver.const(n)}")
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--rom', default=os.path.join(ROOT, 'pokeemerald.gba'))
    ap.add_argument('--elf', default=os.path.join(ROOT, 'pokeemerald.elf'))
    sub = ap.add_subparsers(dest='cmd', required=True)
    r = sub.add_parser('run')
    r.add_argument('scenarios', nargs='+', help='.replay files or directories of them')
    r.add_argument('--param', action='append', default=[], help='k=v, overrides scenario params')
    r.add_argument('--save', help='start from this .sav instead of a fresh boot')
    nm = sub.add_parser('names')
    nm.add_argument('names', nargs='+')
    args = ap.parse_args()
    try:
        return cmd_run(args) if args.cmd == 'run' else cmd_names(args)
    except ReplayError as e:
        print(f"replay: {e}", file=sys.stderr)
        return 2


if __name__ == '__main__':
    sys.exit(main())
