"""Extract the world map (see recenter_map_view / load_map_chunk at $0EE8).

    python extract_maps.py <rom> <outdir>

The world is 16x16 chunks of 16x16 tiles, read with bank 4 in slot 1 and
bank 5 in slot 2. For chunk (cx, cy) a table at $4000 + cx*$200 + cy*$20
holds 16 pointers, one per tile row. A row is run-length coded: a byte
below $80 is one tile, $80+n is followed by a tile repeated n times.
Each tile id is then looked up in the 2-byte table at $6000 (what the game
copies into its map buffer at $D000).
"""
import json
import os
import sys

BANK = 0x4000


def world(rom):
    def rd(addr):  # bank 4 at $4000-$7FFF, bank 5 at $8000-$BFFF
        bank = 4 if addr < 0x8000 else 5
        return rom[bank * BANK + (addr & 0x3FFF)]

    grid = [[0] * 256 for _ in range(256)]
    for cx in range(16):
        for cy in range(16):
            base = 0x4000 + cx * 0x200 + cy * 0x20
            for row in range(16):
                p = rd(base + 2 * row) | rd(base + 2 * row + 1) << 8
                x = 0
                while x < 16:
                    b = rd(p)
                    p += 1
                    if b < 0x80:
                        tiles = [b]
                    else:
                        tiles = [rd(p)] * (b & 0x7F)
                        p += 1
                    for t in tiles:
                        if x < 16:
                            grid[cy * 16 + row][cx * 16 + x] = t
                        x += 1
    lookup = [rd(0x6000 + 2 * t) | rd(0x6001 + 2 * t) << 8 for t in range(128)]
    return grid, lookup


def main():
    rom = open(sys.argv[1], 'rb').read()
    out = os.path.join(sys.argv[2], 'maps')
    os.makedirs(out, exist_ok=True)
    grid, lookup = world(rom)
    with open(os.path.join(out, 'world.json'), 'w', newline='\n') as f:
        json.dump({'width': 256, 'height': 256,
                   'note': 'tile ids as stored in the ROM; tile_lookup = the 2-byte value the game uses per id',
                   'tile_lookup': ['$%04X' % v for v in lookup],
                   'rows': [' '.join('%02X' % t for t in r) for r in grid]}, f, indent=0)
    try:
        from PIL import Image
    except ImportError:
        return
    used = sorted({t for r in grid for t in r})
    import colorsys
    pal = {t: tuple(int(c * 255) for c in colorsys.hsv_to_rgb((i * 0.618) % 1, 0.7, 0.9)) for i, t in enumerate(used)}
    im = Image.new('RGB', (256, 256))
    im.putdata([pal[t] for r in grid for t in r])
    im.resize((768, 768), Image.NEAREST).save(os.path.join(out, 'world_ids.png'))
    print('world: %d distinct tile ids: %s' % (len(used), ' '.join('%02X' % t for t in used)))


if __name__ == '__main__':
    main()


# ---------------------------------------------------------------- graphics --
def rom_palette(rom):
    """The map palette the game loads into the VDP (bank 1 $7C83, 32 bytes)."""
    return [((c & 3) * 85, ((c >> 2) & 3) * 85, ((c >> 4) & 3) * 85) for c in rom[0x7C83:0x7CA3]]


def map_graphic(rom, g):
    """Map/sprite graphic g (0-255) from banks 6-7: 16x12 pixels, stored as two
    8-pixel columns of 12 rows; each row is a transparency mask byte followed by
    the four bit planes. Returns 12 rows of 16 (colour, opaque) pairs."""
    base = 6 * BANK + g * 128
    rows = []
    for r in range(12):
        row = []
        for col in range(2):
            o = base + col * 60 + r * 5
            mask, p = rom[o], rom[o + 1:o + 5]
            for x in range(8):
                c = sum(((p[k] >> (7 - x)) & 1) << k for k in range(4))
                row.append((c, not (mask >> (7 - x)) & 1))
        rows.append(row)
    return rows


def graphic_for_id(lookup, t):
    v = lookup[t]
    if v < 0x100:
        return v
    return {0xA0: 0, 0xC0: 1, 0xE0: 2}.get(v >> 8 & 0xF0, 0)  # water + shoreline variants -> plain water


