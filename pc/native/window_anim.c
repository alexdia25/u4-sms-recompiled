// The text window's frame drawn growing or shrinking, one step a frame, as
// C tasks: the calling task waits in them, its return kept in the slot
// (task_enter_1, $0AA5) and taken back at the end (task_return_1, $0AAE).
// - window_wipe ($310B: frame $B748; $30F6: $BC10; $30F1: $B9AC): the
//   frame (window_frame, $3255), then nine steps: two 17-wide rows of the
//   text area's copy queued for the screen, one moving down from IX+$1A,
//   one up from IX+$1C (a row is $40 bytes).
// - window_grow ($3164: frame $BC10; $315F: $B9AC): nine steps of a
//   block queued from IX+$1A (one row and column more each step: 2 x 1
//   cells to start, at $CC52).
// - window_open ($31B1): the frame ($B748) opened from the middle, nine
//   steps: two 17-wide rows (from $CA42 down, from $CE82 up) and two
//   18-high columns (from $CA42 right, from $CA62 left) a frame.
// Cycle counts are the original's.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define SLOT  cpu.ix

static void ix_set(uint8_t d, uint8_t v) { mem_write((uint16_t)(SLOT + d), v); cpu.cycles += 19; }
static uint16_t ix_word(uint8_t d) { cpu.cycles += 19 + 19; return rd16((uint16_t)(SLOT + d)); }
static void ix_set_word(uint8_t d, uint16_t v) { wr16((uint16_t)(SLOT + d), v); cpu.cycles += 19 + 19; }

static void tcall(uint16_t addr, uint16_t site) {   // a routine that may wait for frames
	cpu.cycles += 17;
	task_call(addr, (uint16_t)(site + 3));
}

static void enter(void) {              // call task_enter_1: the caller's return kept in the slot
	cpu.cycles += 17 + 10 + 19 + 19 + 19 + 10;
	uint16_t back = pop16();
	wr16((uint16_t)(SLOT + 0x24), back);
	SET_HL(back);
}

static void leave(void) {              // jp task_return_1
	cpu.cycles += 10 + 19 + 19 + 4;
	SET_HL(rd16((uint16_t)(SLOT + 0x24)));
	native_continue_at(HL);
}

static void frame(uint16_t frame_at) { // call window_frame
	SET_HL(frame_at);
	cpu.cycles += 17;
	call_original(0x3255);
}

// $311E: the frame, then the two rows each step
static void wipe(uint16_t frame_at) {
	frame(frame_at);
	enter();
	ix_set(0x16, 0x09);
	do {
		uint16_t de = ix_word(0x1A);
		SET_DE(de);
		cpu.cycles += 10 + 11;
		SET_HL(op_add16(0x0040, DE));
		ix_set_word(0x1A, HL);
		cpu.cycles += 10;
		SET_BC(0x0111);
		tcall(0x0499, 0x313B);         // queue_text_copy
		SET_DE(ix_word(0x1C));
		cpu.cycles += 10 + 11;
		SET_HL(op_add16(0xFFC0, DE));
		ix_set_word(0x1C, HL);
		cpu.cycles += 10;
		SET_BC(0x0111);
		tcall(0x0499, 0x3151);
		tcall(0x305A, 0x3154);         // a frame
		cpu.cycles += 23;
		uint8_t n = op_dec8(mem_read((uint16_t)(SLOT + 0x16)));
		mem_write((uint16_t)(SLOT + 0x16), n);
		cpu.cycles += n ? 12 : 7;
	} while (mem_read((uint16_t)(SLOT + 0x16)));
	leave();
}

static void wipe_rows(uint8_t a_lo, uint8_t a_hi, uint8_t b_lo, uint8_t b_hi) {   // $30F9 or $310E
	ix_set(0x1A, a_lo);
	ix_set(0x1B, a_hi);
	ix_set(0x1C, b_lo);
	ix_set(0x1D, b_hi);
}

static void window_wipe(void) {        // $310B
	cpu.cycles += 10;                  // ld hl,$B748
	wipe_rows(0x42, 0xCA, 0x82, 0xCE);
	wipe(0xB748);
}

static void window_wipe_bc10(void) {   // $30F6
	cpu.cycles += 10;
	wipe_rows(0x82, 0xCC, 0x42, 0xCC);
	cpu.cycles += 12;                  // jr $311E
	wipe(0xBC10);
}

