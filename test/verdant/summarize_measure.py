#!/usr/bin/env python3
"""Summarize a measurement run of win-rate tests (see route103_rival_measure.c).

Each measurement test plays 50 battles and prints one "MEASURE-WIN <test name>"
line per battle won. (The runner's own "observed X" ratios can't be used: its
failure summary lists at most 50 tests per process.)

Usage: python3 test/verdant/summarize_measure.py /tmp/measure.log
"""

import collections
import re
import sys

TRIALS = 50


def main(path):
    text = re.sub(r'\x1b\[[0-9;]*m', '', open(path, encoding='utf-8', errors='replace').read())
    wins = collections.defaultdict(dict)  # bucket -> roll -> wins
    for m in re.finditer(r'\] Verdant: \[measure\] (.+?) #(\d+)(?: \(\d+/\d+\))?: (?:PASS|FAIL)', text):
        wins[m.group(1)].setdefault(int(m.group(2)), 0)
    for m in re.finditer(r'MEASURE-WIN Verdant: \[measure\] (.+?) #(\d+)', text):
        bucket, roll = m.group(1), int(m.group(2))
        wins[bucket][roll] = wins[bucket].get(roll, 0) + 1
    if not wins:
        print('no measurement results found (did the filter select the [measure] tests?)')
        return 1
    width = max(len(b) for b in wins)
    print(f"{'bucket':<{width}}  {'wins':>9}  {'rate':>6}  worst roll  per roll")
    for bucket in sorted(wins):
        rolls = wins[bucket]
        total = sum(rolls.values())
        n = len(rolls) * TRIALS
        worst = min(rolls.values())
        per = ' '.join(str(rolls[r]) for r in sorted(rolls))
        print(f"{bucket:<{width}}  {total:>4}/{n:<4}  {100 * total / n:5.1f}%  {worst:>3}/{TRIALS}      {per}")
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1]))
