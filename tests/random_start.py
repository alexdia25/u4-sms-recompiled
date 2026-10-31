"""The key script for tests/random_play.sh: tests/start.keys, then pokes chosen
by the seed: a richer party, things owned, and where leaving Moonglow leads.
    python tests/random_start.py <seed>
"""
import os
import random
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
T0 = 8640                              # the first frame after start.keys

# (x, y) just west or south of something: dungeons, towns, shrines, the sea
PLACES = [(0xF0, 0x4A), (0x5B, 0x44), (0x48, 0xA9), (0x56, 0x6D), (0xE9, 0x43), (0xB6, 0x36), (0x60, 0xD7),
          (0x2D, 0xAD), (0xDF, 0x85), (0x26, 0xE0), (0xA5, 0x13), (0x68, 0xC2), (0xBB, 0xA7), (0x17, 0x7E),
          (0x80, 0x80), (0xC0, 0x20), (0x20, 0x60), (0xE6, 0xD6)]


def main():
    seed = int(sys.argv[1])
    rnd = random.Random(seed)
    out = open(os.path.join(HERE, 'start.keys')).read().splitlines()
    pokes = []
    if rnd.random() < 0.7:             # money, food
        pokes += ['C909:40', 'C90A:1F', 'C907:10', 'C908:27']
    if rnd.random() < 0.6:             # torches, gems, keys, weapons, armour, reagents
        pokes += ['C913:20', 'C914:20', 'C915:20']
        pokes += ['%04X:05' % a for a in range(0xC917, 0xC937)]
    if rnd.random() < 0.5:             # a bigger party
        pokes += ['C957:%02X' % rnd.randint(2, 8)]
    if rnd.random() < 0.3:             # travelling otherwise
        pokes += [rnd.choice(['C959:14', 'C959:12', 'C959:18']), 'C95A:00']
    if rnd.random() < 0.5:             # the magic points to cast with
        pokes += ['C75A:63']
    x, y = rnd.choice(PLACES)
    pokes += ['C96D:%02X' % x, 'C96E:%02X' % y]
    for p in pokes:
        out.append('%d =%s' % (T0, p))
    print('\n'.join(out))


if __name__ == '__main__':
    main()
