#include <stdio.h>
#include <stdlib.h>
// Z80 interpreter. The recompiled code (gen/recomp.c) falls back to this for
// any address it has no translation for (code in RAM, unexplored code).
#include "z80.h"

Z80 cpu;
uint8_t z80_sz[256], z80_szp[256];

static const uint8_t CYC[256] = {
	4,10,7,6,4,4,7,4,4,11,7,6,4,4,7,4,
	8,10,7,6,4,4,7,4,12,11,7,6,4,4,7,4,
	7,10,16,6,4,4,7,4,7,11,16,6,4,4,7,4,
	7,10,13,6,11,11,10,4,7,11,13,6,4,4,7,4,
	4,4,4,4,4,4,7,4,4,4,4,4,4,4,7,4,
	4,4,4,4,4,4,7,4,4,4,4,4,4,4,7,4,
	4,4,4,4,4,4,7,4,4,4,4,4,4,4,7,4,
	7,7,7,7,7,7,4,7,4,4,4,4,4,4,7,4,
	4,4,4,4,4,4,7,4,4,4,4,4,4,4,7,4,
	4,4,4,4,4,4,7,4,4,4,4,4,4,4,7,4,
	4,4,4,4,4,4,7,4,4,4,4,4,4,4,7,4,
	4,4,4,4,4,4,7,4,4,4,4,4,4,4,7,4,
	5,10,10,10,10,11,7,11,5,10,10,0,10,17,7,11,
	5,10,10,11,10,11,7,11,5,4,10,11,10,0,7,11,
	5,10,10,19,10,11,7,11,5,4,10,4,10,0,7,11,
	5,10,10,4,10,11,7,11,5,6,10,4,10,0,7,11,
};

void z80_tables(void) {
	for (int i = 0; i < 256; i++) {
		int p = i; p ^= p >> 4; p ^= p >> 2; p ^= p >> 1;
		z80_sz[i] = (i & 0x80) | (i == 0 ? FZ : 0) | (i & (FX | FY));
		z80_szp[i] = z80_sz[i] | ((p & 1) ? 0 : FP);
	}
}

void z80_reset(void) {
	z80_tables();
	cpu = (Z80){0};
	cpu.a = cpu.f = 0xFF;
	cpu.ix = cpu.iy = 0xFFFF;
	cpu.sp = 0xDFF0;
}

static inline uint8_t f8(void) { return mem_read(cpu.pc++); }
static inline uint16_t f16(void) { uint8_t lo = f8(); return lo | (f8() << 8); }
static inline void inc_r(void) { cpu.r = (cpu.r & 0x80) | ((cpu.r + 1) & 0x7F); }

static int cond(int y) {
	switch (y) {
	case 0: return !(cpu.f & FZ); case 1: return !!(cpu.f & FZ);
	case 2: return !(cpu.f & FC); case 3: return !!(cpu.f & FC);
	case 4: return !(cpu.f & FP); case 5: return !!(cpu.f & FP);
	case 6: return !(cpu.f & FS); default: return !!(cpu.f & FS);
	}
}

// idx: 0 = hl, 1 = ix, 2 = iy
static uint8_t get_r(int i, int idx, uint16_t ea) {
	switch (i) {
	case 0: return cpu.b; case 1: return cpu.c; case 2: return cpu.d; case 3: return cpu.e;
	case 4: return idx == 0 ? cpu.h : idx == 1 ? cpu.ix >> 8 : cpu.iy >> 8;
	case 5: return idx == 0 ? cpu.l : idx == 1 ? (uint8_t)cpu.ix : (uint8_t)cpu.iy;
	case 6: return mem_read(ea);
	default: return cpu.a;
	}
}
static void set_r(int i, int idx, uint16_t ea, uint8_t v) {
	switch (i) {
	case 0: cpu.b = v; break; case 1: cpu.c = v; break; case 2: cpu.d = v; break; case 3: cpu.e = v; break;
	case 4: if (idx == 0) cpu.h = v; else if (idx == 1) cpu.ix = (cpu.ix & 0xFF) | (v << 8); else cpu.iy = (cpu.iy & 0xFF) | (v << 8); break;
	case 5: if (idx == 0) cpu.l = v; else if (idx == 1) cpu.ix = (cpu.ix & 0xFF00) | v; else cpu.iy = (cpu.iy & 0xFF00) | v; break;
	case 6: mem_write(ea, v); break;
	default: cpu.a = v;
	}
}
static uint16_t get_rp(int p, int idx) {
	switch (p) {
	case 0: return BC; case 1: return DE;
	case 2: return idx == 0 ? HL : idx == 1 ? cpu.ix : cpu.iy;
	default: return cpu.sp;
	}
}
static void set_rp(int p, int idx, uint16_t v) {
	switch (p) {
	case 0: SET_BC(v); break; case 1: SET_DE(v); break;
	case 2: if (idx == 0) SET_HL(v); else if (idx == 1) cpu.ix = v; else cpu.iy = v; break;
	default: cpu.sp = v;
	}
}

