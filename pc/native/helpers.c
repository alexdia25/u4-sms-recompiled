// Small helpers in bank 0 that the original code still calls.
// - wait_vdp ($0262): push af / pop af, a pause between VDP accesses.
// - far_read ($0815): A = (HL) in bank A.
// - palette_load ($081F all 32, $0830 the background's, $083B the
//   sprites'): the colours at HL into the palette, sent next frame.
// - text_chars ($2D6C): text at HL into the text area's copy at DE; the
//   numbers ($2D7B four digits of HL, $2D81 three, $2D87 two; $2D90 one
//   digit, by BC; $2D9A one character A).
// - people_ground ($3EB7): carry if tile A is not one people walk on (the
//   list at $3EC8); hazard ($3EEF): carry if tile A hurts this pass (lava,
//   swamp, fields: each its own share of the passes, $C0C8).
// - room_monster_at ($38BD): A = which room monster stands at (L, H), or
//   carry for none.
// - window_frame ($3255): the text window's frame (bank $14) into the text
//   area's copy.
// - cursor_glyph ($3493): the 8 bytes at HL as the glyph under the cursor.
// - font ($2FD8): the font, plain, to VRAM $2C00.
// - the name table ($0265) and patterns ($0277, $0281) cleared.
// Cycle counts are the original's, instruction by instruction (I).
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static void push(uint16_t v) { push16(v); I(11); }
static void out_be(uint8_t v) { io_out((uint16_t)(cpu.a << 8 | 0xBE), v); I(11); }

static void wait_vdp(void) {           // $0262
	push((uint16_t)(cpu.a << 8 | cpu.f));
	pop16(); I(10);
	I(10);
}
static void call_wait_vdp(void) { I(17); wait_vdp(); }

static void far_read(void) {           // $0815
	vram_bank_push(0x0815);
	cpu.a = mem_read(HL); I(7);
	vram_bank_pop();
}

static void palette_tail(void) {       // $0825
	while (op_block(6, 0)) I(21);
	I(16);
	cpu.a = 0xFF; I(7);
	mem_write(0xC009, cpu.a); I(13);
	I(10);
}
static void palette_load(void) { SET_DE(0xC023); I(10); SET_BC(0x0020); I(10); palette_tail(); }
static void palette_load_background(void) { SET_DE(0xC023); I(10); SET_BC(0x0010); I(10); I(12); palette_tail(); }
static void palette_load_sprites(void) { SET_DE(0xC033); I(10); SET_BC(0x0010); I(10); I(12); palette_tail(); }

static void put_cell(void) {           // add a,$40 / ld (de),a / inc e / ld a,1 / ld (de),a / inc e
	op_add8(0x40, 0); I(7);
	mem_write(DE, cpu.a); I(7);
	cpu.e = op_inc8(cpu.e); I(4);
	cpu.a = 0x01; I(7);
	mem_write(DE, cpu.a); I(7);
	cpu.e = op_inc8(cpu.e); I(4);
}

static void text_chars(void) {         // $2D6C
	for (;;) {
		cpu.a = mem_read(HL); I(7);
		SET_HL(HL + 1); I(6);
		op_or(cpu.a); I(4);
		if (cpu.f & FZ) { I(11); return; }
		I(5);
		put_cell();
		I(10);
	}
}

static void put_char(void) { put_cell(); I(10); }   // $2D9A

static void digit(void) {              // $2D90: HL / BC as a digit, HL the rest
	op_xor(cpu.a); I(4);
	do {
		op_sbc16(BC); I(15);
		cpu.a = op_inc8(cpu.a); I(4);
		if (cpu.f & FC) { I(7); break; }
		I(12);
	} while (1);
	SET_HL(op_add16(HL, BC)); I(11);
	cpu.a = op_dec8(cpu.a); I(4);
	op_add8(0x30, 0); I(7);
	put_char();
}