static void window_wipe_b9ac(void) {   // $30F1
	cpu.cycles += 10 + 12;             // ld hl,$B9AC / jr $30F9
	wipe_rows(0x82, 0xCC, 0x42, 0xCC);
	cpu.cycles += 12;
	wipe(0xB9AC);
}

// $3167: the frame, then the block, a row and a column more each step
static void grow(uint16_t frame_at) {
	frame(frame_at);
	enter();
	ix_set(0x1A, 0x52);
	ix_set(0x1B, 0xCC);
	ix_set(0x1C, 0x01);
	ix_set(0x1D, 0x02);
	ix_set(0x16, 0x09);
	do {
		SET_DE(ix_word(0x1A));
		cpu.c = mem_read((uint16_t)(SLOT + 0x1C));
		cpu.b = mem_read((uint16_t)(SLOT + 0x1D));
		cpu.cycles += 19 + 19;
		tcall(0x0499, 0x318D);         // queue_text_copy
		cpu.cycles += 10 + 11;
		SET_HL(op_add16(0xFFBE, DE));
		ix_set_word(0x1A, HL);
		for (int i = 0; i < 2; i++) {
			mem_write((uint16_t)(SLOT + 0x1C), op_inc8(mem_read((uint16_t)(SLOT + 0x1C))));
			cpu.cycles += 23;
		}
		for (int i = 0; i < 2; i++) {
			mem_write((uint16_t)(SLOT + 0x1D), op_inc8(mem_read((uint16_t)(SLOT + 0x1D))));
			cpu.cycles += 23;
		}
		tcall(0x305A, 0x31A6);         // a frame
		cpu.cycles += 23;
		uint8_t n = op_dec8(mem_read((uint16_t)(SLOT + 0x16)));
		mem_write((uint16_t)(SLOT + 0x16), n);
		cpu.cycles += n ? 12 : 7;
	} while (mem_read((uint16_t)(SLOT + 0x16)));
	leave();
}

static void window_grow(void) { cpu.cycles += 10; grow(0xBC10); }               // $3164
static void window_grow_b9ac(void) { cpu.cycles += 10 + 12; grow(0xB9AC); }     // $315F

static void window_open(void) {        // $31B1
	enter();
	cpu.cycles += 10;                  // ld hl,$B748
	frame(0xB748);
	ix_set_word(0x1A, 0xCA42);
	ix_set_word(0x1C, 0xCE82);
	ix_set_word(0x1E, 0xCA42);
	ix_set_word(0x20, 0xCA62);
	ix_set(0x16, 0x09);
	do {
		SET_DE(ix_word(0x1A));
		cpu.cycles += 10;
		SET_BC(0x0111);
		tcall(0x0499, 0x31E7);         // queue_text_copy
		cpu.cycles += 10 + 11;
		SET_HL(op_add16(0x0040, DE));
		ix_set_word(0x1A, HL);
		SET_DE(ix_word(0x1C));
		cpu.cycles += 10;
		SET_BC(0x0111);
		tcall(0x0499, 0x31FD);
		cpu.cycles += 10 + 11;
		SET_HL(op_add16(0xFFC0, DE));
		ix_set_word(0x1C, HL);
		SET_DE(ix_word(0x1E));
		cpu.cycles += 10;
		SET_BC(0x1201);
		tcall(0x0499, 0x3213);
		cpu.e = op_inc8(cpu.e);
		cpu.e = op_inc8(cpu.e);
		cpu.cycles += 4 + 4;
		ix_set_word(0x1E, DE);
		SET_DE(ix_word(0x20));
		cpu.cycles += 10;
		SET_BC(0x1201);
		tcall(0x0499, 0x3227);
		cpu.e = op_dec8(cpu.e);
		cpu.e = op_dec8(cpu.e);
		cpu.cycles += 4 + 4;
		ix_set_word(0x20, DE);
		tcall(0x305A, 0x3232);         // a frame
		cpu.cycles += 23;
		uint8_t n = op_dec8(mem_read((uint16_t)(SLOT + 0x16)));
		mem_write((uint16_t)(SLOT + 0x16), n);
		cpu.cycles += n ? 12 : 7;
	} while (mem_read((uint16_t)(SLOT + 0x16)));
	leave();
}

#define ANIM(name, addr) Native native_##name = { #name, addr, 0, name, 0, 0, 0, .task = 1 };
ANIM(window_wipe, 0x310B)
ANIM(window_wipe_bc10, 0x30F6)
ANIM(window_wipe_b9ac, 0x30F1)
ANIM(window_grow, 0x3164)
ANIM(window_grow_b9ac, 0x315F)
ANIM(window_open, 0x31B1)