int z80_interrupt(void) {
	if (cpu.nmi_pending) {
		cpu.nmi_pending = 0; cpu.halted = 0;
		cpu.iff2 = cpu.iff1; cpu.iff1 = 0;
		push16(cpu.pc); cpu.pc = 0x66; cpu.cycles += 11;
		return 1;
	}
	if (cpu.irq_line && cpu.iff1 && !cpu.ei_delay) {
		static int trace = -1;   // U4_IRQ_TRACE=1: log each interrupt (pc, time)
		if (trace < 0) trace = getenv("U4_IRQ_TRACE") != NULL;
		if (trace) fprintf(stderr, "irq %04X %lld\n", cpu.pc, (long long)cpu.cycles);
		cpu.halted = 0; cpu.iff1 = cpu.iff2 = 0;
		push16(cpu.pc);
		cpu.pc = cpu.im == 2 ? rd16((uint16_t)((cpu.i << 8) | 0xFF)) : 0x38;
		cpu.cycles += 13;
		return 1;
	}
	return 0;
}

static void cb_op(int idx, uint16_t ea) {
	uint8_t op;
	if (idx) op = f8();
	else { inc_r(); op = f8(); ea = HL; }
	int x = op >> 6, y = (op >> 3) & 7, z = op & 7, reg = idx ? 6 : z;
	uint8_t v = get_r(reg, 0, ea), r;
	switch (x) {
	case 0: r = op_rot(y, v); set_r(reg, 0, ea, r); if (idx && z != 6) set_r(z, 0, 0, r); break;
	case 1:
		op_bit(y, v);
		if (reg == 6) cpu.f = (cpu.f & ~(FX | FY)) | ((ea >> 8) & (FX | FY));
		break;
	case 2: r = v & ~(1 << y); set_r(reg, 0, ea, r); if (idx && z != 6) set_r(z, 0, 0, r); break;
	default: r = v | (1 << y); set_r(reg, 0, ea, r); if (idx && z != 6) set_r(z, 0, 0, r); break;
	}
	cpu.cycles += idx ? (x == 1 ? 20 : 23) : (reg == 6 ? (x == 1 ? 12 : 15) : 8);
}

static void ed_op(void) {
	inc_r();
	uint8_t op = f8();
	int x = op >> 6, y = (op >> 3) & 7, z = op & 7, p = y >> 1, q = y & 1, cyc = 8;
	if (x == 1) {
		switch (z) {
		case 0: { uint8_t v = op_in_c(BC); if (y != 6) set_r(y, 0, 0, v); cyc = 12; break; }
		case 1: io_out(BC, y == 6 ? 0 : get_r(y, 0, 0)); cyc = 12; break;
		case 2: if (q == 0) op_sbc16(get_rp(p, 0)); else op_adc16(get_rp(p, 0)); cyc = 15; break;
		case 3: { uint16_t a = f16(); if (q == 0) wr16(a, get_rp(p, 0)); else set_rp(p, 0, rd16(a)); cyc = 20; break; }
		case 4: op_neg(); break;
		case 5: cpu.pc = pop16(); cpu.iff1 = cpu.iff2; cyc = 14; break;
		case 6: { static const uint8_t m[8] = {0, 0, 1, 2, 0, 0, 1, 2}; cpu.im = m[y]; break; }
		default:
			switch (y) {
			case 0: cpu.i = cpu.a; cyc = 9; break;
			case 1: cpu.r = cpu.a; cyc = 9; break;
			case 2: cpu.a = cpu.i; cpu.f = (cpu.f & FC) | z80_sz[cpu.a] | (cpu.iff2 ? FP : 0); cyc = 9; break;
			case 3: cpu.a = cpu.r; cpu.f = (cpu.f & FC) | z80_sz[cpu.a] | (cpu.iff2 ? FP : 0); cyc = 9; break;
			case 4: op_rrd(); cyc = 18; break;
			case 5: op_rld(); cyc = 18; break;
			default: break;
			}
		}
	} else if (x == 2 && y >= 4 && z <= 3) {
		cyc = 16;
		if (op_block(y, z)) { cpu.pc -= 2; cyc = 21; }
	}
	cpu.cycles += cyc;
}

