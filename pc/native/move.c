// The player's turn: reading the pad and moving the party, rewritten from
// bank 2 ($B08B-$B3C8). The turn loops (world $AC8C, town $AF69) call
// input_step every frame until it reports that something happened.
//
// Moving prints the direction ("North", "Ride East", "Sail West", "Turn
// South"), so these are blocking natives. Cycle counts are the original's.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define JOY_HELD      0xC0AF
#define JOY_PRESSED   0xC0B0
#define TRANSPORT     0xC959          // $1F on foot, $14/$15 horse, $10-$13 ship (facing), $18 balloon
#define MOVE_DIR      0xC9B9          // 0 west, 1 north, 2 east, 3 south
#define MOVE_TIME     0xC99A          // 16-bit: how long the move takes
#define TURN_DONE     0xC9A1
#define MAP_KIND      0xC99F          // 0 on the world map

static void call(uint16_t addr) { cpu.cycles += 17; call_original(addr); }

static void say_inline(uint16_t text) {   // call print_inline / the text follows
	uint16_t hl = HL;
	cpu.cycles += 17 + 19 + 17;
	text_print(text);
	cpu.cycles += 19 + 10;
	SET_HL(hl);
}

static int on_horse(void) {             // $B403: Z if riding
	cpu.cycles += 17 + 13 + 7;
	uint8_t t = mem_read(TRANSPORT);
	cpu.a = t;
	if (t == 0x14) { cpu.cycles += 11; return 1; }
	cpu.cycles += 5 + 7 + 10;
	return t == 0x15;
}

static void say_direction(int jumped) { // $B3C6: "North" etc. and a new line (call or jp)
	cpu.cycles += (jumped ? 10 : 17) + 13 + 17;
	cpu.a = mem_read(MOVE_DIR);
	call_original(0x55A0);
	cpu.cycles += 10;                   // jp print_newline
	text_newline();
}

// $AEA0, after a step south on the world: into the waters at ($E5-$E9,
// $D4-$D8), unless $C976 = 1, one of the people (a random one of the first
// eight) becomes a whirlpool ($F0) at ($E7, a row below the party)
static void whirlpool(void) {
	cpu.cycles += 13 + 7;
	cpu.a = mem_read(MOVE_DIR);
	op_sub8(0x03, 0, 0);
	if (cpu.a != 0x03) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 13 + 7;
	cpu.a = mem_read(0xC953);
	op_sub8(0xE5, 0, 0);
	if (cpu.f & FC) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 7;
	op_sub8(0xEA, 0, 0);
	if (!(cpu.f & FC)) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 13 + 7;
	cpu.a = mem_read(0xC954);
	op_sub8(0xD4, 0, 0);
	if (cpu.f & FC) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 7;
	op_sub8(0xD9, 0, 0);
	if (!(cpu.f & FC)) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 13 + 7;
	cpu.a = mem_read(0xC976);
	op_sub8(0x01, 0, 0);
	if (cpu.a == 0x01) { cpu.cycles += 11; return; }
	cpu.cycles += 5;
	call(0x2BBF);                       // random
	cpu.cycles += 7;
	op_and(0x07);
	call(0x48F7);                       // person_record
	cpu.cycles += 19 + 19 + 19 + 13 + 4 + 19 + 10;
	mem_write((uint16_t)(cpu.iy + 3), 0xF0);
	mem_write((uint16_t)(cpu.iy + 0), 0xF0);
	mem_write((uint16_t)(cpu.iy + 1), 0xE7);
	cpu.a = op_inc8(mem_read(0xC954));
	mem_write((uint16_t)(cpu.iy + 2), cpu.a);
}

// the step itself: the four step_* routines on the world map ($B3CF, then
// $AEA0: a whirlpool may appear), the town ones ($B3D7) in towns
static void step(uint8_t dir) {
	cpu.cycles += 13 + 4 + 13;
	if (mem_read(MAP_KIND) == 0) {
		cpu.cycles += 7 + 10 + 11 + 10 + 52;
		cpu.a = dir;
		call_original(rd16(0xB3CF + 2 * dir));
		cpu.cycles += 10;
		whirlpool();
	} else {
		cpu.cycles += 12 + 10 + 10 + 10 + 52;
		cpu.a = dir;
		native_continue_at(rd16(0xB3D7 + 2 * dir));
	}
}

