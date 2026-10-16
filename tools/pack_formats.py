"""Decoded asset formats for make_assets.py.

Each format has:
  - a finder that says which ROM regions it covers (regions_for_bank),
  - an encoder: ROM bytes -> the entry stored in the pack (structured), and
  - a rebuild rule: entry -> the ROM bytes, mirrored in C by
    pc/runtime/assets.c, so the exe can lay the data out as the original
    code expects.
"""
import struct

BANK = 0x4000


# ---------------------------------------------------------------- dialogue --
# Banks 27-30 hold 4 tables of 16 people each (see tools/extract_text.py).
# Entry "dialogue/BB_AAAA" (bank, table address): 16 records, each
#   u8 header length, header bytes, then the 11 strings, each zero-terminated
# (the record's own bytes minus its $FF header terminator). The rebuild writes
# the 16 pointers (table + $20 + offsets) and the records with $FF restored.

def _dialogue_tables(blk):
    tabs = []
    for o in range(0, BANK - 0x20):
        p = blk[o] | blk[o + 1] << 8
        if p == 0x8000 + o + 0x20:
            ps = [blk[o + 2 * i] | blk[o + 2 * i + 1] << 8 for i in range(16)]
            if all(0x8000 + o <= x < 0xC000 for x in ps) and ps == sorted(ps):
                tabs.append(o)
    return tabs


def _record_end(blk, o):
    while blk[o] != 0xFF:
        o += 1
    o += 1
    for _ in range(11):
        o = blk.index(0, o) + 1
    return o


def _dialogue_regions(rom, bank):
    blk = rom[bank * BANK:(bank + 1) * BANK]
    out = []
    for t in _dialogue_tables(blk):
        ptrs = [(blk[t + 2 * i] | blk[t + 2 * i + 1] << 8) - 0x8000 for i in range(16)]
        if ptrs[0] != t + 0x20:
            continue
        end = _record_end(blk, ptrs[15])
        # records must follow each other with nothing in between
        if any(_record_end(blk, ptrs[i]) != ptrs[i + 1] for i in range(15)):
            continue

        def encode(t=t, ptrs=ptrs, end=end, blk=blk, bank=bank):
            data = bytearray()
            for i in range(16):
                a = ptrs[i]
                b = ptrs[i + 1] if i < 15 else end
                h = blk.index(0xFF, a)
                data += bytes([h - a]) + blk[a:h] + blk[h + 1:b]
            return 'dialogue/%02d_%04X' % (bank, 0x8000 + t), bytes(data)
        out.append((t, end - t, 'dialogue', encode))
    return out


def _rebuild_dialogue(data, length, base_offset=0):
    # base_offset: the region's start inside the bank (pointers are absolute)
    records, p = [], 0
    for _ in range(16):
        hl = data[p]
        header = data[p + 1:p + 1 + hl]
        p += 1 + hl
        q = p
        for _ in range(11):
            q = data.index(0, q) + 1
        records.append(header + b'\xFF' + data[p:q])
        p = q
    return records


# ------------------------------------------------------------ map graphics --
# Banks 6-7: 256 graphics of 16x12 pixels (see extract_maps.map_graphic).
# Entries "graphics/bankNN" (banks 6-14; 6-7 are the map tiles, the others
# figures): per graphic 96 bytes of pixels (colour index 0-15,
# two pixels per byte, high nibble first, row by row) then 24 bytes of
# transparency masks (row by row, left byte then right byte). The ROM keeps
# each graphic as two 8-pixel columns of 12 (mask, 4 bit planes) rows plus
# 8 zero bytes.

def _encode_map_graphics(rom, bank, count=128):
    out = bytearray()
    for g in range(count):
        base = bank * BANK + g * 128
        pix, masks = [], []
        for r in range(12):
            for col in range(2):
                o = base + col * 60 + r * 5
                masks.append(rom[o])
                p = rom[o + 1:o + 5]
                for x in range(8):
                    pix.append(sum(((p[k] >> (7 - x)) & 1) << k for k in range(4)))
        out += bytes((pix[i] << 4) | pix[i + 1] for i in range(0, 192, 2)) + bytes(masks)
    return 'graphics/bank%02d' % bank, bytes(out)


