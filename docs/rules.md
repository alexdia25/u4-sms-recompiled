# Game rules found in the code

Each rule names the routine it comes from (`src/banks/`, names from `tools/hints.txt`).
Where a C version exists in `pc/native/` it is shadow-tested against the original.

## Random numbers — `random` ($2BBF), `random_below` ($2BEB)

`seed = seed*3 + (seed & $FF) << 8` (16-bit, `rng_seed` $C0B6); the result is the
seed's high byte plus `input_noise` ($C0B5), which `update_joypad` increases by every
joypad reading — so pressing buttons changes the sequence.
`random_below(n) = random * n / 256`.

## Combat

Numbers come from `combat_strength` (bank 3 $AC00, also `data/stats.json`), indexed
with `combat_strength_index`: monsters by graphic, people at $24+, weapons at $34+,
armour at $44+.

**A party member attacks** (`combat_attack`, bank 3 $A0D8):
- range: the weapon's entry in `weapon_range` ($A292) picks a pattern of reachable
  squares (`target_in_pattern`); out of range is a miss.
- to hit: a member with dexterity 40 or more always hits; otherwise the hit lands
  when `random < dexterity + 128`.
- damage: `random_below(min(weapon_damage + strength, 255))`, applied with
  `damage_monster` ($39B5).

**A monster is damaged** (`damage_monster`): its hit points (`+4` of the monster's
figure) drop by the damage. When they go below zero it is killed and the attacker gains
`strength / 16 + 1` experience (`member_gain_xp`, capped at 9999). Otherwise the
message describes its state from its remaining hit points against its strength:
under 24 *Fleeing*, under ¼ strength *Critical*, under ½ *Heavily Wounded*, under ¾
*Lightly Wounded*, else *Barely Wounded*.

**A monster hits a party member** (`monster_hits_member`, bank 3 $A802):
`r = random_below(monster_strength / 2)`, then damage = `(r >> 4) * 10 + r % 10`
(the code treats the random number as if it were BCD; it is not, so the result is
uneven). `member_take_damage` subtracts it; going **below** zero kills the member
(status 'D', corpse figure) — landing on exactly 0 does not.

**Healing** (`member_heal`): hit points rise by the amount, up to the maximum.

## Karma — `karma_gain` ($4255), `karma_lose` ($426F)

Eight values (`karma` $C90B, Honesty … Humility), 0–99. Gains stop at 99. A value of
0 means that eighth of Avatarhood is already won and is left alone; losing karma in a
virtue at 0 resets it to 99 and prints "Thou hast lost an Eighth!".

## Party record (`party` $C751, 29 bytes per member)

`+0` hp, `+2` max hp, `+4` experience (16-bit each); `+6` strength, `+7` dexterity,
`+8` intelligence, `+9` magic points, `+A` level (experience/100 + 1 in the states seen),
`+B` weapon, `+C` armour, `+D` name (zero-terminated), `+1A` sex glyph, `+1B` class
(Mage … Shepherd = 0–7, which is also the companion's home town in the order Moonglow,
Britain, Jhelom, Yew, Minoc, Trinsic, Skara Brae, Magincia), `+1C` status (`G` good,
`D` dead).

## Talking

`talk_setup` ($34DD) finds the person's dialogue: table `$351D` gives (bank, table)
per place, the person's number is `+7` of their figure minus 1. The record's header
(see `data/text/dialogue.json`) begins with 4 bytes copied to `$C9A9`: byte 0 is the
topic that leads to the yes/no question, byte 2 is compared with a random number to
decide whether the person turns away; bytes 1 and 3 and the topic list that follows
(up to `$FF`) are still being decoded.

**Join** (topic 7, $6F96): only the first person of a companion's home town can
join; they refuse unless the Avatar's karma in that town's virtue is 0 (eighth won)
or at least 40, and unless the party is smaller than the Avatar's level.

## Shops (`pc/native/shop.c`)

Talking across a counter (a tile $60-$7E) to the person behind it opens a shop.
The counter's row picks the shop type from a per-place table ($96C1), and
`shop_number` (0-based, per type) which one; `shop_handlers` ($9741) has the code:

- **Weapons / armour** ($97D5 / $9AE3, one routine with two tables): buy (list,
  pick, "can you use it", how many, pay per item, stock capped at 99) or sell
  (pick an owned kind, how many, the offer from $AB92, deal).
- **Food** ($9DE9): packs of 25 rations at the shop's price per pack.
- **Pub** ($9EE2): plates of food, or ale. Up to 3 ales a visit; tipping 3+
  when the party knows of unfound reagents ($A173) opens a reagent menu, and the
  pub's own secret, once the tips reach its price ($9F6E), sets a flag at $C500.
- **Reagents** ($A1EB): the player names the price. Paying less than asked costs
  Honesty, Justice and Honor karma (a third of the shortfall, at least 4); any
  payment then gains 2 of each.
- **Healer** ($A3CF): cure poison 100gp, heal 200gp, raise the dead 300gp. On
  leaving, an Avatar with 400+ hit points is asked for blood: yes is +5
  Sacrifice and -100 hit points, no is -5 Sacrifice.
- **Inn** ($A647): open when $C959 = $1F. A night's stay moves the party to the
  room, heals each member a random 0-49 (x2 +100, via `member_heal`), and
  1 time in 8 ends in an ambush; at inn 5 a ghost may appear.
- **Guild** ($A869): torches, gems, keys, and the sextant once $C545 is set.
- **Stable** ($AA14, Paws): horses at 100gp a head.
- **Seer** ($AA75, Hawkwind): reads each virtue's karma in five steps (0 = won,
  then <20, <40, <60, <99, 99 "go to the shrine"); asking gains 3 Spirituality,
  at most once per 100 moves ($C969).