static void main_op(uint8_t op, int idx) {
	int x = op >> 6, y = (op >> 3) & 7, z = op & 7, p = y >> 1, q = y & 1;
	int cyc = CYC[op];
	int uses_mem = (x == 1 && (y == 6 || z == 6) && op != 0x76) || (x == 2 && z == 6) ||
		(x == 0 && (z == 4 || z == 5 || z == 6) && y == 6);
	uint16_t ea = 0;
	int extra = idx ? 4 : 0;
	if (uses_mem) {
		if (idx) {
			int8_t d = (int8_t)f8();
			ea = (uint16_t)((idx == 1 ? cpu.ix : cpu.iy) + d);
			cyc = (x == 0 && z != 6) ? 23 : 19;
			extra = 0;
		} else ea = HL;
	}
	cyc += extra;
	int ridx = uses_mem ? 0 : idx;
	switch (x) {
	case 0:
		switch (z) {
		case 0:
			switch (y) {
			case 0: break;
			case 1: op_ex_af(); break;
			case 2: { int8_t d = (int8_t)f8(); if (--cpu.b) { cpu.pc += d; cyc += 5; } break; }
			case 3: { int8_t d = (int8_t)f8(); cpu.pc += d; break; }
			default: { int8_t d = (int8_t)f8(); if (cond(y - 4)) { cpu.pc += d; cyc += 5; } }
			}
			break;
		case 1:
			if (q == 0) set_rp(p, idx, f16());
			else set_rp(2, idx, op_add16(get_rp(2, idx), get_rp(p, idx)));
			break;
		case 2:
			switch (y) {
			case 0: mem_write(BC, cpu.a); break;
			case 1: cpu.a = mem_read(BC); break;
			case 2: mem_write(DE, cpu.a); break;
			case 3: cpu.a = mem_read(DE); break;
			case 4: wr16(f16(), get_rp(2, idx)); break;
			case 5: set_rp(2, idx, rd16(f16())); break;
			case 6: mem_write(f16(), cpu.a); break;
			default: cpu.a = mem_read(f16());
			}
			break;
		case 3: set_rp(p, idx, (uint16_t)(get_rp(p, idx) + (q ? -1 : 1))); break;
		case 4: set_r(y, ridx, ea, op_inc8(get_r(y, ridx, ea))); break;
		case 5: set_r(y, ridx, ea, op_dec8(get_r(y, ridx, ea))); break;
		case 6: set_r(y, ridx, ea, f8()); break;
		default:
			switch (y) {
			case 0: op_rlca(); break; case 1: op_rrca(); break;
			case 2: op_rla(); break; case 3: op_rra(); break;
			case 4: op_daa(); break; case 5: op_cpl(); break;
			case 6: op_scf(); break; default: op_ccf(); break;
			}
		}
		break;
	case 1:
		if (op == 0x76) { cpu.halted = 1; break; }
		set_r(y, z == 6 ? 0 : ridx, ea, get_r(z, y == 6 ? 0 : ridx, ea));
		break;
	case 2:
		op_alu(y, get_r(z, ridx, ea));
		break;
	default:
		switch (z) {
		case 0: if (cond(y)) { cpu.pc = pop16(); cyc += 6; } break;
		case 1:
			if (q == 0) {
				uint16_t v = pop16();
				if (p == 3) { cpu.a = v >> 8; cpu.f = (uint8_t)v; } else set_rp(p, idx, v);
			} else {
				switch (p) {
				case 0: cpu.pc = pop16(); break;
				case 1: op_exx(); break;
				case 2: cpu.pc = get_rp(2, idx); break;
				default: cpu.sp = get_rp(2, idx);
				}
			}
			break;
		case 2: { uint16_t t = f16(); if (cond(y)) cpu.pc = t; break; }
		case 3:
			switch (y) {
			case 0: cpu.pc = f16(); break;
			case 2: { uint8_t n = f8(); io_out((uint16_t)(n | (cpu.a << 8)), cpu.a); break; }
			case 3: { uint8_t n = f8(); cpu.a = io_in((uint16_t)(n | (cpu.a << 8))); break; }
			case 4: { uint16_t v = rd16(cpu.sp); wr16(cpu.sp, get_rp(2, idx)); set_rp(2, idx, v); break; }
			case 5: { uint16_t t = DE; SET_DE(HL); SET_HL(t); break; }
			case 6: cpu.iff1 = cpu.iff2 = 0; break;
			default: cpu.iff1 = cpu.iff2 = 1; cpu.ei_delay = 1; cpu.budget = cpu.cycles;
			}
			break;
		case 4: { uint16_t t = f16(); if (cond(y)) { push16(cpu.pc); cpu.pc = t; cyc += 7; } break; }
		case 5:
			if (q == 0) push16(p == 3 ? (uint16_t)((cpu.a << 8) | cpu.f) : get_rp(p, idx));
			else { uint16_t t = f16(); push16(cpu.pc); cpu.pc = t; }
			break;
		case 6: op_alu(y, f8()); break;
		default: push16(cpu.pc); cpu.pc = y * 8;
		}
	}
	cpu.cycles += cyc;
}

void z80_step(void) {
	z80_interrupt();
	cpu.ei_delay = 0;
	z80_exec();
}

void z80_exec(void) {
	inc_r();
	if (cpu.halted) { cpu.cycles += 4; return; }
	uint8_t op = f8();
	if (op == 0xDD || op == 0xFD) {
		int idx = op == 0xDD ? 1 : 2;
		inc_r();
		op = f8();
		if (op == 0xCB) {
			int8_t d = (int8_t)f8();
			cb_op(idx, (uint16_t)((idx == 1 ? cpu.ix : cpu.iy) + d));
			return;
		}
		if (op == 0xDD || op == 0xFD || op == 0xED) { cpu.cycles += 4; cpu.pc--; return; }
		main_op(op, idx);
		return;
	}
	if (op == 0xCB) { cb_op(0, 0); return; }
	if (op == 0xED) { ed_op(); return; }
	main_op(op, 0);
}
