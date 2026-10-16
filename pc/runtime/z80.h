// Z80 state and ALU helpers shared by the interpreter (z80.c) and the
// recompiled game code (gen/recomp.c).
#ifndef U4_Z80_H
#define U4_Z80_H

#include <stdint.h>

typedef struct Z80 {
	uint8_t a, f, b, c, d, e, h, l;
	uint8_t a_, f_, b_, c_, d_, e_, h_, l_;
	uint16_t ix, iy, sp, pc;
	uint8_t i, r, iff1, iff2, im;
	uint8_t halted, ei_delay, nmi_pending, irq_line;
	int64_t cycles;   // T-states since power on
	int64_t budget;   // run until cycles reach this
} Z80;

extern Z80 cpu;

// bus (sms.c)
uint8_t mem_read(uint16_t a);
void mem_write(uint16_t a, uint8_t v);
uint8_t io_in(uint16_t port);
void io_out(uint16_t port, uint8_t v);
int rom_bank_at(uint16_t a);   // ROM bank mapped at a CPU address, -1 if none

// interpreter (z80.c)
void z80_reset(void);
void z80_step(void);           // take a pending interrupt, then one instruction
void z80_exec(void);           // one instruction, no interrupt check
int z80_interrupt(void);       // take a pending NMI/IRQ; 1 if taken

enum { FS = 0x80, FZ = 0x40, FY = 0x20, FH = 0x10, FX = 0x08, FP = 0x04, FN = 0x02, FC = 0x01 };

extern uint8_t z80_sz[256], z80_szp[256];
void z80_tables(void);

#define BC ((uint16_t)((cpu.b << 8) | cpu.c))
#define DE ((uint16_t)((cpu.d << 8) | cpu.e))
#define HL ((uint16_t)((cpu.h << 8) | cpu.l))
#define SET_BC(v) do { uint16_t _v = (v); cpu.b = _v >> 8; cpu.c = (uint8_t)_v; } while (0)
#define SET_DE(v) do { uint16_t _v = (v); cpu.d = _v >> 8; cpu.e = (uint8_t)_v; } while (0)
#define SET_HL(v) do { uint16_t _v = (v); cpu.h = _v >> 8; cpu.l = (uint8_t)_v; } while (0)

static inline uint16_t rd16(uint16_t a) { return mem_read(a) | (mem_read((uint16_t)(a + 1)) << 8); }
static inline void wr16(uint16_t a, uint16_t v) { mem_write(a, (uint8_t)v); mem_write((uint16_t)(a + 1), v >> 8); }
static inline void push16(uint16_t v) { cpu.sp -= 2; wr16(cpu.sp, v); }
static inline uint16_t pop16(void) { uint16_t v = rd16(cpu.sp); cpu.sp += 2; return v; }

