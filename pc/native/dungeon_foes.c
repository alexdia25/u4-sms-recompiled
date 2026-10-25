// The creatures that roam a dungeon level, four a level (bank 1). Each
// record is 8 bytes: +0 graphic, +1/+2 place (x, y), +3 kind (0: none),
// +4/+5 the place before.
// - cell_address ($6DC0): HL = the cell (L, H) of the level ($C970) in the
//   dungeon's map ($D000 + level * 256); cell_at ($6DB9): A = what is
//   there.
// - level_creatures ($6C97): IY = the level's four records (the table at
//   $6CA9), DE = 8, B = 4; HL kept.
// - creature_at ($6C6A): which creature stands at (L, H): A = its number
//   over the whole dungeon (level * 4 + 0-3), or carry for none.
// - creatures_spawn ($6CB9): each free record may get a creature on an
//   empty cell out of the party's sight (not within 5 of it): one of the
//   level's four kinds ($90 + (level + 0-3) * 4, not $AC).
// - creatures_walk ($6D1A): each creature next to the party attacks (the
//   rest is left to the original: the message, then the fight at $3B52);
//   others but $B0 and $AC try three random steps ($40B9's offsets) onto
//   a cell that isn't a wall-like thing ($A0, $C0, $D0, $F0) and is free.
// - step_cell ($6C59): (L, H) one step in direction A from (E, D), on the
//   16x16 level.
// Cycle counts are the original's, instruction by instruction (I).
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define LEVEL    0xC970
#define PARTY_X  0xC953
#define PARTY_Y  0xC954

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static void push(uint16_t v) { push16(v); I(11); }
static void call(uint16_t addr) { I(17); call_original(addr); }
static uint8_t iy_at(uint8_t d) { return mem_read((uint16_t)(cpu.iy + d)); }
static void iy_set(uint8_t d, uint8_t v) { mem_write((uint16_t)(cpu.iy + d), v); I(19); }

static void cell_address(void) {       // $6DC0
	cpu.a = cpu.h; I(4);
	for (int i = 0; i < 4; i++) { op_add8(cpu.a, 0); I(4); }
	op_or(cpu.l); I(4);
	cpu.l = cpu.a; I(4);
	cpu.a = mem_read(LEVEL); I(13);
	op_add8(0xD0, 0); I(7);
	cpu.h = cpu.a; I(4);
	I(10);
}

static void cell_at(void) {            // $6DB9
	push(HL);
	I(17); cell_address();
	cpu.a = mem_read(HL); I(7);
	SET_HL(pop16()); I(10);
	I(10);
}

static void level_creatures(void) {    // $6C97
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
	cpu.a = mem_read(LEVEL); I(13);
	SET_HL(0x6CA9); I(10);
	I(11); I(10);                      // rst $20: HL = the word HL[A]
	op_add8(cpu.a, 0); I(4);
	if (!(cpu.f & FC)) I(12);
	else { I(7); cpu.h = op_inc8(cpu.h); I(4); }
	op_add8(cpu.l, 0); I(4);
	cpu.l = cpu.a; I(4);
	op_add8(cpu.h, cpu.f & FC); I(4);
	op_sub8(cpu.l, 0, 1); I(4);
	cpu.h = cpu.a; I(4);
	cpu.a = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	cpu.h = mem_read(HL); I(7);
	cpu.l = cpu.a; I(4);
	I(10);
	push(HL);
	cpu.iy = pop16(); I(14);
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
	SET_DE(0x0008); I(10);
	cpu.b = 0x04; I(7);
	I(10);
}

