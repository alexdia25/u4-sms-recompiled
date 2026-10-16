# Game data extracted from the ROM

Regenerate with:

```sh
python tools/extract_text.py rom.sms build/data
python tools/extract_maps.py rom.sms build/data
```

Addresses are CPU addresses with the named bank paged in (`bank:$addr`).

## Text — `text/`

| File | Source | Format |
| --- | --- | --- |
| `dialogue.json`, `dialogue.txt` | banks 27–30 | 4 tables per bank × 16 people = 256 people (16 settlements). A table is 16 pointers followed by the records. A record is a header ending in `$FF`, then 11 zero-terminated strings: name, look, job, health, response 1, response 2, question, yes, no, keyword 1, keyword 2. `$01` in text is a line break. Header: byte 0 = topic after which the yes/no question comes, byte 2 = turn-away value, bytes 1 and 3 unknown, then (topic asked, flag set) pairs; flags `$85`/`$86` add keyword 1/2 to the talk menu (decoded into `question_after`, `turn_away`, `unlocks`). Topics: 0 bye, 1 name, 2 look, 3 job, 4 health, 5/6 keyword 1/2 answers, 7 join, 8 give. |
| `names.json` | bank 16, 167 pointers at `$8302` | monsters (36), weapons (16), armour (8), weapon abbreviations, classes (Mage … Shepherd = 0–7), people types, reagents, spells (Awaken … Down), places, virtues, stone colours. Places are in location-number order: index 0 is Britannia, then The Lycaeum = 1 … Hythloth, then the Abyss. |
| `messages.json` | bank 26, 207 pointers at `$8000` | game messages by number, as printed by `print_message` (`01:$5582`, A = number). Entries 118 on double as the Lord British / Hawkwind table at `$80EC`. |
| `shops.json` | bank 22, 10 pointers at `$8000` | shop and service texts in 10 groups (weapons, armour, food, pub, reagents, healer, inn, guild, horses, Hawkwind's virtue readings); each group is its own pointer list. |

More text sits in the code banks right after `call print_inline` (`01:$5510`); it is shown as `.db "..."` in `src/`.

## Maps — `maps/`

**World** (`world.json`, `world.png`): 256×256 tiles, same coordinates as the PC version. 16×16 chunks of 16×16 tiles, read with bank 4 at `$4000` and bank 5 at `$8000`. The chunk at (cx, cy) has 16 row pointers at `$4000 + cx·$200 + cy·$20`. Rows are run-length coded: a byte below `$80` is a tile, `$80+n` is followed by a tile repeated n times. Tile ids `$20–$4F` are water with shoreline variants; `tile_lookup` (`4:$6000`) is the value the game keeps per tile in its map buffer at `$D000`. Routine: `load_world_chunk` (`$0EE8`).

**Settlements** (`locations.json`, `location_NN.png`): 17 maps of 32×32, bank 15, word table at `$8077` indexed by the location number (`$C9A0`). Each record is 256 bytes of people/objects (copied to `$C651`, not decoded yet) followed by the run-length coded map: `$00` ends, `$01–$7F` n = next byte repeated n times, `$80+n` = n literal bytes. Routine: `decode_location_map` (`$116B`).

**Dungeons** (`dungeons.json`, `dungeons.png`): 8 dungeons (locations `$12`–`$19`: Deceit, Despise, Destard, Wrong, Covetous, Shame, Hythloth, the Abyss), each 8 levels of 16×16 cells — twice the PC version's 8×8. A 3-byte table at `$2A81` gives (bank, header) per dungeon in banks 23–25; the header's first pointer is the run-length coded dungeon (same coding as the settlements, one byte per cell, `decode_rle` at `$06FF`). Cell codes follow the PC version's high nibble (`$F0` wall, `$10`/`$20` ladders, `$C0` door, `$D0` room …); the low nibble is not decoded yet.

**Dungeon rooms** (`dungeons.json`, `rooms` per dungeon): the header's pointers 1–16 are the 16 rooms (`load_dungeon_room`, `$3D82`, room number in `$C9B8`). Each room: 4 triggers (tile + 3 packed positions), 16 monsters (graphic + packed position; their hit points are rolled on entry as `random_below(strength) | strength/2`), the party's 8 start places for each of 4 entry directions, then an 11×11 map coded like the settlements. Packed position = `(y << 4) | x`.

**Combat arenas** (`arenas.json`): 24 arenas for fights outside dungeons, bank 5, pointers at `$A4DF` (`load_arena`, `$377E`). Each: 16 monster places and 8 party places (x, y), then an 11×11 map coded like the settlements.

## Graphics — `maps/map_graphics.png`

Banks 6–7 hold 256 graphics of 16×12 pixels, 128 bytes each (terrain, towns, people, monsters, letters): two 8-pixel columns of 12 rows, each row a transparency mask byte followed by the four bit planes. The palette comes from the VDP (the extractor takes it from a save state). The game draws these into the map window in software (`$1F56`–`$2097`), because 12-pixel rows don't line up with the console's 8×8 tiles. Graphic numbers of `$8000` and above (water animation, sprites) take another path (`map_graphic_address`, `$2098`) that is not decoded yet.

## Pictures — `pictures/`

Full-screen pictures (intro scenes, the gypsy's wagon and abacus, the title) and tile sets live in banks 11, 13, 17–19 and 26. A picture is a 16-colour palette, the 4 bit planes each run-length coded (plane k gives byte k of every tile row), then the name table as two coded layers, tile numbers and attributes; the intro scenes are 22×15 tiles. Loaders: `$938E` (palette to `$C043`, planes via `$0788`, names via `$071A` into `$D000`). Banks 8–14 (first 67 graphics of 11 and 13) are more 16×12 graphics in the map-graphic layout (figures). Written by `tools/extract_pictures.py`.

## Combat numbers — `stats.json`

Bank 3 `$AC00` (`combat_strength`), 76 bytes: monster strength for the 36 monsters (equal to the PC version's maximum hit points, also used for their attacks), 16 values for people and other combatants, the 16 weapons' damage and the 8 armours' defence. Regenerate with `tools/extract_stats.py`.

## Shops — `shops.json`

Bank 2: which settlement has which numbered shop of each type (`$9755`, 8 types × 16 places), weapon and armour shop stock (`$9829`, `$9B37`, 4 items per shop) and prices (`$9841`, `$9B4B`, 16-bit), food prices (`$9ED8`) and reagent prices (`$A3B7`, 6 per shop). Regenerate with `tools/extract_stats.py`. Also pub plate prices (`$9F74`), the tip a barkeep wants for a rumour (`$9F6E`) and inn rooms (`$A83E`: bed position and price). Healer and guild prices only appear in their texts.

## Spells — `spells.json`

The 26 spells with their reagent recipe (bank 2 `$88F3`, one bit per reagent, Sulfur Ash = `$80` … Mandrake = `$01`), magic-point cost (bank 1 `$5DE1`) and where they can be cast (bank 1 `$5D72`: one 26-byte flag table per map mode — world and towns, dungeons, combat). All match the PC version. Mixtures are counted at `$C937` (max 99), reagents at `$C92F`.

## Tile properties — bank 16 `$8000–$82FF`

Three 256-entry tables indexed by tile/graphic number: `$8000` blocked flags (`$FF` = can't walk), `$8100` special tiles (water families `$A0/$C0/$E0`, and `$8x` codes for features), `$8200` object type per graphic (monster/person groups). Meaning of the individual codes is still being confirmed in the code.

## RAM

Named variables are in `tools/hints.txt` (`ram` lines) and appear as `.DEFINE`s at the top of `src/u4.asm`: party position, map view, location, food, gold, karma, party records (29 bytes each from `$C751`), joypad, game mode.
