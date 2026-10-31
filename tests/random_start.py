"""The key script for tests/random_play.sh: tests/start.keys, then what the seed
chooses: a richer party, things owned, and a place to start from: leaving
Moonglow for somewhere on the world, or a shop or a person poked next to the
party (the way the shop_ and lb_ cases of tests/make_cases.py do).
    python tests/random_start.py <seed>
Random play itself starts a little later (U4_RANDOM_FROM in tests/random_play.sh).
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
# (location, the counter's row): every shop type in several towns, from the table at $96C1
SHOPS = [(0x07, 0x03), (0x08, 0x09), (0x0B, 0x1C), (0x0F, 0x0B), (0x10, 0x14), (0x07, 0x07), (0x0F, 0x11), (0x0B, 0x18),
         (0x0E, 0x04), (0x06, 0x0E), (0x07, 0x06), (0x0C, 0x11), (0x0E, 0x1A), (0x07, 0x02), (0x08, 0x13), (0x0B, 0x02),
         (0x0E, 0x05), (0x0F, 0x19), (0x10, 0x16), (0x06, 0x1A), (0x0C, 0x04), (0x0E, 0x07), (0x0F, 0x08), (0x07, 0x1D),
         (0x08, 0x06), (0x0C, 0x1B), (0x06, 0x02), (0x07, 0x0C), (0x08, 0x1A), (0x0A, 0x03), (0x0B, 0x03), (0x0C, 0x0D),
         (0x10, 0x15), (0x0F, 0x07), (0x10, 0x1A), (0x0E, 0x18), (0x01, 0x19)]
X = 0x08                               # the column shops and people are put in


def main():
    seed = int(sys.argv[1])
    rnd = random.Random(seed)
    out = open(os.path.join(HERE, 'start.keys')).read().splitlines()
    f = T0
    def poke(p):
        nonlocal f
        out.append('%d =%s' % (f, p))
        f += 1
    def press(k, gap=60):
        nonlocal f
        out.append('%d %s' % (f, k))
        out.append('%d -' % (f + 6))
        f += gap
    if rnd.random() < 0.7:             # money, food
        for p in ['C909:40', 'C90A:1F', 'C907:10', 'C908:27']:
            poke(p)
    if rnd.random() < 0.6:             # torches, gems, keys, weapons, armour, reagents
        for p in ['C913:20', 'C914:20', 'C915:20'] + ['%04X:05' % a for a in range(0xC917, 0xC937)]:
            poke(p)
    if rnd.random() < 0.5:             # a bigger party
        poke('C957:%02X' % rnd.randint(2, 8))
    if rnd.random() < 0.5:             # the magic points to cast with
        poke('C75A:63')
    if rnd.random() < 0.2:             # hurt, dead or asleep
        poke('C76D:%s' % rnd.choice(['44', '50', '53']))
    kind = rnd.random()
    if kind < 0.45:                    # a shop
        loc, row = rnd.choice(SHOPS)
        cell = 0xD000 + row * 64 + 2 * X
        for p in ['C9A0:%02X' % loc, 'C953:%02X' % X, 'C985:%02X' % X, 'C954:%02X' % (row + 1), 'C986:%02X' % (row + 1),
                  '%04X:60' % cell, 'C749:52', 'C74A:%02X' % X, 'C74B:%02X' % (row - 1), 'C74C:52']:
            poke(p)
        f += 60
        for k in '22U2':
            press(k)
    elif kind < 0.7:                   # a person: often one who can join, or who asks
        loc = rnd.randint(1, 16)
        if rnd.random() < 0.6:
            loc = rnd.randint(6, 13)
            for a in range(0xC90B, 0xC913):
                poke('%04X:63' % a)      # karma enough
            poke('C75B:08')            # and the Avatar's level
        for p in ['C9A0:%02X' % loc, 'C749:%02X' % rnd.choice([0x24, 0x28, 0x2C, 0x30, 0x38, 0x3C, 0x58, 0x5C, 0x5E, 0xAC]),
                  'C74A:02', 'C74B:0F', 'C74C:%02X' % rnd.choice([0x24, 0x38, 0x58, 0x5E]),
                  'C750:%02X' % (1 if loc < 14 and rnd.random() < 0.7 else rnd.randint(1, 16))]:
            poke(p)
        f += 60
        for k in '22R2':
            press(k)
    else:                              # leaving town
        if rnd.random() < 0.3:         # travelling otherwise
            poke(rnd.choice(['C959:14', 'C959:12', 'C959:18']))
            poke('C95A:00')
        x, y = rnd.choice(PLACES)
        poke('C96D:%02X' % x)
        poke('C96E:%02X' % y)
    print('\n'.join(out))
    print(f + 20, '-')


if __name__ == '__main__':
    main()
