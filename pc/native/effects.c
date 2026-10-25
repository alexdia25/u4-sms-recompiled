// Screen effects on the map window's tiles in VRAM (bank 1), and two of
// the ending's (bank 3: fade_row and wipe, below).
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
// - dissolve ($4EAC): the window cleared in a pattern of dots (dots,
//   $4ED5; set_pixel, $4F15: one pixel of the window's patterns).
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

// set_pixel ($4F15): pixel (L, H) of the map window's patterns (tile
// columns $220 bytes apart from VRAM $0020, 4 bytes a pixel row) set to
// colour A, through $C1C0
static void set_pixel(void) {
	push(BC); push(HL); push(DE);
	op_ex_af(); I(4);
	ex_de_hl();
	cpu.a = cpu.e; I(4);
	op_and(0xF8); I(7);
	cpu.l = cpu.a; I(4);
	cpu.h = 0; I(7);
	op_rrca(); I(4);
	op_rrca(); I(4);
	cpu.b = cpu.a; I(4);
	cpu.c = cpu.h; I(4);
	SET_HL(op_add16(HL, HL)); I(11);
	SET_HL(op_add16(HL, HL)); I(11);
	SET_HL(op_add16(HL, BC)); I(11);
	cpu.c = cpu.l; I(4);
	cpu.b = cpu.h; I(4);
	cpu.l = cpu.d; I(4);
	cpu.h = 0; I(7);
	SET_HL(op_add16(HL, HL)); I(11);
	SET_HL(op_add16(HL, HL)); I(11);
	SET_HL(op_add16(HL, BC)); I(11);
	SET_BC(0x0020); I(10);
	SET_HL(op_add16(HL, BC)); I(11);
	cpu.c = 0xBF; I(7);
	cpu.iff1 = cpu.iff2 = 0; I(4);
	io_out(BC, cpu.l); I(12);
	io_out(BC, cpu.h); I(12);
	cpu.iff1 = cpu.iff2 = 1;           // ei / ld a,e
	cpu.a = cpu.e; cpu.cycles += 4 + 4;
	native_irq_point();
	op_and(0x07); I(7);
	cpu.a = op_inc8(cpu.a); I(4);
	cpu.b = cpu.a; I(4);
	cpu.a = 0x01; I(7);
	do {
		op_rrca(); I(4);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	cpu.e = cpu.a; I(4);               // the pixel's bit
	op_ex_af(); I(4);
	cpu.d = cpu.a; I(4);               // the colour, a bit a plane
	for (int p = 0; p < 4; p++) {
		cpu.a = io_in((uint16_t)(cpu.a << 8 | 0xBE)); I(11);
		op_or(cpu.e); I(4);
		cpu.d = op_rot(3, cpu.d); I(8);
		if (cpu.f & FC) I(12);
		else { I(7); op_xor(cpu.e); I(4); }
		mem_write((uint16_t)(0xC1C0 + p), cpu.a); I(13);
	}
	cpu.h |= 0x40; I(8);
	cpu.iff1 = cpu.iff2 = 0; I(4);
	io_out(BC, cpu.l); I(12);
	io_out(BC, cpu.h); I(12);
	cpu.iff1 = cpu.iff2 = 1;           // ei / pop de
	SET_DE(pop16()); cpu.cycles += 4 + 10;
	native_irq_point();
	SET_HL(0xC1C0); I(10);
	SET_BC(0x04BE); I(10);
	block_io(3);
	SET_HL(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

// dots ($4ED5): 22 x 22 pixels from (L, H), 6 apart, cleared
static void dots(void) {
	push(BC); push(HL);
	SET_BC(0x1606); I(10);
	do {
		push(BC); push(HL);
		cpu.b = 0x16; I(7);
		do {
			op_xor(cpu.a); I(4);
			I(17); set_pixel();
			cpu.a = cpu.l; I(4);
			op_add8(cpu.c, 0); I(4);
			cpu.l = cpu.a; I(4);
			if (--cpu.b) I(13); else I(8);
		} while (cpu.b);
		SET_HL(pop16()); I(10);
		SET_BC(pop16()); I(10);
		cpu.a = cpu.h; I(4);
		op_add8(cpu.c, 0); I(4);
		cpu.h = cpu.a; I(4);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	SET_HL(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

static void sra4(void) { for (int i = 0; i < 4; i++) { cpu.a = op_rot(5, cpu.a); I(8); } }

// dissolve ($4EAC): dots from places stepped by the list at $4EF1 (a
// signed nibble each for X and Y; $88 ends): the screen goes dark
static void dissolve(void) {
	SET_HL(0x0000); I(10);
	SET_DE(0x4EF1); I(10);
	for (;;) {
		I(17); dots();
		cpu.a = mem_read(DE); I(7);
		op_sub8(0x88, 0, 0); I(7);
		if (cpu.f & FZ) { I(11); return; }
		I(5);
		for (int i = 0; i < 4; i++) { op_rrca(); I(4); }
		sra4();
		op_add8(cpu.l, 0); I(4);
		cpu.l = cpu.a; I(4);
		cpu.a = mem_read(DE); I(7);
		sra4();
		op_add8(cpu.h, 0); I(4);
		cpu.h = cpu.a; I(4);
		SET_DE(DE + 1); I(6);
		I(12);
	}
}

// rst $28 (A = (HL + A)) and rst $20 (HL = the word HL[A], A < 128)
static void add_a_to_hl(void) {
	op_add8(cpu.l, 0); I(4);
	cpu.l = cpu.a; I(4);
	op_add8(cpu.h, cpu.f & FC); I(4);
	op_sub8(cpu.l, 0, 1); I(4);
	cpu.h = cpu.a; I(4);
}
static void rst28(void) {
	I(11);
	add_a_to_hl();
	cpu.a = mem_read(HL); I(7);
	I(10);
}
static void rst20(void) {
	I(11); I(10);
	op_add8(cpu.a, 0); I(4);
	if (!(cpu.f & FC)) I(12);
	else { I(7); cpu.h = op_inc8(cpu.h); I(4); }
	add_a_to_hl();
	cpu.a = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	cpu.h = mem_read(HL); I(7);
	cpu.l = cpu.a; I(4);
	I(10);
}

// fade_row ($B4FB, bank 3): row A of the ending's dissolve (A & 7: which
// pixel row of a tile, from the masks at $B531; A / 8: which tile row, from
// the VRAM addresses at $B539): that pixel row of eleven 48-byte strips
// ANDed with its mask, through $C1C0
static void fade_row(void) {
	cpu.b = cpu.a; I(4);
	op_and(0x07); I(7);
	SET_HL(0xB531); I(10);
	rst28();
	cpu.c = cpu.a; I(4);
	cpu.a = cpu.b; I(4);
	op_and(0xF8); I(7);
	op_rrca(); I(4);
	op_rrca(); I(4);
	op_rrca(); I(4);
	SET_HL(0xB539); I(10);
	rst20();
	cpu.b = 0x0B; I(7);
	do {
		push(BC);
		read_address();
		SET_DE(0xC1C0); I(10);
		cpu.b = 0x30; I(7);
		do {
			cpu.a = io_in((uint16_t)(cpu.a << 8 | 0xBE)); I(11);
			op_and(cpu.c); I(4);
			mem_write(DE, cpu.a); I(7);
			cpu.e = op_inc8(cpu.e); I(4);
			if (--cpu.b) I(13); else I(8);
		} while (cpu.b);
		ex_de_hl();
		write_address();
		SET_HL(0xC1C0); I(10);
		SET_BC(0x30BE); I(10);
		block_io(3);
		SET_HL(0x0030); I(10);
		SET_HL(op_add16(HL, DE)); I(11);
		SET_BC(pop16()); I(10);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	I(10);
}

// wipe ($B55B, bank 3): the ending's screen wiped from both sides to the
// middle, one frame for each of 33 steps of two 8-pixel columns: the
// columns' 16 tiles cleared to 0 (the VDP waited for between bytes), then
// the four bytes at $B5B8 written at the next tile of each side
static void wait_vdp(void) { I(17); I(11); I(10); I(10); }   // call $0262: push af / pop af / ret

static void wipe(void) {
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: it waits for frames
	SET_HL(0x0020); I(10);
	SET_DE(0x022C); I(10);
	cpu.b = 0x21; I(7);
	do {
		I(17); call_original(0x041C);  // wait_vblank
		cpu.c = 0x02; I(7);
		do {
			push(BC); push(DE); push(HL);
			cpu.b = 0x10; I(7);
			do {
				push(BC);
				cpu.b = 0x02; I(7);
				do {
					write_address();
					ex_de_hl();
					op_xor(cpu.a); I(4);
					for (int i = 0; i < 4; i++) {
						wait_vdp();
						io_out((uint16_t)(cpu.a << 8 | 0xBE), cpu.a); I(11);
					}
					if (--cpu.b) I(13); else I(8);
				} while (cpu.b);
				SET_BC(0x0220); I(10);
				SET_HL(op_add16(HL, BC)); I(11);
				ex_de_hl();
				SET_HL(op_add16(HL, BC)); I(11);
				ex_de_hl();
				SET_BC(pop16()); I(10);
				if (--cpu.b) I(13); else I(8);
			} while (cpu.b);
			cpu.b = 0x02; I(7);
			do {
				push(BC);
				write_address();
				ex_de_hl();
				SET_HL(0xB5B8); I(10);
				SET_BC(0x04BE); I(10);
				block_io(3);
				SET_BC(pop16()); I(10);
				if (--cpu.b) I(13); else I(8);
			} while (cpu.b);
			SET_HL(pop16()); I(10);
			SET_DE(pop16()); I(10);
			SET_BC(0x0008); I(10);
			SET_HL(op_add16(HL, BC)); I(11);
			ex_de_hl();
			SET_BC(0xFFF8); I(10);
			SET_HL(op_add16(HL, BC)); I(11);
			ex_de_hl();
			SET_BC(pop16()); I(10);
			cpu.c = op_dec8(cpu.c); I(4);
			if (!(cpu.f & FZ)) I(12);
			else { I(7); break; }
		} while (1);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	I(10);
}

#define EFFECT(name, addr, out) Native native_##name = { #name, addr, 1, name, out, .long_calls = 1 };
Native native_fade_row = { "fade_row", 0xB4FB, 3, fade_row, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, .long_calls = 1 };
Native native_wipe = { "wipe", 0xB55B, 3, wipe, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, .long_calls = 1 };
EFFECT(shake, 0x4117, OUT_A | OUT_F)
EFFECT(shift_block, 0x4173, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
EFFECT(invert, 0x41A9, OUT_A | OUT_F)
EFFECT(invert_tile, 0x41D2, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
EFFECT(invert_palette, 0x41EC, OUT_A | OUT_F)
Native native_forget_screen = { "forget_screen", 0x276D, 0, forget_screen, OUT_F, .long_calls = 1 };
EFFECT(set_pixel, 0x4F15, OUT_A | OUT_F)
EFFECT(dots, 0x4ED5, OUT_A | OUT_F)
EFFECT(dissolve, 0x4EAC, OUT_A | OUT_F | OUT_DE | OUT_HL)
