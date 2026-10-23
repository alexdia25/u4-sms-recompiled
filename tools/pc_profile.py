"""Sum a U4_PC_PROFILE file (where the machine was at each line's end) by
routine, to see which original code still takes the most time.
    python tools/pc_profile.py profile.txt [N]
Routines are named from tools/hints.txt (the nearest `name` line at or below
the address in the same bank); `park` is time spent by rewritten code.
"""
import os
import sys
from collections import defaultdict

HERE = os.path.dirname(os.path.abspath(__file__))


def load_names():
    names = defaultdict(list)          # bank -> [(addr, name)]
    natives = set()
    for line in open(os.path.join(HERE, 'hints.txt')):
        parts = line.split()
        if len(parts) >= 3 and parts[0] in ('name', 'native') and ':' in parts[1]:
            b, a = parts[1].split(':')
            if parts[0] == 'name':
                names[int(b, 16)].append((int(a, 16), parts[2]))
            else:
                natives.add((int(b, 16), int(a, 16)))
    for b in names:
        names[b].sort()
    return names, natives


def main():
    names, natives = load_names()
    total = 0
    by = defaultdict(int)
    for line in open(sys.argv[1]):
        key, n = line.split()
        n = int(n)
        total += n
        if key == 'park':
            by['(rewritten code)'] += n
            continue
        b, a = (int(x, 16) for x in key.split(':'))
        label = '%02X:%04X' % (b, a & ~0xFF)   # unnamed: by page
        best = None
        for addr, name in names.get(b, ()):
            if addr <= a:
                best = (addr, name)
        if best and a - best[0] < 0x200:
            label = '%02X:%04X %s%s' % (b, best[0], best[1], ' [native]' if (b, best[0]) in natives else '')
        by[label] += n
    top = int(sys.argv[2]) if len(sys.argv) > 2 else 40
    for label, n in sorted(by.items(), key=lambda x: -x[1])[:top]:
        print('%6.2f%%  %s' % (100.0 * n / total, label))


if __name__ == '__main__':
    main()
