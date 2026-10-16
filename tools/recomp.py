"""Static recompiler: turns the traced Z80 code into C (pc/gen/recomp.c).

    python recomp.py <rom> <coverage.bin> <out.c>

Every traced instruction becomes a labelled C statement block inside one
function, recomp_run(), which runs until cpu.cycles reaches cpu.budget.
Straight-line code falls through, branches inside the same bank are direct
gotos, and anything else (returns, jumps through registers, calls into a
paged bank) goes through a dispatcher that looks the address up in per-bank
label tables and falls back to the interpreter for untranslated addresses.
The generated C keeps one comment per instruction with the original
assembly, so it can be read against src/banks/*.asm.
"""
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import disasm  # noqa: E402
from disasm import BANK, START, home  # noqa: E402

CYC = [
    4, 10, 7, 6, 4, 4, 7, 4, 4, 11, 7, 6, 4, 4, 7, 4,
    8, 10, 7, 6, 4, 4, 7, 4, 12, 11, 7, 6, 4, 4, 7, 4,
    7, 10, 16, 6, 4, 4, 7, 4, 7, 11, 16, 6, 4, 4, 7, 4,
    7, 10, 13, 6, 11, 11, 10, 4, 7, 11, 13, 6, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    7, 7, 7, 7, 7, 7, 4, 7, 4, 4, 4, 4, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    5, 10, 10, 10, 10, 11, 7, 11, 5, 10, 10, 0, 10, 17, 7, 11,
    5, 10, 10, 11, 10, 11, 7, 11, 5, 4, 10, 11, 10, 0, 7, 11,
    5, 10, 10, 19, 10, 11, 7, 11, 5, 4, 10, 4, 10, 0, 7, 11,
    5, 10, 10, 4, 10, 11, 7, 11, 5, 6, 10, 4, 10, 0, 7, 11,
]


def cycles(raw):
    """Base T-states (branch not taken) for an encoded instruction."""
    op = raw[0]
    if op in (0xDD, 0xFD):
        op2 = raw[1]
        if op2 == 0xCB:
            return 20 if (raw[3] >> 6) == 1 else 23
        x, y, z = op2 >> 6, (op2 >> 3) & 7, op2 & 7
        mem = (x == 1 and (y == 6 or z == 6) and op2 != 0x76) or (x == 2 and z == 6) or \
            (x == 0 and z in (4, 5, 6) and y == 6)
        if mem:
            return 23 if (x == 0 and z != 6) else 19
        return CYC[op2] + 4
    if op == 0xCB:
        x, z = raw[1] >> 6, raw[1] & 7
        if z == 6:
            return 12 if x == 1 else 15
        return 8
    if op == 0xED:
        op2 = raw[1]
        x, y, z = op2 >> 6, (op2 >> 3) & 7, op2 & 7
        if x == 1:
            return {0: 12, 1: 12, 2: 15, 3: 20, 4: 8, 5: 14, 6: 8}.get(z, 18 if y in (4, 5) else 9)
        if x == 2 and y >= 4 and z <= 3:
            return 16
        return 8
    return CYC[op]


R8 = {'a': 'cpu.a', 'b': 'cpu.b', 'c': 'cpu.c', 'd': 'cpu.d', 'e': 'cpu.e', 'h': 'cpu.h', 'l': 'cpu.l'}
R16 = {'bc': 'BC', 'de': 'DE', 'hl': 'HL', 'sp': 'cpu.sp', 'ix': 'cpu.ix', 'iy': 'cpu.iy',
       'af': '(uint16_t)((cpu.a << 8) | cpu.f)'}
SET16 = {'bc': 'SET_BC(%s)', 'de': 'SET_DE(%s)', 'hl': 'SET_HL(%s)', 'sp': 'cpu.sp = (uint16_t)(%s)',
         'ix': 'cpu.ix = (uint16_t)(%s)', 'iy': 'cpu.iy = (uint16_t)(%s)',
         'af': '{ uint16_t _v = (%s); cpu.a = _v >> 8; cpu.f = (uint8_t)_v; }'}
COND = {'nz': '!(cpu.f & FZ)', 'z': '(cpu.f & FZ)', 'nc': '!(cpu.f & FC)', 'c': '(cpu.f & FC)',
        'po': '!(cpu.f & FP)', 'pe': '(cpu.f & FP)', 'p': '!(cpu.f & FS)', 'm': '(cpu.f & FS)'}
