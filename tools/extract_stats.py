"""Extract combat numbers into data/stats.json.

    python extract_stats.py <rom> <outdir>

Bank 3 $AC00 is one byte table used by the combat code (combat_strength):
  $00-$23  monsters' strength (max hit points; also their attack), in the
           order of the name table (Pirate ... Balron)
  $24-$33  people and other non-monster combatants
  $34-$43  weapon damage, Hands ... Mystic Sword
  $44-$4B  armour defence, Skin ... Mystic Robe
The values match the PC version's monster hit points, weapon damage and
armour ratings.
"""
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from extract_text import name_table  # noqa: E402


def main():
    rom = open(sys.argv[1], 'rb').read()
    t = rom[3 * 0x4000 + 0x2C00: 3 * 0x4000 + 0x2C00 + 0x4C]
    names = name_table(rom)
    stats = {
        'source': 'bank 3 $AC00 (combat_strength)',
        'monster_strength': [[n, v] for n, v in zip(names['monsters'], t[0x00:0x24])],  # two 'Pirate' entries (ship and crew)
        'other_combatants': list(t[0x24:0x34]),
        'weapon_damage': dict(zip(names['weapons'], t[0x34:0x44])),
        'armour_defence': dict(zip(names['armour'], t[0x44:0x4C])),
    }
    with open(os.path.join(sys.argv[2], 'stats.json'), 'w', newline='\n') as f:
        json.dump(stats, f, indent=1)
    with open(os.path.join(sys.argv[2], 'shops.json'), 'w', newline='') as f:
        json.dump(shops(rom, names), f, indent=1)
    with open(os.path.join(sys.argv[2], 'spells.json'), 'w', newline='') as f:
        json.dump(spells(rom, names), f, indent=1)


SHOP_TYPES = ['weapons', 'armour', 'food', 'pub', 'reagents', 'healer', 'inn', 'guild']


def shops(rom, names):
    """Shops, all in bank 2:
      $9755  shop number (1-based, 0 = none) for each of 8 shop types x 16
             settlements, indexed by place (location - 2; Castle Britannia's
             two floors, locations 1-2, are place 0)
      $9829  weapon shop stock: 4 weapon numbers per shop (0 = empty slot)
      $9841  weapon prices, 16-bit, Hands ... Mystic Sword
      $9B37  armour shop stock: 4 per shop
      $9B4B  armour prices, 16-bit, Skin ... Mystic Robe
      $9ED8  food prices, 16-bit, one per food shop
      $A3B7  reagent prices, 6 bytes per reagent shop
      $9F74  pub specialty plate price, $9F6E tip needed for a rumour
      $A83E  inn rooms: bed x, y and price per inn"""
    b2 = 2 * 0x4000 - 0x8000
    word = lambda a: rom[b2 + a] | rom[b2 + a + 1] << 8
    places = names['places'][:16]
    where = {}
    for t, kind in enumerate(SHOP_TYPES):
        where[kind] = {places[p]: rom[b2 + 0x9755 + 16 * t + p]
                       for p in range(16) if rom[b2 + 0x9755 + 16 * t + p]}
    weapons, armour = names['weapons'], names['armour']

    def stock(base, n, items):
        out = []
        for s in range(n):
            row = [rom[b2 + base + 4 * s + k] for k in range(4)]
            out.append([items[v] for k, v in enumerate(row) if v or k == 0])
        return out
    return {
        'source': 'bank 2 ($9755 shop numbers, $9829/$9841 weapons, $9B37/$9B4B armour)',
        'shop_number_by_place': where,
        'weapon_shops': stock(0x9829, 6, weapons),
        'armour_shops': stock(0x9B37, 5, armour),
        'weapon_prices': {weapons[i]: word(0x9841 + 2 * i) for i in range(16)},
        'armour_prices': {armour[i]: word(0x9B4B + 2 * i) for i in range(8)},
        'food_prices': [word(0x9ED8 + 2 * i) for i in range(5)],          # $9ED8, per food shop
        'reagent_prices': [{names['reagents'][r]: rom[b2 + 0xA3B7 + 6 * s + r] for r in range(6)}
                           for s in range(4)],                              # $A3B7, 6 per reagent shop
        'pub_meal_prices': list(rom[b2 + 0x9F74: b2 + 0x9F7A]),          # $9F74, the specialty plate
        'pub_rumour_tips': list(rom[b2 + 0x9F6E: b2 + 0x9F74]),          # $9F6E, gold needed for the rumour
        'inns': [{'bed_x': rom[b2 + 0xA83E + 3 * i], 'bed_y': rom[b2 + 0xA83F + 3 * i],
                  'price': rom[b2 + 0xA840 + 3 * i]} for i in range(7)],   # $A83E (x, y, price)
        'healer_and_guild_prices': 'only in the shop texts (shops.json under data/text)',
    }



def spells(rom, names):
    """Spells:
      bank 2 $88F3  reagent recipe per spell, one bit per reagent
                    (Sulfur Ash = $80 ... Mandrake = $01), checked when mixing
      bank 1 $5DE1  magic points to cast
      bank 1 $5D72  per map mode ($C99F) a table of 26 flags: can be cast here
                    ($5D80 world and towns, $5D9A dungeons, $5DB4 combat)
      bank 1 $5DFB  the spells' routines
    Mixtures are counted at $C937 (26 bytes, at most 99 each); reagents at $C92F."""
    reag = names['reagents']
    b2 = 2 * 0x4000 - 0x8000
    out = []
    for i, name in enumerate(names['spells']):
        mask = rom[b2 + 0x88F3 + i]
        out.append({
            'name': name,
            'reagents': [reag[r] for r in range(8) if mask & (0x80 >> r)],
            'mp': rom[0x5DE1 + i],
            'usable': [where for where, base in (('world and towns', 0x5D80), ('dungeons', 0x5D9A),
                                                 ('combat', 0x5DB4)) if rom[base + i]],
        })
    return out


if __name__ == '__main__':
    main()