def _rebuild_map_graphics(data):
    count = len(data) // 120
    out = bytearray(count * 128)
    for g in range(count):
        d = data[g * 120:(g + 1) * 120]
        pix = [v for b in d[:96] for v in (b >> 4, b & 15)]
        for r in range(12):
            for col in range(2):
                o = g * 128 + col * 60 + r * 5
                out[o] = d[96 + r * 2 + col]
                row = pix[r * 16 + col * 8: r * 16 + col * 8 + 8]
                for k in range(4):
                    out[o + 1 + k] = sum(((c >> k) & 1) << (7 - x) for x, c in enumerate(row))
    return bytes(out)


# --------------------------------------------------------------- world map --
# Banks 4-5 (see extract_maps.world): row pointer tables at 4:$4000 (chunk
# (cx, cy) at $4000 + cx*$200 + cy*$20, 16 rows each), the tile -> cell table
# at 4:$6000 (256 bytes), then the distinct rows, run-length coded, from
# 4:$6100 up to 5:$A4DF. The game's encoder is canonical: runs of 3 or more
# become $80+n, tile; shorter runs stay literal; identical rows are stored once.
# Entry "maps/world": u16 row count R; R rows of 16 tile ids in storage
# order; 4096 u16 row numbers (cx, cy, row order); the 256-byte cell table.

WORLD_END = 0x4000 + 0x24DF       # bytes from 4:$4000 to 5:$A4DF


def _world_rd(rom, a):
    return rom[(4 if a < 0x8000 else 5) * BANK + (a & 0x3FFF)]


def _rle_row(tiles):
    out, i = bytearray(), 0
    while i < 16:
        j = i
        while j < 16 and tiles[j] == tiles[i]:
            j += 1
        n = j - i
        out += bytes([0x80 | n, tiles[i]]) if n >= 3 else bytes([tiles[i]] * n)
        i = j
    return bytes(out)


def _encode_world(rom):
    ptrs = []
    for cx in range(16):
        for cy in range(16):
            base = 0x4000 + cx * 0x200 + cy * 0x20
            ptrs += [_world_rd(rom, base + 2 * r) | _world_rd(rom, base + 2 * r + 1) << 8 for r in range(16)]
    order = sorted(set(ptrs))
    rows = []
    for p in order:
        t = []
        while len(t) < 16:
            b = _world_rd(rom, p)
            p += 1
            if b & 0x80:
                t += [_world_rd(rom, p)] * (b & 0x7F)
                p += 1
            else:
                t.append(b)
        rows.append(bytes(t[:16]))
    index = {p: i for i, p in enumerate(order)}
    data = struct.pack('<H', len(rows)) + b''.join(rows)
    data += b''.join(struct.pack('<H', index[p]) for p in ptrs)
    data += bytes(_world_rd(rom, 0x6000 + i) for i in range(256))
    return 'maps/world', data


def _rebuild_world(data):
    n = struct.unpack_from('<H', data)[0]
    rows = [data[2 + 16 * i: 18 + 16 * i] for i in range(n)]
    idx = struct.unpack_from('<4096H', data, 2 + 16 * n)
    table = data[2 + 16 * n + 8192:]
    out = bytearray(WORLD_END)
    addr, where = 0x6100, []
    for r in rows:
        e = _rle_row(r)
        where.append(addr)
        out[addr - 0x4000: addr - 0x4000 + len(e)] = e
        addr += len(e)
    for k, i in enumerate(idx):
        struct.pack_into('<H', out, 2 * k, where[i])
    out[0x2000:0x2100] = table
    return bytes(out)