ALU = {'add': 0, 'adc': 1, 'sub': 2, 'sbc': 3, 'and': 4, 'xor': 5, 'or': 6, 'cp': 7}
ROT = {'rlc': 0, 'rrc': 1, 'rl': 2, 'rr': 3, 'sla': 4, 'sra': 5, 'sll': 6, 'srl': 7}
BLOCK = {'ldi': (4, 0), 'cpi': (4, 1), 'ini': (4, 2), 'outi': (4, 3),
         'ldd': (5, 0), 'cpd': (5, 1), 'ind': (5, 2), 'outd': (5, 3),
         'ldir': (6, 0), 'cpir': (6, 1), 'inir': (6, 2), 'otir': (6, 3),
         'lddr': (7, 0), 'cpdr': (7, 1), 'indr': (7, 2), 'otdr': (7, 3)}

IDX_RE = re.compile(r'^\((ix|iy)([+-])\$([0-9A-F]{2})\)$')
ABS_RE = re.compile(r'^\(\$([0-9A-F]{4})\)$')
IMM_RE = re.compile(r'^\$([0-9A-F]+)$')


def addr_of(op):
    """C expression for the address of a memory operand, or None."""
    if op == '(hl)':
        return 'HL'
    if op == '(bc)':
        return 'BC'
    if op == '(de)':
        return 'DE'
    m = IDX_RE.match(op)
    if m:
        return '(uint16_t)(cpu.%s %s 0x%s)' % (m.group(1), m.group(2), m.group(3))
    m = ABS_RE.match(op)
    if m:
        return '0x' + m.group(1)
    return None


HALF = {'ixh': ('ix', 8), 'ixl': ('ix', 0), 'iyh': ('iy', 8), 'iyl': ('iy', 0)}


def get8(op):
    if op in R8:
        return R8[op]
    if op in HALF:
        r, sh = HALF[op]
        return '(uint8_t)(cpu.%s >> %d)' % (r, sh)
    a = addr_of(op)
    if a:
        return 'mem_read(%s)' % a
    m = IMM_RE.match(op)
    if m:
        return '0x' + m.group(1)
    raise ValueError(op)


def set8(op, v):
    if op in R8:
        return '%s = (uint8_t)(%s);' % (R8[op], v)
    if op in HALF:
        r, sh = HALF[op]
        keep = '0x00FF' if sh else '0xFF00'
        return 'cpu.%s = (uint16_t)((cpu.%s & %s) | ((uint8_t)(%s) << %d));' % (r, r, keep, v, sh)
    a = addr_of(op)
    if a:
        return 'mem_write(%s, (uint8_t)(%s));' % (a, v)
    raise ValueError(op)


