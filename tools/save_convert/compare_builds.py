#!/usr/bin/env python3
"""Compare the save format of two source trees (e.g. before and after an upstream merge).

    python3 tools/save_convert/compare_builds.py OLD_TREE NEW_TREE

Both trees must have been built once (generated headers). Needs the ARM toolchain
and pyelftools (pip install pyelftools).

1. Layout: compiles a probe with debug info against each tree's headers and
   flattens SaveBlock1/2/3, PokemonStorage and Pokemon from DWARF, then lists
   fields that moved, resized, appeared or disappeared. Retypings that keep the
   bytes (u16 -> enum Species) are ignored.
2. IDs: evaluates the constants saves store (species, items, moves, flags, vars,
   maps, object event graphics, trainers, berry trees) in both trees and lists
   renumbered names, and new names that took an ID whose old meaning moved away.

The output is what a converter has to handle; see expansion_1_17.py for one.
"""

import os
import re
import subprocess
import sys
import tempfile

from elftools.elf.elffile import ELFFile

ARM_FLAGS = ['-mthumb', '-mthumb-interwork', '-O0', '-mabi=apcs-gnu', '-mtune=arm7tdmi', '-march=armv4t',
             '-std=gnu17', '-Wno-trigraphs', '-DMODERN=1', '-DTESTING=0', '-DEMERALD', '-w']

LAYOUT_PROBE = '''#include "global.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
struct SaveBlock1 probe_SaveBlock1;
struct SaveBlock2 probe_SaveBlock2;
struct SaveBlock3 probe_SaveBlock3;
struct PokemonStorage probe_PokemonStorage;
struct Pokemon probe_Pokemon;
'''

ID_CATEGORIES = {  # category: (header, name pattern)
    'species': ('include/constants/species.h', r'SPECIES_\w+'),
    'items': ('include/constants/items.h', r'ITEM_\w+'),
    'moves': ('include/constants/moves.h', r'MOVE_\w+'),
    'flags': ('include/constants/flags.h', r'FLAG_\w+'),
    'vars': ('include/constants/vars.h', r'VAR_\w+'),
    'maps': ('include/constants/map_groups.h', r'MAP_\w+'),
    'object event graphics': ('include/constants/event_objects.h', r'OBJ_EVENT_GFX_\w+'),
    'trainers': ('include/constants/opponents.h', r'TRAINER_\w+'),
    'berry trees': ('include/constants/berry.h', r'BERRY_TREE_\w+'),
}
ID_PROBE_HEADER = ('#include "global.h"\n#include "constants/event_objects.h"\n'
                   '#include "constants/map_groups.h"\n#include "constants/berry.h"\n'
                   'const int probe_vals[] = {\n')
ID_PROBE_FIRST_LINE = ID_PROBE_HEADER.count('\n') + 1


def compile_probe(tree, source, workdir, name, debug=False):
    src = os.path.join(workdir, name + '.c')
    obj = os.path.join(workdir, name + '.o')
    with open(src, 'w') as f:
        f.write(source)
    flags = ARM_FLAGS + (['-g'] if debug else []) + ['-iquote', os.path.join(tree, 'include')]
    subprocess.run(['arm-none-eabi-gcc', *flags, '-c', src, '-o', obj], check=True, capture_output=True)
    return obj


# --- layout ---------------------------------------------------------------

class Dwarf:
    def __init__(self, obj):
        self.dwarf = ELFFile(open(obj, 'rb')).get_dwarf_info()
        self.dies = {die.offset: die for cu in self.dwarf.iter_CUs() for die in cu.iter_DIEs()}

    @staticmethod
    def attr(die, name, default=None):
        a = die.attributes.get(name)
        return a.value if a else default

    def type_of(self, die):
        a = die.attributes.get('DW_AT_type')
        return self.dies[a.value + die.cu.cu_offset] if a else None

    def strip(self, t):
        while t is not None and t.tag in ('DW_TAG_typedef', 'DW_TAG_const_type', 'DW_TAG_volatile_type'):
            t = self.type_of(t)
        return t

    def dims(self, t):
        out = []
        for c in t.iter_children():
            if c.tag == 'DW_TAG_subrange_type':
                if 'DW_AT_count' in c.attributes:
                    out.append(self.attr(c, 'DW_AT_count'))
                else:
                    ub = self.attr(c, 'DW_AT_upper_bound')
                    out.append(ub + 1 if isinstance(ub, int) else 0)
        return out

    def size(self, t):
        t = self.strip(t)
        if t is None:
            return 0
        if t.tag == 'DW_TAG_array_type':
            n = 1
            for d in self.dims(t):
                n *= d
            return n * self.size(self.type_of(t))
        return self.attr(t, 'DW_AT_byte_size', 0)

    def is_aggregate(self, t):
        return t is not None and t.tag in ('DW_TAG_structure_type', 'DW_TAG_union_type')

    def flatten(self, t, prefix, base, rows):
        """rows: (path, offset, size, bitfield). Array-of-struct elements appear once as 'name[]'."""
        t = self.strip(t)
        if t.tag == 'DW_TAG_array_type':
            self.flatten(self.type_of(t), prefix + '[]' * len(self.dims(t)), base, rows)
            return
        if not self.is_aggregate(t):
            return
        for m in t.iter_children():
            if m.tag != 'DW_TAG_member':
                continue
            name = self.attr(m, 'DW_AT_name')
            name = name.decode() if name else '<anon>'
            off = self.attr(m, 'DW_AT_data_member_location', 0)
            bits = ''
            if 'DW_AT_bit_size' in m.attributes:
                bo = self.attr(m, 'DW_AT_data_bit_offset')
                if bo is not None:
                    off, bit = divmod(bo, 8)
                else:
                    bit = self.attr(m, 'DW_AT_bit_offset')
                bits = f"bit{bit}:{self.attr(m, 'DW_AT_bit_size')}"
            path = f'{prefix}.{name}' if prefix else name
            mt = self.type_of(m)
            rows.append((path, base + off, self.size(mt), bits))
            st = self.strip(mt)
            if not bits and (self.is_aggregate(st) or (st is not None and st.tag == 'DW_TAG_array_type'
                                                       and self.is_aggregate(self.strip(self.type_of(st))))):
                self.flatten(mt, path, base + off, rows)