# ------------------------------------------------------------ string tables --
# A table of `count` 16-bit pointers followed directly by the zero-terminated
# strings they point at (messages, shop texts, names). Entry: u16 count,
# u16 string count, the strings in storage order (each zero-terminated), then
# for each pointer the number of the string it points at. Rebuilt by laying
# the strings out after the table and pointing at them.

def _string_table(rom, bank, table, count, name):
    B = bank * BANK - 0x8000
    ptrs = [rom[B + table + 2 * i] | rom[B + table + 2 * i + 1] << 8 for i in range(count)]
    start = table + 2 * count
    end = max(rom.index(0, B + p) + 1 - B for p in ptrs)
    strings, offsets, a = [], {}, start
    while a < end:
        e = rom.index(0, B + a) + 1 - B
        offsets[a] = len(strings)
        strings.append(rom[B + a: B + e])
        a = e
    if any(p not in offsets for p in ptrs):
        return []

    def encode():
        data = struct.pack('<HH', count, len(strings)) + b''.join(strings)
        return name, data + b''.join(struct.pack('<H', offsets[p]) for p in ptrs)
    return [(table - 0x8000, end - table, 'string_table', encode)]


def _rebuild_string_table(data, start):
    count, n = struct.unpack_from('<HH', data)
    p, where, pool = 4, [], bytearray()
    addr = 0x8000 + start + 2 * count
    for _ in range(n):
        e = data.index(0, p) + 1
        where.append(addr + len(pool))
        pool += data[p:e]
        p = e
    idx = struct.unpack_from('<%dH' % count, data, p)
    return b''.join(struct.pack('<H', where[i]) for i in idx) + bytes(pool)


# ----------------------------------------------------- run-length layouts --
# The settlement/dungeon coding ($00 end, $01-$7F n = repeat next byte n
# times, $80+n = n literal bytes) was made by hand or by a tool we can't
# reproduce, so the pack keeps the decoded data plus the token layout:
# u16 token count, then per token one byte (repeat: n, literal: $80+n);
# the values come from the decoded data.

def rle_split(blk, a):
    """Decode a stream at offset a of blk -> (values, tokens, end offset)."""
    values, tokens = bytearray(), bytearray()
    while True:
        b = blk[a]
        a += 1
        if b == 0:
            return bytes(values), bytes(tokens), a
        tokens.append(b)
        if b & 0x80:
            values += blk[a:a + (b & 0x7F)]
            a += b & 0x7F
        else:
            values += bytes([blk[a]]) * b
            a += 1


def rle_join(values, tokens):
    out, p = bytearray(), 0
    for t in tokens:
        if t & 0x80:
            out.append(t)
            out += values[p:p + (t & 0x7F)]
            p += t & 0x7F
        else:
            out += bytes([t, values[p]])
            p += t
    out.append(0)
    return bytes(out)


# Settlements: bank 15 $8077, a word table (entry 0 is not a record; kept as
# it is), then 17 records back to back: 256 bytes of people/objects and the
# run-length coded 32x32 map. Entry "maps/settlements": u16 table[0], then
# per record: 256 people bytes, u16 map length, the map's tiles, u16 token
# count, the tokens.

def _encode_settlements(rom):
    blk = rom[15 * BANK:16 * BANK]
    w = lambda a: blk[a - 0x8000] | blk[a - 0x7FFF] << 8
    ptrs = [w(0x8077 + 2 * i) for i in range(18)]
    data = bytearray(struct.pack('<H', ptrs[0]))
    for p in ptrs[1:]:
        o = p - 0x8000
        tiles, tokens, _ = rle_split(blk, o + 0x100)
        data += blk[o:o + 0x100] + struct.pack('<H', len(tiles)) + tiles
        data += struct.pack('<H', len(tokens)) + tokens
    return 'maps/settlements', bytes(data)