def translate(text):
    """C statements for one non-branching instruction (branches are handled by the caller)."""
    mn, _, rest = text.partition(' ')
    ops = [o.strip() for o in rest.split(',')] if rest else []
    if mn == 'nop':
        return ''
    if mn == 'ld':
        d, s = ops
        if d in ('i', 'r') or s in ('i', 'r'):
            if d == 'i':
                return 'cpu.i = cpu.a;'
            if d == 'r':
                return 'cpu.r = cpu.a;'
            reg = 'cpu.i' if s == 'i' else 'cpu.r'
            return 'cpu.a = %s; cpu.f = (cpu.f & FC) | z80_sz[cpu.a] | (cpu.iff2 ? FP : 0);' % reg
        if d in R16 or (s in R16 and d not in R8):
            # 16-bit load
            if d in R16:
                m = ABS_RE.match(s)
                if m:
                    return SET16[d] % ('rd16(0x%s)' % m.group(1)) + ';'
                m = IMM_RE.match(s)
                if m:
                    return SET16[d] % ('0x' + m.group(1)) + ';'
                return SET16[d] % R16[s] + ';'
            return 'wr16(%s, %s);' % (addr_of(d), R16[s])
        return set8(d, get8(s))
    if mn == 'push':
        return 'push16(%s);' % R16[ops[0]]
    if mn == 'pop':
        return SET16[ops[0]] % 'pop16()' + ';'
    if mn == 'ex':
        if ops[0] == 'de':
            return '{ uint16_t _t = DE; SET_DE(HL); SET_HL(_t); }'
        if ops[0] == 'af':
            return 'op_ex_af();'
        r = ops[1]
        return '{ uint16_t _t = rd16(cpu.sp); wr16(cpu.sp, %s); %s; }' % (R16[r], SET16[r] % '_t')
    if mn == 'exx':
        return 'op_exx();'
    if mn in ALU:
        if len(ops) == 2 and ops[0] in ('hl', 'ix', 'iy'):
            d, s = ops
            if mn == 'add':
                return SET16[d] % ('op_add16(%s, %s)' % (R16[d], R16[s])) + ';'
            return 'op_%s16(%s);' % (mn, R16[s])
        v = ops[-1]
        return 'op_alu(%d, %s);' % (ALU[mn], get8(v))
    if mn in ('inc', 'dec'):
        o = ops[0]
        if o in R16:
            return SET16[o] % ('%s %s 1' % (R16[o], '+' if mn == 'inc' else '-')) + ';'
        a = addr_of(o)
        if a and o != '(hl)':
            return '{ uint16_t _a = %s; mem_write(_a, op_%s8(mem_read(_a))); }' % (a, mn)
        return set8(o, 'op_%s8(%s)' % (mn, get8(o)))
    if mn in ROT:
        o = ops[0]
        a = addr_of(o)
        if a:
            return '{ uint16_t _a = %s; mem_write(_a, op_rot(%d, mem_read(_a))); }' % (a, ROT[mn])
        return set8(o, 'op_rot(%d, %s)' % (ROT[mn], get8(o)))
    if mn == 'bit':
        n, o = ops
        a = addr_of(o)
        if a:
            return '{ uint16_t _a = %s; op_bit(%s, mem_read(_a)); cpu.f = (cpu.f & ~(FX | FY)) | ((_a >> 8) & (FX | FY)); }' % (a, n)
        return 'op_bit(%s, %s);' % (n, get8(o))
    if mn in ('res', 'set'):
        n, o = ops
        expr = ('_v & ~(1 << %s)' if mn == 'res' else '_v | (1 << %s)') % n
        a = addr_of(o)
        if a:
            return '{ uint16_t _a = %s; uint8_t _v = mem_read(_a); mem_write(_a, (uint8_t)(%s)); }' % (a, expr)
        return '{ uint8_t _v = %s; %s }' % (get8(o), set8(o, expr))
    simple = {'rlca': 'op_rlca();', 'rrca': 'op_rrca();', 'rla': 'op_rla();', 'rra': 'op_rra();',
              'daa': 'op_daa();', 'cpl': 'op_cpl();', 'scf': 'op_scf();', 'ccf': 'op_ccf();',
              'neg': 'op_neg();', 'di': 'cpu.iff1 = cpu.iff2 = 0;',
              'ei': 'cpu.iff1 = cpu.iff2 = 1; cpu.ei_delay = 1; cpu.budget = cpu.cycles;',
              'rrd': 'op_rrd();', 'rld': 'op_rld();'}
    if mn in simple:
        return simple[mn]
    if mn == 'im':
        return 'cpu.im = %s;' % ops[0]
    if mn == 'in':
        d, s = ops
        if s == '(c)':
            return set8(d, 'op_in_c(BC)')
        n = ABS_IO.match(s).group(1)
        return 'cpu.a = io_in((uint16_t)(0x%s | (cpu.a << 8)));' % n
    if mn == 'out':
        d, s = ops
        if d == '(c)':
            return 'io_out(BC, %s);' % get8(s)
        n = ABS_IO.match(d).group(1)
        return 'io_out((uint16_t)(0x%s | (cpu.a << 8)), cpu.a);' % n
    raise ValueError('cannot translate: ' + text)


ABS_IO = re.compile(r'^\(\$([0-9A-F]{2})\)$')


