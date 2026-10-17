// The party's turn in a dungeon, as a C task (bank 1 $6A0F): ladders,
// leaving, "It's Dark!", the ">" prompt, then each frame the pad: a
// direction steps ($6B58), button 1 passes, button 2 opens the command menu
// ($8000, a C task of its own, so this one hands over to it). Then the end
// of the turn: the creatures ($3F14) and the dungeon's own ($6AEA). Entering
// a room or a door, and "Blocked!", are original code still (handed over).
// Cycle counts are the original's.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define SLOT        cpu.ix
#define PARTY_XY    0xC953
#define LEVEL       0xC970
#define MOVED       0xC9A1          // the party moved this turn
#define UNDERFOOT   0xC990          // the cell the party stands on
#define LIGHT       0xC95B
#define MOVE_DIR    0xC9B9
#define MOVE_TIME   0xC99A
#define JOY_HELD    0xC0AF
#define JOY_PRESSED 0xC0B0

static void call(uint16_t addr) { cpu.cycles += 17; call_original(addr); }
static void newline(void) { cpu.cycles += 17; text_newline(); }

static void say_inline(uint16_t text) {   // call print_inline / the text follows
	uint16_t hl = HL;
	cpu.cycles += 17 + 19 + 17;
	text_print(text);
	cpu.cycles += 19 + 10;
	SET_HL(hl);
}

static void tcall(uint16_t addr, uint16_t site) {
	cpu.cycles += 17;
	task_call(addr, (uint16_t)(site + 3));
}

// $6A52: the party has left the dungeon (level $FF): original code takes it
// back to the world. 1 when it did (the C task ends).
static int left(uint16_t back) {
	cpu.cycles += 17 + 13 + 4;
	cpu.a = mem_read(LEVEL);
	op_or(cpu.a);
	if (!(cpu.a & 0x80)) { cpu.cycles += 11; return 0; }
	cpu.cycles += 5;
	push16(back);
	native_continue_at(0x6A57);
	return 1;
}

// $6A9B: a ladder underfoot after moving: up or down a level. 1 when the
// level changes (original code draws it; the C task ends).
static int ladder(void) {
	cpu.cycles += 17 + 13 + 4;
	if (!mem_read(MOVED)) { cpu.cycles += 11; return 0; }
	cpu.cycles += 5 + 13 + 7 + 7;
	uint8_t cell = mem_read(UNDERFOOT);
	if (cell == 0x10) {
		cpu.cycles += 5;
		say_inline(0x6AAF);            // "Climb"
		cpu.cycles += 10 + 11 + 10;
		uint8_t lv = (uint8_t)(mem_read(LEVEL) - 1);
		mem_write(LEVEL, lv);
		SET_HL(LEVEL);
		if (lv & 0x80) { text_newline(); return 0; }   // jp m,print_newline: out at the top
		cpu.cycles += 12;
	} else {
		cpu.cycles += 7 + 7;
		if (cell != 0x20) { cpu.cycles += 10; return 0; }
		cpu.cycles += 5;
		say_inline(0x6AC1);            // "Down"
		cpu.cycles += 10 + 11;
		mem_write(LEVEL, mem_read(LEVEL) + 1);
		SET_HL(LEVEL);
	}
	say_inline(0x6ACD);                // " to level "
	cpu.cycles += 13 + 7 + 17;
	text_char((uint8_t)(mem_read(LEVEL) + '1'));
	newline();
	cpu.cycles += 10;                  // pop af
	call(0x31B1);
	cpu.cycles += 10;                  // jp $69E4: the new level
	native_continue_at(0x69E4);
	return 1;
}

// hand over to original code inside the input step, with the returns that
// "call $6B0A" (and "call $6B58") left on the stack
static void hand_over(uint16_t addr, int in_step) {
	push16(0x6A45);
	if (in_step) push16(0x6B56);
	native_continue_at(addr);
}

