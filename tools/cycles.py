"""Cycle counts of original code, for writing C rewrites with the same timing.

    python tools/cycles.py <rom> BB:SSSS-EEEE ...

Lists each instruction from SSSS up to (not including) EEEE in bank BB with
its T-states, and the sum. Conditional instructions show the cost when the
branch is not taken and, after a "+", what taking it adds (jr/jp/call/ret).
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import disasm  # noqa: E402
from disasm import home  # noqa: E402
from recomp import cycles  # noqa: E402


def extra(raw):
    """T-states a taken conditional branch adds."""
    op = raw[0]
    if op == 0x10: return 5                     # djnz
    if op in (0x20, 0x28, 0x30, 0x38): return 5  # jr cc
    if op & 0xC7 == 0xC4: return 7              # call cc
    if op & 0xC7 == 0xC0: return 6              # ret cc
    if op == 0xED and raw[1] in (0xB0, 0xB1, 0xB2, 0xB3, 0xB8, 0xB9, 0xBA, 0xBB): return 5  # repeats
    return 0


def main():
    rom = open(sys.argv[1], 'rb').read()
    for spec in sys.argv[2:]:
        bank, rng = spec.split(':')
        bank = int(bank, 16)
        start, end = (int(x, 16) for x in rng.split('-'))
        base = home(bank)
        mem = bytearray(0x10000)
        mem[base:base + 0x4000] = rom[bank * 0x4000:(bank + 1) * 0x4000]
        if bank != 0:
            mem[0:0x4000] = rom[0:0x4000]
        total = 0
        pc = start
        print('%02X:%04X-%04X' % (bank, start, end))
        while pc < end:
            ins = disasm.decode(lambda a: mem[a & 0xFFFF], pc)
            raw = bytes(mem[pc:pc + ins.length])
            c, x = cycles(raw), extra(raw)
            total += c
            print('  %04X  %-22s %3d%s' % (pc, ins.text, c, '  +%d' % x if x else ''))
            pc += ins.length
        print('  total %d' % total)


if __name__ == '__main__':
    main()