def render_extra(rom, out, grid, lookup, pal):
    from PIL import Image
    sheet = Image.new('RGB', (16 * 17, 16 * 13), (255, 0, 255))
    for g in range(256):
        img = map_graphic(rom, g)
        for y, row in enumerate(img):
            for x, (c, _) in enumerate(row):
                sheet.putpixel(((g % 16) * 17 + x, (g // 16) * 13 + y), pal[c])
    sheet.save(os.path.join(out, 'map_graphics.png'))
    cache = {}
    world = Image.new('RGB', (256 * 16, 256 * 12))
    for y in range(256):
        for x in range(256):
            g = graphic_for_id(lookup, grid[y][x])
            if g not in cache:
                tile = Image.new('RGB', (16, 12))
                tile.putdata([pal[c] for row in map_graphic(rom, g) for c, _ in row])
                cache[g] = tile
            world.paste(cache[g], (x * 16, y * 12))
    world.save(os.path.join(out, 'world.png'))
    world.resize((1024, 768)).save(os.path.join(out, 'world_small.png'))


if __name__ == '__main__':
    rom = open(sys.argv[1], 'rb').read()
    out = os.path.join(sys.argv[2], 'maps')
    grid, lookup = world(rom)
    render_extra(rom, os.path.join(sys.argv[2], 'maps'), grid, lookup, rom_palette(rom))


# ------------------------------------------------------------ towns/castles --
def location_maps(rom):
    """Settlement maps: bank 15, word table at $8077 indexed by the location
    number ($C9A0, 1-17). Each record is 256 bytes copied to $C651 (people and
    objects, still to be decoded) followed by the 32x32 map, run-length coded
    (load_location_map at $116B): $00 ends, $01-$7F n = next byte repeated n
    times, $80+n = n literal bytes."""
    base = 15 * BANK
    rd = lambda a: rom[base + a - 0x8000]
    maps = {}
    for i in range(1, 18):
        p = rd(0x8077 + 2 * i) | rd(0x8078 + 2 * i) << 8
        header = bytes(rd(p + k) for k in range(0x100))
        a, tiles = p + 0x100, []
        while True:
            b = rd(a)
            a += 1
            if b == 0:
                break
            if b & 0x80:
                tiles += [rd(a + k) for k in range(b & 0x7F)]
                a += b & 0x7F
            else:
                tiles += [rd(a)] * b
                a += 1
        maps[i] = {'address': '$%04X' % p, 'header': header.hex(), 'tiles': tiles[:1024]}
    return maps


if __name__ == '__main__':
    from PIL import Image
    pal = rom_palette(rom)
    out = os.path.join(sys.argv[2], 'maps')
    locs = location_maps(rom)
    with open(os.path.join(out, 'locations.json'), 'w', newline='\n') as f:
        json.dump({str(k): {'address': v['address'], 'people_and_objects': v['header'],
                            'rows': [' '.join('%02X' % t for t in v['tiles'][r * 32:(r + 1) * 32]) for r in range(32)]}
                   for k, v in locs.items()}, f, indent=1)
    for k, v in locs.items():
        im = Image.new('RGB', (32 * 16, 32 * 12))
        for n, t in enumerate(v['tiles']):
            tile = Image.new('RGB', (16, 12))
            tile.putdata([pal[c] for row in map_graphic(rom, t) for c, _ in row])
            im.paste(tile, ((n % 32) * 16, (n // 32) * 12))
        im.save(os.path.join(out, 'location_%02d.png' % k))


# ----------------------------------------------------------------- dungeons --
DUNGEON_NAMES = ['Deceit', 'Despise', 'Destard', 'Wrong', 'Covetous', 'Shame', 'Hythloth', 'Abyss']
# high nibble codes as in the PC version's dungeon files
DUNGEON_CODES = {0x0: 'corridor', 0x1: 'ladder up', 0x2: 'ladder down', 0x3: 'ladder up+down',
                 0x4: 'chest', 0x5: 'hole up', 0x6: 'hole down', 0x7: 'orb', 0x8: 'trap',
                 0x9: 'fountain', 0xA: 'field', 0xB: 'altar', 0xC: 'door', 0xD: 'room',
                 0xE: 'secret door', 0xF: 'wall'}


def rle_bytes(rom, bank, a):
    rd = lambda x: rom[bank * BANK + (x - 0x8000)]
    out = []
    while True:
        b = rd(a)
        a += 1
        if b == 0:
            return out
        if b & 0x80:
            out += [rd(a + k) for k in range(b & 0x7F)]
            a += b & 0x7F
        else:
            out += [rd(a)] * b
            a += 1


def dungeon_rooms(rom, bank, header):
    """The header's pointers 1-16 are rooms (load_dungeon_room, $3D82; room
    number in $C9B8). A room: 4 triggers of (tile, 3 packed positions); 16
    monsters of (graphic, packed position); party start places, 8 packed
    positions for each of the 4 directions the party can enter from; then the
    11x11 map, run-length coded like the settlements. A packed position is
    (y << 4) | x."""
    rd = lambda a: rom[bank * BANK + a - 0x8000]
    pos = lambda b: [b & 0x0F, b >> 4]
    rooms = []
    for r in range(16):
        p = rd(header + 2 + 2 * r) | rd(header + 3 + 2 * r) << 8
        triggers = [{'tile': rd(p + 4 * t), 'positions': [pos(rd(p + 4 * t + k)) for k in (1, 2, 3)]}
                    for t in range(4) if rd(p + 4 * t)]
        monsters = [{'graphic': rd(p + 16 + 2 * m), 'at': pos(rd(p + 17 + 2 * m))}
                    for m in range(16) if rd(p + 16 + 2 * m)]
        starts = [[pos(rd(p + 48 + 8 * d + k)) for k in range(8)] for d in range(4)]
        a, tiles = p + 80, []
        while True:
            b = rd(a)
            a += 1
            if b == 0:
                break
            if b & 0x80:
                tiles += [rd(a + k) for k in range(b & 0x7F)]
                a += b & 0x7F
            else:
                tiles += [rd(a)] * b
                a += 1
        rooms.append({'triggers': triggers, 'monsters': monsters, 'party_starts': starts,
                      'map': [' '.join('%02X' % t for t in tiles[y * 11:(y + 1) * 11]) for y in range(11)]})
    return rooms


def dungeons(rom):
    """Table at $2A81 (dungeon_table): (bank, header address) for locations
    $12-$19. The header's first pointer is the run-length coded dungeon
    (same coding as the settlements, one byte per cell): 8 levels of 16x16
    cells. The header's other 16 pointers (rooms?) are not decoded yet."""
    out = {}
    for i, name in enumerate(DUNGEON_NAMES):
        bank, lo, hi = rom[0x2A81 + 3 * i: 0x2A84 + 3 * i]
        hdr = bank * BANK + ((lo | hi << 8) - 0x8000)
        first = rom[hdr] | rom[hdr + 1] << 8
        cells = rle_bytes(rom, bank, first)
        out[name] = {'bank': bank, 'header': '$%04X' % (lo | hi << 8),
                     'levels': [cells[l * 256:(l + 1) * 256] for l in range(8)],
                     'rooms': dungeon_rooms(rom, bank, lo | hi << 8)}
    return out


if __name__ == '__main__':
    dung = dungeons(rom)
    with open(os.path.join(out, 'dungeons.json'), 'w', newline='\n') as f:
        json.dump({'codes': {'%X0' % k: v for k, v in DUNGEON_CODES.items()},
                   'dungeons': {n: {'bank': v['bank'], 'header': v['header'],
                                    'levels': [[' '.join('%02X' % c for c in lv[r * 16:(r + 1) * 16]) for r in range(16)]
                                               for lv in v['levels']],
                                    'rooms': v['rooms']}
                                for n, v in dung.items()}}, f, indent=1)
    colours = {0x0: (40, 40, 40), 0x1: (0, 255, 0), 0x2: (255, 0, 0), 0x3: (255, 255, 0), 0x4: (255, 200, 0),
               0x5: (0, 120, 0), 0x6: (120, 0, 0), 0x7: (255, 0, 255), 0x8: (255, 128, 0), 0x9: (0, 160, 255),
               0xA: (0, 255, 255), 0xB: (255, 255, 255), 0xC: (160, 100, 40), 0xD: (90, 90, 200),
               0xE: (200, 160, 120), 0xF: (150, 150, 150)}
    im = Image.new('RGB', (8 * 17 * 6, 8 * 17 * 6), (0, 0, 0))
    for di, (n, v) in enumerate(dung.items()):
        for li, lv in enumerate(v['levels']):
            for c, val in enumerate(lv):
                x0, y0 = (li * 17 + c % 16) * 6, (di * 17 + c // 16) * 6
                for yy in range(6):
                    for xx in range(6):
                        im.putpixel((x0 + xx, y0 + yy), colours[val >> 4])
    im.save(os.path.join(out, 'dungeons.png'))


# ------------------------------------------------------------------ arenas --
def arenas(rom):
    """Combat arenas outside dungeons: bank 5, 24 pointers at $A4DF, picked by
    load_arena ($377E, A = arena). Each: 16 monster places (x, y), 8 party
    places (x, y), then an 11x11 map coded like the settlements. Some tiles
    are swapped for the terrain the fight happens on afterwards ($1204)."""
    rd = lambda a: rom[5 * BANK + a - 0x8000]
    out = []
    for i in range(24):
        p = rd(0xA4DF + 2 * i) | rd(0xA4E0 + 2 * i) << 8
        monsters = [[rd(p + 2 * k), rd(p + 2 * k + 1)] for k in range(16)]
        party = [[rd(p + 32 + 2 * k), rd(p + 33 + 2 * k)] for k in range(8)]
        a, tiles = p + 48, []
        while True:
            b = rd(a)
            a += 1
            if b == 0:
                break
            if b & 0x80:
                tiles += [rd(a + k) for k in range(b & 0x7F)]
                a += b & 0x7F
            else:
                tiles += [rd(a)] * b
                a += 1
        out.append({'address': '$%04X' % p, 'monster_places': monsters, 'party_places': party,
                    'map': [' '.join('%02X' % t for t in tiles[y * 11:(y + 1) * 11]) for y in range(11)]})
    return out


if __name__ == '__main__':
    with open(os.path.join(out, 'arenas.json'), 'w', newline='') as f:
        json.dump(arenas(rom), f, indent=1)
