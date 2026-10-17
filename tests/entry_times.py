"""Compare when the C tasks are entered, with the rewrites on and off.

    python tests/entry_times.py off.txt on.txt

Each file holds the "enter <name> <cycle>" lines that U4_TRACE prints. With
the rewrites on, a C task that carries on into the next one never passes
that one's address, so only the entries seen with them on are compared: each
must come at a cycle where the original run enters the same code too. Prints the
first difference and exits 1, or exits 0.
"""
import sys


def entries(path):
    out = []
    for line in open(path):
        p = line.split()
        if len(p) == 3 and p[0] == 'enter':
            out.append((p[1], int(p[2])))
    return out


def main():
    off, on = entries(sys.argv[1]), entries(sys.argv[2])
    j = 0
    prev = 'the start'
    for name, t in on:
        k = j                          # the same entry in the original: same name, same cycle
        while k < len(off) and not (off[k][0] == name and off[k][1] >= t):
            k += 1
        if k == len(off) or off[k][1] != t:
            near = [u for n, u in off[j:] if n == name]
            near = min(near, key=lambda u: abs(u - t)) if near else None
            print('%s entered at %d, original %s (after %s)' % (
                name, t, '%d (%+d)' % (near, t - near) if near is not None else 'never', prev))
            return 1
        prev = '%s at %d' % (name, t)
        j = k + 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
