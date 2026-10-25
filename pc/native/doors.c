// Doors in towns (bank 2).
// - town_cell ($B5A4): HL = the cell (L, H) of the town's 32x32 map at
//   $D000 (2 bytes a cell).
// - door_ahead ($B464): the party's step (L, H) in a town: a locked door
//   ($3A) is "Locked!" and stops it; a door ($3B) opens ($3E) for two
//   turns ($C9A4, the one opened before shut again, its place at $C9A2),
//   with a sound, and stops this step too. Carry: the step is stopped.
// Cycle counts are the original's, instruction by instruction (I).
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define MAP_MODE   0xC99F
#define PARTY_X    0xC953
#define PARTY_Y    0xC954
#define DOOR_OPEN  0xC9A4
#define DOOR_AT    0xC9A2

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static void push(uint16_t v) { push16(v); I(11); }
static void call(uint16_t addr) { I(17); call_original(addr); }

static void town_cell(void) {          // $B5A4
	op_xor(cpu.a); I(4);
	cpu.h = op_rot(7, cpu.h); I(8);    // srl h / rra, twice: H's low bits into A
	op_rra(); I(4);
	cpu.h = op_rot(7, cpu.h); I(8);
	op_rra(); I(4);
	op_add8(cpu.l, 0); I(4);
	op_add8(cpu.l, 0); I(4);
	cpu.l = cpu.a; I(4);
	cpu.a = cpu.h; I(4);
	op_or(0xD0); I(7);
	cpu.h = cpu.a; I(4);
	I(10);
}

static int ret_if(int cond) { if (cond) { I(11); return 1; } I(5); return 0; }

static void door_ahead(void) {         // $B464
	if (!native_on_fiber() && mem_read(MAP_MODE)) {   // shadow mode: a door prints, which may wait for frames
		uint8_t x = (uint8_t)(mem_read(PARTY_X) + cpu.l), y = (uint8_t)(mem_read(PARTY_Y) + cpu.h);
		if (x < 0x20 && y < 0x20) {
			uint8_t t = mem_read((uint16_t)(0xD000 | (y << 6) | (x << 1)));
			if (t == 0x3A || t == 0x3B) { native_decline(); return; }
		}
	}
	cpu.a = mem_read(MAP_MODE); I(13);
	op_or(cpu.a); I(4);
	if (ret_if(cpu.f & FZ)) return;
	cpu.a = mem_read(PARTY_X); I(13);
	op_add8(cpu.l, 0); I(4);
	cpu.l = cpu.a; I(4);
	cpu.e = cpu.a; I(4);
	op_sub8(0x20, 0, 0); I(7);
	if (ret_if(!(cpu.f & FC))) return;
	cpu.a = mem_read(PARTY_Y); I(13);
	op_add8(cpu.h, 0); I(4);
	cpu.h = cpu.a; I(4);
	cpu.d = cpu.a; I(4);
	op_sub8(0x20, 0, 0); I(7);
	if (ret_if(!(cpu.f & FC))) return;
	I(17); town_cell();
	cpu.a = mem_read(HL); I(7);
	op_sub8(0x3A, 0, 0); I(7);
	if (cpu.f & FZ) {                  // $B489: locked
		I(12);
		SET_HL(0xBD0E); I(10);
		call(0x5519);                  // print_text_hl
		cpu.a = 0x10; I(7);
		call(0x05F1);                  // play_sound
		SET_HL(0xBD19); I(10);
		call(0x5519);
		op_scf(); I(4);
		I(10);
		return;
	}
	I(7);
	op_sub8(0x3B, 0, 0); I(7);
	if (!(cpu.f & FZ)) { I(7); op_or(cpu.a); I(4); I(10); return; }
	I(12);
	cpu.a = mem_read(DOOR_OPEN); I(13);    // $B49C: a door opens
	op_or(cpu.a); I(4);
	if (cpu.f & FZ) I(12);
	else {                             // the one open before shuts
		I(7);
		push(HL);
		SET_HL(rd16(DOOR_AT)); I(16);
		I(17); town_cell();
		mem_write(HL, 0x3B); I(10);
		cpu.l = op_inc8(cpu.l); I(4);
		mem_write(HL, 0x00); I(10);
		SET_HL(pop16()); I(10);
	}
	mem_write(HL, 0x3E); I(10);
	cpu.l = op_inc8(cpu.l); I(4);
	mem_write(HL, 0x00); I(10);
	cpu.a = 0x02; I(7);
	mem_write(DOOR_OPEN, cpu.a); I(13);
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
	wr16(DOOR_AT, HL); I(16);
	SET_HL(0xBD0E); I(10);
	call(0x5519);
	cpu.a = 0x0E; I(7);
	call(0x05F1);
	op_scf(); I(4);
	I(10);
}

Native native_town_cell = { "town_cell", 0xB5A4, 2, town_cell, OUT_A | OUT_F | OUT_HL, .long_calls = 1 };
Native native_door_ahead = { "door_ahead", 0xB464, 2, door_ahead, OUT_A | OUT_F | OUT_DE | OUT_HL, .long_calls = 1 };
