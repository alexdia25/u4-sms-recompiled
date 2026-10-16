"""Write the test cases (tests/cases/*.keys) from tests/start.keys.

    python tests/make_cases.py

start.keys plays from power-on through creating a character to the town the
game starts in (Moonglow), 8,632 frames. Each case carries on from there:
the party is put next to a shop counter (by writing its position into RAM,
which a key script can do with "<frame> =ADDR:VAL" lines), the conversation
is steered into one path with a few presses, and then random presses follow.
Other cases walk and talk at random, or press buttons at random. The scripts
are inputs only; no game data is in them.
"""
import os
import random

HERE = os.path.dirname(os.path.abspath(__file__))
START = open(os.path.join(HERE, 'start.keys')).read().splitlines()
T0 = 8640                     # the first frame after start.keys

# party x, y, direction to the counter, settling moves (to redraw the view)
SPOTS = {
    'inn':     (0x12, 0x03, 'U', 'LR'),
    'food':    (0x1A, 0x0F, 'U', 'LR'),
    'herbs':   (0x04, 0x1B, 'U', 'RL'),
    'healer':  (0x19, 0x1A, 'D', 'LR'),
}
RICH = ['C909:40', 'C90A:1F']           # 8000 gold
POOR = ['C909:02', 'C90A:00']

# townsfolk who stay put: where to stand to talk to them, and their record
# in the people table ($C651, 8 bytes each)
PEOPLE = {
    'p19': (0x03, 0x1A, 'D', 'LR', 19),
    'p15': (0x07, 0x0E, 'L', 'RL', 15),
    'p31': (0x07, 0x05, 'R', 'LR', 31),
    'p28': (0x12, 0x05, 'D', 'RL', 28),
    'p27': (0x14, 0x05, 'D', 'UD', 27),
    'p18': (0x15, 0x0B, 'U', 'RL', 18),
    'p30': (0x1D, 0x03, 'L', 'LR', 30),
    'p13': (0x1D, 0x11, 'L', 'LR', 13),
    'p22': (0x1C, 0x15, 'U', 'RL', 22),
}
# talks: person, RAM writes once the talk has started, random presses
TALKS = {
    'talk_question': ('p15', ['C9A9:03', 'C9AA:01'], 40),      # JOB leads to the yes/no question
    'talk_turnaway': ('p13', ['C9AB:FF'], 30),                 # asked too often
    'talk_give':     ('p18', ['person+3:58', 'C909:40', 'C90A:1F'], 40),   # a beggar
}
for _p in PEOPLE:
    TALKS['talk_' + _p] = (_p, [], 40)

# name: (spot, RAM writes, steering presses, random presses)
CASES = {
    'inn_stay':      ('inn', [], '22222222', 20),
    'inn_no':        ('inn', [], '2D2', 10),
    'inn_poor':      ('inn', ['C909:00', 'C90A:00'], '22222222', 10),
    'food_buy':      ('food', [], '222UU22', 20),
    'food_buy_quick': ('food', [], '222UU22', 20, 0),   # the same, the first steps 10 frames earlier
    'food_more':     ('food', RICH, '222UUU2D2UU2', 20),
    'food_some':     ('food', ['C909:64', 'C90A:00'], '222UUUU222U22', 20),
    'food_broke':    ('food', ['C909:00', 'C90A:00'], '222UU22', 10),
    'herbs_buy':     ('herbs', [], '2222D2UU2UUUUUUUU22', 20),
    'herbs_cheap':   ('herbs', [], '2222DD2UU2U22', 20),
    'herbs_poor':    ('herbs', POOR, '2222D2UU2UUUUUUUU22', 20),
    'herbs_no':      ('herbs', [], '2D2', 10),
    'healer_cure':   ('healer', ['C76D:50'], '22222222', 20),
    'healer_heal':   ('healer', ['C751:64', 'C752:00'], '222D22222', 20),
    'healer_blood':  ('healer', ['C751:F4', 'C752:01'], '2D22', 10),
    'healer_poor':   ('healer', ['C76D:50', 'C909:10', 'C90A:00'], '22222222', 10),
    'healer_no':     ('healer', [], '2D2', 10),
}


class Script:
    def __init__(self):
        self.lines = list(START)
        self.f = T0

    def poke(self, addr_val):
        self.lines.append('%d =%s' % (self.f, addr_val))

    def press(self, k, gap=40):
        self.lines.append('%d %s' % (self.f, k))
        self.lines.append('%d -' % (self.f + 6))
        self.f += gap

    def write(self, name):
        self.f += 400                  # let the last thing finish printing
        self.lines.append('%d -' % self.f)
        with open(os.path.join(HERE, 'cases', name + '.keys'), 'w', newline='\n') as out:
            out.write('\n'.join(self.lines) + '\n')


def random_presses(s, rnd, n, menu=True):
    for _ in range(n):
        r = rnd.random()
        if menu:
            k = 'D' if r < 0.3 else 'U' if r < 0.5 else '2' if r < 0.95 else '1'
        else:
            k = rnd.choice('UDLR') if r < 0.6 else '2' if r < 0.9 else '1'
        s.press(k, rnd.choice((20, 40, 80)))


