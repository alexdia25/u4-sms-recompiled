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
    SEARCHES = {                       # Search where something is to be found (the table at $8C07)
        'search_town_1':  (False, ['C953:08', 'C985:08', 'C954:06', 'C986:06']),   # Moonglow is 6
        'search_town_2':  (False, ['C9A0:03', 'C953:06', 'C985:06', 'C954:06', 'C986:06']),
        'search_world_1': (True, ['C953:B6', 'C954:36', 'C95D:00', 'C95F:00']),
        'search_world_2': (True, ['C953:E0', 'C954:85']),
        'search_world_3': (True, ['C953:2D', 'C954:AD']),
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
            s.press('2', 40)           # the command menu, Search
            for _ in range(6):
                s.press('D', 30)
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
    ATTACKS = {                        # Attack: where the party stands (or a person's spot), RAM writes, direction
        'attack_person':  ('p15', [], None),
        'attack_guard':   ('p28', [], None),
        'attack_nobody':  (None, [], 'U'),
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
    DUNGEONS['dungeon_room'] = ('despise', ['C95B:FF'], 'RRDRR', 60)       # through a door into a room
    DUNGEONS['dungeon_attack'] = ('despise', ['C95B:FF'], '2D2D2' + '2D2R2', 30)   # at nothing, twice
    DUNGEONS['dungeon_fight'] = ('despise', ['C95B:FF', 'C652:04', 'C653:02', 'C654:90'], '2D2R2', 80)   # an orc east of the party
    DUNGEONS['dungeon_door'] = ('despise', ['C95B:FF', 'D023:C0'], 'RR', 20)        # a door east, nothing behind it
    DUNGEONS['dungeon_room_cell'] = ('despise', ['C95B:FF', 'D023:D1'], 'R', 20)    # a room's cell east, no door
    # the room's foes gone: the party walks out the way it came
    DUNGEONS['dungeon_room_exit'] = ('despise', ['C95B:FF'], 'RRDRR', 0)
    DUNGEON_AFTER = {'dungeon_room_exit': (100, NOFOES, 'L' * 12)}
    SEARCH6 = '2DDDDDD2'               # the command menu, Search
    DUNGEONS['dsearch_nothing'] = ('despise', ['C95B:FF'], SEARCH6, 10)
    for cell, what in ((0x70, 'orb'), (0x90, 'fountain0'), (0x91, 'fountain1'), (0x92, 'fountain2'),
                       (0x93, 'fountain3'), (0x94, 'fountain4'), (0xB0, 'altar')):
        # the cell under the party becomes one, and the party searches
        DUNGEONS['dsearch_' + what] = ('despise', ['C95B:FF', 'C751:20', 'C76D:50' if cell == 0x93 else 'C751:20',
                                                    'D022:%02X' % cell, 'C990:%02X' % cell], SEARCH6 + '2', 10)
    DUNGEONS['dspell_y_top'] = ('wrong', MAGIC, '2DD222' + 'D' * DUNGEON_SPELLS.index('Y') + '2', 20)
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
    ROAMS = {                          # long walks out in the world, from different places: the creatures
        'roam_moonglow': (None, [], 300),
        'roam_britain':  ((0x56, 0x6D), [], 300),
        'roam_yew':      ((0x24, 0xE0), [], 300),
        'roam_sea':      ((0x5B, 0x44), ['C959:11', 'C95A:00'], 200),
        'roam_horse':    ((0x9F, 0x16), ['C959:14', 'C95A:00'], 300),
    }
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
    for seed in range(2):              # anything at all
        rnd = random.Random('play%d' % seed)
        s = Script()
        random_presses(s, rnd, 150, menu=False)
        s.write('play_%d' % seed)


if __name__ == '__main__':
    main()