class Gen:
    def __init__(self, d):
        self.d = d
        self.out = []

    def lab(self, bank, off):
        return 'L_%02X_%04X' % (bank, home(bank) + off)

    def direct(self, bank, target):
        """Label for a jump from code in `bank` to CPU address `target` when
        the mapping is guaranteed (same bank, or bank 0 which never moves)."""
        if target < BANK:
            tb, to = 0, target
        elif bank >= 1 and home(bank) <= target < home(bank) + BANK:
            tb, to = bank, target - home(bank)
        else:
            return None
        if self.d.flags[tb][to] == START and (tb, to) in self.d.insn and not self.d.insn[(tb, to)].undoc:
            return self.lab(tb, to)
        return None

    def jump(self, bank, target_expr, target=None):
        if target is not None:
            lab = self.direct(bank, target)
            if lab:
                return 'cpu.pc = %s; if (cpu.cycles >= cpu.budget) goto leave; goto %s;' % (target_expr, lab)
        return 'cpu.pc = %s; goto dispatch;' % target_expr

    def emit_insn(self, bank, off, ins, raw, alone=False):
        pc = home(bank) + off
        nxt = (pc + ins.length) & 0xFFFF
        o = self.out
        o.append('%s: /* %04X  %s */' % (self.lab(bank, off), pc, ins.text))
        if (bank, off) in self.d.natives and not alone:
            o.append('	if (native_call(&%s)) goto dispatch;   /* rewritten in pc/native */' % self.d.natives[(bank, off)])
        if ins.undoc:
            o.append('\tcpu.pc = 0x%04X; z80_exec(); goto dispatch;' % pc)
            return
        cyc = cycles(raw)
        t = ins.text
        mn = t.split(' ')[0]
        k = ins.kind
        tgt = ins.target
        if k in ('jp', 'jr') or (k == 'jp' and ins.cond):
            texpr = '0x%04X' % tgt
            if ins.cond:
                c = t.split(' ')[1].split(',')[0]
                if mn == 'djnz':
                    o.append('\tcpu.cycles += %d;' % cyc)
                    o.append('\tif (--cpu.b) { cpu.cycles += 5; %s }' % self.jump(bank, texpr, tgt))
                else:
                    extra = 5 if mn == 'jr' else 0
                    o.append('\tcpu.cycles += %d;' % cyc)
                    o.append('\tif (%s) { cpu.cycles += %d; %s }' % (COND[c], extra, self.jump(bank, texpr, tgt)))
            else:
                o.append('\tcpu.cycles += %d; %s' % (cyc, self.jump(bank, texpr, tgt)))
                return
        elif k == 'call':
            texpr = '0x%04X' % tgt
            if ins.cond:
                c = t.split(' ')[1].split(',')[0]
                o.append('\tcpu.cycles += %d;' % cyc)
                o.append('\tif (%s) { cpu.cycles += 7; push16(0x%04X); %s }' % (COND[c], nxt, self.jump(bank, texpr, tgt)))
            else:
                o.append('\tcpu.cycles += %d; push16(0x%04X); %s' % (cyc, nxt, self.jump(bank, texpr, tgt)))
                return
        elif k == 'rst':
            o.append('\tcpu.cycles += %d; push16(0x%04X); %s' % (cyc, nxt, self.jump(bank, '0x%04X' % tgt, tgt)))
            return
        elif k == 'ret':
            if mn in ('reti', 'retn'):
                o.append('\tcpu.cycles += %d; cpu.iff1 = cpu.iff2; cpu.pc = pop16(); goto dispatch;' % cyc)
                return
            if ins.cond:
                c = t.split(' ')[1]
                o.append('\tcpu.cycles += %d;' % cyc)
                o.append('\tif (%s) { cpu.cycles += 6; cpu.pc = pop16(); goto dispatch; }' % COND[c])
            else:
                o.append('\tcpu.cycles += %d; cpu.pc = pop16(); goto dispatch;' % cyc)
                return
        elif k == 'jpind':
            r = t[4:-1]
            o.append('\tcpu.cycles += %d; cpu.pc = %s; goto dispatch;' % (cyc, R16[r]))
            return
        elif k == 'halt':
            o.append('\tcpu.cycles += %d; cpu.halted = 1; cpu.pc = 0x%04X; goto leave;' % (cyc, nxt))
            return
        elif mn in BLOCK:
            y, z = BLOCK[mn]
            if y >= 6:
                o.append('\tcpu.cycles += 16;')
                o.append('\tif (op_block(%d, %d)) { cpu.cycles += 5; cpu.pc = 0x%04X; if (cpu.cycles >= cpu.budget) goto leave; goto %s; }'
                         % (y, z, pc, self.lab(bank, off)))
            else:
                o.append('\tcpu.cycles += 16; op_block(%d, %d);' % (y, z))
        else:
            o.append('\t%s cpu.cycles += %d;' % (translate(t), cyc))
        # fall through to the next instruction, or leave if the time slice is used up
        nb, no = bank, off + ins.length
        if not alone and no < BANK and self.d.flags[nb][no] == START and (nb, no) in self.d.insn:
            o.append('\tif (cpu.cycles >= cpu.budget) { cpu.pc = 0x%04X; goto leave; }' % nxt)
        else:
            o.append('\tcpu.pc = 0x%04X; goto dispatch;' % nxt)

    def generate(self, banks):
        d = self.d
        o = self.out
        o.append('// Generated by tools/recomp.py from the traced disassembly. Do not edit:')
        o.append('// regenerate instead. Each block is one Z80 instruction of the original ROM.')
        o.append('#include "../runtime/z80.h"')
        o.append('#include "../runtime/sms.h"')
        o.append('#include "../runtime/native.h"')
        for name in sorted(set(d.natives.values())):
            o.append('extern Native %s;' % name)
        o.append('')
        o.append('uint64_t recomp_fallbacks;  // instructions the interpreter had to run')
        o.append('void (*recomp_on_fallback)(uint16_t pc);  // optional hook, for finding untranslated code')
        o.append('')
        o.append('void recomp_run(void) {')
        tables = {}
        for b in banks:
            entries = []
            for off in range(BANK):
                if d.flags[b][off] == START and (b, off) in d.insn:
                    entries.append(off)
            tables[b] = entries
        # instructions the emulator saw executing that overlap others (e.g. a
        # "ld bc,nn" that hides an instruction in its operand): translated on
        # their own, continuing through the dispatcher
        extra = {}
        if d.cov is not None:
            for b in banks:
                mem = d.mem_for(b)
                for off in range(BANK):
                    if d.cov[b * BANK + off] & 1 and not (d.flags[b][off] == START and (b, off) in d.insn):
                        ins = disasm.decode(mem, home(b) + off)
                        if off + ins.length <= BANK:
                            extra[(b, off)] = ins
                            tables[b].append(off)
            for b in banks:
                tables[b].sort()
        for b in banks:
            o.append('\tstatic void *const tab_%02X[0x4000] = {' % b)
            for off in tables[b]:
                o.append('\t\t[0x%04X] = &&%s,' % (off, self.lab(b, off)))
            o.append('\t};')
        o.append('\tstatic void *const *const bank_tab[%d] = {' % d.nbanks)
        o.append('\t\t' + ', '.join(('tab_%02X' % b) if b in banks else '0' for b in range(d.nbanks)))
        o.append('\t};')
        o.append('''
	// Interrupts are taken here, between time slices. Anything that can raise
	// an interrupt mid-slice (EI, VDP port access) sets cpu.budget = cpu.cycles
	// so the translated code comes back here after the current instruction.
	const int64_t end = cpu.budget;
top:
	cpu.budget = end;
	if (cpu.halted) {
		if (!z80_interrupt()) { if (cpu.cycles < end) cpu.cycles = end; return; }
	} else if (cpu.ei_delay) {
		// the instruction after EI runs before an interrupt is accepted
		cpu.ei_delay = 0;
		if (cpu.irq_line || cpu.nmi_pending) cpu.budget = cpu.cycles + 1;
	} else z80_interrupt();
dispatch:
	while (cpu.cycles < cpu.budget) {
		if ((cpu.pc >= 0xFFF0 || native_tasks_waiting) && native_special()) continue;   // rewritten code waiting (native.h)
		int slot = cpu.pc >> 14, bank = rom_bank_at(cpu.pc);
		if (bank >= 0 && slot == (bank == 0 ? 0 : bank == 1 ? 1 : 2) && bank_tab[bank]) {
			void *t = bank_tab[bank][cpu.pc & 0x3FFF];
			if (t) goto *t;
		}
		if (recomp_on_fallback) recomp_on_fallback(cpu.pc);
		z80_exec();   // no translation: interpret one instruction
		recomp_fallbacks++;
		if (cpu.halted) break;
	}
leave:
	if (cpu.cycles < end) goto top;
	cpu.budget = end;
	return;
''')
        for b in banks:
            o.append('\t// ---------------- bank %d ----------------' % b)
            for off in tables[b]:
                if (b, off) in extra:
                    continue  # emitted below, so fall-through order stays intact
                ins = d.insn[(b, off)]
                raw = d.rom[b * BANK + off: b * BANK + off + ins.length]
                self.emit_insn(b, off, ins, raw)
        o.append('	// ---------------- overlapping instructions ----------------')
        for (b, off), ins in sorted(extra.items()):
            raw = d.rom[b * BANK + off: b * BANK + off + ins.length]
            self.emit_insn(b, off, ins, raw, alone=True)
        o.append('}')
        return '\n'.join(o) + '\n'


def main():
    rom = open(sys.argv[1], 'rb').read()
    d = disasm.analyze(rom, sys.argv[2], log=lambda *a: None)
    banks = [b for b in range(d.nbanks) if any(k[0] == b for k in d.insn)]
    g = Gen(d)
    src = g.generate(banks)
    os.makedirs(os.path.dirname(os.path.abspath(sys.argv[3])), exist_ok=True)
    old = open(sys.argv[3], newline='').read() if os.path.exists(sys.argv[3]) else None
    if old != src:  # keep the timestamp when nothing changed (saves a long compile)
        with open(sys.argv[3], 'w', newline='\n') as f:
            f.write(src)
    print('translated %d instructions in banks %s' % (len(d.insn), banks))


if __name__ == '__main__':
    main()