static inline void op_add8(uint8_t v, int cy) {
	int a = cpu.a, r = a + v + cy;
	cpu.f = z80_sz[r & 0xFF] | ((r >> 8) & FC) | ((a ^ v ^ r) & FH) | ((((a ^ ~v) & (a ^ r)) & 0x80) ? FP : 0);
	cpu.a = (uint8_t)r;
}
static inline void op_sub8(uint8_t v, int cy, int store) {
	int a = cpu.a, r = a - v - cy;
	uint8_t f = (z80_sz[r & 0xFF] & ~(FX | FY)) | ((r >> 8) & FC) | FN | ((a ^ v ^ r) & FH) |
		((((a ^ v) & (a ^ r)) & 0x80) ? FP : 0);
	if (store) { cpu.a = (uint8_t)r; cpu.f = f | (r & (FX | FY)); }
	else cpu.f = f | (v & (FX | FY));
}
static inline void op_and(uint8_t v) { cpu.a &= v; cpu.f = z80_szp[cpu.a] | FH; }
static inline void op_xor(uint8_t v) { cpu.a ^= v; cpu.f = z80_szp[cpu.a]; }
static inline void op_or(uint8_t v) { cpu.a |= v; cpu.f = z80_szp[cpu.a]; }
static inline void op_alu(int op, uint8_t v) {
	switch (op) {
	case 0: op_add8(v, 0); break;
	case 1: op_add8(v, cpu.f & FC); break;
	case 2: op_sub8(v, 0, 1); break;
	case 3: op_sub8(v, cpu.f & FC, 1); break;
	case 4: op_and(v); break;
	case 5: op_xor(v); break;
	case 6: op_or(v); break;
	default: op_sub8(v, 0, 0); break;
	}
}
static inline uint8_t op_inc8(uint8_t v) {
	uint8_t r = v + 1;
	cpu.f = (cpu.f & FC) | z80_sz[r] | ((r & 0x0F) == 0 ? FH : 0) | (v == 0x7F ? FP : 0);
	return r;
}
static inline uint8_t op_dec8(uint8_t v) {
	uint8_t r = v - 1;
	cpu.f = (cpu.f & FC) | z80_sz[r] | FN | ((v & 0x0F) == 0 ? FH : 0) | (v == 0x80 ? FP : 0);
	return r;
}
static inline uint16_t op_add16(uint16_t a, uint16_t b) {
	uint32_t r = (uint32_t)a + b;
	cpu.f = (cpu.f & (FS | FZ | FP)) | ((r >> 16) & FC) | (((a ^ b ^ r) >> 8) & FH) | ((r >> 8) & (FX | FY));
	return (uint16_t)r;
}
static inline void op_adc16(uint16_t b) {
	int a = HL, c = cpu.f & FC, r = a + b + c;
	cpu.f = ((r >> 16) & FC) | (((a ^ b ^ r) >> 8) & FH) | ((r >> 8) & (FS | FX | FY)) |
		((r & 0xFFFF) == 0 ? FZ : 0) | ((((a ^ ~b) & (a ^ r)) & 0x8000) ? FP : 0);
	SET_HL(r);
}
static inline void op_sbc16(uint16_t b) {
	int a = HL, c = cpu.f & FC, r = a - b - c;
	cpu.f = ((r >> 16) & FC) | FN | (((a ^ b ^ r) >> 8) & FH) | ((r >> 8) & (FS | FX | FY)) |
		((r & 0xFFFF) == 0 ? FZ : 0) | ((((a ^ b) & (a ^ r)) & 0x8000) ? FP : 0);
	SET_HL(r);
}
static inline uint8_t op_rot(int op, uint8_t v) {
	uint8_t r, c;
	switch (op) {
	case 0: c = v >> 7; r = (uint8_t)((v << 1) | c); break;
	case 1: c = v & 1; r = (v >> 1) | (c << 7); break;
	case 2: c = v >> 7; r = (uint8_t)((v << 1) | (cpu.f & FC)); break;
	case 3: c = v & 1; r = (v >> 1) | ((cpu.f & FC) << 7); break;
	case 4: c = v >> 7; r = (uint8_t)(v << 1); break;
	case 5: c = v & 1; r = (v >> 1) | (v & 0x80); break;
	case 6: c = v >> 7; r = (uint8_t)((v << 1) | 1); break;
	default: c = v & 1; r = v >> 1; break;
	}
	cpu.f = z80_szp[r] | c;
	return r;
}
static inline void op_bit(int n, uint8_t v) {
	uint8_t b = v & (1 << n);
	cpu.f = (cpu.f & FC) | FH | (b ? 0 : (FZ | FP)) | (b & FS) | (v & (FX | FY));
}
static inline void op_rlca(void) { uint8_t c = cpu.a >> 7; cpu.a = (uint8_t)((cpu.a << 1) | c); cpu.f = (cpu.f & (FS | FZ | FP)) | c | (cpu.a & (FX | FY)); }
static inline void op_rrca(void) { uint8_t c = cpu.a & 1; cpu.a = (cpu.a >> 1) | (c << 7); cpu.f = (cpu.f & (FS | FZ | FP)) | c | (cpu.a & (FX | FY)); }
static inline void op_rla(void) { uint8_t c = cpu.a >> 7; cpu.a = (uint8_t)((cpu.a << 1) | (cpu.f & FC)); cpu.f = (cpu.f & (FS | FZ | FP)) | c | (cpu.a & (FX | FY)); }
static inline void op_rra(void) { uint8_t c = cpu.a & 1; cpu.a = (cpu.a >> 1) | ((cpu.f & FC) << 7); cpu.f = (cpu.f & (FS | FZ | FP)) | c | (cpu.a & (FX | FY)); }
static inline void op_cpl(void) { cpu.a ^= 0xFF; cpu.f = (cpu.f & (FS | FZ | FP | FC)) | FH | FN | (cpu.a & (FX | FY)); }
static inline void op_scf(void) { cpu.f = (cpu.f & (FS | FZ | FP)) | FC | (cpu.a & (FX | FY)); }
static inline void op_ccf(void) { cpu.f = ((cpu.f & (FS | FZ | FP | FC)) | ((cpu.f & FC) ? FH : 0) | (cpu.a & (FX | FY))) ^ FC; }
static inline void op_neg(void) { uint8_t v = cpu.a; cpu.a = 0; op_sub8(v, 0, 1); }
static inline void op_daa(void) {
	int a = cpu.a, corr = 0, c = cpu.f & FC, n = cpu.f & FN, h;
	if ((cpu.f & FH) || (a & 0x0F) > 9) corr = 6;
	if (c || a > 0x99) { corr |= 0x60; c = FC; }
	uint8_t r = n ? (uint8_t)(a - corr) : (uint8_t)(a + corr);
	h = n ? (((cpu.f & FH) && (a & 0x0F) < 6) ? FH : 0) : ((a & 0x0F) > 9 ? FH : 0);
	cpu.a = r;
	cpu.f = z80_szp[r] | c | n | h;
}
static inline void op_exx(void) {
	uint8_t t;
	t = cpu.b; cpu.b = cpu.b_; cpu.b_ = t; t = cpu.c; cpu.c = cpu.c_; cpu.c_ = t;
	t = cpu.d; cpu.d = cpu.d_; cpu.d_ = t; t = cpu.e; cpu.e = cpu.e_; cpu.e_ = t;
	t = cpu.h; cpu.h = cpu.h_; cpu.h_ = t; t = cpu.l; cpu.l = cpu.l_; cpu.l_ = t;
}
static inline void op_ex_af(void) { uint8_t t = cpu.a; cpu.a = cpu.a_; cpu.a_ = t; t = cpu.f; cpu.f = cpu.f_; cpu.f_ = t; }
static inline uint8_t op_in_c(uint16_t port) { uint8_t v = io_in(port); cpu.f = (cpu.f & FC) | z80_szp[v]; return v; }
static inline void op_rrd(void) {
	uint8_t m = mem_read(HL);
	mem_write(HL, (uint8_t)((cpu.a << 4) | (m >> 4)));
	cpu.a = (cpu.a & 0xF0) | (m & 0x0F); cpu.f = (cpu.f & FC) | z80_szp[cpu.a];
}
static inline void op_rld(void) {
	uint8_t m = mem_read(HL);
	mem_write(HL, (uint8_t)((m << 4) | (cpu.a & 0x0F)));
	cpu.a = (cpu.a & 0xF0) | (m >> 4); cpu.f = (cpu.f & FC) | z80_szp[cpu.a];
}
// one step of a block instruction; returns 1 when a repeating form should repeat
static inline int op_block(int y, int z) {
	int step = (y & 1) ? -1 : 1, rep = y >= 6;
	switch (z) {
	case 0: {
		uint8_t v = mem_read(HL);
		mem_write(DE, v);
		SET_HL(HL + step); SET_DE(DE + step); SET_BC(BC - 1);
		uint8_t n = v + cpu.a;
		cpu.f = (cpu.f & (FS | FZ | FC)) | (BC ? FP : 0) | (n & FX) | ((n << 4) & FY);
		return rep && BC;
	}
	case 1: {
		uint8_t v = mem_read(HL), r = cpu.a - v;
		uint8_t h = (cpu.a ^ v ^ r) & FH;
		SET_HL(HL + step); SET_BC(BC - 1);
		cpu.f = (cpu.f & FC) | (z80_sz[r] & ~(FX | FY)) | h | FN | (BC ? FP : 0);
		return rep && BC && r != 0;
	}
	case 2: {
		uint8_t v = io_in(BC);
		mem_write(HL, v);
		SET_HL(HL + step); cpu.b--;
		cpu.f = z80_sz[cpu.b] | FN;
		return rep && cpu.b;
	}
	default: {
		uint8_t v = mem_read(HL);
		cpu.b--;
		io_out(BC, v);
		SET_HL(HL + step);
		cpu.f = z80_sz[cpu.b] | FN;
		return rep && cpu.b;
	}
	}
}

#endif