def _rebuild_settlements(data):
    first = struct.unpack_from('<H', data)[0]
    p, recs = 2, []
    for _ in range(17):
        people = data[p:p + 0x100]
        p += 0x100
        n = struct.unpack_from('<H', data, p)[0]
        tiles = data[p + 2:p + 2 + n]
        p += 2 + n
        k = struct.unpack_from('<H', data, p)[0]
        tokens = data[p + 2:p + 2 + k]
        p += 2 + k
        recs.append(people + rle_join(tiles, tokens))
    out = bytearray(struct.pack('<H', first))
    addr = 0x8077 + 36
    for r in recs:
        out += struct.pack('<H', addr)
        addr += len(r)
    return bytes(out) + b''.join(recs)


# Dungeon banks 23-25: 2 or 4 dungeon headers of 17 pointers (the dungeon,
# then its 16 rooms), in bank 25 followed by 48 more room pointers (the
# Abyss), then the records in pointer order: the dungeon (run-length coded
# cells) and rooms (80 bytes of triggers, monsters and party places, then
# the run-length coded 11x11 map). Entry "maps/dungeons_BB": u8 dungeons,
# u8 extra rooms, then per record: [80 room bytes], u16 n, n values, u16 k,
# k tokens.

DUNGEON_BANKS = {23: (4, 0), 24: (2, 0), 25: (2, 48)}


def _dungeon_records(blk, bank):
    nd, extra = DUNGEON_BANKS[bank]
    w = lambda o: blk[o] | blk[o + 1] << 8
    recs = []   # (offset, is_room)
    for k in range(nd):
        recs += [(w(k * 0x22 + 2 * i) - 0x8000, i > 0) for i in range(17)]
    recs += [(w(nd * 0x22 + 2 * i) - 0x8000, True) for i in range(extra)]
    return recs


def _dungeon_regions(rom, bank):
    blk = rom[bank * BANK:(bank + 1) * BANK]
    nd, extra = DUNGEON_BANKS[bank]
    pos = nd * 0x22 + 2 * extra
    for o, room in _dungeon_records(blk, bank):
        if o != pos:
            return []
        _, _, pos = rle_split(blk, o + 80 if room else o)

    def encode():
        data = bytearray([nd, extra])
        for o, room in _dungeon_records(blk, bank):
            if room:
                data += blk[o:o + 80]
            values, tokens, _ = rle_split(blk, o + 80 if room else o)
            data += struct.pack('<H', len(values)) + values + struct.pack('<H', len(tokens)) + tokens
        return 'maps/dungeons_%d' % bank, bytes(data)
    return [(0, pos, 'dungeon_bank', encode)]


def _rebuild_dungeon_bank(data):
    nd, extra = data[0], data[1]
    p, recs = 2, []
    nrec = nd * 17 + extra
    for i in range(nrec):
        room = not (i < nd * 17 and i % 17 == 0)
        head = b''
        if room:
            head = data[p:p + 80]
            p += 80
        n = struct.unpack_from('<H', data, p)[0]
        values = data[p + 2:p + 2 + n]
        p += 2 + n
        k = struct.unpack_from('<H', data, p)[0]
        tokens = data[p + 2:p + 2 + k]
        p += 2 + k
        recs.append(head + rle_join(values, tokens))
    addr = 0x8000 + nd * 0x22 + 2 * extra
    table = bytearray()
    for r in recs:
        table += struct.pack('<H', addr)
        addr += len(r)
    return bytes(table) + b''.join(recs)


# Combat arenas: bank 5 $A4DF, 24 pointers then the records: 48 bytes of
# monster and party places, then the run-length coded 11x11 map.
# Entry "maps/arenas": u8 count, per record 48 bytes, u16 n, values, u16 k,
# tokens.

def _arena_regions(rom):
    blk = rom[5 * BANK:6 * BANK]
    t = 0xA4DF - 0x8000
    w = lambda o: blk[o] | blk[o + 1] << 8
    pos, offs = t + 48, []
    for i in range(24):
        o = w(t + 2 * i) - 0x8000
        if o != pos:
            return []
        offs.append(o)
        _, _, pos = rle_split(blk, o + 48)

    def encode():
        data = bytearray([24])
        for o in offs:
            values, tokens, _ = rle_split(blk, o + 48)
            data += blk[o:o + 48] + struct.pack('<H', len(values)) + values
            data += struct.pack('<H', len(tokens)) + tokens
        return 'maps/arenas', bytes(data)
    return [(t, pos - t, 'arenas', encode)]


