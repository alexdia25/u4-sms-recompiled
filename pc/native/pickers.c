// The pickers a waiting task steps once a frame (bank 1): each reads the
// pad and returns Z (nothing picked yet), carry (button 1: cancelled) or
// neither (button 2: picked).
// - direction_pick ($4C42, A = the keys, HL = the table at $4C51): the
//   direction in IX+$1A (0 up, 1 right, 2 down, 3 left, 4 none), its arrow
//   blinked at the cell from $4D00; button 2 with a direction prints its
//   name ($55A0) and a new line.
// - neighbour_pick ($4C8F): the same, the arrow next to the current
//   figure ($4CA3: the direction's step, $40B9, from its place), and only
//   to a cell still on the 11 x 11 view.
// - letter_grid_start ($4D0A): the letter grid ($4E8A, 5 columns x 6 rows,
//   End at column 3 of row 5) for a word of up to A letters, at $C065; the
//   menu at $4E41.
// - letter_grid ($4D2B): the cursor (column IX+$0B, row IX+$0D; IX+$11 =
//   1 on End) moved round the grid, drawn (put_picture) and its colour the
//   next of the six at $4EA6; button 2 types the letter (or ends the
//   word), button 1 takes the last back (carry when there is none).
// Cycle counts are the original's, instruction by instruction (I).
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define SLOT  cpu.ix
#define WORD  0xC065
#define JOY   0xC0B0

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static void call(uint16_t addr) { I(17); call_original(addr); }

static uint8_t ix_at(uint8_t d) { return mem_read((uint16_t)(SLOT + d)); }
static void ix_set(uint8_t d, uint8_t v) { mem_write((uint16_t)(SLOT + d), v); }

