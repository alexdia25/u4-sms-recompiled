"""Extract the game's text into data/text/.

    python extract_text.py <rom> <outdir>

Dialogue (banks 27-30): each bank holds four tables of 16 people; a table is
16 little-endian pointers followed by the records. A record is a header
(ending in $FF, meaning still being worked out) and then 11 zero-terminated
strings in the order of the PC version's TLK files, without the pronoun:
name, look, job, health, response 1, response 2, question, yes answer,
no answer, keyword 1, keyword 2.

Message banks (22, 26) are written out as every zero-terminated printable
string with its address, until their tables are decoded.
"""
import json
import os
import sys

BANK = 0x4000
FIELDS = ['name', 'look', 'job', 'health', 'response1', 'response2',
          'question', 'yes', 'no', 'keyword1', 'keyword2']


def text(raw):
    out = []
    for c in raw:
        if c == 1:
            out.append('\n')
        elif 32 <= c < 127:
            out.append(chr(c))
        else:
            out.append('{%02X}' % c)
    return ''.join(out)


def dialogue_tables(blk):
    tabs = []
    for o in range(0, BANK - 0x20):
        p = blk[o] | blk[o + 1] << 8
        if p == 0x8000 + o + 0x20:
            ps = [blk[o + 2 * i] | blk[o + 2 * i + 1] << 8 for i in range(16)]
            if all(0x8000 + o <= x < 0xC000 for x in ps) and ps == sorted(ps):
                tabs.append(o)
    return tabs


def parse_record(blk, o):
    start = o
    while blk[o] != 0xFF:
        o += 1
    header = list(blk[start:o])
    o += 1
    strings = []
    for _ in FIELDS:
        e = blk.index(0, o)
        strings.append(text(blk[o:e]))
        o = e + 1
    rec = {'address': '$%04X' % (0x8000 + start), 'header': ' '.join('%02X' % h for h in header)}
    rec.update(decode_header(header))
    rec.update(dict(zip(FIELDS, strings)))
    return rec


# Talk topics (talk menu at $35D6, names at $360D): the number is also the
# string shown (1 = name ... 4 = health); asking keyword 1/2 (topics 10/11)
# shows response 1/2 (topics 5/6).
TOPICS = {0: 'bye', 1: 'name', 2: 'look', 3: 'job', 4: 'health', 5: 'keyword1', 6: 'keyword2',
          7: 'join', 8: 'give'}


def decode_header(h):
    """Header bytes 0-3 are copied to $C9A9 by talk_setup ($34DD): 0 = topic
    after which the yes/no question is asked, 2 = turn-away value (compared
    with a random number), 1 and 3 not decoded. Then (after, unlocks) pairs
    up to $FF (talk_topics, $3578): once topic `after` has been asked (flag
    $C580 + topic), set flag `unlocks`; $85/$86 make keyword 1/2 appear in the
    menu."""
    out = {'question_after': TOPICS.get(h[0], h[0]), 'turn_away': h[2],
           'unknown_1': h[1], 'unknown_3': h[3], 'unlocks': []}
    for a, v in zip(h[4::2], h[5::2]):
        after = TOPICS.get(a - 0x80, '$%02X' % a) if a >= 0x80 else '$%02X' % a
        what = {0x85: 'keyword1', 0x86: 'keyword2'}.get(v, '$%02X' % v)
        out['unlocks'].append([after, what])
    return out


def printable_strings(blk, minlen=4):
    out, o = [], 0
    while o < BANK:
        e = o
        while e < BANK and (32 <= blk[e] < 127 or blk[e] == 1):
            e += 1
        if e < BANK and blk[e] == 0 and e - o >= minlen:
            out.append(('$%04X' % (0x8000 + o), text(blk[o:e])))
            o = e + 1
        else:
            o = max(e, o) + 1
    return out


NAME_GROUPS = [  # index ranges of the bank 16 name table ($8302)
    ('monsters', 0, 36), ('weapons', 36, 52), ('armour', 52, 60), ('weapon_abbrev', 60, 76),
    ('classes', 76, 84), ('people', 84, 92), ('reagents', 92, 100), ('spells', 100, 126),
    ('places', 126, 150), ('virtues', 150, 158), ('misc', 158, 159), ('stone_colours', 159, 167)]