def _rebuild_arenas(data, start):
    n, p, recs = data[0], 1, []
    for _ in range(n):
        head = data[p:p + 48]
        p += 48
        nv = struct.unpack_from('<H', data, p)[0]
        values = data[p + 2:p + 2 + nv]
        p += 2 + nv
        k = struct.unpack_from('<H', data, p)[0]
        tokens = data[p + 2:p + 2 + k]
        p += 2 + k
        recs.append(head + rle_join(values, tokens))
    addr, table = 0x8000 + start + 2 * n, bytearray()
    for r in recs:
        table += struct.pack('<H', addr)
        addr += len(r)
    return bytes(table) + b''.join(recs)


# --------------------------------------------------------------- pictures --
# The picture banks are runs of: pictures (16-byte palette, the 4 bit planes
# each run-length coded, then the name table as two run-length coded layers,
# tile numbers and attributes), tile sets (4 coded planes) and single coded
# streams (screen layouts). Found by walking the bank from a known start;
# whatever the walk can't read stays raw. Entry "pictures/bank_BB": a list
# of chunks, each a type byte (0 raw: u16 length, bytes; 1 stream: u16 n,
# values, u16 k, tokens) and a u8 group tag (0 none, 1 picture, 2 tile set)
# for the extractor.

PICTURE_BANKS = {11: 0x2180, 13: 0x2180, 17: 0, 18: 0, 19: 0, 26: 0x2F91}


def _stream_ok(blk, a):
    try:
        v, t, e = rle_split(blk, a)
    except IndexError:
        return None
    return (v, t, e) if v and e <= len(blk) else None


def _planes(blk, a):
    out, o = [], a
    for _ in range(4):
        r = _stream_ok(blk, o)
        if not r:
            return None
        out.append(r)
        o = r[2]
    n = len(out[0][0])
    return out if n and n % 8 == 0 and all(len(x[0]) == n for x in out) else None


def _picture_chunks(blk, start):
    chunks, a = [], start
    while a < BANK:
        if a + 16 <= BANK and all(x < 0x40 for x in blk[a:a + 16]):
            pl = _planes(blk, a + 16)
            if pl:
                n1 = _stream_ok(blk, pl[-1][2])
                n2 = _stream_ok(blk, n1[2]) if n1 else None
                if n1 and n2 and len(n1[0]) == len(n2[0]):
                    chunks.append((0, blk[a:a + 16], 1))
                    chunks += [(1, x, 1) for x in pl + [n1, n2]]
                    a = n2[2]
                    continue
        pl = _planes(blk, a)
        if pl:
            chunks += [(1, x, 2) for x in pl]
            a = pl[-1][2]
            continue
        r = _stream_ok(blk, a)
        if r and r[2] - a < 0x2000:
            chunks.append((1, r, 0))
            a = r[2]
            continue
        break
    return chunks, a


def _encode_chunks(chunks):
    data = bytearray(struct.pack('<H', len(chunks)))
    for kind, x, tag in chunks:
        if kind == 0:
            data += bytes([0, tag]) + struct.pack('<H', len(x)) + x
        else:
            v, t, _ = x
            data += bytes([1, tag]) + struct.pack('<H', len(v)) + v + struct.pack('<H', len(t)) + t
    return bytes(data)


def _rebuild_chunks(data):
    n = struct.unpack_from('<H', data)[0]
    p, out = 2, bytearray()
    for _ in range(n):
        kind = data[p]
        p += 2
        if kind == 0:
            ln = struct.unpack_from('<H', data, p)[0]
            out += data[p + 2:p + 2 + ln]
            p += 2 + ln
        else:
            nv = struct.unpack_from('<H', data, p)[0]
            v = data[p + 2:p + 2 + nv]
            p += 2 + nv
            nk = struct.unpack_from('<H', data, p)[0]
            t = data[p + 2:p + 2 + nk]
            p += 2 + nk
            out += rle_join(v, t)
    return bytes(out)


