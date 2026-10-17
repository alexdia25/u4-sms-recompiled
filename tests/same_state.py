"""Do two runs end in the same state?

    python tests/same_state.py original.out rewritten.out

Each file is the headless exe's output with U4_SETTLE: the state-hash at the
end of the run, then a few more frames' pc and state-hash. A frame can end
while the rewrites are half way through some work that the C did at once
(the CPU parked at $FFF8), where the original is part of the way through it,
so the two are compared at the end, or else at the first frame after it
where both are at the same instruction. Prints why not, and exits 1, or
exits 0.
"""
import re
import sys


def states(path):
    out = []
    for line in open(path):
        m = re.search(r'pc ([0-9A-F]{4}) .*state-hash ([0-9A-F]{8})', line)
        if m and (line.startswith('frames') or line.startswith('settle')):
            out.append((m.group(1), m.group(2)))
    return out


def main():
    orig, new = states(sys.argv[1]), states(sys.argv[2])
    if not orig or not new:
        print('no state-hash in the output')
        return 1
    if orig[0][1] == new[0][1]:
        return 0
    for (opc, oh), (npc, nh) in zip(orig, new):
        if opc == npc and npc != 'FFF8':
            if oh == nh:
                return 0
            print('state %s vs original %s at pc %s' % (nh, oh, npc))
            return 1
    print('state %s vs original %s (never at the same place after)' % (new[0][1], orig[0][1]))
    return 1


if __name__ == '__main__':
    sys.exit(main())