// player_move ($B263): move the party one step in direction A
static void player_move(uint8_t dir) {
	cpu.cycles += 13 + 10 + 16;
	mem_write(MOVE_DIR, dir);
	wr16(MOVE_TIME, 0x0100);
	uint8_t t = mem_read(TRANSPORT);
	cpu.cycles += 13 + 7 + 10;
	if (t == 0x18) { native_continue_at(0x473F); return; }   // balloon: drifts with the wind
	cpu.cycles += 7 + 10;
	if (t < 0x14) {                     // ship ($B326): turn to face, or sail
		cpu.cycles += 13 + 7 + 10 + 7 + 7;
		uint8_t facing = (uint8_t)(dir | 0x10);
		if (facing != mem_read(TRANSPORT)) {
			cpu.cycles += 4 + 7 + 16;
			wr16(TRANSPORT, facing);
			say_inline(0xB33A);         // "Turn "
			say_direction(1);
			return;
		}
		cpu.cycles += 5;
		say_inline(0xB346);             // "Sail "
		say_direction(0);
		call(0xB412);                   // A = the tile ahead
		call(0xB423);                   // carry: no water there
		cpu.cycles += 10;
		if (cpu.f & FC) { native_continue_at(0x40A6); return; }   // "Blocked!"
		call(0xB36C);                   // the wind: slower against it
		cpu.cycles += 7;
		cpu.a = 0x1F;
		call(0x05F1);                   // sound
		call(0xB40C);                   // the turn is done
		cpu.cycles += 13 + 10 + 10 + 10 + 52;
		cpu.a = dir;
		native_continue_at(rd16(0xB3CF + 2 * dir));
		return;
	}
	cpu.cycles += 13;                   // on foot or riding
	cpu.a = dir;
	cpu.cycles += 17 + 100;             // $40B9: HL = the step (dx, dy)
	SET_HL(rd16(0x40BF + 2 * dir));
	call(0xB464);                       // a door in the way (towns)
	if (cpu.f & FC) { cpu.cycles += 11; return; }
	cpu.cycles += 5;
	int riding = on_horse();
	cpu.cycles += 7;
	if (riding) say_inline(0xB28B);     // "Ride "
	else cpu.cycles += 5;
	say_direction(0);
	cpu.cycles += 13 + 7 + 7;
	int checked = 0;
	if (dir == 1) {                     // north: special tiles at the edge
		cpu.cycles += 13 + 7 + 10;
		if (mem_read(0xC9BB) == 0x0E) { native_continue_at(0x40A6); return; }
		cpu.cycles += 13 + 7 + 7;
		if (mem_read(0xC992) == 0x0E) { cpu.cycles += 5; checked = 1; }
	} else cpu.cycles += 5;
	if (!checked) {
		call(0xB412);                   // A = the tile ahead
		call(0x3EB7);                   // carry: can't go there
		cpu.cycles += 7;
		if (cpu.f & FC) {
			cpu.cycles += 13 + 4 + 10;
			if (mem_read(MAP_KIND) == 0) { cpu.cycles += 7; call_original(0xB40C); }
			cpu.cycles += 10;
			native_continue_at(0x40A6);
			return;
		}
		cpu.cycles += 5;
	}
	uint8_t a = cpu.a, f = cpu.f;       // $B2BC
	cpu.cycles += 11;
	riding = on_horse();
	cpu.cycles += 7;
	if (riding) { cpu.cycles += 10 + 16; wr16(MOVE_TIME, 0x0080); }
	else cpu.cycles += 5;
	cpu.cycles += 10;
	cpu.a = a; cpu.f = f;
	call(0xB309);                       // slow ground ("Slow progress")
	riding = on_horse();
	cpu.cycles += 7 + 7;
	uint8_t sound = 0x0A;
	if (riding) {
		cpu.cycles += 13 + 7 + 7;
		if (dir == 0) {                 // the horse faces the way it goes
			cpu.cycles += 5 + 10 + 12 + 16 + 12 + 7;
			wr16(TRANSPORT, 0x8614);
		} else {
			cpu.cycles += 7 + 7;
			if (dir == 2) { cpu.cycles += 5 + 10 + 16 + 12 + 7; wr16(TRANSPORT, 0x8715); }
			else cpu.cycles += 7;
		}
		sound = 0x0B;
	} else cpu.cycles += 5;
	cpu.a = sound;
	call(0x05F1);
	call(0xB40C);
	step(dir);
}

static void nat_player_move(void) { player_move(cpu.a); }
Native native_player_move = { "player_move", 0xB263, 2, nat_player_move, 0, 0, 0, .blocking = 1 };

