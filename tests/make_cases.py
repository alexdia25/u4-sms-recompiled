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
    'food_digits':   ('food', [], '222ULULRRLLR22', 20),
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
    'healer_cure_no': ('healer', ['C76D:50'], '2222D2', 10),
    'healer_heal_no': ('healer', ['C751:64', 'C752:00'], '222D22D2', 10),
    'healer_raise':  ('healer', [], ['=C957:02', '=C78A:44'] + list('222DD2D22222'), 20),
    'healer_raise_no': ('healer', [], ['=C957:02', '=C78A:44'] + list('222DD2D2D2'), 10),
    'healer_raise_poor': ('healer', ['C909:10', 'C90A:00'], ['=C957:02', '=C78A:44'] + list('222DD2D22222'), 10),
    'healer_blood_no': ('healer', ['C751:F4', 'C752:01'], '2D2D2', 10),
    'healer_raise_alive': ('healer', [], '222DD2222', 10),
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
        at = [int(l.split()[0]) for l in self.lines]
        assert at == sorted(at), name + ': lines out of order: the game would never read those after them'
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


WORD = list(open(os.path.join(HERE, '..', 'build', 'u4.sms'), 'rb').read()[0x734B:0x7352])


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
            if k[0] == '=':
                s.poke(k[1:])
            else:
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
    for k in (0, 1, 5):                # camping out: no time to rest, healed, ambushed (by the random numbers)
        name = 'camp_world_%d' % k
        rnd = random.Random(name)
        s = Script()
        for key in 'LLLLLL':
            s.press(key, 30)
        s.f += 3 * k
        if k:
            s.poke('C965:FF'); s.poke('C966:FF')   # it is time to rest

        s.press('2', 40)
        for _ in range(7):
            s.press('D', 30)
        for key in '22':
            s.press(key, 60)
        s.f += 60 + 4 * k
        s.poke('C0B5:%02X' % (k * 37 & 0xFF))   # the random numbers, steered
        s.f += 840
        random_presses(s, rnd, 6, menu=False)
        s.write(name)
    WALKS = {                          # walking: in town, out into the world, riding, sailing
        'walk_town':  ([], 'UDLR', 120),
        'walk_world': ([], 'LLLLLLUDLR', 150),
        'ride_world': (['C959:14', 'C95A:00'], 'LLLLLLUDLR', 150),
        'sail':       (['C959:11', 'C95A:00'], 'UDLR', 60),
        'walk_poisoned': (['C76D:50', 'C957:02', 'C78A:50'], 'UDLR', 60),
        'walk_starving': (['C907:00', 'C908:00', 'C957:03'], 'UDLR', 60),
        'walk_asleep':   (['C957:02', 'C78A:53'], 'UDLR', 60),
        'sail_damaged':  (['C959:11', 'C95A:00', 'C961:10'], 'UDLR', 60),
        'walk_door':     (['C9A2:02', 'C9A3:0F', 'C9A4:03'], 'UDRR', 30),   # an opened door closes again
        'walk_door_in':  (['C9A2:01', 'C9A3:0F', 'C9A4:02'], 'UDUD', 10),   # ... not while the party stands in it
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
        'ready_swap':      (5, '2' + '2' + 'D2' + '2'),        # a dagger readied, the hands' put away
        'ready_not_allowed': (5, '2' + '2' + 'D2' + '22'),     # a mage may not use an axe
        'ready_armour_not_allowed': (5, '2D2' + '2' + 'D2' + '22'),
        'ready_cancel':    (5, '2' + '1' + '2' + '2' + '2' + '1'),   # at who, then at the list
        'camp_status_keys': (7, '2DD2' + 'D' * 7 + 'UU' + 'D' + 'LRR' + 'U' + 'LR' + 'D' + 'L' + '2' + 'LRR' + 'D' + '1'),   # every page and key
    }
    for name, (downs, presses) in COMMANDS.items():
        rnd = random.Random(name)
        s = Script()
        if name in ('camp_order', 'camp_status_keys'):
            s.poke('C957:03')          # three in the party, to swap
        for p in {'ready_swap': ['C921:01'], 'ready_not_allowed': ['C924:01'],
                  'ready_armour_not_allowed': ['C91A:01'], 'ready_cancel': ['C921:01']}.get(name, []):
            s.poke(p)                  # something to ready
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
    # Save out in the world (in a town it is refused): the slot moved and
    # saved, and moved and cancelled
    for name, keys in (('save_world', 'DD' + 'U' + '2' + '2'), ('save_cancel', 'UD' + '1')):
        rnd = random.Random(name)
        s = Script()
        for k in 'LLLLLL':
            s.press(k, 30)             # out through the west gate
        s.f += 10
        s.press('2', 40)               # the command menu, Camp, Save
        for _ in range(7):
            s.press('D', 30)
        s.press('2', 60)
        for k in 'DDD2':
            s.press(k, 60)
        for k in keys:
            s.press(k, 60)
        random_presses(s, rnd, 8)
        s.write(name)
    # Saved in the world, Reset, and Journey Onward: the saved games' menu,
    # moved about, and the saved game loaded, or an empty slot (a new game)
    for name, keys in (('journey_load', 'DUDU' + '2'), ('journey_empty', 'DU' + 'D' + 'D' + '2')):
        s = Script()
        for k in 'LLLLLL':
            s.press(k, 30)
        s.f += 10
        s.press('2', 40)
        for _ in range(7):
            s.press('D', 30)
        s.press('2', 60)
        for k in 'DDD2' + '2':
            s.press(k, 60)             # saved in the first slot
        s.f += 308                     # (while the game waits: a rewrite stopped half way
        s.press('X', 60)               # by Reset leaves what its last step did) Reset
        s.f += 1140
        s.press('1', 100)              # past the opening
        for k in 'DU2':
            s.press(k, 30)             # Journey Onward
        s.f += 400
        for k in keys:
            s.press(k, 50)
        s.f += 600
        s.write(name)
    SEARCHES = {                       # Search where something is to be found (the table at $8C07)
        'search_town_1':  (False, ['C953:08', 'C985:08', 'C954:06', 'C986:06']),   # Moonglow is 6
        'search_town_2':  (False, ['C9A0:03', 'C953:06', 'C985:06', 'C954:06', 'C986:06']),
        'search_world_1': (True, ['C953:B6', 'C954:36', 'C95D:00', 'C95F:00', 'C967:FF', 'C968:FF']),
        'search_world_2': (True, ['C953:E0', 'C954:85']),
        'search_world_3': (True, ['C953:2D', 'C954:AD']),
        'search_full':    (True, ['C953:B6', 'C954:36', 'C95D:00', 'C95F:00', 'C967:FF', 'C968:FF'] + ['%04X:62' % (0xC92F + i) for i in range(8)]),
        'search_bell':    (True, ['C953:B0', 'C954:D0']),
        'search_wheel':   (True, ['C953:60', 'C954:D7']),
        'search_skull':   (True, ['C953:C5', 'C954:F5', 'C95D:00', 'C95F:00']),
        'search_black_stone': (True, ['C953:E0', 'C954:85', 'C95D:00', 'C95F:00']),
        'search_nightshade': (True, ['C953:2E', 'C954:95', 'C95D:00', 'C95F:00', 'C967:FF', 'C968:FF']),
        'search_telescope': (False, ['C9A0:03', 'C953:16', 'C985:16', 'C954:03', 'C986:03']),
        'search_candle':  (False, ['C9A0:11', 'C953:16', 'C985:16', 'C954:01', 'C986:01']),
        'search_mystic_armour': (False, ['C9A0:04', 'C953:16', 'C985:16', 'C954:04', 'C986:04', 'C90B:00', 'C90C:00', 'C90D:00', 'C90E:00', 'C90F:00', 'C910:00', 'C911:00', 'C912:00']),
        'search_mystic_weapons': (False, ['C9A0:05', 'C953:08', 'C985:08', 'C954:0F', 'C986:0F', 'C90B:00', 'C90C:00', 'C90D:00', 'C90E:00', 'C90F:00', 'C910:00', 'C911:00', 'C912:00']),
    }
    for name, (world, pokes) in SEARCHES.items():
        rnd = random.Random(name)
        s = Script()
        if world:
            for k in 'LLLLLL':
                s.press(k, 30)         # out through the west gate
        for p in pokes:
            s.poke(p)
        s.f += 10
        for _ in range(2):
            if 'C95D:00' in pokes:     # the moons new again: they move with time
                for p in ('C95D:00', 'C95E:00', 'C95F:00', 'C960:00'):
                    s.poke(p)
            s.press('2', 40)           # the command menu, Search
            for _ in range(6):
                s.press('D', 30)
            if 'C967:FF' in pokes:
                for p in ('C95D:00', 'C95E:00', 'C95F:00', 'C960:00'):
                    s.poke(p)
            s.press('2', 60)
            random_presses(s, rnd, 8, menu=False)
        s.write(name)
    USES = {                           # Use: the item (Downs in its menu) and what the party has
        'use_torch':   (0, ['C913:03']),
        'use_gem':     (1, ['C914:02']),
        'use_key':     (2, ['C915:02', 'D3C4:3A']),   # a locked door east of the party
        'use_key_none': (2, ['C915:02']),
        'use_sextant': (3, ['C916:01']),
        'use_none':    (1, ['C914:00']),
        'use_stone':   (None, []),
    }
    for name, (item, pokes) in USES.items():
        rnd = random.Random(name)
        s = Script()
        for p in pokes:
            s.poke(p)
        s.f += 10
        s.press('2', 40)               # the command menu, Use
        for _ in range(4):
            s.press('D', 30)
        s.press('2', 60)
        if item is None:
            s.press('D', 30)           # Stone
            s.press('2', 60)
        else:
            s.press('2', 60)           # Item
            for _ in range(item):
                s.press('D', 30)
            s.press('2', 60)
        random_presses(s, rnd, 12)
        s.write(name)
    # the special things (Use, then the second choice): pokes, whether out in
    # the world, and the presses in the list for each use in turn
    def use_things_presses(s, keys):
        s.press('2', 40)               # the command menu, Use, the things
        for _ in range(4):
            s.press('D', 30)
        s.press('2', 60)
        s.press('D', 30)
        s.press('2', 60)
        for k in keys:
            s.press(k, 40)
        s.f += 200
    ABYSS = ['C953:E9', 'C954:E9']
    THINGS = {
        'things_cancel':     (['C951:FD', 'C952:03', 'C955:03'], False, ['DDD1']),
        'things_key':        (['C951:20'], False, ['2']),
        'things_keys':       (['C951:60'], False, ['2']),
        'things_stone_town': (['C955:01'], False, ['2']),
        'things_bell_town':  (['C951:04'], False, ['2']),
        'things_skull_town': (['C951:01'], False, ['2']),
        'things_skull_world': (['C951:01'], True, ['2']),
        'things_horn':       (['C952:01'], True, ['2']),
        'things_horn_town':  (['C952:01'], False, ['2']),
        'things_wheel':      (['C952:02', 'C959:10', 'C961:32'], True, ['2']),
        'things_wheel_land': (['C952:02'], True, ['2']),
        'things_wheel_whole': (['C952:02', 'C959:10', 'C961:63'], True, ['2']),
        'things_abyss':      (['C951:1D'] + ABYSS, True, ['2', 'D2', 'DD2', 'DDD2']),
        'things_book_first': (['C951:08'] + ABYSS, True, ['2']),
        'things_skull_abyss': (['C951:01'] + ABYSS, True, ['2']),
        'things_skull_5e': (['C951:01', 'C654:5E', 'C651:5E', 'C652:03', 'C653:03'], False, ['2']),
    }
    for name, (pokes, world, uses) in THINGS.items():
        rnd = random.Random(name)
        s = Script()
        if world:
            for k in 'LLLLLL':
                s.press(k, 30)         # out through the west gate
        for p in pokes:
            s.poke(p)
        s.f += 10
        for keys in uses:
            use_things_presses(s, keys)
        random_presses(s, rnd, 6)
        s.write(name)
    ATTACKS = {                        # Attack: where the party stands (or a person's spot), RAM writes, direction
        'attack_person':  ('p15', [], None),
        'attack_guard':   ('p28', [], None),
        'attack_nobody':  (None, [], 'U'),
        'attack_no_karma': ('p15', ['%04X:00' % (0xC90B + i) for i in range(8)], None),
        'ship_fire':      ('world', ['C959:10', 'C95A:00'], 'U'),
        'ship_no_fire':   ('world', ['C959:10', 'C95A:00'], 'L'),
        'ship_fire_odd':  ('world', ['C959:11', 'C95A:00'], 'L'),
    }
    for name, (where, pokes, d) in ATTACKS.items():
        rnd = random.Random(name)
        s = Script()
        if where == 'world':
            for k in 'LLLLLL':
                s.press(k, 30)
        elif where:
            x, y, d, moves, rec = PEOPLE[where]
            for p in ['C953:%02X' % x, 'C985:%02X' % x, 'C954:%02X' % y, 'C986:%02X' % y]:
                s.poke(p)
            s.f += 10
            s.press(moves[0], 30)
            s.press(moves[1], 50)
        for p in pokes:
            s.poke(p)
        s.f += 10
        for k in '2D2':                # command menu, Attack
            s.press(k, 30)
        s.press(d, 30)                 # the direction, confirmed
        s.press('2', 30)
        s.f += 60
        random_presses(s, rnd, 30, menu=False)
        for _ in range(4):             # out of any menu, so the run doesn't end inside one
            s.press('1', 60)
        s.write(name)
    FIVE = ['%04X:05' % (0xC92F + i) for i in range(8)]   # five of each reagent
    MIXES = {                          # Magic, Mix: RAM writes, presses from the spell list on
        'mix_awaken':  (FIVE, '2' + 'D2D2UUU2' + 'U2'),          # spell 0: reagents 1 and 2
        'mix_three':   (FIVE, '2' + 'D2D2UUU2' + 'UUU2'),        # three of them
        'mix_wrong':   (FIVE, '2' + '2D2UU2' + 'U2'),            # reagents 0 and 1: fizzles
        'mix_page2':   (FIVE, 'R2' + '2DD2DDDDD2D2' + 'U2'),      # spell 13: reagents 0, 2, 7
        'mix_short':   (['C930:01', 'C931:01'], '2' + 'D2D2UUU2' + 'UUU2'),   # only enough for one
        'mix_nothing': (FIVE, '2' + 'UUU2'),                     # Done with nothing ticked
        'mix_none':    (['%04X:00' % (0xC92F + i) for i in range(8)], ''),   # no reagents
        'mix_random':  (FIVE, ''),
    }
    for name, (pokes, keys) in MIXES.items():
        rnd = random.Random(name)
        s = Script()
        for p in pokes:
            s.poke(p)
        s.f += 10
        for k in '2DD2D2':             # command menu, Magic, Mix
            s.press(k, 30)
        s.f += 30
        for k in keys:
            s.press(k, 30)
        s.f += 600                     # the mixing takes 30 turns
        random_presses(s, rnd, 20)
        for _ in range(4):
            s.press('1', 60)
        s.write(name)
    ALL_MIX = ['%04X:03' % (0xC937 + i) for i in range(26)]   # three of each mixture
    CASTS = {                          # Magic, Cast: RAM writes, out in the world?, presses in the list
        'cast_first':   (ALL_MIX, False, '2'),
        'cast_scroll':  (ALL_MIX, False, 'DDDDDDDDDDD2'),        # past the first page
        'cast_back':    (ALL_MIX, False, 'DDDDDDDDDUUUUUUUUUU2'),
        'cast_world':   (ALL_MIX, True, 'DD2'),
        'cast_cancel':  (ALL_MIX, False, 'D1'),
        'cast_none':    ([], False, ''),
        'cast_random':  (ALL_MIX, True, ''),
    }
    for name, (pokes, world, keys) in CASTS.items():
        rnd = random.Random(name)
        s = Script()
        if world:
            for k in 'LLLLLL':
                s.press(k, 30)
        for p in pokes:
            s.poke(p)
        s.f += 10
        for k in '2DD22':              # command menu, Magic, Cast
            s.press(k, 30)
        s.f += 30
        s.press('2', 40)               # the Avatar casts
        for k in keys:
            s.press(k, 30)
        s.f += 120
        random_presses(s, rnd, 20)
        for _ in range(4):
            s.press('1', 60)
        s.write(name)
    MAGIC = ['C75A:63'] + ALL_MIX      # 99 M.P. and three of each mixture
    SPELLS = {                         # each spell outside combat: RAM writes, outdoors?, Downs in the list, then presses
        'spell_awaken':    (['C957:02', 'C78A:53'], False, 0, '2D2'),   # Iolo asleep
        'spell_cure':      (['C76D:50'], False, 2, '22'),
        'spell_cure_none': ([], False, 2, '22'),                 # nobody poisoned: it fails
        'spell_resurrect': (['C957:02', 'C78A:44'], False, 7, '2D2'),
        'spell_heal':      (['C751:10', 'C752:00'], False, 5, '22'),
        'spell_open':      ([], False, 6, '2'),
        'spell_view':      ([], False, 8, '2' + '2'),
        'spell_blink':     ([], True, 1, '2U2'),
        'spell_blink_in':  ([], False, 1, '2'),                  # outdoors only
        'spell_gate':      ([], True, 4, '2DDD2'),
        'spell_winds':     ([], True, 9, '2R2'),
        'spell_negated':   (['C976:4E', 'C76D:50'], False, 2, '22'),
        'spell_dispel':    ([], False, 3, '2R2'),
        'spell_blink_balloon': (['C959:18', 'C95A:00'], True, 1, '2U2'),
        'spell_open_balloon': (['C959:18', 'C95A:00'], True, 6, '2'),
        'spell_gate_balloon': (['C959:18', 'C95A:00'], True, 4, '2DDD2'),
        'spell_winds_balloon': (['C959:18', 'C95A:00'], True, 9, '2R2'),
        'spell_dispel_44': (['D3C4:44'], False, 3, '2R2'),
        'spell_dispel_47': (['D3C4:47'], False, 3, '2R2'),
        'spell_dispel_48': (['D3C4:48'], False, 3, '2R2'),
        'spell_dispel_world': ([], True, 3, '2U2'),
    }
    for name, (pokes, world, downs, keys) in SPELLS.items():
        rnd = random.Random(name)
        s = Script()
        if world:
            for k in 'LLLLLL':
                s.press(k, 30)
        for p in MAGIC + pokes:
            s.poke(p)
        s.f += 10
        for k in '2DD22':              # command menu, Magic, Cast
            s.press(k, 30)
        s.f += 30
        s.press('2', 40)               # the Avatar casts
        for _ in range(downs):
            s.press('D', 30)
        for k in keys:
            s.press(k, 40)
        s.f += 200
        random_presses(s, rnd, 12, menu=False)
        for _ in range(4):
            s.press('1', 60)
        s.write(name)
    STEER = {'fight_flee': 'LLLLLLLLLLLLLLLLLLLLLLLL', 'fight_blocked': 'UUUUUURRRRRRDDDDDDLLLLLL'}
    for _n in ('fight_dagger', 'fight_oil', 'fight_halberd', 'fight_axe', 'fight_wand'):
        STEER[_n] = '2222' * 6              # menu, Attack, aim, (next member)
    STEER['fight_commands'] = ('2DD2' + '2DDD22' + '2DDDD2D22' + '2DDDDD22' + '2DDDDD2' + '1' + '2DDD2D2')
    STEER['fight_cast'] = '2D22' + '2D2D2' + '2D2DD2' + '2D2U2' + '2D2DDD22'
    FIGHTS = {                         # combat: attack someone, then fight at random
        'fight_1':       ('p15', [], 1, 120),
        'fight_2':       ('p28', [], 2, 120),
        'fight_party':   ('p19', ['C957:03'], 3, 150),     # three in the party
        'fight_magic':   ('p31', ['C957:02'] + MAGIC, 4, 150),
        'fight_long':    ('p13', ['C957:04'], 5, 300),
        'fight_flee':    ('p15', ['C957:02'], 6, 0),       # walk off the field
        'fight_blocked': ('p28', [], 7, 0),                 # into the trees and the foe
        'fight_dagger':  ('p15', ['C75C:02', 'C921:01'], 8, 40),   # thrown: one spare, then the last
        'fight_oil':     ('p28', ['C75C:09', 'C928:00'], 9, 40),   # flaming oil, the last one
        'fight_halberd': ('p19', ['C75C:0A'], 10, 40),
        'fight_axe':     ('p31', ['C75C:0B'], 11, 40),             # the magic axe comes back
        'fight_wand':    ('p13', ['C75C:0E', 'C757:10'], 12, 40),  # low dexterity
        'fight_commands': ('p15', ['C957:02', 'C914:02'], 13, 30), # each command from the combat menu
        'fight_cast':    ('p28', MAGIC, 14, 30),                   # spells in combat
    }
    COMBAT_SPELLS = 'ACDEFHIJKMNOPQSTU'      # the spell list in combat
    for i, sp in enumerate(COMBAT_SPELLS):
        FIGHTS['cspell_' + sp.lower()] = (('p15', 'p28', 'p19')[i % 3], MAGIC + ['C957:02', 'C76D:50', 'C78A:53'], 40 + i, 20)
        STEER['cspell_' + sp.lower()] = '2D2' + 'D' * i + '2' + '2' + 'U2' + '2'
    for i, g in enumerate((0x90, 0xA4, 0xB0, 0xC0, 0xD0, 0xDC, 0xE0, 0xE8, 0xEC, 0xF0, 0xF8, 0xFC)):
        FIGHTS['foe_%02x' % g] = (('p15', 'p28', 'p19')[i % 3], ['C957:04', 'person+3:%02X' % g], 20 + i, 120)
    for i, g in enumerate((0xB8, 0x84, 0xE8, 0xE8, 0xB8)):   # missiles: sleep, lava (and where they stop)
        FIGHTS['shot_%02x_%d' % (g, i)] = (('p15', 'p28', 'p19')[i % 3], ['C957:04', 'person+3:%02X' % g], 60 + i, 200)
    # the ends of a fight: every foe gone, or the party down
    NOFOES = ['%04X:00' % (0xC83E + 8 * i) for i in range(16)]
    AFTER = {'fight_won': NOFOES, 'fight_lost': ['C751:00', 'C752:00', 'C76D:44']}
    FIGHTS['fight_won'] = ('p15', ['C957:01'], 30, 10)
    FIGHTS['fight_lost'] = ('p28', ['C957:01'], 31, 10)
    STEER['fight_won'] = STEER['fight_lost'] = 'R'
    for k in range(32):                # a chest under the member on the field: every trap
        name = 'fight_chest_%d' % k
        FIGHTS[name] = ('p15', ['C957:03', 'C758:00'], 20 + k, 6)
        AFTER[name] = ['C8B9:05', 'C8BA:05', 'D878:3C', 'D879:00', 'C0B6:%02X' % (k * 41 & 0xFF), 'C0B7:%02X' % (k * 113 & 0xFF)]
        STEER[name] = '2DD2'
    # Awaken on a member asleep on the field; each spell when the foes have
    # just gone; each with other luck, against foes too strong for some ($17)
    # won against someone who isn't a monster, out in the world ($C975)
    for g in (0x10, 0x14, 0x18, 0x24, 0x30, 0x50, 0x58, 0x60, 0x7C, 0x8C):
        name = 'fight_won_%02x' % g
        FIGHTS[name] = ('p15', ['C957:01'], 30, 10)
        AFTER[name] = NOFOES + ['C975:00', 'C97D:%02X' % g]
        STEER[name] = 'R'
    # Undead against the undead
    for g in (0x9C, 0xBC, 0xC4, 0xE4):
        for k in range(3):
            name = 'cspell_undead_%02x_%d' % (g, k)
            FIGHTS[name] = ('p15', MAGIC + ['C957:02', 'person+3:%02X' % g], 80 + k, 20)
            AFTER[name] = ['%04X:40' % (0xC842 + 8 * i) for i in range(16)] + ['C0B6:%02X' % (k * 41 & 0xFF), 'C0B7:%02X' % (k * 113 & 0xFF)]
            STEER[name] = '2D2' + 'D' * COMBAT_SPELLS.index('U') + '2' + '2' + 'U2' + '2'
    # a member stepping on each kind of field
    for tile in (0x44, 0x45, 0x46, 0x47):
        name = 'fight_field_%02x' % tile
        FIGHTS[name] = ('p15', ['C957:02'], 90, 30)
        AFTER[name] = ['C8B9:05', 'C8BA:05', 'C8BE:06', 'C8BF:05', 'D878:%02X' % tile, 'D87A:%02X' % tile, 'D87C:%02X' % tile]
        STEER[name] = 'RRLR'
    # every kind of foe, with two sets of luck
    for g in range(0x90, 0x100, 4):
        for k in (0, 1):
            FIGHTS['foe_all_%02x_%d' % (g, k)] = (('p15', 'p28')[k], ['C957:03', 'person+3:%02X' % g], 100 + k, 200)
    FIGHTS['cspell_awaken'] = ('p15', MAGIC + ['C957:02'], 70, 10)
    AFTER['cspell_awaken'] = ['C78A:53']
    STEER['cspell_awaken'] = '2D2' + '2' + 'D2'
    STRONG = ['%04X:40' % (0xC842 + 8 * i) for i in range(16)]
    for i, sp in enumerate(COMBAT_SPELLS):
        name = 'cspell_%s_gone' % sp.lower()
        FIGHTS[name] = ('p15', MAGIC + ['C957:02'], 71, 10)
        STEER[name] = ['2', 'D', '2'] + ['=' + p for p in NOFOES] + ['D'] * i + ['2', '2', 'U', '2', '2']
        for k in range(4):
            name = 'cspell_%s_%d' % (sp.lower(), k)
            FIGHTS[name] = ('p15', MAGIC + ['C957:02'], 72 + k, 20)
            AFTER[name] = STRONG + ['C0B6:%02X' % (k * 41 + i & 0xFF), 'C0B7:%02X' % (k * 113 + i & 0xFF)]
            STEER[name] = '2D2' + 'D' * i + '2' + '2' + 'U2' + '2'
    for name, (who, pokes, seed, n) in FIGHTS.items():
        rnd = random.Random(name)
        x, y, d, moves, rec = PEOPLE[who]
        s = Script()
        for p in ['C953:%02X' % x, 'C985:%02X' % x, 'C954:%02X' % y, 'C986:%02X' % y] + pokes:
            if p.startswith('person+'):    # the townsperson becomes a monster
                off, val = p[7:].split(':')
                p = '%04X:%s' % (0xC651 + rec * 8 + int(off, 16), val)
            s.poke(p)
        s.f += 10
        s.press(moves[0], 30)
        s.press(moves[1], 50)
        for k in '2D2':                # command menu, Attack
            s.press(k, 30)
        s.press(d, 30)
        s.press('2', 30)
        s.f += 200                     # the battlefield is set up
        for p in AFTER.get(name, []):
            s.poke(p)
        for k in STEER.get(name, ''):
            if k[0] == '=':
                s.poke(k[1:])
            else:
                s.press(k, 40)
        for _ in range(n):
            r = rnd.random()
            k = rnd.choice('UDLR') if r < 0.45 else '2' if r < 0.9 else '1'
            s.press(k, rnd.choice((20, 30, 40, 60)))
        for _ in range(4):
            s.press('1', 60)
        s.write(name)
    # dungeons: leave Moonglow to just south of one (the town's way out is
    # set by RAM) and step in
    DUNGEON_DOOR = {'deceit': (0xF0, 0x4A), 'despise': (0x5B, 0x44), 'destard': (0x48, 0xA9),
                    'wrong': (0x7E, 0x15), 'covetous': (0x9C, 0x1C), 'shame': (0x3A, 0x67)}
    DUNGEON_SPELLS = 'ACDEHLORVXYZ'          # the spell list in dungeons
    DUNGEONS = {}
    for i, d in enumerate(DUNGEON_DOOR):
        DUNGEONS['dungeon_' + d] = (d, ['C95B:FF', 'C913:05'], '', 150)
        DUNGEONS['dungeon_dark_' + d] = (d, ['C913:00'], '', 60)
    for sp in 'DELXYZ':
        DUNGEONS['dspell_' + sp.lower()] = ('despise', MAGIC, '2DD222' + 'D' * DUNGEON_SPELLS.index(sp) + '2U2', 20)
    DUNGEONS['dungeon_climb'] = ('despise', ['C95B:FF'], 'L', 20)          # the ladder up, next to the way in
    DUNGEONS['dungeon_climb_abyss'] = ('despise', ['C95B:FF', 'C96D:EF', 'C96E:F0'], 'L', 20)   # out where the Abyss is
    DUNGEONS['dspell_y_deep'] = ('despise', MAGIC + ['C970:03'], '2DD222' + 'D' * DUNGEON_SPELLS.index('Y') + '2U2', 20)
    DUNGEONS['dspell_z_deep'] = ('despise', MAGIC + ['C970:03'], '2DD222' + 'D' * DUNGEON_SPELLS.index('Z') + '2U2', 20)
    DUNGEONS['dspell_z_bottom'] = ('despise', MAGIC + ['C970:07'], '2DD222' + 'D' * DUNGEON_SPELLS.index('Z') + '2U2', 20)
    DUNGEONS['dungeon_room'] = ('despise', ['C95B:FF'], 'RRDRR', 60)       # through a door into a room
    DUNGEONS['dungeon_attack'] = ('despise', ['C95B:FF'], '2D2D2' + '2D2R2', 30)   # at nothing, twice
    DUNGEONS['dungeon_fight'] = ('despise', ['C95B:FF', 'C652:04', 'C653:02', 'C654:90'], '2D2R2', 80)   # an orc east of the party
    DUNGEONS['dungeon_fight_next'] = ('despise', ['C95B:FF', 'C652:03', 'C653:02', 'C654:90', 'C651:90'], '2D2R2', 80)
    DUNGEONS['dungeon_karma'] = ('despise', ['C95B:FF'], 'a', 20)          # controller 2's button 1
    DUNGEONS['dungeon_help'] = ('despise', ['C95B:FF'], 'bDDDUUUUD2', 20)  # its button 2: the pages
    DUNGEONS['dungeon_help_all'] = ('despise', ['C95B:FF'], 'b' + 'D' * 11 + 'U' * 11 + '1', 10)
    DUNGEONS['dungeon_door'] = ('despise', ['C95B:FF', 'D023:C0'], 'RR', 20)        # a door east, nothing behind it
    DUNGEONS['dungeon_chest'] = ('despise', ['C95B:FF', 'D023:40'], 'R' + '2DDD2' + '2', 20)
    DUNGEONS['dungeon_chest_none'] = ('despise', ['C95B:FF'], '2DDD2' + '2', 20)
    DUNGEONS['dungeon_chest_deep'] = ('despise', ['C95B:FF', 'D023:40', 'C9A0:14'], 'R' + '2DDD2' + '2', 20)
    DUNGEONS['dungeon_fight_abyss'] = ('despise', ['C95B:FF', 'C652:04', 'C653:02', 'C654:90', 'C9A0:19'], '2D2R2', 80)
    # the last room of a level: told where it stands, by the party's column
    DUNGEONS['dungeon_room_f_0'] = ('despise', ['C95B:FF', 'C953:00', 'D021:DF'], 'R', 40)
    DUNGEONS['dungeon_room_f_1'] = ('despise', ['C95B:FF', 'D023:DF'], 'R', 40)
    DUNGEONS['dungeon_room_f_2'] = ('despise', ['C95B:FF', 'C953:0A', 'D02B:DF'], 'R', 40)
    DUNGEONS['dungeon_room_cell'] = ('despise', ['C95B:FF', 'D023:D1'], 'R', 20)    # a room's cell east, no door
    # the room's foes gone: the party walks out the way it came
    DUNGEONS['dungeon_room_exit'] = ('despise', ['C95B:FF'], 'RRDRR', 0)
    DUNGEON_AFTER = {'dungeon_room_exit': (100, NOFOES, 'L' * 12)}
    # out by each side, walking off the field, or by a room's exit to another place
    for way, key in (('up', 'U'), ('right', 'R'), ('down', 'D'), ('left', 'L')):
        DUNGEONS['dungeon_room_side_' + way] = ('despise', ['C95B:FF'], 'RRDRR', 0)
        at = {'U': ['C8B9:05', 'C8BA:00'], 'D': ['C8B9:05', 'C8BA:0A']}.get(key, [])   # walls: start at the edge
        DUNGEON_AFTER['dungeon_room_side_' + way] = (100, NOFOES + at, key * 12)
        for room in (0, 1, 2):            # the table has three
            name = 'dungeon_room_way_%s_%d' % (way, room)
            DUNGEONS[name] = ('despise', ['C95B:FF'], 'RRDRR', 0)
            DUNGEON_AFTER[name] = (100, NOFOES + at + ['C984:%02X' % room], key * 12)
    for k in (0, 2, 6, 7):             # the last altar: wrong colours, the end of the game, cancelled
        DUNGEONS['abyss_altar_%d' % k] = ('despise', ['C95B:FF', 'C9A0:19', 'C990:B0', 'C970:07', 'D722:B0', 'C955:80'],
                                          '2DDDD2D2' + '-' * 3 + 'D' * k + '2' + '-' * 3 + '2', 10)
    # a lower altar (level 7): a wrong colour, and the right one (then the
    # one stone: a ladder down)
    for name, k in (('abyss_low_0', 0), ('abyss_low_2', 2)):
        DUNGEONS[name] = ('despise', ['C95B:FF', 'C9A0:19', 'C990:B0', 'C970:06', 'D622:B0', 'C955:40'],
                          '2DDDD2D2' + '-' * 3 + 'D' * k + '2' + '-' * 3 + '2', 10)
    DUNGEONS['abyss_altar_cancel'] = ('despise', ['C95B:FF', 'C9A0:19', 'C990:B0', 'C970:02', 'D222:B0', 'C955:FF'],
                                      '2DDDD2D2' + '-' * 3 + '1', 10)
    DUNGEONS['dungeon_stone'] = ('despise', ['C95B:FF', 'C955:01'], '2DDDD2D2' + '-' * 3 + '2', 10)
    DUNGEONS['dungeon_gem'] = ('despise', ['C95B:FF', 'C914:02'], '2DDDD22D2', 10)   # the gem's view of the level
    SEARCH6 = '2DDDDDD2'               # the command menu, Search
    DUNGEONS['dsearch_nothing'] = ('despise', ['C95B:FF'], SEARCH6, 10)
    for cell, what in ((0x70, 'orb'), (0x90, 'fountain0'), (0x91, 'fountain1'), (0x92, 'fountain2'),
                       (0x93, 'fountain3'), (0x94, 'fountain4'), (0xB0, 'altar')):
        # the cell under the party becomes one, and the party searches
        DUNGEONS['dsearch_' + what] = ('despise', ['C95B:FF', 'C751:20', 'C76D:50' if cell == 0x93 else 'C751:20',
                                                    'D022:%02X' % cell, 'C990:%02X' % cell], SEARCH6 + '2', 10)
    for cell, what in ((0x80, 'winds'), (0x84, 'rocks'), (0x88, 'pit'), (0xA0, 'poison'), (0xA2, 'fire'), (0xA3, 'sleep')):
        DUNGEONS['dtrap_' + what] = ('despise', ['C95B:FF', 'D023:%02X' % cell], 'R', 20)   # the cell east of the party
    DUNGEONS['dspell_y_top'] = ('wrong', MAGIC, '2DD222' + 'D' * DUNGEON_SPELLS.index('Y') + '2', 20)
    # a trigger of the room ($C8E1) stepped on, by one place or the other of
    # its places in the table; two members out by different sides
    for i, plate in enumerate(((6, 5), (5, 6))):
        for targets in ((0, 0, 0, 0), (3, 3, 0, 0), (3, 3, 4, 4)):
            name = 'dungeon_room_plate_%d_%d' % (i, sum(1 for t in targets if t) // 2)
            DUNGEONS[name] = ('despise', ['C95B:FF'], 'RRDRR', 0)
            DUNGEON_AFTER[name] = (100, ['C8B9:05', 'C8BA:05', 'C8E1:44'] +
                                   ['%04X:%02X' % (0xC8E2 + k, v) for k, v in enumerate(plate + targets)], 'R2222R')
    DUNGEONS['dungeon_room_two_ways'] = ('despise', ['C95B:FF', 'C957:02'], 'RRDRR', 0)
    DUNGEON_AFTER['dungeon_room_two_ways'] = (100, NOFOES + ['C8B9:00', 'C8BA:05', 'C8BE:0A', 'C8BF:05'], 'LRLRLRLR')
    # an altar room's altar ($6662): in a room of the Abyss's kind ($C984 the room),
    # the member at the middle, Use, Stone, four stones: the order the room wants,
    # another, one twice
    ROM = open(os.path.join(HERE, '..', 'build', 'u4.sms'), 'rb').read()
    for room in range(3):
        want = ROM[0x66F1 + room]
        rows = [i for i in range(8) if want & 1 << i]
        wrong = [i for i in range(8) if not want & 1 << i][:4]
        for what, picks in (('right', rows), ('wrong', wrong), ('twice', rows[:1] * 2)):
            name = 'altar_room_%d_%s' % (room, what)
            DUNGEONS[name] = ('despise', ['C95B:FF'], 'RRDRR', 0)
            keys = '2DDDD2D2---'
            at = 0
            for r in picks:              # the cursor stays where it was
                keys += 'D' * (r - at) + '2--'
                at = r
            DUNGEON_AFTER[name] = (300, NOFOES[1:] + ['C984:%02X' % room, 'C8B9:05', 'C8BA:05', 'C955:FF'], keys)
    # the right stones, the last three taken as read: the first stone's bit is 1
    DUNGEONS['altar_room_key'] = ('despise', ['C95B:FF'], 'RRDRR', 0)
    DUNGEON_AFTER['altar_room_key'] = (300, NOFOES[1:] + ['C984:00', 'C8B9:05', 'C8BA:05', 'C955:FF', 'C951:00'],
                                       list('2DDDD2D2---') + ['=C316:01', '=C317:%02X' % (ROM[0x66F1] & 0xFE), '2', '-', '-', '2'])
    for name, (d, pokes, steer, n) in DUNGEONS.items():
        rnd = random.Random(name)
        x, y = DUNGEON_DOOR[d]
        s = Script()
        s.poke('C96D:%02X' % x)
        s.poke('C96E:%02X' % y)
        s.press('L', 40)
        s.press('L', 70)
        s.press('U', 400)              # in: the level is drawn
        for p in pokes:
            s.poke(p)
        s.f += 60
        for k in steer:
            s.press(k, 40)
        if name in DUNGEON_AFTER:
            wait, pokes, keys = DUNGEON_AFTER[name]
            s.f += wait
            for p in pokes:
                s.poke(p)
            for k in keys:
                if k[0] == '=':
                    s.poke(k[1:])
                else:
                    s.press(k, 40)
        for _ in range(n):
            r = rnd.random()
            k = rnd.choice('UDLR') if r < 0.6 else '2' if r < 0.9 else '1'
            s.press(k, rnd.choice((20, 30, 40, 60)))
        for _ in range(4):
            s.press('1', 60)
        s.write(name)
    # a fight out in the world: an orc set east of the party, attacked
    for name, steps in (('world_fight', 200), ('world_fight_2', 300)):
        rnd = random.Random(name)
        s = Script()
        s.poke('C96D:5B')
        s.poke('C96E:44')
        s.press('L', 40)
        s.press('L', 70)
        s.f += 300
        for p in ('C652:5C', 'C653:44', 'C654:90', 'C655:90'):
            s.poke(p)
        s.f += 30
        for k in '2D2R2':
            s.press(k, 40)
        s.f += 200
        for _ in range(steps):
            r = rnd.random()
            k = rnd.choice('UDLR') if r < 0.3 else '2' if r < 0.9 else '1'
            s.press(k, rnd.choice((20, 30, 40, 60)))
        for _ in range(4):
            s.press('1', 60)
        s.write(name)
    # the moongate of the moons new ($A6, $13, both moons kept at 4), from each side
    NEW = ['C95D:04', 'C95E:80', 'C95F:04', 'C960:80', 'C98C:43', 'C98D:A6', 'C98E:13']
    for key, x, y in (('R', 0xA5, 0x13), ('L', 0xA7, 0x13), ('D', 0xA6, 0x12), ('U', 0xA6, 0x14)):
        s = Script()
        s.poke('C96D:%02X' % x)
        s.poke('C96E:%02X' % y)
        s.press('L', 40)
        s.press('L', 70)
        s.f += 300
        for i in range(60):            # (poked all through the step: it moves the moons)
            for p in NEW:
                s.poke(p)
            if i == 14:
                s.lines.append('%d %s' % (s.f, key))
            if i == 20:
                s.lines.append('%d -' % s.f)
            s.f += 1
        s.f += 300
        for k in 'UDLR2122':
            s.press(k, 60)
        s.write('moongate_new_' + key)
    # Moonglow's moongate ($E0, $85), stepped onto from the west while open
    for name, moons in (('moongate', []),
                        ('moongate_new', ['C95D:04', 'C95E:80', 'C95F:04', 'C960:80', 'C98D:E0', 'C98E:85'])):
        rnd = random.Random(name)
        s = Script()
        s.poke('C96D:DF')   # west of the gate of the moon's phase
        s.poke('C96E:85')
        s.press('L', 40)
        s.press('L', 70)
        s.f += 300
        for p in ['C98C:43'] + moons:
            s.poke(p)
        if moons:                      # kept so until the step: the moons move
            for _ in range(12):
                s.f += 1
                for p in moons:
                    s.poke(p)
        s.press('R', 300)
        for _ in range(40):
            r = rnd.random()
            k = rnd.choice('UDLR') if r < 0.6 else '2' if r < 0.9 else '1'
            s.press(k, rnd.choice((20, 30, 40, 60)))
        s.write(name)
    # Castle Britannia's ladders, tried in Moonglow (the party starts at (1, 15)):
    # the cell to the east ($D3C4) made a ladder, then a step onto it; the
    # way down from row 2 goes into the dungeon below
    LADDERS = {
        'ladder_up':     ['D3C4:1B'],
        'ladder_down':   ['D3C4:1C'],
        'ladder_depths': ['C954:02', 'C986:02', 'D084:1C'],
    }
    for name, pokes in LADDERS.items():
        rnd = random.Random(name)
        s = Script()
        for p in pokes:
            s.poke(p)
            s.f += 120
        s.press('R', 600)
        for _ in range(30):
            r = rnd.random()
            k = rnd.choice('UDLR') if r < 0.7 else '2' if r < 0.9 else '1'
            s.press(k, rnd.choice((20, 30, 40, 60)))
        s.write(name)
    # a chest under the party (at (1, 15) in Moonglow): one of the people
    # (the last record), or the town's own cell ($D3C2: it costs karma); the
    # random numbers steered, for the traps; dexterity 0 so that few are evaded
    for k in range(48):
        name = 'chest_%X' % k
        rnd = random.Random(name)
        s = Script()
        s.poke('C758:00')
        if k & 8:
            s.poke('D3C2:3C')
        else:
            for p in ('C749:3C', 'C74A:01', 'C74B:0F', 'C74C:3C'):
                s.poke(p)
        s.press('2', 40)               # the command menu, Get Chest
        for _ in range(3):
            s.press('D', 30)
        s.press('2', 60)
        s.poke('C0B6:%02X' % (k * 41 & 0xFF))
        s.poke('C0B7:%02X' % (k * 113 & 0xFF))
        s.press('2', 400)              # the first member
        for _ in range(12):
            r = rnd.random()
            s.press('2' if r < 0.6 else rnd.choice('UDLR'), rnd.choice((20, 30, 60)))
        s.write(name)
    # the shops Moonglow hasn't, tried there: the party put below an open cell
    # made a counter ($60) on the shop's row, a person (the last record, tile
    # $52) behind it, and the location poked for the table at $96C1
    OTHER_SHOPS = {                    # location, the counter's row
        'weapons': (0x07, 0x03), 'weapons2': (0x08, 0x09),
        'armour': (0x07, 0x07), 'armour2': (0x0F, 0x11),
        'pub': (0x07, 0x02), 'pub2': (0x0F, 0x19),
        'guild': (0x10, 0x1A), 'guild2': (0x0F, 0x07),
        'stable': (0x0E, 0x18), 'seer': (0x01, 0x19),
    }
    X = 0x08
    for shop, (loc, row) in OTHER_SHOPS.items():
        for seed in range(9):
            name = 'shop_%s_%d' % (shop, seed)
            rnd = random.Random(name)
            s = Script()
            cell = 0xD000 + row * 64 + 2 * X
            for p in ['C9A0:%02X' % loc, 'C953:%02X' % X, 'C985:%02X' % X,
                      'C954:%02X' % (row + 1), 'C986:%02X' % (row + 1),
                      '%04X:60' % cell, 'C749:52', 'C74A:%02X' % X,
                      'C74B:%02X' % (row - 1), 'C74C:52'] + RICH:
                s.poke(p)
                s.f += 2
            if seed:                   # things to sell
                for a in range(0xC91F, 0xC92F):
                    s.poke('%04X:05' % a)
                for a in range(0xC917, 0xC91F):
                    s.poke('%04X:05' % a)
                s.f += 2
            if seed >= 6:              # the words a pub's keeper can be asked about
                for a in (0xC535, 0xC51F, 0xC52B, 0xC526, 0xC51A, 0xC529):
                    s.poke('%04X:01' % a)
                s.f += 2
            s.f += 60
            for k in '22U2':
                s.press(k, 60)
            s.f += 300
            for _ in range(70):
                r = rnd.random()
                k = 'D' if r < 0.3 else 'U' if r < 0.45 else '2' if r < 0.95 else '1'
                s.press(k, rnd.choice((30, 60, 120)))
            s.write(name)
    # the shops worked by hand: selling some, all, more than there are; the
    # pub's food, with money and without, and its grog, a tip, the keeper's words
    def at_counter(shop, pokes):
        loc, row = OTHER_SHOPS[shop]
        s = Script()
        for p in ['C9A0:%02X' % loc, 'C953:%02X' % X, 'C985:%02X' % X,
                  'C954:%02X' % (row + 1), 'C986:%02X' % (row + 1),
                  '%04X:60' % (0xD000 + row * 64 + 2 * X), 'C749:52', 'C74A:%02X' % X,
                  'C74B:%02X' % (row - 1), 'C74C:52'] + pokes:
            s.poke(p)
            s.f += 2
        s.f += 60
        for k in '22U2':
            s.press(k, 60)
        s.f += 300
        return s
    OWNED = ['%04X:05' % a for a in list(range(0xC91F, 0xC92F)) + list(range(0xC917, 0xC91F))]
    for shop in ('weapons', 'weapons2', 'armour', 'armour2'):
        for row in (0, 3):
            for count in (0, 1, 2, 5, 6):
                s = at_counter(shop, RICH + OWNED)
                for k in ['2', 'D', '2', 'D'] [:3] + ['D'] * row + ['2'] + ['U'] * count + ['2', '2', '2', '2', 'D', '2', '2', '1', '1']:
                    s.press(k, 120)
                s.f += 300
                s.write('sell_%s_%d_%d' % (shop, row, count))
    WORDS = ['%04X:01' % a for a in (0xC535, 0xC51F, 0xC52B, 0xC526, 0xC51A, 0xC529)]
    PUBS = {
        'pub_food':      (RICH, '2' + '2' + 'U' + '2'),
        'pub_food_some': (['C909:30', 'C90A:00'], '2' + '2' + 'UUUU' + 'R' + 'U' + '2'),
        'pub_food_none': (POOR, '2' + '2' + 'U' + '2'),
        'pub_grog_poor': (POOR, '2' + 'D2' + 'UUU' + '2'),
        'pub_grog_1':    (RICH, '2' + 'D2' + 'U' + '2'),
        'pub_grog_2':    (RICH, '2' + 'D2' + 'UU' + '2'),
        'pub_grog_3':    (RICH, '2' + 'D2' + 'UUU' + '2' + '2222'),
        'pub_grog_9':    (RICH, '2' + 'D2' + 'U' * 9 + '2' + '2222'),
        'pub_tip_more':  (['C909:05', 'C90A:00'] + WORDS, '2' + 'D2' + 'UUU' + '2' + '2' + 'UUUU' + '2' + '22'),
        'pub_tips':      (RICH, '2' + ('D2' + 'U' + '2' + '2') * 6 + '22'),
    }
    for word in range(6):
        PUBS['pub_words_%d' % word] = (RICH + WORDS, '2' + 'D2' + 'UUU' + '2' + '2' + 'D' * word + '2' + '2222')
    for name, (pokes, keys) in PUBS.items():
        s = at_counter('pub', pokes)
        for k in keys:
            s.press(k, 120)
        s.f += 400
        s.write(name)
    # the pause button, in the world, a town and a fight
    for name, leave in (('pause_town', False), ('pause_world', True)):
        s = Script()
        if leave:
            s.press('L', 40)
            s.press('L', 70)
        s.f += 100
        s.press('P', 60)
        s.f += 200
        s.press('U', 40)
        s.press('P', 60)
        s.press('P', 60)
        s.press('P', 60)
        for k in 'RLRL':
            s.press(k, 40)
        s.write(name)
    # every inn ($96C1, $97B5: the place, its counter's row), the nights the
    # random numbers make of them; inn 3 has a choice of rooms
    ROM2 = open(os.path.join(HERE, '..', 'build', 'u4.sms'), 'rb').read()
    for place in range(16):
        number = ROM2[0x8000 + 0x1755 + 6 * 16 + place] - 1
        if number < 0:
            continue
        loc, row = place + 2, ROM2[0x8000 + 0x16C1 + place * 8 + 6]
        for seed in range(10):
            name = 'inn_%d_%d' % (number, seed)
            s = Script()
            cell = 0xD000 + row * 64 + 2 * X
            for p in ['C9A0:%02X' % loc, 'C953:%02X' % X, 'C985:%02X' % X,
                      'C954:%02X' % (row + 1), 'C986:%02X' % (row + 1),
                      '%04X:60' % cell, 'C749:52', 'C74A:%02X' % X,
                      'C74B:%02X' % (row - 1), 'C74C:52'] + RICH:
                s.poke(p)
                s.f += 2
            s.f += 60
            for k in '22U2':
                s.press(k, 60)
            s.f += 300
            s.poke('C0B6:%02X' % (seed * 41 & 0xFF))
            s.poke('C0B7:%02X' % (seed * 113 & 0xFF))
            s.press('2', 200)          # the welcome
            s.press('2', 100)          # a room: yes
            s.press('2', 200)
            if number == 3:
                for _ in range(seed % 3):
                    s.press('D', 40)
            s.press('2', 200)
            s.press('2', 200)
            for _ in range(12):
                s.press('2', 150)
            s.f += 400
            s.write(name)
    # a ship's cannon: what stands in the line of fire ($8586), 1 to 4 cells on
    for record in (0, 12):
        at = 0xC651 + 8 * record
        for cells in (1, 2, 3, 4):
            for figure in (0x90, 0x8C, 0x18):
                s = Script()
                s.poke('C96D:9F')
                s.poke('C96E:16')
                s.press('L', 40)
                s.press('L', 70)
                s.f += 200
                s.press('1', 8)
                s.poke('C959:10')
                s.poke('C95A:00')
                for a, v in ((at, figure), (at + 1, 0x9F), (at + 2, 0x16 - cells), (at + 3, figure)):
                    s.poke('%04X:%02X' % (a, v))
                s.f += 100
                for k in '2D2U2':
                    s.press(k, 40)
                s.f += 300
                s.write('cannon_%d_%d_%02X' % (record, cells, figure))
    # Lord British, as a person poked next to the party (at (2, 15) in Moonglow)
    BRITISH = {
        'lb_first':   [],
        'lb_greeted': ['C963:FF'],
        'lb_dead':    ['C963:FF', 'C76D:44'],
        'lb_two':     ['C963:FF', 'C957:02'],
        'lb_party':   ['C963:FF', 'C957:03'],
        'lb_levels':  ['C963:FF', 'C957:03', 'C755:10', 'C756:27', 'C772:F4', 'C773:01', 'C766:30'],
        'lb_topics':  ['C963:FF', 'C5E0:01', 'C5E1:02', 'C5E2:03', 'C5E3:04', 'C5E4:05', 'C5E5:06',
                       'C5E6:07', 'C5E7:08', 'C5E8:09', 'C5E9:0A', 'C5EA:0B', 'C5EB:0C', 'C5EC:0D'],
        'lb_advice':  ['C963:FF', 'C5E0:06', 'C5E1:05', 'C5E2:06', 'C8FD:FF', 'C8FE:FF', 'C957:03', 'C956:FF'],
        'lb_advice2': ['C963:FF', 'C5E0:06', 'C5E1:06', 'C8FD:FF', 'C8FE:FF', 'C957:03', 'C956:FF', 'C90B:00',
                       'C955:C0'] + ['C9%02X:00' % a for a in range(0x0C, 0x13)] + ['C951:1C'],
    }
    for name, pokes in BRITISH.items():
        rnd = random.Random(name)
        s = Script()
        for p in ['C749:5E', 'C74A:02', 'C74B:0F', 'C74C:5E', 'C750:01'] + pokes:
            s.poke(p)
            s.f += 1
        s.f += 40
        for k in '22R2':
            s.press(k, 60)
        s.f += 500
        for _ in range(90):
            r = rnd.random()
            k = 'D' if r < 0.3 else 'U' if r < 0.4 else '2' if r < 0.93 else '1'
            s.press(k, rnd.choice((30, 60, 120)))
        s.write(name)
    # the Abyss's isle: stepping east onto ($DC, $E0) puts its people there
    rnd = random.Random('abyss_isle')
    s = Script()
    s.poke('C96D:DB')
    s.poke('C96E:E0')
    s.press('L', 40)
    s.press('L', 70)
    s.f += 300
    s.poke('C959:12')              # by ship, facing east
    s.poke('C95A:00')
    s.press('R', 200)
    for _ in range(20):
        s.press(rnd.choice('UDLR12'), rnd.choice((30, 60)))
    s.write('abyss_isle')
    # a bridge under the party on the world: one pass in eight, trolls
    for k in range(8):
        name = 'trolls_%d' % k
        rnd = random.Random(name)
        s = Script()
        for key in 'LLLLLL':
            s.press(key, 30)
        for _ in range(6):
            s.poke('C990:17')
            s.poke('C0B5:%02X' % (k * 33 & 0xFF))
            s.press('1', 60)
        for _ in range(16):
            s.press(rnd.choice('UDLR12'), rnd.choice((30, 60, 120)))
        s.write(name)
    # other hardware: a Japanese console ($C005 = 0: the logo first, a press or
    # none), the FM sound unit ($C002, the sound driver's other paths)
    for name, pokes, press in (('logo_jp', ['C005:00'], 300), ('logo_jp_wait', ['C005:00'], None),
                               ('fm_sound', ['C002:01'], None), ('fm_rhythm', ['C002:04'], None), ('fm_detected', ['C002:FF'], None)):
        lines = ['%d =%s' % (f, p) for f in range(1, 9) for p in pokes]
        if press:
            lines += ['%d 2' % press, '%d -' % (press + 6)]
        lines += START if name.startswith('fm') else []
        last = 1500 if name.startswith('logo') else T0 + 3000
        rnd = random.Random(name)
        if name.startswith('fm'):
            f = T0
            for _ in range(60):
                lines += ['%d %s' % (f, rnd.choice('UDLR2221')), '%d -' % (f + 6)]
                f += 40
        lines.append('%d -' % last)
        with open(os.path.join(HERE, 'cases', name + '.keys'), 'w', newline=chr(10)) as out:
            out.write(chr(10).join(lines) + chr(10))
    # joining: the first person (the record's +7) of location 7, whose town is
    # the virtue of class 1, poked next to the party
    JOINS = {
        'join_ok':        ['C90C:63', 'C75B:08'],
        'join_won':       ['C90C:00', 'C75B:08'],
        'join_low_karma': ['C90C:10', 'C75B:08'],
        'join_level':     ['C90C:63', 'C75B:01'],
        'join_own_town':  ['C90C:63', 'C75B:08', 'C76C:01'],
        'join_not_first': ['C90C:63', 'C75B:08', 'C750:02'],
    }
    for name, pokes in JOINS.items():
        s = Script()
        for p in ['C9A0:07', 'C749:24', 'C74A:02', 'C74B:0F', 'C74C:24', 'C750:01'] + pokes:
            s.poke(p)
            s.f += 1
        s.f += 40
        for k in '22R2':
            s.press(k, 60)
        s.f += 300
        s.press('2', 120)              # past the greeting
        for _ in range(5):             # JOIN is the sixth word
            s.press('D', 40)
        s.press('2', 400)
        for _ in range(3):
            s.press('2', 200)
        s.write(name)
    # a person's question: the JOB topic made the one that leads to it ($C9A9),
    # the answer either way, karma or none ($C9AA); kinds of person ($C309, the
    # task's IX+9) that then offer eight words, the right one or a wrong one;
    # the keyword topic 5 of kind $AC
    ASKS = {
        'ask_yes':       ('', [], '2', ''),
        'ask_no':        ('', [], 'D2', ''),
        'ask_yes_karma': ('', ['C9AA:01', 'C96B:FF', 'C96C:FF'], '2', ''),
        'ask_no_karma':  ('', ['C9AA:01'], 'D2', ''),
        'ask_again':     ('', ['C9AA:01', 'C96B:00', 'C96C:00'], '2', ''),
        'ask_n2':        ('', [], 'D2', ''),
        'ask_n2_karma':  ('', ['C9AA:01', 'C96B:FF', 'C96C:FF'], 'D2', ''),
        'ask_n2_again':  ('', ['C9AA:01', 'C96B:00', 'C96C:00'], 'D2', ''),
        'ask_n2_word':   ('', ['C309:18'], 'D2', 'DD2'),
        'ask_yes_r_karma': ('', ['C9AA:01', 'C96B:FF', 'C96C:FF'], 'R2', ''),
        'ask_yes_r_again': ('', ['C9AA:01', 'C96B:00', 'C96C:00'], 'R2', ''),
    }
    for i, kind in enumerate((0x18, 0x8D, 0x99, 0xA3, 0xCE, 0xD7)):
        for pick in (0, 3):
            ASKS['ask_word_%X_%d' % (i, pick)] = ('', ['C309:%02X' % kind], '2', 'D' * pick + '2')
            ASKS['ask_word_no_%X_%d' % (i, pick)] = ('', ['C309:%02X' % kind], 'D2', 'D' * pick + '2')
    for name, (_, pokes, answer, word) in ASKS.items():
        s = Script()
        for p in ['C9A0:07', 'C749:24', 'C74A:02', 'C74B:0F', 'C74C:24', 'C750:03']:
            s.poke(p)
            s.f += 1
        s.f += 40
        for k in '22R2':
            s.press(k, 60)
        s.f += 300
        for p in ['C9A9:03', 'C9AB:00'] + pokes:
            s.poke(p)
            s.f += 1
        s.press('2', 120)              # past the greeting
        for _ in range(3):             # JOB is the fourth word
            s.press('D', 40)
        s.press('2', 500)              # its answer
        if not name.startswith('ask_n2'):
            s.press('2', 300)          # the question
        for p in pokes:                # (the record may have set them by now)
            s.poke(p)
        for k in answer:
            s.press(k, 40)
        s.f += 300
        s.press('2', 400)              # what they say to that
        for k in word:
            s.press(k, 40)
        s.f += 300
        for _ in range(4):
            s.press('2', 200)
        s.write(name)
    # the list of words moved past its end and back; a question's other answer
    # where it counts; a beggar and someone else given money, without the money
    s = Script()
    for p in ['C9A0:07', 'C749:24', 'C74A:02', 'C74B:0F', 'C74C:24', 'C750:03']:
        s.poke(p)
        s.f += 1
    s.f += 40
    for k in '22R2':
        s.press(k, 60)
    s.f += 300
    s.press('2', 120)
    for k in 'D' * 14 + 'U' * 14:
        s.press(k, 40)
    s.write('talk_scroll')
    for name, kind, gold in (('give_beggar_poor', 0x58, 0), ('give_other', 0x24, 0x10), ('give_other_poor', 0x24, 0)):
        s = Script()
        for p in ['C9A0:07', 'C749:%02X' % kind, 'C74A:02', 'C74B:0F', 'C74C:%02X' % kind, 'C750:03',
                  'C909:%02X' % gold, 'C90A:00']:
            s.poke(p)
            s.f += 1
        s.f += 40
        for k in '22R2':
            s.press(k, 60)
        s.f += 300
        for k in '2' + 'D' * 6 + '2':  # GIVE
            s.press(k, 60)
        s.f += 300
        for k in 'UU2':
            s.press(k, 60)
        s.f += 300
        for _ in range(4):
            s.press('2', 200)
        s.write(name)
    # the keyword topic of the person who asks a name ($AC): a wrong one, the right one
    for name, right in (('ask_name_wrong', False), ('ask_name_right', True), ('ask_name_back', False)):
        s = Script()
        for p in ['C9A0:07', 'C749:24', 'C74A:02', 'C74B:0F', 'C74C:24', 'C750:03']:
            s.poke(p)
            s.f += 1
        s.f += 40
        for k in '22R2':
            s.press(k, 60)
        s.f += 300
        for p in ['C309:AC', 'C595:01', 'C9AB:00']:
            s.poke(p)
            s.f += 1
        s.press('2', 120)              # past the greeting
        for _ in range(7):             # the keyword is the eighth word
            s.press('D', 40)
        s.press('2', 500)              # its answer
        s.press('2', 300)              # then the letters
        for k in ['2', 'R', 'R', 'D', '2', 'D', '2']:
            s.press(k, 40)
        if right:
            for i in range(7):
                s.poke('%04X:%02X' % (0xC065 + i, WORD[i]))
        if name == 'ask_name_back':    # letters, and taken back one past the first
            for k in ['1', '2', '2', 'R', '2', '1', '1', '1', '1', 'R', 'L', 'L', 'L', 'L', 'L']:
                s.press(k, 40)
        for k in ['R', 'D', 'D', 'D', '2']:   # End
            s.press(k, 40)
        s.f += 200
        for _ in range(4):
            s.press('2', 200)
        s.write(name)
    # the shrine of honesty (location $1A at $E9, $42): stepped in from the
    # south; the virtue, the cycles, the mantra (AHM) for each cycle
    AHM = ['2', 'R', 'R', 'D', '2', 'D', '2', 'R', 'D', 'D', 'D', '2']   # on the letter grid, then End
    def mantra(times):
        out = []
        for _ in range(times):
            out += ['wait'] + AHM + ['wait']
        return out
    SHRINES = {
        'shrine_no_rune':  (['C956:00'], []),
        'shrine_cancel':   (['C956:FF'], ['1']),
        'shrine_wrong':    (['C956:FF'], ['D', '2', '2']),
        'shrine_none':     (['C956:FF'], ['2', '2']),                      # no cycles
        'shrine_quest':    (['C956:FF', 'C969:FF', 'C96A:FF'], ['2', 'D', '2'] + mantra(1)),
        'shrine_vision':   (['C956:FF', 'C90B:63', 'C969:FF', 'C96A:FF'], ['2', 'D', 'D', 'D', '2'] + mantra(3) + ['wait', '2']),
        'shrine_bad_mantra': (['C956:FF', 'C969:FF', 'C96A:FF'], ['2', 'D', '2', 'wait', 'R', '2', 'R', 'R', 'D', 'D', 'D', 'D', 'D', '2']),
    }
    for name, (pokes, keys) in SHRINES.items():
        rnd = random.Random(name)
        s = Script()
        s.poke('C96D:E9')
        s.poke('C96E:43')
        s.press('L', 40)
        s.press('L', 70)
        for p in pokes:
            s.poke(p)
        s.press('U', 500)              # in: the shrine's picture
        for k in keys:
            if k == 'wait':
                s.f += 700
            else:
                s.press(k, 40)
        s.f += 600
        random_presses(s, rnd, 4)
        s.write(name)
    ROAMS = {                          # long walks out in the world, from different places: the creatures
        'roam_moonglow': (None, [], 300),
        'roam_britain':  ((0x56, 0x6D), [], 300),
        'roam_yew':      ((0x24, 0xE0), [], 300),
        'roam_sea':      ((0x5B, 0x44), ['C959:11', 'C95A:00'], 200),
        'roam_horse':    ((0x9F, 0x16), ['C959:14', 'C95A:00'], 300),
    }
    def full(base, n, figure):         # a full list far away: leaving one must make room
        return ['%04X:%02X' % (base + 8 * i + k, v) for i in range(n)
                for k, v in ((0, figure), (1, 0x10 + i), (2, 0x10), (3, figure))]
    ROAMS['roam_sea_full'] = ((0x5B, 0x44), ['C959:11', 'C95A:00'] + full(0xC699, 5, 0x10), 200)
    ROAMS['roam_horse_full'] = ((0x9F, 0x16), ['C959:14', 'C95A:00'] + full(0xC6C1, 5, 0x14), 300)
    for name, (at, pokes, n) in ROAMS.items():
        rnd = random.Random(name)
        s = Script()
        if at:
            s.poke('C96D:%02X' % at[0])
            s.poke('C96E:%02X' % at[1])
        s.press('L', 40)
        s.press('L', 70)
        for p in pokes:
            s.poke(p)
        for _ in range(n):
            r = rnd.random()
            s.press(rnd.choice('UDLR') if r < 0.9 else '1', rnd.choice((20, 30, 40)))
        for _ in range(6):
            s.press('1', 60)
        s.write(name)
    # every place of the world ($AE60), walked into from the west; the Abyss's
    # kind with and without the parts of the key it wants ($C951, $C952)
    PLACES = open(os.path.join(HERE, '..', 'build', 'u4.sms'), 'rb').read()[0xAE60:0xAEA0]
    def enter_place_case(name, i, pokes):
        x, y = PLACES[2 * i], PLACES[2 * i + 1]
        s = Script()
        s.poke('C96D:%02X' % ((x - 1) & 0xFF))
        s.poke('C96E:%02X' % y)
        s.press('L', 40)
        s.press('L', 70)
        s.f += 200
        for p in pokes:
            s.poke(p)
        s.f += 10
        s.press('R', 500)
        s.f += 600
        s.write(name)
    for i in range(32):
        enter_place_case('place_%02d' % i, i, [])
        enter_place_case('place_%02d_horse' % i, i, ['C959:14', 'C95A:00'])
    for i in (22, 23, 24):
        enter_place_case('place_%02d_key' % i, i, ['C951:1C', 'C952:1C'])
    # what stands under the party, boarded: ships, horses, the balloon
    for figure in (0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x18):
        for last in (0, 1):
            s = Script()
            s.poke('C96D:9F')
            s.poke('C96E:16')
            s.press('L', 40)
            s.press('L', 70)
            s.f += 200
            s.press('1', 8)            # (during a turn: not while the frame reads the lists)
            for a, v in ((0xC699, figure), (0xC69A, 0x9F), (0xC69B, 0x16), (0xC69C, figure)):
                s.poke('%04X:%02X' % (a, v))
            if last:
                s.poke('C973:9F')
                s.poke('C974:16')
            s.f += 100
            s.press('2', 200)
            s.f += 100
            for k in 'RRUU':
                s.press(k, 60)
            s.write('board_%02X_%d' % (figure, last))
    # the balloon: the wind ($C98A) carries it, one step in fifteen tries ($C99E)
    for wind in range(5):
        for high in (0, 1):
            s = Script()
            s.poke('C96D:9F')
            s.poke('C96E:16')
            s.press('L', 40)
            s.press('L', 70)
            s.f += 200
            for p in ['C959:18', 'C95A:00', 'C98A:%02X' % wind, 'C98F:%02X' % high, 'C990:%02X' % (4 - 4 * high)]:
                s.poke(p)
            for _ in range(30):
                s.press('U', 60)
            s.f += 1200
            s.write('balloon_wind_%d_%d' % (wind, high))
    # the special things all owned, on the status pages
    s = Script()
    for p in ['C955:FF', 'C956:FF', 'C951:FF', 'C952:FF']:
        s.poke(p)
    for k in '2' + 'D' * 7 + '2DD2' + 'D' * 7 + 'UU':
        s.press(k, 60)
    s.write('camp_status_items')
    # what's at the party's cell: a whirlpool, a twister; on foot and by ship
    for figure in (0x8C, 0x8E):
        for slot in (0, 3):
            for ship in (0, 1):
                s = Script()
                s.poke('C96D:9F')
                s.poke('C96E:16')
                s.press('L', 40)
                s.press('L', 70)
                s.f += 200
                s.press('1', 8)
                at = 0xC651 + 8 * slot
                for a, v in ((at, figure), (at + 1, 0x9F), (at + 2, 0x16), (at + 3, figure)):
                    s.poke('%04X:%02X' % (a, v))
                if ship:
                    s.poke('C959:10')
                    s.poke('C95A:00')
                s.f += 100
                for _ in range(8):
                    s.press('U', 100)
                s.f += 400
                s.write('taken_%02X_%d_%d' % (figure, slot, ship))
    # sailing, by the wind ($C98A, 4 calm): with it, against it, across
    for wind in range(5):
        for key in 'UDLR':
            s = Script()
            s.poke('C96D:D0')
            s.poke('C96E:E0')
            s.press('L', 40)
            s.press('L', 70)
            s.f += 200
            s.press('1', 8)
            for p in ['C959:10', 'C95A:00']:
                s.poke(p)
            s.f += 100
            for _ in range(4):
                s.poke('C98A:%02X' % wind)
                s.f += 1
                s.press(key, 80)
            s.write('sail_%d_%s' % (wind, key))
    # the lists of the world full of ships and horses near the party, on it,
    # and far off; a town's list full when a horse is left there; ships by the Abyss
    def near(base, figure, at):
        return ['%04X:%02X' % (base + 8 * i + k, v) for i, (x, y) in enumerate(at)
                for k, v in ((0, figure), (1, x), (2, y), (3, figure))]
    ROUND = [(0xD1, 0xE0), (0xCF, 0xE0), (0xD0, 0xE1), (0xD0, 0xDF), (0xD2, 0xE1)]
    ON_IT = [(0xD0, 0xE0)] * 5
    for name, base, figure, at, by in (('near_ships', 0xC699, 0x10, ROUND, 'C959:10'), ('near_ships_on', 0xC699, 0x10, ON_IT, 'C959:10'),
                                       ('near_horses', 0xC6C1, 0x14, ROUND, 'C959:14'), ('near_horses_on', 0xC6C1, 0x14, ON_IT, 'C959:14'),
                                       ('near_both', 0xC699, 0x10, ROUND, 'C959:14')):
        s = Script()
        s.poke('C96D:D0')
        s.poke('C96E:E0')
        s.press('L', 40)
        s.press('L', 70)
        s.f += 200
        s.press('1', 8)
        for p in [by, 'C95A:00'] + near(base, figure, at) + (near(0xC6C1, 0x14, ROUND) if name == 'near_both' else []):
            s.poke(p)
        s.f += 100
        rnd = random.Random(name)
        for _ in range(120):
            s.press(rnd.choice('UDLR'), 40)
        s.write(name)
    FULL = ['%04X:%02X' % (0xC651 + 8 * i + k, v) for i in range(32) for k, v in ((0, 0x20), (1, 0x10 + i), (2, 0x02), (3, 0x20))]
    for name, extra in (('town_horse_full', []),
                        ('town_horse_full_horse', ['C6B4:14']),
                        ('town_horse_full_3c', ['C6B4:3C'])):
        s = Script()
        s.press('1', 8)
        for p in FULL + ['C959:14', 'C95A:00'] + extra:
            s.poke(p)
        s.f += 100
        for k in 'RR1':
            s.press(k, 100)
        s.f += 200
        s.write(name)
    for name, at in (('abyss_isle_ships', (0xE0, 0xE0)), ('abyss_isle_ships_far', (0xE7, 0xE0))):
        s = Script()
        s.poke('C96D:DB')
        s.poke('C96E:E0')
        s.press('L', 40)
        s.press('L', 70)
        s.f += 200
        s.press('1', 8)
        for p in ['C959:12', 'C95A:00'] + near(0xC6B1, 0x10, [at]):
            s.poke(p)
        s.f += 100
        s.press('R', 200)
        for k in 'UDLR':
            s.press(k, 60)
        s.write(name)
    # doors walked into: locked; one open while another still is
    for name, pokes, keys in (('door_locked', ['D3C4:3A'], 'RR'),
                              ('door_two', ['D3C4:3B', 'D3C6:3B'], 'RRRR')):
        s = Script()
        s.press('1', 8)
        for p in pokes:
            s.poke(p)
        s.f += 100
        for k in keys:
            s.press(k, 80)
        s.f += 200
        s.write(name)
    # the seer on a virtue already won ($C90B on), each row
    for row in (0, 3, 7):
        s = at_counter('seer', ['%04X:00' % (0xC90B + i) for i in range(8)])
        for k in ['2', '2'] + ['D'] * row + ['2', '2', '2', '2', '2']:
            s.press(k, 120)
        s.write('seer_won_%d' % row)
    # the seer's kind with the Avatar not able
    s = at_counter('seer', ['C76D:53'])
    for k in '2222':
        s.press(k, 120)
    s.write('seer_asleep')
    # the opening words cut short by a button, then the title
    with open(os.path.join(HERE, 'cases', 'opening_skip.keys'), 'w', newline=chr(10)) as f:
        f.write(chr(10).join(('60 1', '66 -', '900 -')) + chr(10))
    # the title: the demo to its end (no buttons); Journey Onward; Return
    # to the view, then the menu again (up wraps round to the last choice)
    TITLES = {
        'title_demo':    [('9000', '-')],                 # and the interlude after it, and the opening again
        'interlude_skip': [('8100', '1'), ('8700', '-')],
        'title_journey': [('1200', '1'), ('1300', 'D'), ('1330', 'U'), ('1360', '2'), ('2000', '-')],
        'title_view':    [('1200', '1'), ('1300', 'D'), ('1330', 'D'), ('1360', '1'),
                          ('1700', '1'), ('1800', 'U'), ('1830', 'U'), ('1860', '2'), ('2500', '-')],
    }
    # a new game: every key in naming (up and left wrap round, the hand on
    # End, End with no name, a thirteenth letter), she, and on into the
    # story; and the name SOUND (the sound test)
    def create(keys, start=1400, gap=24, end=900):
        out = [('1200', '1'), ('1272', '1')]
        f = start
        for k in keys:
            out.append((str(f), k))
            f += gap
        out.append((str(f + end), '-'))
        return out
    TITLES['create_keys'] = create('UL2' + 'UUR' + 'RL' + 'DRRR2' + '11' + '2' * 13 + 'D' + 'R' * 8 + 'LR' + 'RL' + '2' + 'DUD2')
    TITLES['create_sound'] = create('D' + 'R' * 5 + '2' + 'L' * 4 + '2' + 'R' * 6 + '2' + 'L' * 7 + '2' + 'U' + 'RRR2' + 'U' + 'LLLL2', end=600)
    # the sound test: a song, quiet, the sound's number down past 1 and up,
    # the sound, then Exit
    TITLES['sound_test'] = create('D' + 'R' * 5 + '2' + 'L' * 4 + '2' + 'R' * 6 + '2' + 'L' * 7 + '2' + 'U' + 'RRR2' + 'U' + 'LLLL2'
                                  + '-' * 12 + 'D2' + '-' * 4 + '1UUL' + 'LLR2' + '-' * 4 + 'D2', end=600)
    # every song played through and every sound, as they are and on the FM
    # unit ($C002, poked before the game looks at it: 1 sound, 4 rhythm too)
    INTO_TEST = 'D' + 'R' * 5 + '2' + 'L' * 4 + '2' + 'R' * 6 + '2' + 'L' * 7 + '2' + 'U' + 'RRR2' + 'U' + 'LLLL2' + '-' * 12
    def sound_case(name, unit, keys, last):
        out = [(str(f), '=C002:%02X' % unit) for f in range(1, 9)] if unit else []
        out += create(INTO_TEST, end=0)[:-1]
        f = 1400 + 24 * len(INTO_TEST) + 200
        for k, wait in keys:
            out.append((str(f), k))
            f += wait
        out.append((str(f + last), '-'))
        TITLES[name] = out
    for unit in (0, 1, 4):
        for row in range(1, 10):
            sound_case('song_%d_%d' % (row, unit), unit, [('D', 24)] * (row - 1) + [('2', 3000)], 0)
        sound_case('sounds_%d' % unit, unit, [('D', 24)] * 9 + [('D', 24)] +
                   [('2', 200), ('R', 24)] * 23, 100)
    # the gypsy answered the other way: B every time, and A and B in turn
    # (the start script answers A); the draws differ with the timing
    for name, pick in (('gypsy_b', lambda i: 'D'), ('gypsy_mixed', lambda i: 'DU'[i % 2])):
        keys, i = [], 0
        for line in START:
            f, k = line.split()
            if k == '2' and int(f) >= 5800:
                keys.append((str(int(f) - 30), pick(i)))
                i += 1
            if k != '-':
                keys.append((f, k))
        keys.append(('9000', '-'))
        TITLES[name] = keys
    for name, keys in TITLES.items():
        lines = []
        for f, k in keys:
            lines.append(f + ' ' + k)
            if k != '-':
                lines.append('%d -' % (int(f) + 6))
        with open(os.path.join(HERE, 'cases', name + '.keys'), 'w', newline=chr(10)) as out:
            out.write(chr(10).join(lines) + chr(10))
    # the end of the game: fixed scripts (made by answering each prompt as
    # it comes): won, and each way to fail before the Codex
    import shutil
    for f in sorted(os.listdir(os.path.join(HERE, 'endings'))):
        shutil.copy(os.path.join(HERE, 'endings', f), os.path.join(HERE, 'cases', f))
    for seed in range(2):              # anything at all
        rnd = random.Random('play%d' % seed)
        s = Script()
        random_presses(s, rnd, 150, menu=False)
        s.write('play_%d' % seed)


if __name__ == '__main__':
    main()
