"""Bank-aware recursive-descent disassembler for the Ultima IV SMS ROM.

Writes a WLA-DX project that reassembles to the original ROM byte-for-byte:
    python disasm.py <rom> <outdir>

Memory model (Sega mapper, set up at $0042): bank 0 in slot 0 ($0000),
bank 1 in slot 1 ($4000), banks 2..31 paged into slot 2 ($8000) through a
write to $FFFF.  $DFFF holds a RAM shadow of the slot-2 bank.  Code is traced
from the reset/interrupt vectors; the slot-2 bank is tracked through
"ld a,N / ld ($FFFF),a" so calls into $8000-$BFFF land in the right bank.
Bytes never reached as code are emitted as .db, so the rebuild is exact
whatever the coverage.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from z80dec import decode, h8, h16  # noqa: E402

BANK = 0x4000
UNK, START, CONT = 0, 1, 2

# RST $18 / RST $20: "ld hl,table / rst" dispatches through a word table
# indexed by A (handlers at $0083 and $008E).
DISPATCH_RST = (0x18, 0x20)
# Writes A to the slot-2 / slot-1 mapper registers
MAPPER_SLOT2 = 0xFFFF


def home(bank):
    return 0 if bank == 0 else (BANK if bank == 1 else 2 * BANK)


def slot_of(bank):
    return 0 if bank == 0 else (1 if bank == 1 else 2)


class Disasm:
    def __init__(self, rom):
        self.rom = rom
        self.nbanks = len(rom) // BANK
        self.flags = [bytearray(BANK) for _ in range(self.nbanks)]
        self.insn = {}      # (bank, off) -> Insn
        self.ctx = {}       # (bank, off) -> slot-2 bank in force (or None)
        self.labels = {}    # (bank, off) -> 'F' (call target) / 'L' (jump)
        self.tables = {}    # (bank, off) -> (kind 'word'/'far', entries, slot-2 context)
        self.names = {}     # (bank, off) -> name from hints.txt
        self.switch_fns = set()  # helpers taking "ld hl,table": (key, word)... $FF
        self.inline_fns = set()  # helpers followed by inline zero-terminated text
        self.ram_names = {}      # work-RAM address -> name (hints: ram)
        self.incbin = True       # data as .INCBIN from the player's ROM (False: inline .db)
        self.natives = {}        # (bank, off) -> C Native object replacing the routine
        self.ram_notes = {}      # name -> comment
        self.strings = {}        # (bank, off) -> length of inline text incl. terminator
        self.queue = []
        self.seen = set()
        self.cov = None     # emulator coverage (bit0 opcode, bit1 operand, bit2 data read)

    def is_data(self, bank, off):
        if self.cov is None:
            return False
        v = self.cov[bank * BANK + off]
        return (v & 4) and not (v & 3)

    def executed(self, bank, off):
        return self.cov is not None and (self.cov[bank * BANK + off] & 1)

    # -- address helpers -------------------------------------------------
    def resolve(self, addr, slot2):
        """CPU address -> (bank, offset), or None when not ROM / unknown."""
        if addr < BANK:
            return 0, addr
        if addr < 2 * BANK:
            return 1, addr - BANK
        if addr < 3 * BANK and slot2 is not None and 2 <= slot2 < self.nbanks:
            return slot2, addr - 2 * BANK
        if addr < 3 * BANK and slot2 is None and self.cov is not None:
            # unknown page: accept the target if exactly one bank ran code there
            hits = [b for b in range(2, self.nbanks) if self.executed(b, addr - 2 * BANK)]
            if len(hits) == 1:
                return hits[0], addr - 2 * BANK
        return None

    def byte(self, bank, off):
        return self.rom[bank * BANK + off]

    def mem_for(self, bank):
        base = home(bank)
        rom = self.rom

        def m(a):
            off = a - base
            if 0 <= off < BANK:
                return rom[bank * BANK + off]
            return 0xFF  # outside this bank: forces a stop at boundary
        return m

    # -- tracing ------------------------------------------------------------
    def add(self, bank, off, slot2, kind):
        if kind and (bank, off) not in self.labels or kind == 'F':
            self.labels[(bank, off)] = kind
        if bank >= 2:
            slot2 = bank
        key = (bank, off, slot2)
        if key not in self.seen:
            self.seen.add(key)
            self.queue.append(key)

    def run(self):
        while self.queue:
            bank, off, slot2 = self.queue.pop()
            self.trace(bank, off, slot2)

    def trace(self, bank, off, slot2):
        mem = self.mem_for(bank)
        base = home(bank)
        a_const = None
        prev = None
        fl = self.flags[bank]
        while 0 <= off < BANK:
            if fl[off] == START:
                return  # already decoded from here on
            if fl[off] == CONT:
                return  # jumps into the middle of an instruction: leave it
            ins = decode(mem, base + off)
            if off + ins.length > BANK:
                return
            if any(fl[off + k] for k in range(ins.length)):
                return
            if not self.executed(bank, off) and self.cov is not None and (
                    self.cov[bank * BANK + off] & 2 or
                    any(self.cov[bank * BANK + off + k] & 1 for k in range(1, ins.length))):
                return  # would swallow an instruction the emulator saw execute
            if not self.executed(bank, off) and any(self.is_data(bank, off + k) for k in range(ins.length)):
                return  # the game reads these bytes as data
            if ins.undoc and ins.text.startswith('.db') and ins.length == 1:
                return  # stray prefix: almost certainly data
            fl[off] = START
            for k in range(1, ins.length):
                fl[off + k] = CONT
            self.insn[(bank, off)] = ins
            self.ctx[(bank, off)] = slot2
            t = ins.text
            # constant tracking for the mapper
            if t.startswith('ld a,$'):
                a_const = int(t[6:], 16)
            elif t in ('xor a', 'sub a'):
                a_const = 0
            elif t == 'ld ($FFFF),a':
                slot2 = a_const
            elif _writes_a(t):
                a_const = None
            k = ins.kind
            nxt = off + ins.length
            if k in ('jp', 'jr', 'call'):
                tgt = self.resolve(ins.target, slot2)
                if tgt:
                    self.add(tgt[0], tgt[1], slot2, 'F' if k == 'call' else 'L')
                if ins.target in DISPATCH_RST and k != 'jr' and prev is not None \
                        and prev.text.startswith('ld hl,$'):
                    # "ld hl,table / jp $0018" dispatches like rst $18
                    self.dispatch_table(prev.imm16, slot2, ins.target)
                if k == 'call':
                    a_const = None
                    if tgt in self.switch_fns and prev is not None and prev.text.startswith('ld hl,$'):
                        self.switch_table(prev.imm16, slot2)
                    if tgt in self.inline_fns and not ins.cond:
                        # the callee prints the zero-terminated text after the call
                        end = nxt
                        while end < BANK and self.byte(bank, end) != 0:
                            end += 1
                        if end >= BANK or any(fl[k] for k in range(nxt, end + 1)):
                            return
                        self.strings[(bank, nxt)] = end + 1 - nxt
                        for k in range(nxt, end + 1):
                            fl[k] = CONT
                        nxt = end + 1
                elif not ins.cond:
                    return
            elif k == 'rst':
                if ins.target == 0:
                    return
                if ins.target in DISPATCH_RST and prev is not None \
                        and prev.text.startswith('ld hl,$'):
                    self.dispatch_table(prev.imm16, slot2, ins.target)
                a_const = None
            elif k == 'ret' and not ins.cond:
                return
            elif k == 'jpind':
                return
            prev = ins
            off = nxt

    def dispatch_table(self, addr, slot2, rst):
        loc = self.resolve(addr, slot2)
        if loc is None:
            return
        bank, off = loc
        if (bank, off) in self.tables:
            return
        lo, hi = home(bank), home(bank) + BANK
        n = 0
        limit = 128 if rst == 0x18 else 256
        entries = []
        while n < limit and off + 2 * n + 1 < BANK:
            o = off + 2 * n
            if n and ((bank, o) in self.labels or self.flags[bank][o]
                      or self.executed(bank, o) or self.executed(bank, o + 1)):
                break
            v = self.byte(bank, o) | (self.byte(bank, o + 1) << 8)
            tl = self.resolve(v, slot2 if bank < 2 else bank)
            if tl is None or v < 0x40:
                break
            if self.flags[tl[0]][tl[1]] == CONT:
                break
            entries.append(tl)
            n += 1
        if n == 0:
            return
        self.reserve_table(bank, off, 'word', n, slot2 if bank < 2 else bank)
        for b, o in entries:
            self.add(b, o, slot2, 'F')

    def switch_table(self, addr, slot2):
        loc = self.resolve(addr, slot2)
        if loc is None or loc in self.tables:
            return
        bank, off = loc
        ctx = slot2 if bank < 2 else bank
        n, entries = 0, []
        while off + 3 * n < BANK and self.byte(bank, off + 3 * n) != 0xFF:
            e = off + 3 * n
            if e + 2 >= BANK or any(self.flags[bank][e + k] for k in range(3)):
                return
            v = self.byte(bank, e + 1) | (self.byte(bank, e + 2) << 8)
            t = self.resolve(v, ctx)
            if t is None or n > 64:
                return
            entries.append(t)
            n += 1
        if off + 3 * n >= BANK or any(self.flags[bank][off + k] for k in range(3 * n + 1)):
            return
        self.reserve_table(bank, off, 'switch', n, ctx)
        for b, o in entries:
            self.add(b, o, ctx, 'L')

    def reserve_table(self, bank, off, kind, n, ctx):
        self.tables[(bank, off)] = (kind, n, ctx)
        self.labels.setdefault((bank, off), 'T')
        size = n * (3 if kind in ('far', 'switch') else 2) + (1 if kind == 'switch' else 0)
        for k in range(size):
            self.flags[bank][off + k] = CONT  # reserve as data

    def load_hints(self, path):
        """Hand-written knowledge: code entries, pointer tables, names."""
        def loc(t):
            b, a = t.split(':')
            b, a = int(b, 16), int(a, 16)
            return b, a - home(b)
        for raw in open(path):
            p = raw.split('#')[0].split()
            if not p:
                continue
            if p[0] == 'ram':
                # ram <addr> <name> [comment]: a named work-RAM variable
                self.ram_names[int(p[1], 16)] = p[2]
                self.ram_notes[p[2]] = raw.split('#', 1)[1].strip() if '#' in raw else ''
                continue
            cmd, (b, o) = p[0], loc(p[1])
            if cmd == 'code':
                self.add(b, o, b if b >= 2 else None, 'F')
            elif cmd == 'jumpvec':
                for k in range(int(p[2])):
                    self.add(b, o + 3 * k, b if b >= 2 else None, 'F')
            elif cmd == 'wordtable':
                n = int(p[2])
                ctx = int(p[3], 16) if len(p) > 3 else (b if b >= 2 else None)
                self.reserve_table(b, o, 'word', n, ctx)
                for k in range(n):
                    v = self.byte(b, o + 2 * k) | (self.byte(b, o + 2 * k + 1) << 8)
                    t = self.resolve(v, ctx)
                    if t:
                        self.add(t[0], t[1], ctx, 'F')
            elif cmd == 'fartable':
                n = int(p[2])
                self.reserve_table(b, o, 'far', n, None)
                for k in range(n):
                    e = o + 3 * k
                    tb = self.byte(b, e)
                    v = self.byte(b, e + 1) | (self.byte(b, e + 2) << 8)
                    t = self.resolve(v, tb)
                    if v and t:
                        self.add(t[0], t[1], tb, 'F')
            elif cmd == 'native':
                self.natives[(b, o)] = p[2]
            elif cmd == 'inlinestr':
                self.inline_fns.add((b, o))
            elif cmd == 'switchfn':
                self.switch_fns.add((b, o))
            elif cmd == 'name':
                self.names[(b, o)] = p[2]
                self.labels.setdefault((b, o), 'L')
            else:
                raise ValueError('hints: ' + raw)

    def probe(self, banks):
        """Find code nothing reached: a run of unknown bytes that decodes
        cleanly up to a ret/jp and calls known functions or touches work RAM."""
        found = 0
        for bank in banks:
            mem = self.mem_for(bank)
            base = home(bank)
            fl = self.flags[bank]
            off = 0
            while off < BANK:
                if fl[off] or self.is_data(bank, off) or (off and fl[off - 1] == UNK
                                                          and not self._after_end(bank, off)):
                    off += 1
                    continue
                score, p, ok = 0, off, False
                for _ in range(200):
                    if p >= BANK or fl[p] or self.is_data(bank, p):
                        break
                    ins = decode(mem, base + p)
                    if ins.undoc or ins.kind == 'halt' or p + ins.length > BANK or                             (ins.kind == 'rst' and ins.target in (0, 0x38)):
                        break
                    if any(fl[p + k] or self.is_data(bank, p + k) for k in range(ins.length)):
                        break
                    if ins.kind in ('call', 'jp') and ins.target is not None:
                        t = self.resolve(ins.target, bank if bank >= 2 else None)
                        if t and self.flags[t[0]][t[1]] == START and t in self.labels:
                            score += 2
                    if ins.imm16_kind == 'addr' and 0xC000 <= ins.imm16 < 0xE000:
                        score += 1
                    p += ins.length
                    if (ins.kind == 'ret' and not ins.cond) or ins.kind == 'jpind' or                             (ins.kind in ('jp', 'jr') and not ins.cond):
                        ok = True
                        break
                if ok and score >= 3:
                    self.add(bank, off, bank if bank >= 2 else None, 'P')
                    self.run()
                    found += 1
                    while off < BANK and fl[off]:
                        off += 1
                else:
                    off += 1
        return found

    def _after_end(self, bank, off):
        # candidate starts follow data/code boundaries, not the middle of junk
        return True

    # -- output -------------------------------------------------------------
    def label(self, bank, off):
        kind = self.labels.get((bank, off))
        if kind is None:
            return None
        if (bank, off) in self.names:
            return self.names[(bank, off)]
        pre = {'F': 'fn', 'L': 'loc', 'T': 'tbl', 'P': 'sub'}[kind]
        return '%s_%02X_%04X' % (pre, bank, home(bank) + off)

    def target_label(self, addr, slot2):
        loc = self.resolve(addr, slot2)
        if loc and self.flags[loc[0]][loc[1]] == START and (loc in self.labels):
            return self.label(*loc)
        return None

    def emit_bank(self, bank):
        out = ['; Bank %d (ROM $%05X-$%05X), mapped at $%04X' %
               (bank, bank * BANK, bank * BANK + BANK - 1, home(bank)),
               '.BANK %d SLOT %d' % (bank, slot_of(bank)), '.ORG $0000', '']
        base = home(bank)
        fl = self.flags[bank]
        off = 0
        data = []
        data_start = [0]

        def incbin(start, n, what='data'):
            # the game's own bytes are not in the source: they are read from
            # the player's ROM, split into banks by tools/setup.py
            out.append('\t.INCBIN "bank%02d.bin" SKIP $%04X READ %d\t; %s, %d bytes'
                       % (bank, start, n, what, n))

        def flush():
            if self.incbin:
                if data:
                    incbin(data_start[0], len(data))
                data.clear()
                return
            for i in range(0, len(data), 16):
                chunk = data[i:i + 16]
                out.append('\t.db ' + ','.join(h8(b) for b in chunk))
            data.clear()

        while off < BANK:
            lab = self.label(bank, off) if (bank, off) in self.labels else None
            if lab and (fl[off] == START or (bank, off) in self.tables
                        or fl[off] == UNK):
                flush()
                out.append('')
                out.append(lab + ':')
            if (bank, off) in self.strings:
                flush()
                n = self.strings[(bank, off)]
                if self.incbin:
                    incbin(off, n, 'text')
                else:
                    out.append('\t.db ' + text_db(self.rom[bank * BANK + off: bank * BANK + off + n]))
                off += n
                continue
            if (bank, off) in self.tables:
                flush()
                kind, n, ctx = self.tables[(bank, off)]
                for i in range(n):
                    if kind == 'switch':
                        e = off + 3 * i
                        v = self.byte(bank, e + 1) | (self.byte(bank, e + 2) << 8)
                        tl = self.target_label(v, ctx)
                        out.append('\t.db %s\n\t.dw %s' % (h8(self.byte(bank, e)), tl or h16(v)))
                    elif kind == 'far':
                        e = off + 3 * i
                        tb = self.byte(bank, e)
                        v = self.byte(bank, e + 1) | (self.byte(bank, e + 2) << 8)
                        tl = self.target_label(v, tb) if v else None
                        out.append('\t.db %s\n\t.dw %s' % (h8(tb), tl or h16(v)))
                    else:
                        v = self.byte(bank, off + 2 * i) | (self.byte(bank, off + 2 * i + 1) << 8)
                        tl = self.target_label(v, ctx)
                        out.append('\t.dw ' + (tl or h16(v)))
                off += n * (3 if kind in ('far', 'switch') else 2)
                if kind == 'switch':
                    out.append('\t.db $FF')
                    off += 1
                continue
            if fl[off] == START:
                flush()
                ins = self.insn[(bank, off)]
                raw = self.rom[bank * BANK + off: bank * BANK + off + ins.length]
                out.append(self.render(bank, off, ins, raw))
                off += ins.length
                continue
            if not data:
                data_start[0] = off
            data.append(self.byte(bank, off))
            if len(data) == 16 and not self.incbin:
                flush()
            off += 1
        flush()
        return '\n'.join(out) + '\n'

    def render(self, bank, off, ins, raw):
        comment = '\t; $%04X' % (home(bank) + off)
        if ins.undoc:
            return '\t.db ' + ','.join(h8(b) for b in raw) + '\t; ' + ins.text + ' (undocumented)'
        slot2 = self.ctx.get((bank, off))
        if ins.kind == 'jr':
            tl = self.target_label(ins.target, bank if bank >= 2 else slot2)
            loc = self.resolve(ins.target, slot2 if bank < 2 else bank)
            if tl and loc and loc[0] == bank:
                return '\t' + ins.fmt.replace('{T}', tl) + comment
            return '\t' + ins.fmt.replace('{T}', h8(ins.disp)) + comment
        if ins.kind in ('jp', 'call') and ins.target is not None:
            tl = self.target_label(ins.target, slot2)
            if tl:
                return '\t' + ins.fmt.replace('{T}', tl) + comment
        if ins.fmt and ins.imm16 is not None and ins.imm16 in self.ram_names:
            return '\t' + ins.fmt.replace('{T}', self.ram_names[ins.imm16]) + comment
        return '\t' + ins.text + comment

    def ram_defines(self):
        lines = []
        for a, n in sorted(self.ram_names.items()):
            note = self.ram_notes.get(n)
            lines.append('.DEFINE %-24s $%04X%s' % (n, a, ('\t; ' + note) if note else ''))
        return '\n'.join(lines)


def text_db(raw):
    """WLA-DX .db operands for game text: printable runs quoted, the rest hex."""
    parts, run = [], ''
    for b in raw:
        if 0x20 <= b < 0x7F and chr(b) not in '"\\':
            run += chr(b)
            continue
        if run:
            parts.append('"%s"' % run)
            run = ''
        parts.append(h8(b))
    if run:
        parts.append('"%s"' % run)
    return ','.join(parts)


_A_WRITERS = ('ld a,', 'in a,', 'pop af', 'add a,', 'adc a,', 'sub ', 'sbc a,',
              'and ', 'xor ', 'or ', 'inc a', 'dec a', 'rla', 'rra', 'rlca',
              'rrca', 'cpl', 'neg', 'daa', "ex af,af'", 'rlc a', 'rrc a',
              'rl a', 'rr a', 'sla a', 'sra a', 'srl a', 'rld', 'rrd',
              'res ', 'set ')


def _writes_a(t):
    if t.startswith(('res ', 'set ')):
        return t.endswith(',a')
    return t.startswith(_A_WRITERS)


MAIN = """; Ultima IV: Quest of the Avatar (Sega Master System)
; Disassembly that rebuilds the original ROM byte-for-byte with WLA-DX.
; Generated by tools/disasm.py; see README.md.