def main():
    os.makedirs(os.path.join(HERE, 'cases'), exist_ok=True)
    for name, case in CASES.items():
        spot, pokes, steer, n = case[:4]
        wait = case[4] if len(case) > 4 else 10
        x, y, d, moves = SPOTS[spot]
        rnd = random.Random(name)
        s = Script()
        for p in ['C953:%02X' % x, 'C985:%02X' % x, 'C954:%02X' % y, 'C986:%02X' % y] + pokes:
            s.poke(p)
        s.f += wait
        s.press(moves[0], 30)          # two steps, so the view is drawn there
        s.press(moves[1], 50)
        for k in '22' + d + '2':       # command menu, Talk, direction, confirm
            s.press(k, 30)
        s.f += 90
        for k in steer:
            s.press(k, 60)
        random_presses(s, rnd, n)
        for _ in range(6):
            s.press('1', 60)
        s.write(name)
    for name, (who, pokes, n) in TALKS.items():
        x, y, d, moves, rec = PEOPLE[who]
        rnd = random.Random(name)
        s = Script()
        for p in ['C953:%02X' % x, 'C985:%02X' % x, 'C954:%02X' % y, 'C986:%02X' % y]:
            s.poke(p)
        s.f += 10
        s.press(moves[0], 30)
        s.press(moves[1], 50)
        for k in '22' + d + '2':
            s.press(k, 30)
        s.f += 60
        for p in pokes:                # after talk_setup has read the person
            if p.startswith('person+'):
                off, val = p[7:].split(':')
                p = '%04X:%s' % (0xC651 + rec * 8 + int(off, 16), val)
            s.poke(p)
        s.f += 30
        s.press('2', 60)               # past the greeting
        random_presses(s, rnd, n)
        for _ in range(4):
            s.press('1', 60)
        s.write(name)
    for name, pokes in (('camp_order_swap', ['C957:03']), ('camp_rest_world', [])):
        rnd = random.Random(name)    # New Order with a party; Rest out in the world
        s = Script()
        for p in pokes:
            s.poke(p)
        if name == 'camp_rest_world':
            for k in 'LLLLLL':
                s.press(k, 30)
        s.press('2', 40)
        for _ in range(7):
            s.press('D', 30)
        for k in ('2D22D22' if pokes else '22'):
            s.press(k, 60)
        random_presses(s, rnd, 20, menu=False)
        s.write(name)
    WALKS = {                          # walking: in town, out into the world, riding, sailing
        'walk_town':  ([], 'UDLR', 120),
        'walk_world': ([], 'LLLLLLUDLR', 150),
        'ride_world': (['C959:14', 'C95A:00'], 'LLLLLLUDLR', 150),
        'sail':       (['C959:11', 'C95A:00'], 'UDLR', 60),
    }
    for name, (pokes, dirs, n) in WALKS.items():
        rnd = random.Random(name)
        s = Script()
        for p in pokes:
            s.poke(p)
        for k in dirs[:6] if dirs.startswith('LLLLLL') else '':
            s.press(k, 30)             # out through the west gate
        for _ in range(n):
            r = rnd.random()
            s.press(rnd.choice(dirs[-4:]) if r < 0.85 else '1' if r < 0.95 else '2', rnd.choice((20, 30, 60)))
        s.write(name)
    COMMANDS = {                       # each command from the menu: how many Downs, then presses
        'cmd_talk_nobody': (0, '2U2'),
        'cmd_attack':      (1, '2U2222'),
        'cmd_magic':       (2, '2222222'),
        'cmd_get_chest':   (3, '22222'),
        'cmd_use':         (4, '2222222'),
        'cmd_ready_weapon': (5, '22D2222'),
        'cmd_ready_armour': (5, '2D2D2222'),
        'cmd_search':      (6, '2222'),
        'cmd_camp':        (7, '2222222'),
        'cmd_cancel':      (3, '1'),
        'camp_rest':       (7, '22'),
        'camp_order':      (7, '2D2'),
        'camp_status':     (7, '2DD22222'),
        'camp_save':       (7, '2DDD2222'),
    }
    for name, (downs, presses) in COMMANDS.items():
        rnd = random.Random(name)
        s = Script()
        if name == 'camp_order':
            s.poke('C957:03')          # three in the party, to swap
        for _ in range(2):
            s.press('2', 40)           # the command menu
            for _ in range(downs):
                s.press('D', 30)
            for k in presses:
                s.press(k, 60)
            random_presses(s, rnd, 12)
            for _ in range(3):
                s.press('1', 60)
        s.write(name)
    for seed in range(2):              # anything at all
        rnd = random.Random('play%d' % seed)
        s = Script()
        random_presses(s, rnd, 150, menu=False)
        s.write('play_%d' % seed)


if __name__ == '__main__':
    main()