// $6B58: a step in direction dir. 1 when original code took over.
static int step(uint8_t dir) {
	cpu.cycles += 17 + 13 + 10 + 16 + 16;
	mem_write(MOVE_DIR, dir);
	wr16(MOVE_TIME, 0x0100);
	SET_HL(rd16(PARTY_XY));
	cpu.a = dir;
	call(0x6C59);                      // HL = the cell that way
	cpu.cycles += 19 + 19;
	mem_write(SLOT + 0x1C, cpu.l);
	mem_write(SLOT + 0x1D, cpu.h);
	call(0x6DB9);                      // A = what's there
	cpu.cycles += 19 + 7 + 7 + 7;
	uint8_t cell = cpu.a;
	mem_write(SLOT + 0x1B, cell);
	if (cell == 0xC0) { cpu.cycles += 5; cpu.a = 0x0E; }   // a door
	else { cpu.cycles += 7; cpu.a = 0x0A; }
	call(0x05F1);
	cpu.cycles += 13;
	cpu.a = dir;
	call(0x55A0);                      // "North" etc.
	newline();
	cpu.cycles += 19 + 7 + 7;
	if (cell == 0xC0) { cpu.cycles += 5; cpu.a = cell; hand_over(0x6BD7, 1); return 1; }
	cpu.cycles += 7 + 7 + 7;
	if ((cell & 0xF0) == 0xD0) {       // a room: original code sets it up
		cpu.cycles += 19 + 19 + 19 + 19 + 12;
		mem_write(SLOT + 0x1A, cell);
		cpu.a = cell;
		SET_HL(rd16(SLOT + 0x1C));
		hand_over(0x6C07, 1);
		return 1;
	}
	cpu.cycles += 5 + 19 + 19;         // $6BA2
	SET_HL(rd16(SLOT + 0x1C));
	call(0x6C6A);                      // carry clear: something stands there
	cpu.cycles += 10;
	if (!(cpu.f & FC)) { hand_over(0x40A6, 1); return 1; }   // "Blocked!"
	cpu.cycles += 19 + 7 + 10;
	cpu.a = cell;
	if (cell == 0xF0) { hand_over(0x40A6, 1); return 1; }    // a wall
	cpu.cycles += 7 + 10;
	if (cell == 0xA1) { hand_over(0x40A6, 1); return 1; }
	cpu.cycles += 7 + 7;
	if (cell == 0xA2) {                // slow going
		cpu.cycles += 10 + 16;
		wr16(MOVE_TIME, 0x0200);
		call(0x4719);
	} else cpu.cycles += 5;
	cpu.cycles += 19 + 19 + 16 + 7 + 13 + 10;   // $6BC8: there
	SET_HL(rd16(SLOT + 0x1C));
	wr16(PARTY_XY, HL);
	cpu.a = 0xFF;
	mem_write(MOVED, 0xFF);
	return 0;
}

static void dungeon_turn(void) {
	for (;;) {
		call(0x129D);                  // $6A0F: the screen's moving parts
		cpu.cycles += 17 + 58;         // yield
		SET_HL(0x6A15);
		task_wait_turn(0x6A15);
		if (ladder()) return;
		if (left(0x6A1B)) return;
		call(0xAF57);                  // carry: everyone asleep
		cpu.cycles += 7;
		if (!(cpu.f & FC)) {
			cpu.cycles += 13 + 4 + 7;
			if (!mem_read(LIGHT)) say_inline(0x6A29);   // "It's Dark!"
			else cpu.cycles += 5;
			cpu.cycles += 7 + 17;
			text_char(0x7F);           // ">"
			cpu.cycles += 12;
			for (int first = 1;; first = 0) {
				if (!first) task_frame_tick(0x6A3C);
				tcall(0x5726, 0x6A3F); // both buttons: the party's stats
				// $6B0A: one frame of input
				cpu.cycles += 17 + 4 + 13 + 13 + 10 + 17;
				mem_write(MOVED, 0);
				uint8_t held = mem_read(JOY_HELD);
				static const uint8_t keys[4] = { 0x01, 0x02, 0x04, 0x08 }, dirs[4] = { 1, 3, 0, 2 };
				int i;
				for (i = 0; i < 4 && keys[i] != held; i++) {}
				if (i < 4) {           // a step
					cpu.cycles += 58 * i + 79 + 7 + (i < 3 ? 12 : 0);
					cpu.a = dirs[i];
					if (step(dirs[i])) return;
					cpu.cycles += 4 + 10;   // or a / ret
					op_or(cpu.a);
					break;
				}
				cpu.cycles += 58 * 4 + 22 + 13 + 7 + 7;
				uint8_t pressed = mem_read(JOY_PRESSED);
				cpu.a = pressed;
				if (pressed == 0x10) { // pass
					cpu.cycles += 5 + 10 + 16;
					wr16(MOVE_TIME, 0x0100);
					call(0x46B5);
					cpu.cycles += 4 + 10;
					op_or(cpu.a);
					break;
				}
				cpu.cycles += 7 + 7;
				if (pressed == 0x20) { // the command menu (a C task of its own)
					cpu.cycles += 5 + 10 + 16 + 10;
					wr16(MOVE_TIME, 0x0100);
					hand_over(0x8000, 0);
					return;
				}
				cpu.cycles += 4 + 10 + 7 + 5;   // scf / ret / jr c,$6A3C
				cpu.f |= FC;
			}
			cpu.cycles += 7;           // jr c not taken
			if (left(0x6A4A)) return;
		} else cpu.cycles += 5;
		tcall(0x3F14, 0x6A4A);         // the end of the turn
		tcall(0x6AEA, 0x6A4D);
		cpu.cycles += 12;
	}
}
Native native_dungeon_turn = { "dungeon_turn", 0x6A0F, 1, dungeon_turn, 0, 0, 0, .task = 1 };