static void add_a_to_hl(void) {        // add a,l / ld l,a / adc a,h / sub l / ld h,a
	op_add8(cpu.l, 0); I(4);
	cpu.l = cpu.a; I(4);
	op_add8(cpu.h, cpu.f & FC); I(4);
	op_sub8(cpu.l, 0, 1); I(4);
	cpu.h = cpu.a; I(4);
}
static void rst20(void) {              // HL = the word HL[A]
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
static void rst28(void) {              // A = (HL + A)
	I(11);
	add_a_to_hl();
	cpu.a = mem_read(HL); I(7);
	I(10);
}
static void rst30(void) {              // HL += A
	I(11);
	op_add8(cpu.l, 0); I(4);
	cpu.l = cpu.a; I(4);
	if (!(cpu.f & FC)) { I(11); return; }
	I(5);
	cpu.h = op_inc8(cpu.h); I(4);
	I(10);
}

// call switch_on_a ($334A) from `back` - 3, on the table at HL: the
// address it jumps to, or 0 for no match (it returns, Z set, B = 0).
static uint16_t switch_on(uint16_t back) {
	I(17);
	for (;;) {
		cpu.b = mem_read(HL); I(7);
		cpu.b = op_inc8(cpu.b); I(4);
		if (cpu.f & FZ) { I(11); return 0; }
		I(5);
		op_sub8(mem_read(HL), 0, 0); I(7);   // cp (hl)
		SET_HL(HL + 1); I(6);
		if (cpu.f & FZ) break;
		I(7);
		SET_HL(HL + 2); I(6); I(6);
		I(10);
	}
	I(12);
	cpu.a = back >> 8; cpu.f = (uint8_t)back; I(10);   // pop af: its return
	cpu.a = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	cpu.h = mem_read(HL); I(7);
	cpu.l = cpu.a; I(4);
	I(4);                              // jp (hl)
	return HL;
}

// $4C64-$4C6D: ld a,n then the "ld bc" that skip the other entries
static void direction_key(uint16_t to) {
	static const uint8_t dir[4] = { 1, 3, 0, 2 };
	int k = (to - 0x4C64) / 3;         // also $4CC7.. (k by the same steps)
	cpu.a = dir[k]; I(7);
	static const uint16_t bc[3] = { 0x033E, 0x003E, 0x023E };
	for (int i = k; i < 3; i++) { SET_BC(bc[i]); I(10); }
}

static int printing_key(uint8_t key) { return key == 0x10 || key == 0x20; }

static void picked(void) {             // $4C84 / $4CF5: the name, a new line, then NZ
	call(0x55A0);
	call(0x54CE);                      // print_newline
	op_xor(cpu.a); I(4);
	cpu.a = op_dec8(cpu.a); I(4);
	I(10);
	native_continue_at(0x40C9);        // sound_0c, its ret ours
}

static void cancelled(void) {          // $4C77 / $4CE8: carry
	cpu.a = 0xFF; I(7);
	op_add8(cpu.a, 0); I(4);
	I(10);
	native_continue_at(0x40D1);        // sound_0d, its ret ours
}

static void direction_pick(void) {     // $4C42
	if (printing_key(cpu.a) && !native_on_fiber()) { native_decline(); return; }   // shadow mode: printing may wait
	uint16_t to = switch_on(0x4C45);
	if (to == 0x4C77) { cancelled(); return; }
	if (to == 0x4C7D) {
		cpu.a = ix_at(0x1A); I(19);
		op_sub8(0x04, 0, 0); I(7);
		if (!(cpu.f & FZ)) { I(7); picked(); return; }
		I(12);
	} else if (to) {
		direction_key(to);
		ix_set(0x1A, cpu.a); I(19);
		call(0x40D9);                  // sound_1e
		I(12);
	}
	// $4C45
	cpu.a = ix_at(0x1A); I(19);
	SET_HL(0x4D00); I(10);
	rst20();
	call(0x3095);                      // selection_blink
	op_xor(cpu.a); I(4);
	I(10);
}

static void next_to_figure(void) {     // $4CA3: HL = the cell a step (A) from the figure
	call(0x40B9);
	call(0x48E1);                      // current_object
	cpu.a = mem_read(cpu.iy); I(19);
	op_add8(cpu.l, 0); I(4);
	cpu.l = cpu.a; I(4);
	cpu.a = mem_read((uint16_t)(cpu.iy + 1)); I(19);
	op_add8(cpu.h, 0); I(4);
	cpu.h = cpu.a; I(4);
	I(10);
}

static void neighbour_pick(void) {     // $4C8F
	cpu.a = mem_read(JOY); I(13);
	if (printing_key(cpu.a) && !native_on_fiber()) { native_decline(); return; }
	SET_HL(0x4CB4); I(10);
	uint16_t to = switch_on(0x4C98);
	if (to == 0x4CE8) { cancelled(); return; }
	if (to == 0x4CEE) {
		cpu.a = ix_at(0x1A); I(19);
		op_sub8(0x04, 0, 0); I(7);
		if (!(cpu.f & FZ)) { I(7); picked(); return; }
		I(12);
	} else if (to) {
		direction_key((uint16_t)(to - 0x4CC7 + 0x4C64));
		cpu.d = cpu.a; I(4);
		I(17); next_to_figure();
		cpu.a = cpu.l; I(4);
		op_sub8(0x0B, 0, 0); I(7);
		if (cpu.f & FC) {
			I(7);
			cpu.a = cpu.h; I(4);
			op_sub8(0x0B, 0, 0); I(7);
			if (cpu.f & FC) {
				I(7);
				ix_set(0x1A, cpu.d); I(19);
				call(0x40D9);
				I(12);
			} else I(12);
		} else I(12);
	}
	// $4C98
	cpu.a = ix_at(0x1A); I(19);
	I(17); next_to_figure();
	call(0x3095);
	op_xor(cpu.a); I(4);
	I(10);
}

static void letter_grid_start(void) {  // $4D0A
	ix_set(0x0E, cpu.a); I(19);
	ix_set(0x0F, 0x00); I(19);
	ix_set(0x10, 0xFF); I(19);
	ix_set(0x11, 0x00); I(19);
	ix_set(0x0B, 0x00); I(19);
	ix_set(0x0D, 0x00); I(19);
	op_xor(cpu.a); I(4);
	mem_write(WORD, cpu.a); I(13);
	SET_HL(0x4E41); I(10);
	I(10);
	native_continue_at(0x2E7F);        // menu_draw
}

static void wrap_row(void) {           // $4DA2..$4DAF: row A, round the six
	if (cpu.f & FS) { I(10); cpu.a = 0x05; I(7); }
	else I(10);
	op_sub8(0x06, 0, 0); I(7);
	if (cpu.f & FC) I(12);
	else { I(7); op_xor(cpu.a); I(4); }
	ix_set(0x0D, cpu.a); I(19);
	I(12);
}

static void letter_grid(void) {        // $4D2B
	uint8_t key = mem_read(JOY) & 0x7F;
	if (printing_key(key) && !native_on_fiber()) { native_decline(); return; }
	cpu.a = mem_read(JOY); I(13);
	op_and(0x7F); I(7);
	SET_HL(0x4D83); I(10);
	uint16_t to = switch_on(0x4D36);
	switch (to) {
	case 0x4D96:                       // up
	case 0x4D9A:                       // down
		if (to == 0x4D96) { cpu.a = 0xFF; I(7); I(12); }
		else { cpu.a = 0x01; I(7); }
		call(0x40D9);
		op_add8(ix_at(0x0D), 0); I(19);
		wrap_row();
		break;
	case 0x4DB1:                       // left
		call(0x40D9);
		cpu.a = ix_at(0x0B); I(19);
		cpu.a = op_dec8(cpu.a); I(4);
		if (cpu.f & FS) { I(10); cpu.a = 0x04; I(7); }
		else I(10);
		ix_set(0x0B, cpu.a); I(19);
		op_sub8(0x03, 0, 0); I(7);
		if (!(cpu.f & FZ)) I(12);
		else {
			I(7);
			cpu.a = ix_at(0x0D); I(19);
			op_sub8(0x05, 0, 0); I(7);
			if (cpu.f & FC) I(12);
			else { I(7); ix_set(0x0B, 0x02); I(19); }
		}
		I(10);
		break;
	case 0x4DD2:                       // right
		call(0x40D9);
		cpu.a = ix_at(0x0B); I(19);
		cpu.a = op_inc8(cpu.a); I(4);
		op_sub8(0x05, 0, 0); I(7);
		if (cpu.f & FC) I(12);
		else { I(7); op_xor(cpu.a); I(4); }
		ix_set(0x0B, cpu.a); I(19);
		op_sub8(0x04, 0, 0); I(7);
		if (!(cpu.f & FZ)) I(12);
		else {
			I(7);
			cpu.a = ix_at(0x0D); I(19);
			op_sub8(0x05, 0, 0); I(7);
			if (cpu.f & FC) I(12);
			else { I(7); ix_set(0x0B, 0x00); I(19); }
		}
		I(10);
		break;
	case 0x4DF3:                       // button 1: the last letter back
		cpu.a = ix_at(0x0F); I(19);
		cpu.a = op_dec8(cpu.a); I(4);
		op_scf(); I(4);
		if (cpu.f & FS) { I(11); return; }
		I(5);
		call(0x40D1);
		ix_set(0x0F, cpu.a); I(19);
		SET_HL(WORD); I(10);
		rst30();
		mem_write(HL, 0x00); I(10);
		cpu.a = 0x02; I(7);
		call(0x548F);                  // print_char: back a space
		I(10);
		break;
	case 0x4E0D:                       // button 2: the letter, or End
		cpu.a = ix_at(0x11); I(19);
		op_or(cpu.a); I(4);
		if (!(cpu.f & FZ)) { I(10); native_continue_at(0x40C9); return; }
		I(10);
		cpu.a = ix_at(0x0F); I(19);
		op_sub8(ix_at(0x0E), 0, 0); I(19);
		if (!(cpu.f & FC)) { I(10); break; }
		I(10);
		call(0x40C9);
		ix_set(0x0F, op_inc8(ix_at(0x0F))); I(23);
		SET_HL(WORD); I(10);
		rst30();
		{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
		cpu.a = ix_at(0x0D); I(19);
		cpu.b = cpu.a; I(4);
		op_add8(cpu.a, 0); I(4);
		op_add8(cpu.a, 0); I(4);
		op_add8(cpu.b, 0); I(4);
		op_add8(ix_at(0x0B), 0); I(19);
		SET_HL(0x4E8A); I(10);
		rst28();
		{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
		mem_write(HL, cpu.a); I(7);
		SET_HL(HL + 1); I(6);
		mem_write(HL, 0x00); I(10);
		call(0x548F);                  // print_char
		I(10);
		break;
	}
	// $4D36: the cursor drawn
	ix_set(0x11, 0x00); I(19);
	cpu.e = ix_at(0x0B); I(19);
	cpu.d = ix_at(0x0D); I(19);
	cpu.a = cpu.d; I(4);
	op_sub8(0x05, 0, 0); I(7);
	if (cpu.f & FC) I(12);
	else {
		I(7);
		cpu.a = cpu.e; I(4);
		op_sub8(0x03, 0, 0); I(7);
		if (cpu.f & FC) I(12);
		else {
			I(7);
			ix_set(0x11, op_inc8(ix_at(0x11))); I(23);
			cpu.e = 0x03; I(7);
		}
	}
	cpu.a = cpu.e; I(4);
	for (int i = 0; i < 4; i++) { op_add8(cpu.a, 0); I(4); }
	op_add8(0xA0, 0); I(7);
	cpu.e = cpu.a; I(4);
	cpu.a = cpu.d; I(4);
	for (int i = 0; i < 4; i++) { op_add8(cpu.a, 0); I(4); }
	op_add8(0x18, 0); I(7);
	cpu.d = cpu.a; I(4);
	cpu.a = 0x02; I(7);
	op_bit(0, ix_at(0x11)); I(20);
	if (cpu.f & FZ) I(12);
	else { I(7); cpu.a = op_inc8(cpu.a); I(4); }
	call(0x3082);                      // put_picture
	cpu.a = ix_at(0x10); I(19);
	cpu.a = op_inc8(cpu.a); I(4);
	op_sub8(0x06, 0, 0); I(7);
	if (cpu.f & FC) I(12);
	else { I(7); op_xor(cpu.a); I(4); }
	ix_set(0x10, cpu.a); I(19);
	SET_HL(0x4EA6); I(10);
	rst28();
	cpu.h = cpu.a; I(4);
	cpu.l = 0x1F; I(7);
	call(0x0843);                      // set_palette_entry
	op_xor(cpu.a); I(4);
	I(10);
}

#define PICKER(name, addr) Native native_##name = { #name, addr, 1, name, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, .long_calls = 1 };
PICKER(direction_pick, 0x4C42)
PICKER(neighbour_pick, 0x4C8F)
PICKER(letter_grid_start, 0x4D0A)
PICKER(letter_grid, 0x4D2B)