.MEMORYMAP
SLOTSIZE $4000
DEFAULTSLOT 0
SLOT 0 $0000
SLOT 1 $4000
SLOT 2 $8000
.ENDME

.ROMBANKMAP
BANKSTOTAL {n}
BANKSIZE $4000
BANKS {n}
.ENDRO

.EMPTYFILL $FF

; work RAM variables (tools/hints.txt)
{ram}

{includes}
"""


def analyze(rom, cov_path=None, log=print):
    """Trace the ROM (hints, emulator coverage, vectors, probing) -> Disasm."""
    d = Disasm(rom)
    hints = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'hints.txt')
    if os.path.exists(hints):
        d.load_hints(hints)
    if cov_path:
        d.cov = open(cov_path, 'rb').read()
        # instructions seen executing are certain; trace them first
        # added high to low so the queue (last in, first out) traces them in
        # address order: each linear run then keeps its context (e.g. the
        # "ld hl,table" before a dispatch)
        for o in range(len(d.cov) - 1, -1, -1):
            if d.cov[o] & 1:
                b = o // BANK
                d.add(b, o % BANK, b if b >= 2 else None, None)
        d.run()
    for v in (0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x66):
        d.add(0, v, None, 'F')
    d.run()
    while True:
        n = d.probe(range(4))
        log('probe: %d more routines' % n)
        if not n:
            break
    return d


def main():
    # --inline-data: write the game's data into the source as .db/.db "text"
    # (handy for reading locally; never commit that output)
    inline = '--inline-data' in sys.argv
    args = [a for a in sys.argv[1:] if a != '--inline-data']
    rom = open(args[0], 'rb').read()
    outdir = args[1]
    d = analyze(rom, args[2] if len(args) > 2 else None)
    d.incbin = not inline
    os.makedirs(os.path.join(outdir, 'banks'), exist_ok=True)
    inc = []
    for b in range(d.nbanks):
        name = 'banks/bank%02d.asm' % b
        with open(os.path.join(outdir, name), 'w', newline='\n') as f:
            f.write(d.emit_bank(b))
        inc.append('.INCLUDE "%s"' % name)
    with open(os.path.join(outdir, 'u4.asm'), 'w', newline='\n') as f:
        f.write(MAIN.format(n=d.nbanks, ram=d.ram_defines(), includes='\n'.join(inc)))
    # cross-bank calls/jumps from the fixed slots into a paged bank (for Ghidra)
    with open(os.path.join(outdir, 'xrefs.txt'), 'w', newline='\n') as f:
        f.write('# from_bank:from_addr kind to_bank:to_addr\n')
        for (b, o), ins in sorted(d.insn.items()):
            if b < 2 and ins.kind in ('call', 'jp') and ins.target is not None and 0x8000 <= ins.target < 0xC000:
                loc = d.resolve(ins.target, d.ctx.get((b, o)))
                if loc:
                    f.write('%02X:%04X %s %02X:%04X\n' % (b, home(b) + o, ins.kind, loc[0], ins.target))
    code = sum(fl.count(START) + fl.count(CONT) for fl in d.flags)
    print('instructions: %d, code+table bytes: %d of %d (%.1f%%), labels: %d, tables: %d'
          % (len(d.insn), code, len(rom), 100.0 * code / len(rom), len(d.labels), len(d.tables)))
    per = ['%d:%.0f%%' % (b, 100.0 * (d.flags[b].count(START) + d.flags[b].count(CONT)) / BANK)
           for b in range(d.nbanks)]
    print(' '.join(per))


if __name__ == '__main__':
    main()