static void digits(int from) {         // from 0: thousands, 1: hundreds, 2: tens
	static const uint16_t by[3] = { 1000, 100, 10 };
	for (int i = from; i < 3; i++) { SET_BC(by[i]); I(10); I(17); digit(); }
	cpu.a = cpu.l; I(4);
	I(12);                             // jr $2D98: the units
	op_add8(0x30, 0); I(7);
	put_char();
}
static void number4(void) { digits(0); }   // $2D7B
static void number3(void) { digits(1); }   // $2D81
static void number2(void) { digits(2); }   // $2D87

static void people_ground(void) {      // $3EB7
	push(BC); push(HL);
	SET_HL(0x3EC8); I(10);
	SET_BC(0x0027); I(10);
	while (op_block(6, 1)) I(21);
	I(16);
	int found = cpu.f & FZ;
	op_scf(); I(4);
	if (!found) I(12);
	else { I(7); op_or(cpu.a); I(4); }
	SET_HL(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

static void hazard(void) {             // $3EEF
	push(BC);
	int hit;
	cpu.b = 0x07; I(7);                // lava: every 8th pass
	op_sub8(0x03, 0, 0); I(7);
	if ((hit = cpu.f & FZ)) I(12);
	else {
		I(7);
		cpu.b = 0x03; I(7);            // swamp, poison fields: every 4th
		op_sub8(0x06, 0, 0); I(7);
		if ((hit = cpu.f & FZ)) I(12);
		else {
			I(7);
			op_sub8(0x05, 0, 0); I(7);
			if ((hit = cpu.f & FZ)) I(12);
			else {
				I(7);
				cpu.b = 0x01; I(7);    // the other fields: every other
				op_sub8(0x07, 0, 0); I(7);
				if ((hit = cpu.f & FZ)) I(12);
				else {
					I(7);
					op_sub8(0x46, 0, 0); I(7);
					hit = cpu.f & FZ;
					I(hit ? 7 : 12);
				}
			}
		}
	}
	if (hit) {                         // $3F0A
		cpu.a = mem_read(0xC0C8); I(13);
		op_and(cpu.b); I(4);
		op_scf(); I(4);
		if (cpu.f & FZ) { I(12); goto out; }
		I(7);
	}
	op_or(cpu.a); I(4);                // $3F11
out:
	SET_BC(pop16()); I(10);
	I(10);
}

static void room_monster_at(void) {    // $38BD
	push(BC); push(DE); push16(cpu.iy); I(15);
	cpu.iy = 0xC839; I(14);
	SET_DE(0x0008); I(10);
	cpu.b = 0x10; I(7);
	int found = 0;
	do {
		cpu.a = mem_read((uint16_t)(cpu.iy + 5)); I(19);
		op_or(cpu.a); I(4);
		if (cpu.f & FZ) I(12);
		else {
			I(7);
			cpu.a = mem_read(cpu.iy); I(19);
			op_sub8(cpu.l, 0, 0); I(4);
			if (!(cpu.f & FZ)) I(12);
			else {
				I(7);
				cpu.a = mem_read((uint16_t)(cpu.iy + 1)); I(19);
				op_sub8(cpu.h, 0, 0); I(4);
				if (!(cpu.f & FZ)) I(12);
				else {
					I(7);
					cpu.a = 0x10; I(7);
					op_sub8(cpu.b, 0, 1); I(4);
					I(12);
					found = 1;
					break;
				}
			}
		}
		cpu.iy = op_add16(cpu.iy, DE); I(15);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	if (!found) { op_scf(); I(4); }
	cpu.iy = pop16(); I(14);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

static void window_frame(void) {       // $3255
	SET_DE(0xCA42); I(10);
	cpu.b = 0x12; I(7);
	do {
		push(BC);
		cpu.a = 0x14; I(7);
		SET_BC(0x0022); I(10);
		I(17); call_original(0x07EC);  // far_ldir
		{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
		SET_BC(0x001E); I(10);
		SET_HL(op_add16(HL, BC)); I(11);
		{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
		SET_BC(pop16()); I(10);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	I(10);
}

static void cursor_glyph(void) {       // $3493
	SET_DE(0x25F0); I(10);
	vram_set_write_address();
	op_xor(cpu.a); I(4);
	SET_BC(0x08BE); I(10);
	do {
		out_be(cpu.a);
		call_wait_vdp();
		op_block(4, 3); I(16);         // outi
		call_wait_vdp();
		out_be(cpu.a);
		call_wait_vdp();
		out_be(cpu.a);
		call_wait_vdp();
		I(10);
	} while (!(cpu.f & FZ));
	I(10);
}

static void font(void) {               // $2FD8
	cpu.a = 0x0F; I(7);
	vram_bank_push(0x2FDA);
	SET_DE(0x2C00); I(10);
	vram_set_write_address();
	SET_HL(0xBB58); I(10);
	cpu.c = 0xBE; I(7);
	cpu.d = 0x60; I(7);
	op_xor(cpu.a); I(4);
	do {
		cpu.b = 0x08; I(7);
		for (;;) {
			op_block(4, 3); I(16);     // outi
			out_be(cpu.a);
			out_be(cpu.a);
			out_be(cpu.a);
			if (cpu.f & FZ) { I(7); break; }
			I(12);
		}
		cpu.d = op_dec8(cpu.d); I(4);
		if (cpu.f & FZ) { I(7); break; }
		I(12);
	} while (1);
	vram_bank_pop();
}

static void clear_name_table(void) {   // $0265, then jp vram_fill
	op_xor(cpu.a); I(4);
	mem_write(0xC0A8, cpu.a); I(13);
	cpu.l = cpu.a; I(4);
	cpu.a = mem_read(0xC0A8); I(13);
	cpu.h = cpu.a; I(4);
	SET_DE(0x3800); I(10);
	SET_BC(0x0380); I(10);
	I(10);
	native_continue_at(0x022A);
}
static void clear_patterns(uint16_t de, uint16_t bc) {   // then jp vram_fill_bytes
	SET_DE(de); I(10);
	SET_BC(bc); I(10);
	op_xor(cpu.a); I(4);
	I(10);
	native_continue_at(0x0219);
}
static void clear_all_patterns(void) { clear_patterns(0x0000, 0x2000); }   // $0277
static void clear_upper_patterns(void) { clear_patterns(0x2000, 0x1800); } // $0281

#define HELPER(name, addr, out) Native native_##name = { #name, addr, 0, name, out, .long_calls = 1 };
HELPER(wait_vdp, 0x0262, 0)
HELPER(far_read, 0x0815, OUT_A)
HELPER(palette_load, 0x081F, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
HELPER(palette_load_background, 0x0830, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
HELPER(palette_load_sprites, 0x083B, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
HELPER(text_chars, 0x2D6C, OUT_A | OUT_F | OUT_DE | OUT_HL)
HELPER(number4, 0x2D7B, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
HELPER(number3, 0x2D81, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
HELPER(number2, 0x2D87, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
HELPER(digit, 0x2D90, OUT_A | OUT_F | OUT_DE | OUT_HL)
HELPER(put_char, 0x2D9A, OUT_A | OUT_F | OUT_DE)
HELPER(people_ground, 0x3EB7, OUT_A | OUT_F)
HELPER(hazard, 0x3EEF, OUT_A | OUT_F)
HELPER(room_monster_at, 0x38BD, OUT_A | OUT_F)
HELPER(window_frame, 0x3255, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
HELPER(cursor_glyph, 0x3493, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
HELPER(font, 0x2FD8, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
HELPER(clear_name_table, 0x0265, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
HELPER(clear_all_patterns, 0x0277, OUT_A | OUT_F | OUT_BC | OUT_DE)
HELPER(clear_upper_patterns, 0x0281, OUT_A | OUT_F | OUT_BC | OUT_DE)
