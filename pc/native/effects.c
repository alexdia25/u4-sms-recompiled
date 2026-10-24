// Screen effects on the map window's tiles in VRAM (bank 1).
// - shake ($4117): the earthquake. The window's top-left 8-byte strip is
//   kept at $DA00; five times the window's patterns are moved one tile
//   column over (shift_block, $4173: 5 x 17 rows of $68 bytes through
//   $DB00) and back, and the kept strip put back; then the screen redrawn
//   (forget_screen, $276D, in bank 0).
// - invert ($41A9): every pattern of the window XORed with $FF (the last
//   column of each row with $F0): a flash, done twice to undo it
//   (invert_tile, $41D2, 48 bytes through $C1C0).
// - invert_palette ($41EC): background colours 4-15 inverted, then a wait
//   for the vertical blank.
// The C goes instruction by instruction (I), as the VDP accesses must land
// where the original's do. Cycle counts are the original's.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static void out_bf(uint8_t v) { io_out((uint16_t)(cpu.a << 8 | 0xBF), v); }

// rst $08 (HL) / rst $10 (DE): the VDP's read / write address
static void vdp_address(uint8_t lo, uint8_t hi, uint8_t or_bits) {
	I(11); I(10);
	cpu.iff1 = cpu.iff2 = 0; I(4);
	cpu.a = lo; I(4);
	out_bf(cpu.a); I(11);
	cpu.a = hi; I(4);
	if (or_bits) { op_or(or_bits); I(7); }
	out_bf(cpu.a); I(11);
	cpu.iff1 = cpu.iff2 = 1;           // ei / ret: an interrupt pending comes after the ret
	cpu.cycles += 4 + 10;
	native_irq_point();
}
static void read_address(void) { vdp_address(cpu.l, cpu.h, 0); }
static void write_address(void) { vdp_address(cpu.e, cpu.d, 0x40); }

static void ex_de_hl(void) { uint16_t t = HL; SET_HL(DE); SET_DE(t); I(4); }

static void block_io(int z) {          // ini (z = 2) or outi (3) / jr nz, B times
	for (;;) {
		op_block(4, z); I(16);
		if (cpu.f & FZ) { I(7); break; }
		I(12);
	}
}

static void push(uint16_t v) { push16(v); I(11); }

static void shift_block(void) {        // $4173: BC = step, HL = from, DE = to
	cpu.a = 0x05; I(7);
	do {
		op_ex_af(); I(4);
		push(BC); push(DE); push(HL);
		SET_BC(0x11BE); I(10);
		do {
			push(BC); push(HL);
			read_address();
			SET_HL(0xDB00); I(10);
			cpu.b = 0x68; I(7);
			block_io(2);
			write_address();
			SET_HL(0xDB00); I(10);
			cpu.b = 0x68; I(7);
			block_io(3);
			SET_HL(pop16()); I(10);
			SET_BC(0x0220); I(10);
			SET_HL(op_add16(HL, BC)); I(11);
			ex_de_hl();
			SET_HL(op_add16(HL, BC)); I(11);
			ex_de_hl();
			SET_BC(pop16()); I(10);
			if (--cpu.b) I(13); else I(8);
		} while (cpu.b);
		SET_HL(pop16()); I(10);
		SET_DE(pop16()); I(10);
		SET_BC(pop16()); I(10);
		SET_HL(op_add16(HL, BC)); I(11);
		ex_de_hl();
		SET_HL(op_add16(HL, BC)); I(11);
		ex_de_hl();
		op_ex_af(); I(4);
		cpu.a = op_dec8(cpu.a); I(4);
		if (cpu.f & FZ) { I(7); break; }
		I(12);
	} while (1);
	I(10);
}

