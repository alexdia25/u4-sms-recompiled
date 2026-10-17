"""Do two runs end in the same state?

    python tests/same_state.py original.out rewritten.out

Each file is the headless exe's output with U4_SETTLE: the state-hash at the
end of the run, then the state at each pass of the task scheduler in the
frames after it. A frame can end while the rewrites have done at once some
work the original is still part way through, so the end states may differ;
at the scheduler neither is in the middle of anything, and the two must be
the same there, pass for pass. Prints why not and exits 1, or exits 0.
"""
import re
import sys


def read(path):
    end, passes = None, []
    for line in open(path):
        m = re.search(r'state-hash ([0-9A-F]{8})', line)
        if not m:
            continue
        if line.startswith('frames'):
            end = m.group(1)
        elif line.startswith('settle'):
            t = re.search(r' at (\d+) ', line)
            passes.append((int(t.group(1)), m.group(1)))
    return end, passes


def main():
    (oend, opass), (nend, npass) = read(sys.argv[1]), read(sys.argv[2])
    if oend is None or nend is None:
        print('no state-hash in the output')
        return 1
    if not opass or not npass:
        return 0 if oend == nend else (print('state %s vs original %s' % (nend, oend)) or 1)
    for i, ((ot, oh), (nt, nh)) in enumerate(zip(opass, npass)):
        if ot != nt:
            print('scheduler pass %d at %d, original at %d' % (i, nt, ot))
            return 1
        if oh != nh:
            print('state %s vs original %s at scheduler pass %d' % (nh, oh, i))
            return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