def layouts(tree, workdir):
    d = Dwarf(compile_probe(tree, LAYOUT_PROBE, workdir, 'layout_' + os.path.basename(tree), debug=True))
    out = {}
    for cu in d.dwarf.iter_CUs():
        for die in cu.get_top_DIE().iter_children():
            name = d.attr(die, 'DW_AT_name', b'')
            if die.tag == 'DW_TAG_variable' and name.startswith(b'probe_'):
                rows = []
                d.flatten(d.type_of(die), '', 0, rows)
                out[name.decode()[6:]] = (d.size(d.type_of(die)), rows)
    return out


def print_layout_diff(old, new):
    print('# Layout')
    for block in old:
        (osize, orows), (nsize, nrows) = old[block], new[block]
        o, n = {r[0]: r for r in orows}, {r[0]: r for r in nrows}
        key = lambda r: (r[1], r[3]) if r[3] else (r[1], r[2])
        lines = []
        for p in o:
            if p in n and key(o[p]) != key(n[p]):
                a, b = o[p], n[p]
                if a[2] == b[2] or a[3]:
                    if '.' not in p and '[' not in p:
                        lines.append(f'  moved   {p:40s} {a[1]:6d} -> {b[1]:6d} ({b[1] - a[1]:+d})')
                else:
                    lines.append(f'  resized {p:40s} offset {a[1]} -> {b[1]}, size {a[2]} -> {b[2]}')
        lines += [f'  added   {p:40s} offset {n[p][1]}, size {n[p][2]}' for p in n if p not in o]
        lines += [f'  removed {p:40s} offset {o[p][1]}, size {o[p][2]}' for p in o if p not in n]
        print(f'{block}: {osize} -> {nsize} bytes' + ('' if lines else ', same layout'))
        print('\n'.join(lines)) if lines else None


# --- IDs ------------------------------------------------------------------

def id_names(tree, header, pattern):
    text = re.sub(r'//.*', '', open(os.path.join(tree, header), encoding='utf-8', errors='replace').read())
    names = set(re.findall(r'^\s*#define\s+(' + pattern + r')\s+\S', text, re.M))
    names |= set(re.findall(r'^\s*(' + pattern + r')\s*(?:=[^,\n]*)?,', text, re.M))
    return names


def id_values(tree, names, workdir, tag):
    """name -> value; names that don't compile in this tree (dead macros) are dropped."""
    names = sorted(names)
    while True:
        source = ID_PROBE_HEADER + ',\n'.join(names) + '\n};\n'
        try:
            obj = compile_probe(tree, source, workdir, 'ids_' + tag)
            break
        except subprocess.CalledProcessError as e:
            lines = {int(m) for m in re.findall(r'ids_\w+\.c:(\d+):', e.stderr.decode())}
            bad = {names[l - ID_PROBE_FIRST_LINE] for l in lines if 0 <= l - ID_PROBE_FIRST_LINE < len(names)}
            if not bad:
                raise
            names = [n for n in names if n not in bad]
    elf = ELFFile(open(obj, 'rb'))
    sym = elf.get_section_by_name('.symtab').get_symbol_by_name('probe_vals')[0]
    data = elf.get_section(sym['st_shndx']).data()[sym['st_value']:sym['st_value'] + 4 * len(names)]
    return {n: int.from_bytes(data[4 * i:4 * i + 4], 'little', signed=True) for i, n in enumerate(names)}


def print_id_diff(old_tree, new_tree, workdir):
    print('# IDs')
    for i, (category, (header, pattern)) in enumerate(ID_CATEGORIES.items()):
        o, n = id_names(old_tree, header, pattern), id_names(new_tree, header, pattern)
        vo = id_values(old_tree, o, workdir, f'old{i}')
        vn = id_values(new_tree, n, workdir, f'new{i}')
        common = sorted(set(vo) & set(vn))
        moved = [(k, vo[k], vn[k]) for k in common if vo[k] != vn[k]]
        by_old_value = {}
        for k, v in vo.items():
            by_old_value.setdefault(v, []).append(k)
        reused = []
        for k in sorted(set(vn) - set(vo)):
            olds = [x for x in by_old_value.get(vn[k], []) if 'UNUSED' not in x and vn.get(x) != vn[k]]
            if vn[k] and olds:
                reused.append((k, vn[k], olds))
        print(f'{category}: {len(common)} kept, {len(set(vn) - set(vo))} new, {len(set(vo) - set(vn))} removed, '
              f'{len(moved)} renumbered, {len(reused)} IDs reused with a new meaning')
        for k, a, b in moved[:20]:
            print(f'  renumbered {k}: {a} -> {b}')
        for k, v, olds in reused[:20]:
            print(f'  reused     {v}: was {", ".join(olds)}, now {k}')


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    old_tree, new_tree = map(os.path.abspath, sys.argv[1:])
    with tempfile.TemporaryDirectory() as workdir:
        print_layout_diff(layouts(old_tree, workdir), layouts(new_tree, workdir))
        print()
        print_id_diff(old_tree, new_tree, workdir)


if __name__ == '__main__':
    main()