def _picture_regions(rom, bank):
    blk = rom[bank * BANK:(bank + 1) * BANK]
    start = PICTURE_BANKS[bank]
    chunks, end = _picture_chunks(blk, start)
    if not chunks:
        return []
    return [(start, end - start, 'chunks', lambda: ('pictures/bank_%02d' % bank, _encode_chunks(chunks)))]


def rebuild(kind, data, length, start=0, bank=None):
    if kind == 'chunks':
        return _rebuild_chunks(data)
    if kind == 'arenas':
        return _rebuild_arenas(data, start)
    if kind == 'dungeon_bank':
        return _rebuild_dungeon_bank(data)
    if kind == 'settlements':
        return _rebuild_settlements(data)
    if kind == 'string_table':
        return _rebuild_string_table(data, start)
    if kind == 'world_map':
        full = _rebuild_world(data)
        return full[:BANK] if bank == 4 else full[BANK:BANK + length]
    if kind == 'map_graphics':
        return _rebuild_map_graphics(data)[:length]
    if kind == 'dialogue':
        records = _rebuild_dialogue(data, length)
        out = bytearray()
        addr = 0x8000 + start + 0x20
        for r in records:
            out += struct.pack('<H', addr)
            addr += len(r)
        for r in records:
            out += r
        return bytes(out)
    raise ValueError(kind)


def regions_for_bank(rom, bank):
    """(start, length, kind, encode) for the decoded regions of a bank."""
    if 27 <= bank <= 30:
        return _dialogue_regions(rom, bank)
    if bank in DUNGEON_BANKS:
        extra = []
        if bank == 24:   # the gypsy's virtue questions and answers follow the dungeons
            extra = _string_table(rom, 24, 0x9E1E, 76, 'text/gypsy')
        return _dungeon_regions(rom, bank) + extra
    if bank == 15:   # settlement maps and their people
        blk = rom[15 * BANK:16 * BANK]
        last = (blk[0x77 + 34] | blk[0x77 + 35] << 8) - 0x8000
        _, _, end = rle_split(blk, last + 0x100)
        return [(0x77, end - 0x77, 'settlements', lambda: _encode_settlements(rom))]
    if bank == 26:   # game messages, then a picture
        return _string_table(rom, 26, 0x8000, 207, 'text/messages') + _picture_regions(rom, 26)
    if bank == 16:   # names of monsters, items, spells, places...
        return _string_table(rom, 16, 0x8302, 167, 'text/names')
    if bank == 22:   # shop texts: all the groups' pointer lists back to back at $8014
        B = 22 * BANK - 0x8000
        first = min(rom[B + 0x8014 + 2 * i] | rom[B + 0x8015 + 2 * i] << 8 for i in range(300))
        return (_string_table(rom, 22, 0x8014, (first - 0x8014) // 2, 'text/shops') +
                _string_table(rom, 22, 0xB1CE, 10, 'text/lists'))   # place lists etc.
    if bank == 4:
        return [(0, BANK, 'world_map', lambda: _encode_world(rom))]
    if bank == 5:
        return [(0, WORLD_END - BANK, 'world_map', lambda: _encode_world(rom))] + _arena_regions(rom)
    if bank in (6, 7, 8, 9, 10, 12, 14):   # 128 graphics of 16x12 each (6-7: map, 8 on: figures)
        return [(0, BANK, 'map_graphics', lambda: _encode_map_graphics(rom, bank))]
    if bank in (11, 13):   # 67 graphics, then pictures
        return ([(0, 67 * 128, 'map_graphics', lambda: _encode_map_graphics(rom, bank, 67))] +
                _picture_regions(rom, bank))
    if bank in (17, 18, 19):
        return _picture_regions(rom, bank)
    return []
