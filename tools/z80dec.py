"""Table-free Z80 instruction decoder emitting WLA-DX syntax.

decode(mem, pc) -> Insn, where mem(addr) returns the byte at a CPU address.
Insn.undoc is True for encodings WLA-DX would not reproduce byte-for-byte
(undocumented or duplicate opcodes); callers emit those as .db.
"""

R = ['b', 'c', 'd', 'e', 'h', 'l', '(hl)', 'a']
RP = ['bc', 'de', 'hl', 'sp']
RP2 = ['bc', 'de', 'hl', 'af']
CC = ['nz', 'z', 'nc', 'c', 'po', 'pe', 'p', 'm']
ALU = ['add a,', 'adc a,', 'sub ', 'sbc a,', 'and ', 'xor ', 'or ', 'cp ']
ROT = ['rlc', 'rrc', 'rl', 'rr', 'sla', 'sra', 'sll', 'srl']
BLI = {(4, 0): 'ldi', (4, 1): 'cpi', (4, 2): 'ini', (4, 3): 'outi',
       (5, 0): 'ldd', (5, 1): 'cpd', (5, 2): 'ind', (5, 3): 'outd',
       (6, 0): 'ldir', (6, 1): 'cpir', (6, 2): 'inir', (6, 3): 'otir',
       (7, 0): 'lddr', (7, 1): 'cpdr', (7, 2): 'indr', (7, 3): 'otdr'}


def h8(v):
    return '$%02X' % (v & 0xFF)


def h16(v):
    return '$%04X' % (v & 0xFFFF)


class Insn:
    __slots__ = ('pc', 'length', 'text', 'kind', 'target', 'cond', 'undoc',
                 'imm16', 'imm16_kind', 'fmt', 'disp')

    def __init__(self):
        self.pc = 0
        self.length = 1
        self.text = ''
        self.kind = 'op'      # op, jp, jr, call, ret, rst, jpind, halt
        self.target = None
        self.cond = False
        self.undoc = False
        self.imm16 = None     # 16-bit immediate / address operand
        self.imm16_kind = None  # 'addr' (memory operand) or 'imm'
        self.disp = None      # raw jr/djnz displacement byte
        self.fmt = None       # text with {T} placeholder for the 16-bit operand


def decode(mem, pc):
    ins = Insn()
    ins.pc = pc
    p = pc
    op = mem(p)
    idx = None
    if op in (0xDD, 0xFD):
        idx = 'ix' if op == 0xDD else 'iy'
        p += 1
        op = mem(p)
        if op in (0xDD, 0xFD, 0xED):
            ins.length = 1
            ins.text = '.db ' + h8(mem(pc))
            ins.undoc = True
            return ins
    if op == 0xED:
        return _ed(mem, pc, ins)
    if op == 0xCB:
        return _cb(mem, pc, p, idx, ins)
    return _main(mem, pc, p, op, idx, ins)


def _imm16(mem, a):
    return mem(a) | (mem(a + 1) << 8)


