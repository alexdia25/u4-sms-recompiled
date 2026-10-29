// The party's turn in a dungeon, as a C task (bank 1 $6A0F): ladders,
// leaving, "It's Dark!", the ">" prompt, then each frame the pad: a
// direction steps ($6B58), button 1 passes, button 2 opens the command menu
// ($8000, a C task of its own, so this one hands over to it). Then the end
// of the turn: the creatures ($3F14) and the dungeon's own ($6AEA, here). Entering
// a room (by a door or not) loads it and goes on to the room's fight.
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
	if (window_anim_call(addr, site)) return;
	cpu.cycles += 17;
	task_call(addr, (uint16_t)(site + 3));
}

// $5726, each frame of the dungeon's input: button 2 alone ($20) opens the
// party's stats ($5777, a page at a time), button 1 alone ($10) lists the
// karma of the eight virtues ($5730); both wait, so they run as the
// original's, and return to $6A42
static void stats_buttons(void) {
	cpu.cycles += 17;
	call(0x064C);                      // read_pads_both
	cpu.cycles += 7;
	if (cpu.a == 0x20) {
		op_sub8(0x20, 0, 0);
		cpu.cycles += 12;
		task_call(0x5777, 0x6A42);
		return;
	}
	op_sub8(0x20, 0, 0);
	cpu.cycles += 7 + 7;
	op_sub8(0x10, 0, 0);
	if (cpu.a != 0x10) { cpu.cycles += 11; return; }
	cpu.cycles += 5;
	task_call(0x5730, 0x6A42);
}