static int someone_here(void) {         // $B152: carry (0) if nobody stands here, else "pass" ($B15C)
	cpu.cycles += 17 + 16 + 7;
	SET_HL(rd16(0xC953));
	cpu.b = 0x20;
	call(0x42B7);                       // person_at
	if (cpu.f & FC) { cpu.cycles += 11; return 0; }
	cpu.cycles += 5 + 10;               // pop af: out of input_step too
	return 1;
}

static void pass(void) {                // $B15C
	call(0x46F1);
	cpu.cycles += 4 + 10;
	op_or(cpu.a);
}

// $B0A6, button 1: on foot, pass ($46B5); else "Exit" the vehicle: a ship
// or the balloon ($C959 $10-$13, $18+) leaves it where it is, but not on
// someone, and the balloon only when landed ($C990 = 4); a horse is left
// as one of the people on the town map (a free slot, else the first one
// not already a horse after a random one, else $4933's); the party on
// foot ($C959 = $1F), the step's sound
static void leave_or_pass(void) {
	cpu.cycles += 10 + 16 + 13 + 7;
	wr16(MOVE_TIME, 0x0100);
	uint8_t t = mem_read(TRANSPORT);
	cpu.a = t;
	op_sub8(0x1F, 0, 0);
	if (t == 0x1F) {
		cpu.cycles += 7;
		call(0x46B5);
		cpu.cycles += 4 + 10;
		op_or(cpu.a);
		return;
	}
	cpu.cycles += 12 + 10;
	SET_HL(0xB224); call(0x5519);       // print_text_hl
	cpu.cycles += 13 + 7;
	cpu.a = mem_read(TRANSPORT);
	op_sub8(0x14, 0, 0);
	if (t < 0x14) {                     // a ship
		cpu.cycles += 12 + 10;
		SET_HL(0xB232); call(0x5519);
		cpu.cycles += 16 + 16;
		SET_HL(rd16(0xC953));
		wr16(0xC973, HL);
		call(0x0E01);
		cpu.cycles += 12;
	} else if (t >= 0x16) {             // the balloon
		cpu.cycles += 7 + 7 + 7 + 10;
		op_sub8(0x16, 0, 0);
		SET_HL(0xB23C); call(0x5519);
		if (someone_here()) { pass(); return; }
		cpu.cycles += 13 + 7;
		cpu.a = mem_read(0xC990);
		op_sub8(0x04, 0, 0);
		if (cpu.a != 0x04) { cpu.cycles += 10; pass(); return; }
		cpu.cycles += 10;
		call(0x0DEB);
		cpu.cycles += 12;
	} else {                            // a horse
		cpu.cycles += 7 + 7 + 12 + 10;
		op_sub8(0x16, 0, 0);
		SET_HL(0xB22A); call(0x5519);
		if (someone_here()) { pass(); return; }
		cpu.cycles += 13 + 7;
		cpu.a = mem_read(MAP_KIND);
		op_sub8(0x01, 0, 0);
		if (cpu.a != 0x01) {
			cpu.cycles += 7;
			call(0x0DF6);
			cpu.cycles += 12;
		} else {
			cpu.cycles += 12 + 4;
			op_xor(cpu.a);
			call(0x4906);               // person_with_graphic: a free slot
			if (!(cpu.f & FC)) cpu.cycles += 12;
			else {
				cpu.cycles += 7;
				call(0x2BBF);           // random
				cpu.cycles += 7;
				cpu.b = 0x20;
				for (;;) {
					cpu.a = op_inc8(cpu.a);
					op_and(0x1F);
					cpu.c = cpu.a;
					cpu.cycles += 4 + 7 + 4;
					call(0x48F7);       // person_record
					cpu.cycles += 19 + 7;
					cpu.a = mem_read((uint16_t)(cpu.iy + 3));
					op_sub8(0x14, 0, 0);
					if (cpu.a == 0x14) { cpu.cycles += 12; break; }
					cpu.cycles += 7 + 7;
					op_sub8(0x15, 0, 0);
					if (cpu.a == 0x15) { cpu.cycles += 12; break; }
					cpu.cycles += 7 + 4;
					cpu.a = cpu.c;
					if (--cpu.b) { cpu.cycles += 13; continue; }
					cpu.cycles += 8;
					call(0x4933);
					break;
				}
			}
			cpu.cycles += 13 + 19 + 19 + 16 + 19 + 19 + 19 + 19;
			cpu.a = mem_read(TRANSPORT);
			mem_write((uint16_t)(cpu.iy + 3), cpu.a);
			mem_write((uint16_t)(cpu.iy + 0), cpu.a);
			SET_HL(rd16(0xC953));
			mem_write((uint16_t)(cpu.iy + 1), cpu.l);
			mem_write((uint16_t)(cpu.iy + 2), cpu.h);
			mem_write((uint16_t)(cpu.iy + 6), 0);
			mem_write((uint16_t)(cpu.iy + 7), 0);
		}
	}
	cpu.cycles += 10 + 16 + 7 + 13;     // $B142: on foot
	SET_HL(0x8A1F);
	wr16(TRANSPORT, HL);
	cpu.a = 0xFF;
	mem_write(0xC98F, 0xFF);
	call(0x40D1);                       // sound_0d
	cpu.cycles += 4 + 10;
	op_or(cpu.a);
}