def _main(mem, pc, p, op, idx, ins):
    x, y, z = op >> 6, (op >> 3) & 7, op & 7
    q, pp = y & 1, y >> 1
    p += 1
    used_idx = False   # did ix/iy replace something?
    uses_hl_half = False
    disp = None

    def r(i):
        nonlocal used_idx, uses_hl_half, disp, p
        if idx is None:
            return R[i]
        if i == 6:
            used_idx = True
            if disp is None:
                disp = mem(p)
                p += 1
            d = disp - 256 if disp >= 128 else disp
            return '(%s%s%s)' % (idx, '+' if d >= 0 else '-', h8(abs(d)))
        if i in (4, 5):
            uses_hl_half = True
            used_idx = True  # ixh/ixl/iyh/iyl: undocumented but WLA-DX assembles them
            return idx + ('h' if i == 4 else 'l')
        return R[i]

    def rp(i, tbl=RP):
        nonlocal used_idx
        if i == 2 and idx is not None:
            used_idx = True
            return idx
        return tbl[i]

    def n16():
        nonlocal p
        v = _imm16(mem, p)
        p += 2
        return v

    def n8():
        nonlocal p
        v = mem(p)
        p += 1
        return v

    t = None
    if x == 0:
        if z == 0:
            if y == 0:
                t = 'nop'
            elif y == 1:
                t = "ex af,af'"
            else:
                d = n8()
                ins.disp = d
                d = d - 256 if d >= 128 else d
                tgt = (p + d) & 0xFFFF
                ins.target = tgt
                ins.kind = 'jr'
                if y == 2:
                    t = 'djnz {T}'
                    ins.cond = True
                elif y == 3:
                    t = 'jr {T}'
                else:
                    t = 'jr %s,{T}' % CC[y - 4]
                    ins.cond = True
        elif z == 1:
            if q == 0:
                v = n16()
                ins.imm16, ins.imm16_kind = v, 'imm'
                t = 'ld %s,{T}' % rp(pp)
            else:
                t = 'add %s,%s' % (rp(2), rp(pp))
        elif z == 2:
            if pp == 0:
                t = 'ld (bc),a' if q == 0 else 'ld a,(bc)'
            elif pp == 1:
                t = 'ld (de),a' if q == 0 else 'ld a,(de)'
            else:
                v = n16()
                ins.imm16, ins.imm16_kind = v, 'addr'
                reg = rp(2) if pp == 2 else 'a'
                t = ('ld ({T}),%s' % reg) if q == 0 else ('ld %s,({T})' % reg)
        elif z == 3:
            t = ('inc ' if q == 0 else 'dec ') + rp(pp)
        elif z == 4:
            t = 'inc ' + r(y)
        elif z == 5:
            t = 'dec ' + r(y)
        elif z == 6:
            dst = r(y)
            t = 'ld %s,%s' % (dst, h8(n8()))
        else:
            t = ['rlca', 'rrca', 'rla', 'rra', 'daa', 'cpl', 'scf', 'ccf'][y]
    elif x == 1:
        if z == 6 and y == 6:
            t = 'halt'
            ins.kind = 'halt'
        else:
            # with an (ix+d) operand the other register stays h/l
            if idx is not None and (y == 6 or z == 6):
                a = r(y) if y == 6 else R[y]
                b = r(z) if z == 6 else R[z]
            else:
                a, b = r(y), r(z)
            t = 'ld %s,%s' % (a, b)
    elif x == 2:
        t = ALU[y] + r(z)
    else:
        if z == 0:
            t = 'ret ' + CC[y]
            ins.kind = 'ret'
            ins.cond = True
        elif z == 1:
            if q == 0:
                t = 'pop ' + rp(pp, RP2)
            else:
                if pp == 0:
                    t = 'ret'
                    ins.kind = 'ret'
                elif pp == 1:
                    t = 'exx'
                elif pp == 2:
                    t = 'jp (%s)' % rp(2)
                    ins.kind = 'jpind'
                else:
                    t = 'ld sp,%s' % rp(2)
        elif z == 2:
            v = n16()
            ins.target = v
            ins.kind = 'jp'
            ins.cond = True
            t = 'jp %s,{T}' % CC[y]
        elif z == 3:
            if y == 0:
                v = n16()
                ins.target = v
                ins.kind = 'jp'
                t = 'jp {T}'
            elif y == 2:
                t = 'out (%s),a' % h8(n8())
            elif y == 3:
                t = 'in a,(%s)' % h8(n8())
            elif y == 4:
                t = 'ex (sp),%s' % rp(2)
            elif y == 5:
                t = 'ex de,hl'
            elif y == 6:
                t = 'di'
            else:
                t = 'ei'
        elif z == 4:
            v = n16()
            ins.target = v
            ins.kind = 'call'
            ins.cond = True
            t = 'call %s,{T}' % CC[y]
        elif z == 5:
            if q == 0:
                t = 'push ' + rp(pp, RP2)
            else:
                v = n16()
                ins.target = v
                ins.kind = 'call'
                t = 'call {T}'
        elif z == 6:
            t = ALU[y] + h8(n8())
        else:
            ins.kind = 'rst'
            ins.target = y * 8
            t = 'rst %s' % h8(y * 8)
    ins.length = p - pc
    if idx is not None and not used_idx:
        # prefix without effect
        ins.undoc = True
        if True:
            ins.length = 1
            ins.kind = 'op'
            ins.target = None
            ins.imm16 = None
            ins.text = '.db ' + h8(mem(pc))
            return ins
    if '{T}' in t:
        ins.fmt = t
        v = ins.target if ins.target is not None else ins.imm16
        ins.text = t.replace('{T}', h16(v))
    else:
        ins.text = t
    return ins


def _cb(mem, pc, p, idx, ins):
    if idx is not None:
        d = mem(p + 1)
        op = mem(p + 2)
        ins.length = 4
        dd = d - 256 if d >= 128 else d
        m = '(%s%s%s)' % (idx, '+' if dd >= 0 else '-', h8(abs(dd)))
        if (op & 7) != 6:
            ins.undoc = True
    else:
        op = mem(p + 1)
        ins.length = 2
        m = R[op & 7]
    x, y = op >> 6, (op >> 3) & 7
    if x == 0:
        ins.text = '%s %s' % (ROT[y], m)  # sll (y == 6) is undocumented; WLA-DX takes it
    else:
        ins.text = '%s %d,%s' % (['', 'bit', 'res', 'set'][x], y, m)
    return ins


def _ed(mem, pc, ins):
    op = mem(pc + 1)
    x, y, z = op >> 6, (op >> 3) & 7, op & 7
    q, pp = y & 1, y >> 1
    ins.length = 2
    t = None
    if x == 1:
        if z == 0:
            t = 'in %s,(c)' % R[y] if y != 6 else None
        elif z == 1:
            t = 'out (c),%s' % R[y] if y != 6 else None
        elif z == 2:
            t = ('sbc hl,' if q == 0 else 'adc hl,') + RP[pp]
        elif z == 3:
            v = _imm16(mem, pc + 2)
            ins.length = 4
            ins.imm16, ins.imm16_kind = v, 'addr'
            if pp == 2:
                t = None  # duplicate of 22/2A
            else:
                ins.fmt = ('ld ({T}),%s' % RP[pp]) if q == 0 else ('ld %s,({T})' % RP[pp])
                t = ins.fmt.replace('{T}', h16(v))
        elif z == 4:
            t = 'neg' if y == 0 else None
        elif z == 5:
            if y == 0:
                t = 'retn'
                ins.kind = 'ret'
            elif y == 1:
                t = 'reti'
                ins.kind = 'ret'
        elif z == 6:
            t = {0: 'im 0', 2: 'im 1', 3: 'im 2'}.get(y)
        else:
            t = ['ld i,a', 'ld r,a', 'ld a,i', 'ld a,r', 'rrd', 'rld', None, None][y]
    elif x == 2 and (y, z) in BLI:
        t = BLI[(y, z)]
    if t is None:
        ins.undoc = True
        ins.kind = 'op'
        ins.fmt = None
        ins.imm16 = None
        ins.text = '.db $ED,%s' % h8(op)
        if x != 1 or z != 3:
            ins.length = 2
    else:
        ins.text = t
    return ins
