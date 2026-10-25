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
// - read_pads ($064C): both pads' six buttons as one byte, 1 = held.
// - terrain_edges ($1204): for each open cell of the 11x11 view at $D800
//   (two bytes a cell: the tile, then its edge bits), a bit set for each
//   side whose neighbour is a wall-like tile that doesn't join it (bit 0
//   above, 1 below, 2 left, 3 right); $17, $19, $1A and $48 never count.
// - dungeon_header ($2A6E): A = the first byte, HL = the word after it, of
//   the current dungeon's 3-byte entry at $2A81.
// - place_index ($33EE): the location less 2 (the castle's floors as 0).
// - unpack_position ($3EAA): the byte at HL as two nibbles, the high one
//   in A', the low one in A.
// - aim ($2BFE): the step from (L, H) towards (E, D): HL = its x part,
//   DE = its y part, from the tables at $2C7C and $2CBD by six bits of the
//   smaller distance over the larger, signs and swap by the octant (C).
// - text_cell ($2D0C): DE = the address in the text area's copy ($CA00 +
//   row * 64 + column * 2) of column E, row D.
// - selection_blink ($3095): picture cell 0 at HL (put_picture_cell,
//   $30B6), and colour $1F the next of the six shades at $30CB ($C63B).
// - queue_glyphs ($30DA): $100 bytes of bank $13 from $A40F + A * 256
//   queued for VRAM $2920.
// - times_11 ($36ED), times_12 ($36F7): A times 11 or 12.
// - far_word ($0807): HL = the word HL[C] in bank A.
// - input_reset ($06E2): the pads as if every button were held and none
//   just pressed ($C0AF = $FF, $C0B0 = $C0B2 = $C0B5 = 0, direction 8).
// - input_start ($06CD): the pads read apart ($C0AE = 0), input_reset,
//   then key repeat 1 ($06D4: $C0B3 = 1); input_repeat ($06DA): key repeat
//   HL ($C0B3), and $C0B2 = 0. restart_input ($0DD7): input_start, then
//   every task cleared.
// - person_blocks ($3974): carry when tile A is not one people walk on
//   ($3EB7), unless it is one of the eight at $3989.
// - window_redraw ($323D): the text window's frame again and its 18x17
//   cells queued from $CA42, then a wait for the vertical blank.
// - slot_step_add ($0BA4) / slot_step_sub ($0BC9): the task's 16-bit
//   place (IX+$0A/$0B, $0C/$0D) moved by its step (IX+$0E/$0F, $10/$11).
// - end_tasks_of ($0B6D): every task slot ($C300, 10 x 48 bytes) of kind A
//   cleared.
// - tile_effect ($3F67): what the tile under the party ($C990) does: $03
//   and $44 ($4071), $4C and $46 ($4812), $47 ($404A).
// - screen_place ($3991): (L, H) cells as pixels (12 a cell);
//   screen_cell ($399C): back from the missile's pixels (IX+$0B, $0D,
//   from a cell's middle) to its cell (L, H).
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

static void read_pads(void) {          // $064C
	cpu.a = io_in((uint16_t)(cpu.a << 8 | 0xDC)); I(11);
	cpu.b = cpu.a; I(4);
	cpu.a = io_in((uint16_t)(cpu.a << 8 | 0xDD)); I(11);
	cpu.b = op_rot(2, cpu.b); I(8);    // rl b / rla, twice: the second pad's
	op_rla(); I(4);                    // first two buttons, from port $DC's top
	cpu.b = op_rot(2, cpu.b); I(8);
	op_rla(); I(4);
	op_cpl(); I(4);
	op_and(0x3F); I(7);
	I(10);
}

static int ret_z(void) { if (cpu.f & FZ) { I(11); return 1; } I(5); return 0; }
static int ret_c(void) { if (cpu.f & FC) { I(11); return 1; } I(5); return 0; }
static int cp_ret_z(uint8_t v) { op_sub8(v, 0, 0); I(7); return ret_z(); }