// input_step ($B08B): one frame of the player's turn. A direction moves;
// button 1 passes or leaves the vehicle ($B0A6, leave_or_pass); button 2 opens the command
// menu ($8000, a C task; on foot, next to some people, their own: the
// original's). Returns carry set while nothing happened.
static void nat_input_step(void) {
	uint8_t held = mem_read(JOY_HELD);
	static const uint8_t keys[4] = { 0x01, 0x02, 0x04, 0x08 };
	int i;
	for (i = 0; i < 4 && keys[i] != held; i++) {}
	cpu.cycles += 4 + 13 + 13 + 10 + 17;
	mem_write(TURN_DONE, 0);
	cpu.a = held;
	cpu.cycles += i < 4 ? 58 * i + 79 : 58 * 4 + 22;
	if (i < 4) {                        // $B253-$B25C: the direction
		static const uint8_t dirs[4] = { 1, 3, 0, 2 };
		static const int entry[4] = { 7 + 30, 7 + 20, 7 + 10, 7 };
		cpu.cycles += entry[i] + 17;
		push16(0xB261);                 // call $B263: it returns to "or a / ret"
		player_move(dirs[i]);
		if (!native_jumped()) {         // it returned here: do that part now
			pop16();
			cpu.cycles += 4 + 10;
			op_or(cpu.a);
		}
		return;
	}
	uint8_t pressed = mem_read(JOY_PRESSED);
	cpu.a = pressed;
	cpu.cycles += 13 + 7 + 7;
	if (pressed == 0x10) { cpu.cycles += 5; leave_or_pass(); return; }
	cpu.cycles += 7 + 10;
	op_sub8(0x20, 0, 0);
	if (pressed != 0x20) { cpu.cycles += 4 + 10; cpu.f |= FC; return; }
	// $B161: button 2, the command menu ($8000); on foot, next to someone
	// who stands out (the table at $B189, by their graphic), theirs instead
	cpu.cycles += 10 + 16 + 13 + 7 + 10;
	wr16(MOVE_TIME, 0x0100);
	cpu.a = mem_read(TRANSPORT);
	op_sub8(0x1F, 0, 0);
	if (cpu.a != 0x1F) { SET_HL(0x0100); native_continue_at(0x8000); return; }
	cpu.cycles += 16 + 7 + 17;
	SET_HL(rd16(0xC953));
	cpu.b = 0x20;
	call_original(0x42B7);             // person_at
	cpu.cycles += 10;
	if (cpu.f & FC) { native_continue_at(0x8000); return; }
	cpu.cycles += 17;
	call_original(0x48F7);             // person_record
	cpu.cycles += 19 + 10 + 17;
	cpu.a = mem_read(cpu.iy);
	SET_HL(0xB189);
	for (;;) {                         // switch_on_a
		cpu.b = op_inc8(mem_read(HL));
		if (cpu.f & FZ) break;
		op_sub8(mem_read(HL), 0, 0);
		SET_HL(HL + 1);
		if (cpu.f & FZ) {
			cpu.cycles += 41 + 38;
			SET_HL(rd16(HL));
			cpu.a = cpu.l;
			cpu.f = 0x86;              // (its return, $B186, popped into AF)
			native_continue_at(HL);
			return;
		}
		cpu.cycles += 58;
		SET_HL(HL + 2);
	}
	cpu.cycles += 22 + 10;             // none: jp command_menu
	native_continue_at(0x8000);
}
Native native_input_step = { "input_step", 0xB08B, 2, nat_input_step, OUT_A | OUT_F, 0, 0, .blocking = 1 };