// $6AEA: the dungeon's end of the turn: everyone dead is the end
// ($465D), afflictions and food ($B4EE), magic points ($4776), the level's
// creatures walk and appear ($6D1A, $6CB9), the light ($C95B) wanes, the
// move counts ($372E, $3754), the status window ($4324, by jp: its ret
// goes back to $6A50)
static void dungeon_turn_end(void) {
	tcall(0x465D, 0x6AEA);
	tcall(0xB4EE, 0x6AED);
	tcall(0x4776, 0x6AF0);
	tcall(0x6D1A, 0x6AF3);
	tcall(0x6CB9, 0x6AF6);
	cpu.cycles += 10 + 7 + 4;
	SET_HL(LIGHT);
	cpu.a = mem_read(LIGHT);
	op_or(cpu.a);
	if (cpu.f & FZ) cpu.cycles += 12;
	else { cpu.cycles += 7 + 11; mem_write(LIGHT, op_dec8(cpu.a)); }
	tcall(0x372E, 0x6B01);
	tcall(0x3754, 0x6B04);
	cpu.cycles += 10;                  // jp $4324
	task_call(0x4324, 0x6A50);
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
	tcall(0x31B1, 0x6AE4);             // the window opens (it waits for frames)
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

// "Blocked!" ($40A6, reached by jp): back in the input step after it
static void blocked(void) {
	cpu.cycles += 7;
	cpu.a = 0x10;
	call(0x05F1);
	say_inline(0x40AE);
	cpu.cycles += 10;
}

// $6C07: into the room at HL (the cell kind in +$1A): the party on the
// room's cell nearest the way in, the room loaded ($3D82), and on to the
// room's fight (game mode $16); the stack is reset there
static void room(void) {
	push16(0x6A45);                    // the returns the original has on the stack
	push16(0x6B56);
	for (int i = 0; i < 2; i++) {      // north (else back south), then east (else back west)
		cpu.cycles += 7;
		cpu.a = (uint8_t)i;
		call(0x6C59);
		call(0x6DB9);
		cpu.cycles += 19 + 7;
		op_sub8(mem_read(SLOT + 0x1A), 0, 0);
		if (cpu.f & FZ) cpu.cycles += 5;
		else {
			cpu.cycles += 7;
			cpu.a = (uint8_t)(i + 2);
			call(0x6C59);
		}
	}
	cpu.cycles += 16 + 13 + 7 + 7 + 7;
	wr16(PARTY_XY, HL);
	cpu.a = mem_read(0xC9A0);
	op_sub8(0x19, 0, 0);
	cpu.b = 0;
	if (cpu.a != 0x19) cpu.cycles += 5;
	else {                             // the Abyss: the room by level too
		cpu.cycles += 13 + 4 + 4 + 4 + 7 + 4;
		cpu.a = (uint8_t)(mem_read(LEVEL) * 8);
		op_and(0xF0);
		cpu.b = cpu.a;
	}
	cpu.cycles += 19 + 7 + 4 + 13;
	cpu.a = mem_read(SLOT + 0x1A);
	op_and(0x0F);
	op_or(cpu.b);
	mem_write(0xC9B8, cpu.a);
	tcall(0x3D82, 0x6C49);             // load_dungeon_room
	cpu.cycles += 7 + 13 + 7 + 13 + 10;
	mem_write(0xC975, 2);
	mem_write(0xC600, 0x16);
	cpu.a = 0x16;
	native_continue_at(0x0CD2);
}

// $6BD7: a door: a room beyond it is entered at once
static int door(void) {
	cpu.cycles += 13 + 19 + 19;
	cpu.a = mem_read(MOVE_DIR);
	SET_HL(rd16(SLOT + 0x1C));
	call(0x6C59);
	cpu.cycles += 19 + 19;
	mem_write(SLOT + 0x1E, cpu.l);
	mem_write(SLOT + 0x1F, cpu.h);
	call(0x6DB9);
	cpu.cycles += 19 + 7 + 7 + 7;
	mem_write(SLOT + 0x1A, cpu.a);
	op_and(0xF0);
	op_sub8(0xD0, 0, 0);
	if (cpu.a != 0xD0) { cpu.cycles += 5; return 0; }
	cpu.cycles += 19 + 19 + 16;
	SET_HL(rd16(SLOT + 0x1C));
	wr16(PARTY_XY, HL);
	call(0x129D);
	cpu.cycles += 19 + 19;
	SET_HL(rd16(SLOT + 0x1E));
	room();
	return 1;
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
	if (cell == 0xC0) {                // a door
		cpu.cycles += 5;
		cpu.a = cell;
		if (door()) return 1;
	} else {
		cpu.cycles += 7 + 7 + 7;
		cpu.a = cell & 0xF0;
		op_sub8(0xD0, 0, 0);
		if ((cell & 0xF0) == 0xD0) {   // a room
			cpu.cycles += 19 + 19 + 19 + 19 + 12;
			mem_write(SLOT + 0x1A, cell);
			cpu.a = cell;
			SET_HL(rd16(SLOT + 0x1C));
			room();
			return 1;
		}
		cpu.cycles += 5;
	}
	cpu.cycles += 19 + 19;             // $6BA2
	SET_HL(rd16(SLOT + 0x1C));
	call(0x6C6A);                      // carry clear: something stands there
	cpu.cycles += 10;
	if (!(cpu.f & FC)) { blocked(); return 0; }   // something stands there
	cpu.cycles += 19 + 7 + 10;
	cpu.a = cell;
	if (cell == 0xF0) { blocked(); return 0; }    // a wall
	cpu.cycles += 7 + 10;
	if (cell == 0xA1) { blocked(); return 0; }
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

// stage 0: the turn's top ($6A0F); 1: back at $6A45 from the input step
// (a command or a room, which the original runs, returns there)
static void dungeon_loop(int stage) {
	int first;
	if (stage == 1) {                  // jr c,$6A3C: no move yet, more input
		if (cpu.f & FC) { cpu.cycles += 12; first = 0; goto input; }
		goto after_input;
	}
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
			for (first = 1;; first = 0) {
			input:
				if (!first) task_frame_tick(0x6A3C);
				stats_buttons();
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
		after_input:
			cpu.cycles += 7;           // jr c not taken
			if (left(0x6A4A)) return;
		} else cpu.cycles += 5;
		tcall(0x3F14, 0x6A4A);         // the end of the turn
		cpu.cycles += 17;              // call $6AEA
		dungeon_turn_end();
		cpu.cycles += 12;
	}
}
static void dungeon_turn(void) { dungeon_loop(0); }          // $6A0F
static void dungeon_after_input(void) { dungeon_loop(1); }   // $6A45
Native native_dungeon_turn = { "dungeon_turn", 0x6A0F, 1, dungeon_turn, 0, 0, 0, .task = 1 };
Native native_dungeon_after_input = { "dungeon_after_input", 0x6A45, 1, dungeon_after_input, 0, 0, 0, .task = 1 };