def name_table(rom):
    blk = rom[16 * BANK:17 * BANK]
    ptrs = [blk[0x302 + 2 * i] | blk[0x303 + 2 * i] << 8 for i in range(167)]
    names = []
    for p in ptrs:
        a = p - 0x8000
        names.append(blk[a:blk.index(0, a)].decode('latin1'))
    return {g: names[a:b] for g, a, b in NAME_GROUPS}


def pointer_strings(rom, bank, table, count):
    blk = rom[bank * BANK:(bank + 1) * BANK]
    out = []
    for i in range(count):
        p = blk[table - 0x8000 + 2 * i] | blk[table - 0x8000 + 2 * i + 1] << 8
        a = p - 0x8000
        out.append(text(blk[a:blk.index(0, a)]))
    return out


def messages(rom):
    """Game messages: bank 26 ($1A), 207 pointers at $8000, printed by
    print_message ($5582) with A = index. Entries 118 onward are also used as
    their own table at $80EC (Lord British, Hawkwind)."""
    return pointer_strings(rom, 26, 0x8000, 207)


def shop_texts(rom):
    """Shop and service texts: bank 22, 10 groups. $8000 holds a pointer per
    group to that group's own pointer list; a list runs up to the next one."""
    blk = rom[22 * BANK:23 * BANK]
    word = lambda a: blk[a - 0x8000] | blk[a - 0x7FFF] << 8
    heads = [word(0x8000 + 2 * i) for i in range(10)]
    groups = []
    for i, h in enumerate(heads):
        end = min([x for x in heads if x > h] + [0xC000])
        n, lowest = 0, 0xC000
        while h + 2 * n < min(end, lowest):   # a list ends where its strings begin
            lowest = min(lowest, word(h + 2 * n))
            n += 1
        groups.append(pointer_strings(rom, 22, h, n))
    return groups


def main():
    rom = open(sys.argv[1], 'rb').read()
    out = os.path.join(sys.argv[2], 'text')
    os.makedirs(out, exist_ok=True)
    tables = []
    for b in (27, 28, 29, 30):
        blk = rom[b * BANK:(b + 1) * BANK]
        for t in dialogue_tables(blk):
            people = []
            for i in range(16):
                p = blk[t + 2 * i] | blk[t + 2 * i + 1] << 8
                people.append(parse_record(blk, p - 0x8000))
            tables.append({'bank': b, 'table': '$%04X' % (0x8000 + t), 'people': people})
    with open(os.path.join(out, 'dialogue.json'), 'w', encoding='utf-8', newline='\n') as f:
        json.dump(tables, f, indent=1, ensure_ascii=False)
    with open(os.path.join(out, 'dialogue.txt'), 'w', encoding='utf-8', newline='\n') as f:
        for t in tables:
            f.write('=== bank %d table %s ===\n\n' % (t['bank'], t['table']))
            for p in t['people']:
                f.write('%s (%s) [header %s]\n' % (p['name'], p['look'], p['header']))
                f.write('  asks the question after: %s; turn-away %d; unlocks: %s\n' % (
                    p['question_after'], p['turn_away'],
                    ', '.join('%s -> %s' % tuple(u) for u in p['unlocks']) or 'nothing'))
                for k in FIELDS[2:]:
                    f.write('  %-9s %s\n' % (k + ':', p[k].replace('\n', ' / ')))
                f.write('\n')
    with open(os.path.join(out, 'names.json'), 'w', encoding='utf-8', newline='\n') as f:
        json.dump(name_table(rom), f, indent=1)
    with open(os.path.join(out, 'messages.json'), 'w', encoding='utf-8', newline='\n') as f:
        json.dump(messages(rom), f, indent=1, ensure_ascii=False)
    with open(os.path.join(out, 'shops.json'), 'w', encoding='utf-8', newline='\n') as f:
        json.dump(shop_texts(rom), f, indent=1, ensure_ascii=False)
    print('%d dialogue tables, %d people' % (len(tables), sum(len(t['people']) for t in tables)))


if __name__ == '__main__':
    main()
