#!/usr/bin/env python3
"""Summarize a measurement run (see route103_rival_measure.c). Informational: no target rates.

Each measurement test is one battle with its own rolls. Won battles print
"MEASURE-WIN <test name>"; each test prints "MEASURE-ROLL <test name>|<player starter
gender>|<rival starter gender>" (genders: 0 male, 254 female, 255 genderless).

Usage: python3 test/verdant/summarize_measure.py /tmp/measure.log
"""

import collections
import re
import sys

GENDER = {0: 'M', 254: 'F', 255: '-'}


def main(path):
    text = re.sub(r'\x1b\[[0-9;]*m', '', open(path, encoding='utf-8', errors='replace').read())
    battles = set()
    for m in re.finditer(r'\] Verdant: \[measure\] (.+? #\d+): (?:PASS|FAIL)', text):
        battles.add(m.group(1))
    wins = set(m.group(1) for m in re.finditer(r'MEASURE-WIN Verdant: \[measure\] (.+? #\d+)', text))
    genders = {m.group(1): (int(m.group(2)), int(m.group(3)))
               for m in re.finditer(r'MEASURE-ROLL Verdant: \[measure\] (.+? #\d+)\|(\d+)\|(\d+)', text)}
    if not battles:
        print('no measurement results found (did the filter select the [measure] tests?)')
        return 1

    buckets = collections.defaultdict(lambda: [0, 0])                       # bucket -> [wins, battles]
    by_gender = collections.defaultdict(lambda: collections.defaultdict(lambda: [0, 0]))
    for name in battles:
        bucket = name.rsplit(' #', 1)[0]
        won = name in wins
        buckets[bucket][0] += won
        buckets[bucket][1] += 1
        if name in genders:
            p, r = genders[name]
            key = f'{GENDER.get(p, "?")} vs {GENDER.get(r, "?")}'
            by_gender[bucket][key][0] += won
            by_gender[bucket][key][1] += 1

    width = max(len(b) for b in buckets)
    print(f"{'bucket':<{width}}  {'wins':>9}  {'rate':>6}   by starter genders (player vs rival)")
    for bucket in sorted(buckets):
        w, n = buckets[bucket]
        split = '  '.join(f'{k}: {gw}/{gn}' for k, (gw, gn) in sorted(by_gender[bucket].items()))
        print(f'{bucket:<{width}}  {w:>4}/{n:<4}  {100 * w / n:5.1f}%   {split}')
    total_w = sum(w for w, _ in buckets.values())
    total_n = sum(n for _, n in buckets.values())
    print(f"{'total':<{width}}  {total_w:>4}/{total_n:<4}  {100 * total_w / total_n:5.1f}%")
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1]))