static int joins_anything(void) {      // $1291: Z for the tiles that join every side
	if (cp_ret_z(0x17) || cp_ret_z(0x19) || cp_ret_z(0x1A)) return 1;
	op_sub8(0x48, 0, 0); I(7);
	I(10);
	return 0;
}

// $1243 / $1255 / $1267 / $127C: the neighbour on one side (A, or the cell
// at IY + from); this cell's edge bit set unless the neighbour joins
// anything, is one of the two tiles that join this side (k1, k2), or is
// open itself (below 3)
static void edge(int8_t from, uint8_t k1, uint8_t k2, int bit) {
	I(17);
	if (from) { cpu.a = mem_read((uint16_t)(cpu.iy + from)); I(19); }
	I(17);
	joins_anything();
	if (ret_z()) return;
	if (cp_ret_z(k1) || cp_ret_z(k2)) return;
	op_sub8(0x03, 0, 0); I(7);
	if (ret_c()) return;
	uint16_t at = (uint16_t)(cpu.iy + 1);
	mem_write(at, mem_read(at) | 1 << bit); I(23);
	I(10);
}
static void no_call(void) { I(10); }   // call nz, not taken

static void terrain_edges(void) {      // $1204
	push(BC); push(DE); push(HL);
	push16(cpu.iy); I(15);
	cpu.iy = 0xD800; I(14);
	SET_DE(0x0002); I(10);
	SET_HL(0x0B01); I(10);
	cpu.c = cpu.h; I(4);
	do {
		cpu.b = cpu.h; I(4);
		do {
			cpu.a = mem_read(cpu.iy); I(19);
			op_sub8(0x03, 0, 0); I(7);
			if (!(cpu.f & FC)) I(12);
			else {
				I(7);
				cpu.a = cpu.c; I(4);
				op_sub8(cpu.h, 0, 0); I(4);
				cpu.a = mem_read((uint16_t)(cpu.iy - 0x16)); I(19);
				if (cpu.f & FZ) no_call(); else edge(0, 0x33, 0x34, 0);
				cpu.a = cpu.c; I(4);
				op_sub8(cpu.l, 0, 0); I(4);
				cpu.a = mem_read((uint16_t)(cpu.iy + 0x16)); I(19);
				if (cpu.f & FZ) no_call(); else edge(0, 0x31, 0x32, 1);
				cpu.a = cpu.b; I(4);
				op_sub8(cpu.h, 0, 0); I(4);
				if (cpu.f & FZ) no_call(); else edge(-2, 0x31, 0x33, 2);
				cpu.a = cpu.b; I(4);
				op_sub8(cpu.l, 0, 0); I(4);
				if (cpu.f & FZ) no_call(); else edge(2, 0x32, 0x34, 3);
			}
			cpu.iy = op_add16(cpu.iy, DE); I(15);
			if (--cpu.b) I(13); else I(8);
		} while (cpu.b);
		cpu.c = op_dec8(cpu.c); I(4);
		if (!(cpu.f & FZ)) I(12); else I(7);
	} while (cpu.c);
	cpu.iy = pop16(); I(14);
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

static void dungeon_header(void) {     // $2A6E
	cpu.a = mem_read(0xC9A0); I(13);
	op_sub8(0x12, 0, 1); I(7);
	cpu.b = cpu.a; I(4);
	op_add8(cpu.a, 0); I(4);
	op_add8(cpu.b, 0); I(4);
	SET_HL(0x2A81); I(10);
	I(11);                             // rst $30: HL += A
	op_add8(cpu.l, 0); I(4);
	cpu.l = cpu.a; I(4);
	if (!(cpu.f & FC)) I(11);
	else { I(5); cpu.h = op_inc8(cpu.h); I(4); I(10); }
	cpu.a = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	cpu.e = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	cpu.d = mem_read(HL); I(7);
	uint16_t t = HL; SET_HL(DE); SET_DE(t); I(4);
	I(10);
}

static void place_index(void) {        // $33EE
	cpu.a = mem_read(0xC9A0); I(13);
	op_sub8(0x02, 0, 1); I(7);
	op_add8(0x00, cpu.f & FC); I(7);
	I(10);
}

static void unpack_position(void) {    // $3EAA
	cpu.a = mem_read(HL); I(7);
	op_rlca(); I(4);
	op_rlca(); I(4);
	op_rlca(); I(4);
	op_rlca(); I(4);
	op_and(0x0F); I(7);
	op_ex_af(); I(4);
	cpu.a = mem_read(HL); I(7);
	op_and(0x0F); I(7);
	SET_HL(HL + 1); I(6);
	I(10);
}

static int sub_a_l_step(void) {        // add a,a / jr c / cp l / jr c / sub l / inc e
	op_add8(cpu.a, 0); I(4);
	if (cpu.f & FC) I(12);
	else {
		I(7);
		op_sub8(cpu.l, 0, 0); I(4);
		if (cpu.f & FC) { I(12); return 0; }
		I(7);
	}
	op_sub8(cpu.l, 0, 1); I(4);
	cpu.e = op_inc8(cpu.e); I(4);
	return 1;
}

static void negate_into(uint8_t *lo, uint8_t *hi) {   // xor a / sub lo / ld lo,a / sbc a,a / sub hi / ld hi,a
	op_xor(cpu.a); I(4);
	op_sub8(*lo, 0, 1); I(4);
	*lo = cpu.a; I(4);
	op_sub8(cpu.a, cpu.f & FC, 1); I(4);
	op_sub8(*hi, 0, 1); I(4);
	*hi = cpu.a; I(4);
}

static void aim(void) {                // $2BFE
	SET_BC(0x0000); I(10);
	cpu.a = cpu.e; I(4);
	op_sub8(cpu.l, 0, 1); I(4);
	if (!(cpu.f & FC)) I(12);
	else { I(7); cpu.c = op_inc8(cpu.c); I(4); op_neg(); I(8); }
	cpu.l = cpu.a; I(4);
	cpu.a = cpu.d; I(4);
	op_sub8(cpu.h, 0, 1); I(4);
	if (!(cpu.f & FC)) I(12);
	else { I(7); cpu.c |= 0x02; I(8); op_neg(); I(8); }
	cpu.h = cpu.a; I(4);
	op_sub8(cpu.l, 0, 0); I(4);
	if (cpu.f & FC) I(12);
	else { I(7); cpu.c |= 0x04; I(8); cpu.h = cpu.l; I(4); cpu.l = cpu.a; I(4); }
	cpu.d = cpu.b; I(4);
	cpu.e = cpu.b; I(4);
	cpu.a = cpu.h; I(4);
	sub_a_l_step();                    // six bits of the smaller over the larger
	for (int i = 0; i < 5; i++) {
		cpu.e = op_rot(4, cpu.e); I(8);   // sla e
		sub_a_l_step();
	}
	SET_HL(0x2C7C); I(10);
	SET_HL(op_add16(HL, DE)); I(11);
	cpu.a = mem_read(HL); I(7);
	SET_HL(0x2CBD); I(10);
	SET_HL(op_add16(HL, DE)); I(11);
	cpu.l = mem_read(HL); I(7);
	cpu.e = cpu.a; I(4);
	cpu.d = cpu.b; I(4);
	cpu.h = cpu.b; I(4);
	op_bit(2, cpu.c); I(8);
	if (cpu.f & FZ) I(12);
	else { I(7); uint16_t t = HL; SET_HL(DE); SET_DE(t); I(4); }
	op_bit(1, cpu.c); I(8);
	if (cpu.f & FZ) I(12);
	else { I(7); negate_into(&cpu.e, &cpu.d); }
	op_bit(0, cpu.c); I(8);
	if (cpu.f & FZ) { I(11); return; }
	I(5);
	negate_into(&cpu.l, &cpu.h);
	I(10);
}

static void text_cell(void) {          // $2D0C
	cpu.a = cpu.e; I(4);
	for (int i = 0; i < 3; i++) { op_add8(cpu.a, 0); I(4); }
	cpu.e = cpu.a; I(4);
	cpu.a = cpu.d; I(4);
	op_rra(); I(4);
	cpu.e = op_rot(3, cpu.e); I(8);    // rr e
	op_rra(); I(4);
	cpu.e = op_rot(3, cpu.e); I(8);
	op_add8(0xCA, 0); I(7);
	cpu.d = cpu.a; I(4);
	I(10);
}

static void selection_blink(void) {    // $3095
	push(BC); push(DE); push(HL);
	op_xor(cpu.a); I(4);
	I(17); call_original(0x30B6);      // put_picture_cell
	cpu.a = mem_read(0xC63B); I(13);
	cpu.a = op_inc8(cpu.a); I(4);
	op_sub8(0x06, 0, 0); I(7);
	if (cpu.f & FC) I(12);
	else { I(7); op_xor(cpu.a); I(4); }
	mem_write(0xC63B, cpu.a); I(13);
	SET_HL(0x30CB); I(10);
	I(11);                             // rst $28: A = (HL + A)
	op_add8(cpu.l, 0); I(4);
	cpu.l = cpu.a; I(4);
	op_add8(cpu.h, cpu.f & FC); I(4);
	op_sub8(cpu.l, 0, 1); I(4);
	cpu.h = cpu.a; I(4);
	cpu.a = mem_read(HL); I(7);
	I(10);
	cpu.h = cpu.a; I(4);
	cpu.l = 0x1F; I(7);
	I(17); call_original(0x0843);      // set_palette_entry
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

static void queue_glyphs(void) {       // $30DA
	// shadow mode: the queue may be full, and the original waits for room
	if (!native_on_fiber() && mem_read(0xC063) >= 0x20) { native_decline(); return; }
	push(BC); push(DE); push(HL);
	op_add8(0xA4, 0); I(7);
	cpu.h = cpu.a; I(4);
	cpu.l = 0x0F; I(7);
	cpu.a = 0x13; I(7);
	SET_DE(0x2920); I(10);
	SET_BC(0x0100); I(10);
	I(17); call_original(0x0465);      // queue_vram_copy
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

static void times_11(void) {           // $36ED
	push(BC);
	cpu.b = cpu.a; I(4);
	op_add8(cpu.a, 0); I(4);
	cpu.c = cpu.a; I(4);
	op_add8(cpu.a, 0); I(4);
	op_add8(cpu.a, 0); I(4);
	op_add8(cpu.b, 0); I(4);
	op_add8(cpu.c, 0); I(4);
	SET_BC(pop16()); I(10);
	I(10);
}

static void times_12(void) {           // $36F7
	push(BC);
	op_add8(cpu.a, 0); I(4);
	op_add8(cpu.a, 0); I(4);
	cpu.b = cpu.a; I(4);
	op_add8(cpu.a, 0); I(4);
	op_add8(cpu.b, 0); I(4);
	SET_BC(pop16()); I(10);
	I(10);
}
static uint8_t times_12_of(uint8_t v) { cpu.a = v; I(17); times_12(); return cpu.a; }

static void screen_place(void) {       // $3991
	cpu.a = cpu.l; I(4);
	cpu.l = times_12_of(cpu.a); I(4);
	cpu.a = cpu.h; I(4);
	cpu.h = times_12_of(cpu.a); I(4);
	I(10);
}

static void screen_cell(void) {        // $399C
	cpu.a = mem_read((uint16_t)(cpu.ix + 0x0D)); I(19);
	op_add8(0x06, 0); I(7);
	cpu.l = cpu.a; I(4);
	cpu.h = 0x0C; I(7);
	I(17); call_original(0x2AFC);      // divide_8
	cpu.b = cpu.l; I(4);
	cpu.a = mem_read((uint16_t)(cpu.ix + 0x0B)); I(19);
	op_add8(0x06, 0); I(7);
	cpu.l = cpu.a; I(4);
	cpu.h = 0x0C; I(7);
	I(17); call_original(0x2AFC);
	cpu.h = cpu.b; I(4);
	I(10);
}

static void far_word(void) {           // $0807
	vram_bank_push(0x0807);
	cpu.b = 0x00; I(7);
	SET_HL(op_add16(HL, BC)); I(11);
	SET_HL(op_add16(HL, BC)); I(11);
	cpu.a = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	cpu.h = mem_read(HL); I(7);
	cpu.l = cpu.a; I(4);
	vram_bank_pop();
}

static void input_reset(void) {        // $06E2
	op_xor(cpu.a); I(4);
	mem_write(0xC0B0, cpu.a); I(13);
	mem_write(0xC0B2, cpu.a); I(13);
	mem_write(0xC0B5, cpu.a); I(13);
	op_cpl(); I(4);
	mem_write(0xC0AF, cpu.a); I(13);
	cpu.a = 0x08; I(7);
	mem_write(0xC0B1, cpu.a); I(13);
	I(10);
}

static void input_repeat(void) {       // $06DA
	wr16(0xC0B3, HL); I(16);
	op_xor(cpu.a); I(4);
	mem_write(0xC0B2, cpu.a); I(13);
	I(10);
}

static void input_repeat_1(void) {     // $06D4
	cpu.a = 0x01; I(7);
	mem_write(0xC0B3, cpu.a); I(13);
	I(10);
}

static void input_start(void) {        // $06CD
	op_xor(cpu.a); I(4);
	mem_write(0xC0AE, cpu.a); I(13);
	I(17); input_reset();
	input_repeat_1();                  // falls into it
}

static void restart_input(void) {      // $0DD7
	I(17); input_start();
	I(10);
	native_continue_at(0x0B21);        // jp clear_tasks
}

static void person_blocks(void) {      // $3974
	I(17); call_original(0x3EB7);      // people_ground
	if (!(cpu.f & FC)) { I(11); return; }
	I(5);
	push(BC); push(HL);
	SET_HL(0x3989); I(10);
	SET_BC(0x0008); I(10);
	while (op_block(6, 1)) I(21);      // cpir
	I(16);
	int found = cpu.f & FZ;
	op_scf(); I(4);
	if (!found) I(12);
	else { I(7); op_or(cpu.a); I(4); }
	SET_HL(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

static void window_redraw(void) {      // $323D
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: it waits for a frame
	push(BC); push(DE); push(HL);
	SET_HL(0xB748); I(10);
	I(17); call_original(0x3255);      // window_frame
	SET_DE(0xCA42); I(10);
	SET_BC(0x1211); I(10);
	I(17); call_original(0x0499);      // queue_text_copy
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
	native_continue_at(0x041C);        // jp wait_vblank
}

static uint8_t ix_at(uint8_t d) { return mem_read((uint16_t)(cpu.ix + d)); }

static void slot_step(int sub) {       // $0BA4 / $0BC9
	for (int k = 0; k < 2; k++) {
		uint8_t at = (uint8_t)(0x0A + 2 * k), by = (uint8_t)(0x0E + 2 * k);
		cpu.a = ix_at(at); I(19);
		if (sub) op_sub8(ix_at(by), 0, 1); else op_add8(ix_at(by), 0);
		I(19);
		mem_write((uint16_t)(cpu.ix + at), cpu.a); I(19);
		cpu.a = ix_at((uint8_t)(at + 1)); I(19);
		if (sub) op_sub8(ix_at((uint8_t)(by + 1)), cpu.f & FC, 1); else op_add8(ix_at((uint8_t)(by + 1)), cpu.f & FC);
		I(19);
		mem_write((uint16_t)(cpu.ix + at + 1), cpu.a); I(19);
	}
	I(10);
}
static void slot_step_add(void) { slot_step(0); }
static void slot_step_sub(void) { slot_step(1); }

static void end_tasks_of(void) {       // $0B6D
	SET_HL(0xC300); I(10);
	SET_DE(0x0030); I(10);
	SET_BC(0x0A00); I(10);
	do {
		op_sub8(mem_read(HL), 0, 0); I(7);
		if (!(cpu.f & FZ)) I(12);
		else {
			I(7);
			push(BC); push(HL);
			cpu.d = cpu.h; I(4);
			cpu.e = cpu.l; I(4);
			cpu.e = op_inc8(cpu.e); I(4);
			mem_write(HL, cpu.c); I(7);
			SET_BC(0x002F); I(10);
			while (op_block(6, 0)) I(21);   // ldir: the slot cleared
			I(16);
			SET_HL(pop16()); I(10);
			SET_BC(pop16()); I(10);
			SET_DE(0x0030); I(10);
		}
		SET_HL(op_add16(HL, DE)); I(11);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	I(10);
}

static void tile_effect(void) {        // $3F67
	static const struct { uint8_t tile; uint16_t to; } effects[5] = {
		{ 0x03, 0x4071 }, { 0x44, 0x4071 }, { 0x4C, 0x4812 }, { 0x46, 0x4812 }, { 0x47, 0x404A },
	};
	cpu.a = mem_read(0xC990); I(13);
	for (int i = 0; i < 5; i++) {
		op_sub8(effects[i].tile, 0, 0); I(7);
		I(10);
		if (cpu.f & FZ) { native_continue_at(effects[i].to); return; }   // jp z
	}
	I(10);
}

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
HELPER(read_pads, 0x064C, OUT_A | OUT_F | OUT_BC)
HELPER(terrain_edges, 0x1204, OUT_A | OUT_F)
HELPER(dungeon_header, 0x2A6E, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
HELPER(place_index, 0x33EE, OUT_A | OUT_F)
HELPER(unpack_position, 0x3EAA, OUT_A | OUT_F | OUT_HL)
HELPER(aim, 0x2BFE, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
HELPER(text_cell, 0x2D0C, OUT_A | OUT_F | OUT_DE)
HELPER(selection_blink, 0x3095, OUT_A | OUT_F)
HELPER(queue_glyphs, 0x30DA, OUT_A | OUT_F)
HELPER(times_11, 0x36ED, OUT_A | OUT_F)
HELPER(times_12, 0x36F7, OUT_A | OUT_F)
HELPER(screen_place, 0x3991, OUT_A | OUT_F | OUT_HL)
HELPER(screen_cell, 0x399C, OUT_A | OUT_F | OUT_BC | OUT_HL)
HELPER(far_word, 0x0807, OUT_A | OUT_F | OUT_BC | OUT_HL)
HELPER(input_reset, 0x06E2, OUT_A | OUT_F)
HELPER(input_repeat, 0x06DA, OUT_A | OUT_F)
HELPER(input_repeat_1, 0x06D4, OUT_A | OUT_F)
HELPER(input_start, 0x06CD, OUT_A | OUT_F)
HELPER(restart_input, 0x0DD7, OUT_A | OUT_F)
HELPER(person_blocks, 0x3974, OUT_A | OUT_F)
HELPER(window_redraw, 0x323D, OUT_A | OUT_F)
HELPER(tile_effect, 0x3F67, OUT_A | OUT_F)
HELPER(slot_step_add, 0x0BA4, OUT_A | OUT_F)
HELPER(slot_step_sub, 0x0BC9, OUT_A | OUT_F)
HELPER(end_tasks_of, 0x0B6D, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