static void creature_at(void) {        // $6C6A
	push(BC); push(DE);
	push16(cpu.iy); I(15);
	I(17); level_creatures();
	int found = 0;
	do {
		cpu.a = iy_at(3); I(19);
		op_or(cpu.a); I(4);
		if (cpu.f & FZ) I(12);
		else {
			I(7);
			cpu.a = iy_at(1); I(19);
			op_sub8(cpu.l, 0, 0); I(4);
			if (!(cpu.f & FZ)) I(12);
			else {
				I(7);
				cpu.a = iy_at(2); I(19);
				op_sub8(cpu.h, 0, 0); I(4);
				if (cpu.f & FZ) { I(12); found = 1; break; }
				I(7);
			}
		}
		cpu.iy = op_add16(cpu.iy, DE); I(15);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	if (found) {
		cpu.a = mem_read(LEVEL); I(13);
		op_add8(cpu.a, 0); I(4);
		op_add8(cpu.a, 0); I(4);
		op_add8(0x04, 0); I(7);
		op_sub8(cpu.b, 0, 1); I(4);
	} else {
		op_scf(); I(4);
		I(12);
	}
	cpu.iy = pop16(); I(14);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

// ld a,(party_?) / sub v / add a,k / and $0F / cp n: C when within
static void near(uint16_t party, uint8_t v, int add_iy, uint8_t k, uint8_t n) {
	cpu.a = mem_read(party); I(13);
	op_sub8(v, 0, 1); I(add_iy ? 19 : 4);
	if (k == 1) { cpu.a = op_inc8(cpu.a); I(4); }
	else { op_add8(k, 0); I(7); }
	op_and(0x0F); I(7);
	op_sub8(n, 0, 0); I(7);
}

static void creatures_spawn(void) {    // $6CB9
	I(17); level_creatures();
	do {
		cpu.a = iy_at(3); I(19);
		op_or(cpu.a); I(4);
		if (!(cpu.f & FZ)) { I(12); goto next; }
		I(7);
		call(0x2BBF);                  // random
		op_and(0x0F); I(7);
		cpu.l = cpu.a; I(4);
		call(0x2BBF);
		op_and(0x0F); I(7);
		cpu.h = cpu.a; I(4);
		near(PARTY_X, cpu.l, 0, 0x05, 0x0B);
		if (!(cpu.f & FC)) I(12);
		else {
			I(7);
			near(PARTY_Y, cpu.h, 0, 0x05, 0x0B);
			if (cpu.f & FC) { I(12); goto next; }   // in sight
			I(7);
		}
		I(17); cell_at();
		op_or(cpu.a); I(4);
		if (!(cpu.f & FZ)) { I(12); goto next; }
		I(7);
		I(17); creature_at();
		if (!(cpu.f & FC)) { I(12); goto next; }
		I(7);
		call(0x2BBF);
		op_and(0x03); I(7);
		cpu.c = cpu.a; I(4);
		cpu.a = mem_read(LEVEL); I(13);
		op_add8(cpu.c, 0); I(4);
		op_add8(cpu.a, 0); I(4);
		op_add8(cpu.a, 0); I(4);
		op_add8(0x90, 0); I(7);
		op_sub8(0xAC, 0, 0); I(7);
		if (cpu.f & FZ) { I(12); goto next; }
		I(7);
		iy_set(3, cpu.a);
		iy_set(0, cpu.a);
		iy_set(1, cpu.l);
		iy_set(4, cpu.l);
		iy_set(2, cpu.h);
		iy_set(5, cpu.h);
	next:
		cpu.iy = op_add16(cpu.iy, DE); I(15);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	I(10);
}

static int beside(uint8_t a, uint8_t party) { return ((uint8_t)(party - a + 1) & 0x0F) < 3; }

static void creatures_walk(void) {     // $6D1A
	if (!native_on_fiber()) {          // shadow mode: an attack prints, which may wait for frames
		uint16_t t = rd16((uint16_t)(0x6CA9 + 2 * mem_read(LEVEL)));
		for (int i = 0; i < 4; i++, t += 8)
			if (mem_read(t + 3) && beside(mem_read(t + 1), mem_read(PARTY_X))
			    && beside(mem_read(t + 2), mem_read(PARTY_Y))) { native_decline(); return; }
	}
	I(17); level_creatures();
	do {
		cpu.a = iy_at(3); I(19);
		op_or(cpu.a); I(4);
		if (cpu.f & FZ) { I(12); goto next; }
		I(7);
		near(PARTY_X, iy_at(1), 1, 1, 0x03);
		if (!(cpu.f & FC)) I(12);
		else {
			I(7);
			near(PARTY_Y, iy_at(2), 1, 1, 0x03);
			if (cpu.f & FC) {          // beside the party: it attacks
				I(12);
				native_continue_at(0x6D96);
				return;
			}
			I(7);
		}
		cpu.a = iy_at(3); I(19);
		op_sub8(0xB0, 0, 0); I(7);
		if (cpu.f & FZ) { I(12); goto next; }
		I(7);
		op_sub8(0xAC, 0, 0); I(7);
		if (cpu.f & FZ) { I(12); goto next; }
		I(7);
		cpu.c = 0x03; I(7);
		for (;;) {
			call(0x2BBF);              // random
			op_and(0x03); I(7);
			call(0x40B9);              // HL = the step
			cpu.a = iy_at(1); I(19);
			op_add8(cpu.l, 0); I(4);
			op_and(0x0F); I(7);
			cpu.l = cpu.a; I(4);
			cpu.a = iy_at(2); I(19);
			op_add8(cpu.h, 0); I(4);
			op_and(0x0F); I(7);
			cpu.h = cpu.a; I(4);
			I(17); cell_at();
			op_and(0xF0); I(7);
			int wall = 0;
			static const uint8_t walls[4] = { 0xA0, 0xC0, 0xD0, 0xF0 };
			for (int i = 0; i < 4 && !wall; i++) {
				op_sub8(walls[i], 0, 0); I(7);
				if (cpu.f & FZ) { I(12); wall = 1; } else I(7);
			}
			if (!wall) {
				I(17); creature_at();
				if (cpu.f & FC) {      // free: the step
					I(12);
					cpu.a = iy_at(1); I(19);
					iy_set(4, cpu.a);
					cpu.a = iy_at(2); I(19);
					iy_set(5, cpu.a);
					iy_set(1, cpu.l);
					iy_set(2, cpu.h);
					break;
				}
				I(7);
			}
			cpu.c = op_dec8(cpu.c); I(4);
			if (cpu.f & FZ) { I(7); I(12); break; }
			I(12);
		}
	next:
		cpu.iy = op_add16(cpu.iy, DE); I(15);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	I(10);
}

static void step_cell(void) {          // $6C59
	push(DE);
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
	call(0x40B9);                      // HL = the step
	cpu.a = cpu.e; I(4);
	op_add8(cpu.l, 0); I(4);
	op_and(0x0F); I(7);
	cpu.l = cpu.a; I(4);
	cpu.a = cpu.d; I(4);
	op_add8(cpu.h, 0); I(4);
	op_and(0x0F); I(7);
	cpu.h = cpu.a; I(4);
	SET_DE(pop16()); I(10);
	I(10);
}

#define FOES(name, addr, out) Native native_##name = { #name, addr, 1, name, out, .long_calls = 1 };
FOES(cell_address, 0x6DC0, OUT_A | OUT_F | OUT_HL)
FOES(cell_at, 0x6DB9, OUT_A | OUT_F)
FOES(level_creatures, 0x6C97, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL | OUT_IY)
FOES(creature_at, 0x6C6A, OUT_A | OUT_F)
FOES(creatures_spawn, 0x6CB9, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL | OUT_IY)
FOES(creatures_walk, 0x6D1A, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL | OUT_IY)
FOES(step_cell, 0x6C59, OUT_A | OUT_F | OUT_HL)