static void strip(int out) {           // 17 rows of 8 bytes between VRAM and $DA00
	do {
		push(BC);
		if (out) write_address(); else { read_address(); ex_de_hl(); }
		cpu.b = 0x08; I(7);
		block_io(out ? 3 : 2);
		ex_de_hl();
		SET_BC(0x0220); I(10);
		SET_HL(op_add16(HL, BC)); I(11);
		if (out) ex_de_hl();
		SET_BC(pop16()); I(10);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
}

// $276D: what is on the screen ($DA00) forgotten, so all of it is redrawn
static void forget_screen(void) {
	push(BC); push(DE); push(HL);
	SET_HL(0xDA00); I(10);
	SET_DE(0xDA01); I(10);
	SET_BC(0x01E3); I(10);
	mem_write(HL, 0xFF); I(10);
	while (op_block(6, 0)) I(21);
	I(16);
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

static void shake(void) {              // $4117
	push(BC); push(DE); push(HL);
	SET_HL(0x0020); I(10);
	SET_DE(0xDA00); I(10);
	SET_BC(0x11BE); I(10);
	strip(0);
	cpu.b = 0x05; I(7);
	do {
		push(BC);
		SET_HL(0x0028); I(10);
		SET_DE(0x0020); I(10);
		SET_BC(0x0068); I(10);
		I(17); shift_block();
		SET_HL(0x01C0); I(10);
		SET_DE(0x01C8); I(10);
		SET_BC(0xFF98); I(10);
		I(17); shift_block();
		SET_HL(0xDA00); I(10);
		SET_DE(0x0020); I(10);
		SET_BC(0x11BE); I(10);
		strip(1);
		SET_BC(pop16()); I(10);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	I(17); forget_screen();            // so the screen is redrawn
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

static void invert_tile(void) {        // $41D2: 48 bytes at HL, XORed with C
	read_address();
	ex_de_hl();
	SET_HL(0xC1C0); I(10);
	cpu.b = 0x30; I(7);
	do {
		cpu.a = io_in((uint16_t)(cpu.a << 8 | 0xBE)); I(11);
		op_xor(cpu.c); I(4);
		mem_write(HL, cpu.a); I(7);
		cpu.l = op_inc8(cpu.l); I(4);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	write_address();
	SET_HL(0xC1C0); I(10);
	SET_BC(0x30BE); I(10);
	block_io(3);
	I(10);
}

static void invert(void) {             // $41A9
	push(BC); push(DE); push(HL);
	SET_HL(0x0020); I(10);
	cpu.b = 0x0B; I(7);
	do {
		push(BC); push(HL);
		SET_BC(0x10FF); I(10);
		do {
			push(BC);
			I(17); invert_tile();
			SET_HL(0x0220); I(10);
			SET_HL(op_add16(HL, DE)); I(11);
			SET_BC(pop16()); I(10);
			if (--cpu.b) I(13); else I(8);
		} while (cpu.b);
		cpu.c = 0xF0; I(7);
		I(17); invert_tile();
		SET_HL(pop16()); I(10);
		SET_BC(pop16()); I(10);
		SET_DE(0x0030); I(10);
		SET_HL(op_add16(HL, DE)); I(11);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

static void invert_palette(void) {     // $41EC
	push(BC); push(HL);
	SET_HL(0xC027); I(10);
	SET_BC(0x0C3F); I(10);
	do {
		cpu.a = mem_read(HL); I(7);
		op_cpl(); I(4);
		op_and(cpu.c); I(4);
		mem_write(HL, cpu.a); I(7);
		cpu.l = op_inc8(cpu.l); I(4);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	cpu.a = 0xFF; I(7);
	mem_write(0xC009, cpu.a); I(13);
	SET_HL(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
	native_continue_at(0x041C);        // jp wait_vblank
}

#define EFFECT(name, addr, out) Native native_##name = { #name, addr, 1, name, out, .long_calls = 1 };
EFFECT(shake, 0x4117, OUT_A | OUT_F)
EFFECT(shift_block, 0x4173, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
EFFECT(invert, 0x41A9, OUT_A | OUT_F)
EFFECT(invert_tile, 0x41D2, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
EFFECT(invert_palette, 0x41EC, OUT_A | OUT_F)
Native native_forget_screen = { "forget_screen", 0x276D, 0, forget_screen, OUT_F, .long_calls = 1 };
