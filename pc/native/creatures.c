// The creatures on the map, rewritten from bank 2 ($B5B3-$BBC6): at the end
// of the party's turn the monsters of the world chase or flee the party
// (pirate ships and sea serpents fire at it), the townsfolk wander or come
// after it, and now and then a new monster appears out of view. They move
// once per 256 units of the turn's time ($C99A: a horse is quick, slow
// ground slow). A monster next to the party starts a fight.
//
// The people table ($C651, 8 bytes each): +0 the graphic shown (ships: $80-
// $83 by heading), +1 x, +2 y, +3 the graphic, +4 +5 where it was, +6 its
// mood in towns ($FF hostile; bit 7: comes after the party).
//
// Called from the turn loops (turn.c), so they are C functions of that task;
// each takes the original's cycles including its ret, the caller adds its
// call. Cycle counts are the original's.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define PEOPLE      0xC651
#define PARTY_XY    0xC953
#define VIEW_XY     0xC985          // the party within the loaded 32x32 window
#define CHUNK_XY    0xC987          // the window's corner, in 16-cell chunks
#define MAP_KIND    0xC99F          // 0 the world, 1 towns
#define MOVE_TIME   0xC99A
#define TIME_SPENT  0xC99C          // 16-bit: the creatures move each time the high byte goes up

static void call(uint16_t addr) { cpu.cycles += 17; call_original(addr); }

static void tcall(uint16_t addr, uint16_t site) {
	cpu.cycles += 17;
	task_call(addr, (uint16_t)(site + 3));
}

static uint8_t rnd(void) { call(0x2BBF); return cpu.a; }   // call random

static uint8_t abs8(uint8_t a) {       // call $000B
	cpu.cycles += 17 + 4;
	if (!(a & 0x80)) { cpu.cycles += 11; cpu.a = a; return a; }
	cpu.cycles += 5 + 8 + 10;
	cpu.a = (uint8_t)-a;
	return cpu.a;
}

static uint8_t sign_body(uint8_t a) {  // $BB0F (entered by call or falling in)
	cpu.cycles += 4;
	if (!a) { cpu.cycles += 11; cpu.a = 0; return 0; }
	cpu.cycles += 5 + 7;
	if (!(a & 0x80)) { cpu.cycles += 11; cpu.a = 1; return 1; }
	cpu.cycles += 5 + 7 + 10;
	cpu.a = 0xFF;
	return 0xFF;
}
static uint8_t sign_of(uint8_t a) { cpu.cycles += 17; return sign_body(a); }      // call $BB0F
static uint8_t random_sign(void) {    // call $BB0C
	cpu.cycles += 17;
	rnd();
	return sign_body(cpu.a);
}
static void random_step(void) {        // call $BB03: L, H = a random step
	cpu.cycles += 17;
	cpu.l = random_sign();
	cpu.cycles += 4;
	cpu.h = random_sign();
	cpu.cycles += 4 + 10;
}

// $BAF3: A = the map cell at window coordinates E (x), D (y); DE -> it
static uint8_t cell_body(void) {
	cpu.cycles += 4 + 8 + 4 + 8 + 4 + 4 + 4 + 4 + 4 + 7 + 4 + 7 + 10;
	uint8_t y = cpu.d;
	cpu.e = (uint8_t)(((y & 3) << 6) + 2 * cpu.e);
	cpu.d = (uint8_t)((y >> 2) | 0xD0);
	cpu.a = mem_read(DE);
	return cpu.a;
}

// world coordinate minus the window's corner (chunk * 16)
static uint8_t in_window(uint8_t world, uint16_t chunk_at) {
	cpu.cycles += 13 + 4 * 4 + 8;
	return (uint8_t)(world - 16 * mem_read(chunk_at));
}

// $BAD5: the cell at world (E, D); carry when outside the window
static uint8_t world_cell(void) {
	cpu.cycles += 17;
	cpu.e = in_window(cpu.e, CHUNK_XY);
	cpu.a = cpu.e;
	cpu.cycles += 4 + 4 + 7 + 4;
	if (cpu.e >= 0x20) { cpu.cycles += 11; cpu.f |= FC; return 0; }
	cpu.cycles += 5;
	cpu.d = in_window(cpu.d, CHUNK_XY + 1);
	cpu.a = cpu.d;
	cpu.cycles += 4 + 4 + 7 + 4;
	if (cpu.d >= 0x20) { cpu.cycles += 11; cpu.f |= FC; return 0; }
	cpu.cycles += 5;
	uint8_t c = cell_body();
	cpu.f &= ~FC;
	return c;
}

// $B5E2: can the creature IY step onto (L, H), on tile `tile`? Carry if not.
static int blocked(uint8_t tile) {
	cpu.cycles += 17 + 11 + 11 + 15 + 4 + 13 + 4 + 7;
	uint16_t ix = cpu.ix, de = DE, bc = BC;
	cpu.c = tile;
	int r;
	if (mem_read(MAP_KIND)) {          // towns: not off the map
		cpu.cycles += 4 + 7 + 10;
		if (cpu.l >= 0x20) { r = 1; goto set; }
		cpu.cycles += 4 + 7 + 10;
		if (cpu.h >= 0x20) { r = 1; goto set; }
	} else cpu.cycles += 5;
	cpu.cycles += 4;
	cpu.a = tile;
	call(0x3EEF);                      // carry: something solid there
	cpu.cycles += 10;
	if (cpu.f & FC) { r = 1; goto done; }
	cpu.cycles += 19 + 7 + 7;
	uint8_t g = mem_read(cpu.iy + 3);
	int twister = 0;
	if (g == 0x8C) { cpu.cycles += 5; twister = 1; }
	else {
		cpu.cycles += 7 + 7;
		if (g == 0x8E) { cpu.cycles += 5; twister = 1; }
	}
	if (!twister) {                    // nobody may stand there already
		cpu.cycles += 14 + 10 + 7;
		cpu.ix = PEOPLE;
		SET_DE(0x0008);
		cpu.b = 0x20;
		do {
			cpu.cycles += 19 + 4 + 7;
			uint8_t o = mem_read(cpu.ix + 3);
			if (o && (cpu.cycles += 5 + 7 + 7 - 5, o != 0x3C)) {
				cpu.cycles += 19 + 4 + 7;
				if (mem_read(cpu.ix + 1) == cpu.l) {
					cpu.cycles += 19 + 4 + 10;
					if (mem_read(cpu.ix + 2) == cpu.h) { r = 1; goto set; }
				} else cpu.cycles += 5;
			} else cpu.cycles += 5;
			cpu.cycles += 15;
			cpu.ix += 8;
			cpu.cycles += --cpu.b ? 13 : 8;
		} while (cpu.b);
		cpu.cycles += 13 + 4 + 7;      // nor where the party is
		if (mem_read(PARTY_XY) == cpu.l) {
			cpu.cycles += 13 + 4 + 7;
			if (mem_read(PARTY_XY + 1) == cpu.h) { cpu.cycles += 5; r = 1; goto set; }
		} else cpu.cycles += 5;
	}
	// $B63B: not straight back where it came from (3 times in 4)
	cpu.cycles += 19 + 4 + 7 + 7;
	g = mem_read(cpu.iy + 3);
	cpu.b = g;
	if (g != 0x80) {
		cpu.cycles += 19 + 4 + 7;
		if (mem_read(cpu.iy + 4) == cpu.l) {
			cpu.cycles += 19 + 4 + 7;
			if (mem_read(cpu.iy + 5) == cpu.h) {
				rnd();
				cpu.cycles += 7 + 7;
				if (!(cpu.a & 3)) { cpu.cycles += 5; r = 1; goto set; }
			} else cpu.cycles += 5;
		} else cpu.cycles += 5;
	} else cpu.cycles += 5;
	// $B656: by kind, the ground it can go on
	cpu.cycles += 4 + 7 + 7;
	if (g < 0x80) {                    // people: as the party walks
		cpu.cycles += 4;
		cpu.a = tile;
		call(0x3EB7);
		cpu.cycles += 12;
		r = (cpu.f & FC) != 0;
		goto done;
	}
	cpu.cycles += 5 + 7 + 7;
	if (g < 0x8E) {                    // ships and twisters: on water
		cpu.cycles += 7 + 4 + 7;
		cpu.a = tile;
		if (g == 0x80) {
			cpu.cycles += 7 + 4 + 12;
			r = tile >= 2;             // cp 2 / ccf
		} else {
			cpu.cycles += 5 + 7 + 4 + 12;
			r = tile >= 3;
		}
		goto flag;
	}
	cpu.cycles += 5 + 7;               // $B674: jr z (the cp $8E above)
	int water_too = 0;
	if (g == 0x8E) { cpu.cycles += 5; water_too = 1; }
	else {
		static const uint8_t kinds[4] = { 0x94, 0xB4, 0xF0, 0xF8 };
		for (int i = 0; i < 4 && !water_too; i++) {
			cpu.cycles += 7 + 7;
			if (g == kinds[i]) { cpu.cycles += 5; water_too = 1; }
		}
		if (!water_too) {
			cpu.cycles += 7 + 7;       // cp $FC / jr nz
			if (g == 0xFC) water_too = 1;
			else cpu.cycles += 5;
		}
	}
	if (water_too) {                   // $B68A: sea monsters and fliers: also on water
		cpu.cycles += 4 + 7 + 7;
		cpu.a = tile;
		if (tile < 3) { cpu.cycles += 5 + 4 + 12; r = 0; goto flag; }
		call(0x3EB7);
		cpu.cycles += 12;
		r = (cpu.f & FC) != 0;
		goto done;
	}
	cpu.cycles += 7 + 7;               // $B694
	int sea = g == 0xEC;
	if (sea) cpu.cycles += 5;
	else { cpu.cycles += 7 + 7; sea = g == 0x9C; if (!sea) cpu.cycles += 5; }
	if (sea) {                         // $B69C: only on water (and not $45)
		cpu.cycles += 4 + 7 + 7;
		cpu.a = tile;
		if (tile < 3) { cpu.cycles += 5; r = 1; goto flag; }
		cpu.cycles += 7 + 7;
		if (tile == 0x45) { cpu.cycles += 5; r = 1; goto set; }
		cpu.cycles += 12 + 4 + 12;
		r = 0;
		goto flag;
	}
	cpu.cycles += 4;                   // $B6A7
	cpu.a = tile;
	call(0x3EB7);
	cpu.cycles += 12;
	r = (cpu.f & FC) != 0;
	goto done;
set:                                   // $B6B0: scf
	cpu.cycles += 4;
flag:
	if (r) cpu.f |= FC; else cpu.f &= ~FC;
done:
	cpu.cycles += 14 + 10 + 10 + 10;
	cpu.ix = ix;
	SET_DE(de);
	SET_BC(bc);
	return r;
}

// $B6BE: the creature steps to (L, H), remembering where it was
static void move_body(void) {
	cpu.cycles += 19 * 6 + 10;
	mem_write(cpu.iy + 4, mem_read(cpu.iy + 1));
	mem_write(cpu.iy + 1, cpu.l);
	mem_write(cpu.iy + 5, mem_read(cpu.iy + 2));
	mem_write(cpu.iy + 2, cpu.h);
	cpu.a = mem_read(cpu.iy + 5);
}

// $B6D1: a world monster steps by (L, H) (one axis at random), C tries
static void world_step(void) {
	for (;;) {
		cpu.cycles += 11 + 19 + 4 + 4;
		uint16_t de = DE;
		cpu.e = (uint8_t)(mem_read(cpu.iy + 1) + cpu.l);
		cpu.a = in_window(cpu.e, CHUNK_XY);
		cpu.cycles += 4 + 7 + 10;
		if (cpu.a >= 0x20) goto failed;
		cpu.cycles += 4 + 19 + 4 + 4;
		cpu.e = cpu.a;
		cpu.d = (uint8_t)(mem_read(cpu.iy + 2) + cpu.h);
		cpu.a = in_window(cpu.d, CHUNK_XY + 1);
		cpu.cycles += 4 + 7 + 10;
		if (cpu.a >= 0x20) goto failed;
		cpu.cycles += 4;
		cpu.d = cpu.a;
		rnd();
		cpu.cycles += 7 + 7;
		int along_x;
		if (cpu.a & 1) {
			cpu.cycles += 4 + 4 + 7;
			if (!cpu.l) { cpu.cycles += 5; along_x = 0; }
			else along_x = 1;
		} else {
			cpu.cycles += 5 + 4 + 4 + 7;
			if (!cpu.h) { cpu.cycles += 5; along_x = 1; }
			else along_x = 0;
		}
		if (along_x) {                 // $B707
			cpu.cycles += 19 + 4 + 4 + 4 + 4 + 4 + 19 + 10;
			cpu.l = (uint8_t)(mem_read(cpu.iy + 1) + cpu.l);
			cpu.d = (uint8_t)(cpu.d - cpu.h);
			cpu.h = mem_read(cpu.iy + 2);
		} else {                       // $B719
			cpu.cycles += 19 + 4 + 4 + 4 + 4 + 4 + 19;
			cpu.h = (uint8_t)(mem_read(cpu.iy + 2) + cpu.h);
			cpu.e = (uint8_t)(cpu.e - cpu.l);
			cpu.l = mem_read(cpu.iy + 1);
		}
		cpu.cycles += 17;
		uint8_t tile = cell_body();
		if (!blocked(tile)) {
			cpu.cycles += 7 + 10 + 10;
			SET_DE(de);
			move_body();
			return;
		}
		cpu.cycles += 7 + 5;           // jr c,$B730
	failed:                            // $B730
		cpu.cycles += 10 + 4;
		SET_DE(de);
		if (--cpu.c == 0) { cpu.cycles += 11; return; }
		cpu.cycles += 5;
		random_step();
		cpu.cycles += 10;              // jp $B6D1
	}
}

// $B739: a townsperson steps by (L, H) (one axis at random), C tries
static void town_step(void) {
	for (;;) {
		rnd();
		cpu.cycles += 7 + 7;
		int along_x;
		if (cpu.a & 1) {
			cpu.cycles += 4 + 4 + 7;
			if (!cpu.l) { cpu.cycles += 5; along_x = 0; }
			else along_x = 1;
		} else {
			cpu.cycles += 5 + 4 + 4 + 7;
			if (!cpu.h) { cpu.cycles += 5; along_x = 1; }
			else along_x = 0;
		}
		if (along_x) {
			cpu.cycles += 19 + 4 + 4 + 19 + 10;
			cpu.l = (uint8_t)(mem_read(cpu.iy + 1) + cpu.l);
			cpu.h = mem_read(cpu.iy + 2);
		} else {
			cpu.cycles += 19 + 4 + 4 + 19;
			cpu.h = (uint8_t)(mem_read(cpu.iy + 2) + cpu.h);
			cpu.l = mem_read(cpu.iy + 1);
		}
		cpu.cycles += 11;              // $B75B
		uint16_t at = HL;
		call(0xB5A4);
		cpu.cycles += 7 + 10;
		uint8_t tile = mem_read(HL);
		SET_HL(at);
		cpu.a = tile;
		int b = blocked(tile);
		cpu.cycles += 10;
		if (!b) {                      // $B6B6: on the map
			cpu.cycles += 4 + 7;
			if (cpu.l >= 0x20) { cpu.cycles += 11; return; }
			cpu.cycles += 5 + 4 + 7;
			if (cpu.h >= 0x20) { cpu.cycles += 11; return; }
			cpu.cycles += 5;
			move_body();
			return;
		}
		cpu.cycles += 4;
		if (--cpu.c == 0) { cpu.cycles += 11; return; }
		cpu.cycles += 5;
		random_step();
		cpu.cycles += 10;
	}
}

// the creature next to the party attacks: "Attacked by <name>!", then the
// fight ($3BD9). The original's calls are on the stack: hand over to it.
static uint16_t turn_back;             // where the turn loop's "call $B4CA" returns
static void attacked(uint16_t af_back) {
	push16(turn_back);
	push16(0xB4DE);                    // call $B5B3
	push16((uint16_t)(mem_read(MAP_KIND) << 8 | z80_szp[mem_read(MAP_KIND)]));   // push af (after or a)
	push16(af_back);                   // call z,$B9E4 / call nz,$B76F
	call(0x129D);
	cpu.cycles += 10 + 17;
	SET_HL(0xBD2F);
	text_print(0xBD2F);                // "Attacked by "
	cpu.cycles += 19;
	cpu.a = mem_read(cpu.iy + 3);
	call(0x47D3);                      // HL = its name
	cpu.cycles += 17;
	text_print(HL);
	cpu.cycles += 17;
	text_newline();
	cpu.cycles += 10;
	native_continue_at(0x3BD9);
}

// $B76F: the townsfolk: the calm wander, the hostile come at the party
static int town_folk(void) {
	cpu.cycles += 14 + 10 + 7;
	cpu.iy = PEOPLE;
	SET_DE(0x0008);
	cpu.b = 0x20;
	do {
		cpu.cycles += 19 + 4 + 7;
		if (!mem_read(cpu.iy + 3)) { cpu.cycles += 5; goto next; }
		cpu.cycles += 19 + 4 + 7;
		uint8_t mood = mem_read(cpu.iy + 6);
		if (!mood) { cpu.cycles += 5; goto next; }
		cpu.cycles += 10;
		if (!(mood & 0x80)) {          // wanders, half the time
			rnd();
			cpu.cycles += 7 + 7;
			if (!(cpu.a & 1)) { cpu.cycles += 5; goto next; }
			random_step();
			cpu.cycles += 7 + 17;
			cpu.c = 2;
			uint8_t b = cpu.b;
			town_step();
			cpu.b = b;
			SET_DE(0x0008);
			cpu.cycles += 12;
			goto next;
		}
		// $B798: comes after the party
		cpu.cycles += 13 + 19 + 4 + 10;
		uint8_t dx = (uint8_t)(mem_read(PARTY_XY) - mem_read(cpu.iy + 1));
		cpu.l = dx;
		uint8_t adx = dx;
		if (dx & 0x80) { cpu.cycles += 8; adx = (uint8_t)-dx; }
		cpu.cycles += 4 + 13 + 19 + 4 + 10;
		cpu.c = adx;
		uint8_t dy = (uint8_t)(mem_read(PARTY_XY + 1) - mem_read(cpu.iy + 2));
		cpu.h = dy;
		uint8_t ady = dy;
		if (dy & 0x80) { cpu.cycles += 8; ady = (uint8_t)-dy; }
		cpu.cycles += 4 + 7 + 7;
		if ((uint8_t)(ady + adx) >= 2) {
			cpu.cycles += 4;
			cpu.l = sign_of(cpu.l);
			cpu.cycles += 4 + 4;
			cpu.h = sign_of(cpu.h);
			cpu.cycles += 4 + 7 + 17;
			cpu.c = 2;
			uint8_t b = cpu.b;
			town_step();
			cpu.b = b;
			SET_DE(0x0008);
			cpu.cycles += 12;
			goto next;
		}
		cpu.cycles += 5 + 19 + 7 + 7;  // $B7C7: next to the party
		if (mem_read(cpu.iy + 6) == 0xFF) { attacked(0xB5D0); return 1; }
		cpu.cycles += 5;
	next:
		cpu.cycles += 15;
		cpu.iy += 8;
		cpu.cycles += --cpu.b ? 13 : 8;
	} while (cpu.b);
	cpu.cycles += 10;
	return 0;
}

// $B7EB: the ship steps to (L, H) if it can, else turns at random
static void ship_step(void) {
	cpu.cycles += 17 + 11 + 4 + 4;
	uint16_t de = DE;
	cpu.e = cpu.l;
	cpu.d = cpu.h;
	world_cell();                      // (outside the window: A as it is)
	int b = blocked(cpu.a);
	cpu.cycles += 7;
	if (!b) {
		cpu.cycles += 19 + 7;
		cpu.a = mem_read(cpu.iy + 0) & 3;
		call(0xB42E);                  // carry: the wind is against it
		if (!(cpu.f & FC)) { cpu.cycles += 17; move_body(); }
		else cpu.cycles += 10;
		cpu.cycles += 12;
	} else {
		cpu.cycles += 5;
		uint8_t s = random_sign();
		cpu.cycles += 19 + 7 + 7 + 19;
		mem_write(cpu.iy + 0, (uint8_t)(((s + mem_read(cpu.iy + 0)) & 3) | 0x80));
		cpu.a = mem_read(cpu.iy + 0);
	}
	cpu.cycles += 10 + 10;
	SET_DE(de);
}

// $B812: the ship heads for the party (or keeps going, now and then)
static void ship_heads(void) {
	cpu.cycles += 11 + 4;
	uint16_t hl = HL;
	cpu.l = abs8(cpu.e);
	cpu.cycles += 4 + 4;
	cpu.h = abs8(cpu.d);
	cpu.cycles += 4 + 4 + 7 + 7;
	int turn = 1;
	if ((uint8_t)(cpu.h + cpu.l) < 6) {
		rnd();
		cpu.cycles += 7 + 7;
		if (cpu.a & 3) { cpu.cycles += 5; turn = 0; }
	} else cpu.cycles += 5;
	if (turn) {                        // $B829: face the party
		cpu.cycles += 4 + 4 + 7;
		uint8_t heading;
		if (cpu.l < cpu.h) {
			cpu.cycles += 8 + 7 + 7;
			if (!(cpu.d & 0x80)) { cpu.cycles += 5; heading = 0x83; }
			else { cpu.cycles += 7 + 12; heading = 0x81; }
		} else {
			cpu.cycles += 5 + 8 + 7 + 7;
			if (!(cpu.e & 0x80)) { cpu.cycles += 5; heading = 0x82; }
			else { cpu.cycles += 7; heading = 0x80; }
		}
		cpu.cycles += 19 + 12;
		cpu.a = heading;
		mem_write(cpu.iy + 0, heading);
	} else {                           // $B844: ahead, the way it faces
		cpu.cycles += 19 + 19 + 19 + 7 + 4 + 7;
		cpu.l = mem_read(cpu.iy + 1);
		cpu.h = mem_read(cpu.iy + 2);
		uint8_t heading = mem_read(cpu.iy + 0) & 3;
		if (heading == 1) { cpu.cycles += 5 + 4; cpu.h--; cpu.cycles += 12; }
		else {
			cpu.cycles += 4 + 7;
			if (heading == 2) { cpu.cycles += 5 + 4 + 12; cpu.l++; }
			else {
				cpu.cycles += 4 + 7;
				if (heading == 3) { cpu.cycles += 5 + 4; cpu.h++; }
				else { cpu.cycles += 4 + 12; cpu.l--; }
			}
		}
		ship_step();
	}
	cpu.cycles += 10 + 10;
	SET_HL(hl);
}

// $B867: a ship moves: along its heading if the party lies that way, else
// heads for it
static void ship_moves(void) {
	cpu.cycles += 19 + 7 + 7;
	uint8_t h0 = mem_read(cpu.iy + 0);
	cpu.a = h0;
	int axis_y;
	if (h0 == 0x81) { cpu.cycles += 5; axis_y = 1; goto north; }
	cpu.cycles += 7 + 7;
	if (h0 == 0x82) { cpu.cycles += 5; goto east; }
	cpu.cycles += 7 + 7;
	if (h0 == 0x83) { cpu.cycles += 5; goto south; }
	cpu.cycles += 4 + 4 + 10;          // west
	if (!(cpu.l & 0x80)) { ship_heads(); return; }
	goto along_x;
north:
	cpu.cycles += 4 + 4 + 10;
	if (!(cpu.h & 0x80)) { ship_heads(); return; }
	goto along_y;
east:
	cpu.cycles += 4 + 4 + 10;
	if (!cpu.l) { ship_heads(); return; }
	cpu.cycles += 10;
	if (cpu.l & 0x80) { ship_heads(); return; }
	cpu.cycles += 12;
	goto along_x;
south:
	cpu.cycles += 4 + 4 + 10;
	if (!cpu.h) { ship_heads(); return; }
	cpu.cycles += 10;
	if (cpu.h & 0x80) { ship_heads(); return; }
	cpu.cycles += 12;
	goto along_y;
along_x:                               // $B87B
	cpu.cycles += 19 + 4 + 19 + 10;
	cpu.l = (uint8_t)(cpu.l + mem_read(cpu.iy + 1));
	cpu.a = cpu.l;
	cpu.h = mem_read(cpu.iy + 2);
	cpu.cycles -= 17;                  // (jp, not call)
	ship_step();
	return;
along_y:                               // $B88A
	cpu.cycles += 19 + 4 + 19 + 10;
	cpu.h = (uint8_t)(cpu.h + mem_read(cpu.iy + 2));
	cpu.a = cpu.h;
	cpu.l = mem_read(cpu.iy + 1);
	cpu.cycles -= 17;
	ship_step();
	(void)axis_y;
}

// $B9C3: one step of the shot: the sprites cleared, its place on by its
// step ($0BA4), its cell kept at IX+$2C, its glyph turned unless the map
// cell there is $7E ($3074), then a frame (jp wait_vblank: back to `back`)
static void shot_step(uint16_t back) {
	call(0x0950);                      // sprites_clear
	call(0x0BA4);                      // slot_step_add
	call(0x399C);                      // screen_cell
	cpu.cycles += 19 + 19 + 4;
	mem_write(cpu.ix + 0x2C, cpu.l);
	mem_write(cpu.ix + 0x2D, cpu.h);
	cpu.a = cpu.h;
	call(0x36ED);                      // times_11
	cpu.cycles += 4 + 4 + 4 + 7 + 7 + 7;
	op_add8(cpu.l, 0);
	op_add8(cpu.a, 0);
	cpu.l = cpu.a;
	cpu.h = 0xD9;
	cpu.a = mem_read(HL);
	op_sub8(0x7E, 0, 0);
	if (!(cpu.f & FZ)) call(0x3074);   // missile_flip
	else cpu.cycles += 10;
	cpu.cycles += 10;
	task_call(0x041C, back);           // jp wait_vblank
}

// $B8C7: a creature at IY shoots glyph A at the party from (E, D) on the
// screen: the missile ($30DA) aimed with 8 steps ($2BFE, four times over),
// placed on the creature (the party being at (5, 5) of the window); a
// sound, 3 steps, then up to 12 more until it reaches the party or
// someone ($42B7); a burst there (glyphs 4, 5, 6 for 3, 4, 5 frames);
// the party is harmed ($4812), or the person hit is gone (a creature
// among the first 8, one time in four)
static void creature_shot(void) {
	cpu.cycles += 11 + 11 + 11 + 15;
	push16(BC); push16(DE); push16(HL); push16(cpu.iy);
	call(0x30DA);                      // queue_glyphs
	cpu.cycles += 19 + 4 + 4 + 4 + 10;
	mem_write(cpu.ix + 0x09, 0x08);
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); }
	cpu.e++; cpu.d++;
	SET_HL(0x0101);
	call(0x2BFE);                      // aim
	cpu.cycles += 11 + 11 + 19 + 19 + 4 + 11 + 11 + 19 + 19;
	SET_HL((uint16_t)(HL << 2));
	mem_write(cpu.ix + 0x0E, cpu.l);
	mem_write(cpu.ix + 0x0F, cpu.h);
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); }
	SET_HL((uint16_t)(HL << 2));
	mem_write(cpu.ix + 0x10, cpu.l);
	mem_write(cpu.ix + 0x11, cpu.h);
	for (int axis = 0; axis < 2; axis++) {   // the screen x, then y
		cpu.cycles += 13 + 8 + 19 + 7;
		cpu.a = mem_read(PARTY_XY + axis);
		op_neg();
		op_add8(mem_read(cpu.iy + 1 + axis), 0);
		op_add8(0x05, 0);
		call(0x36F7);                  // times_12
		cpu.cycles += 19 + 19;
		mem_write(cpu.ix + 0x0B + 2 * axis, cpu.a);
		mem_write(cpu.ix + 0x0A + 2 * axis, 0);
	}
	cpu.cycles += 7;
	cpu.a = 0x1C;
	call(0x05F1);                      // play_sound
	call(0x129D);                      // refresh
	cpu.cycles += 7;
	cpu.b = 3;
	do {
		cpu.cycles += 11 + 17;
		push16(BC);
		shot_step(0xB923);
		cpu.cycles += 10;
		SET_BC(pop16());
		cpu.cycles += --cpu.b ? 13 : 8;
	} while (cpu.b);
	cpu.cycles += 7;
	cpu.b = 0x0C;
	int hit = 0;
	for (;;) {
		cpu.cycles += 11 + 17;
		push16(BC);
		shot_step(0xB92C);
		cpu.cycles += 19 + 19 + 7 + 4;
		cpu.l = mem_read(cpu.ix + 0x2C);
		cpu.h = mem_read(cpu.ix + 0x2D);
		cpu.a = 0x05;
		op_sub8(cpu.l, 0, 0);
		if (cpu.f & FZ) {
			cpu.cycles += 7 + 4;
			op_sub8(cpu.h, 0, 0);
		}
		if (cpu.f & FZ) {              // at the party
			cpu.cycles += 7 + 7 + 12;
			cpu.a = 0xFF;
			hit = 1;
			break;
		}
		cpu.cycles += 12 + 13 + 4 + 7 + 4 + 13 + 4 + 7 + 4 + 7;
		cpu.a = (uint8_t)(mem_read(PARTY_XY) + cpu.l);
		op_add8(0xFB, 0);
		cpu.l = cpu.a;
		cpu.a = (uint8_t)(mem_read(PARTY_XY + 1) + cpu.h);
		op_add8(0xFB, 0);
		cpu.h = cpu.a;
		cpu.b = 0x20;
		call(0x42B7);                  // person_at
		if (!(cpu.f & FC)) { cpu.cycles += 12; hit = 1; break; }
		cpu.cycles += 7 + 10;
		SET_BC(pop16());
		if (--cpu.b) { cpu.cycles += 13; continue; }
		cpu.cycles += 8 + 12;
		break;
	}
	if (hit) {                         // $B958
		cpu.cycles += 10 + 11 + 19 + 19;
		SET_BC(pop16());
		push16((uint16_t)(cpu.a << 8 | cpu.f));
		cpu.l = mem_read(cpu.ix + 0x2C);
		cpu.h = mem_read(cpu.ix + 0x2D);
		call(0x3991);                  // screen_place
		cpu.cycles += 19 + 19 + 7;
		mem_write(cpu.ix + 0x0B, cpu.l);
		mem_write(cpu.ix + 0x0D, cpu.h);
		cpu.a = 0x12;
		call(0x05F1);
		static const uint16_t waits[3] = { 0xB978, 0xB985, 0xB992 };
		for (int i = 0; i < 3; i++) {  // the burst
			call(0x0950);
			cpu.cycles += 7;
			cpu.a = (uint8_t)(4 + i);
			call(0x307C);              // missile_draw
			cpu.cycles += 7;
			cpu.b = (uint8_t)(3 + i);
			tcall(0x0437, waits[i]);   // wait_frames
		}
		call(0x0950);
		tcall(0x041C, 0xB998);         // wait_vblank
		cpu.cycles += 10 + 4 + 10;
		{ uint16_t af = pop16(); cpu.a = af >> 8; cpu.f = (uint8_t)af; }
		op_or(cpu.a);
		if (cpu.f & FS) {              // the party
			tcall(0x4812, 0xB9A0);
			cpu.cycles += 12;
		} else {                       // $B9A5: someone else
			cpu.cycles += 11;
			push16((uint16_t)(cpu.a << 8 | cpu.f));
			call(0x48F7);              // person_record
			cpu.cycles += 10 + 7;
			{ uint16_t af = pop16(); cpu.a = af >> 8; cpu.f = (uint8_t)af; }
			op_sub8(0x08, 0, 0);
			int gone = 1;
			if (cpu.f & FC) {
				cpu.cycles += 7;
				rnd();
				cpu.cycles += 7;
				op_and(0x03);
				if (!(cpu.f & FZ)) { cpu.cycles += 12; gone = 0; }
				else cpu.cycles += 7;
			} else cpu.cycles += 12;
			if (gone) {
				cpu.cycles += 19 + 19;
				mem_write(cpu.iy + 3, 0);
				mem_write(cpu.iy + 0, 0);
			}
		}
	}
	cpu.cycles += 14 + 10 + 10 + 10 + 10;   // $B9BD
	cpu.iy = pop16();
	SET_HL(pop16());
	SET_DE(pop16());
	SET_BC(pop16());
}

// $B8A8: a ship near the party fires when broadside on, else moves
static void ship_near(uint16_t site) {
	cpu.cycles += 19 + 7 + 7;
	uint8_t h0 = mem_read(cpu.iy + 0);
	int ew = h0 == 0x80;
	if (ew) cpu.cycles += 5;
	else { cpu.cycles += 7 + 7; ew = h0 == 0x82; if (!ew) cpu.cycles += 5; }
	if (ew) {                          // $B8B3: fires north or south
		cpu.cycles += 4 + 4 + 10;
		if (cpu.l) { ship_moves(); return; }
	} else {                           // $B8BD
		cpu.cycles += 4 + 4 + 10;
		if (cpu.h) { ship_moves(); return; }
	}
	cpu.cycles += 7 + 10;              // ld a,7 / jp $B8C7
	cpu.a = 7;
	(void)site;
	creature_shot();
}

// $B9E4: the world's monsters (the first 8 of the table)
static int world_monsters(void) {
	cpu.cycles += 14 + 7;
	cpu.iy = PEOPLE;
	cpu.b = 8;
	do {
		cpu.cycles += 19 + 4 + 10;
		uint8_t g = mem_read(cpu.iy + 3);
		if (!g) goto next;
		// out of the loaded window: gone
		uint8_t wx = in_window(mem_read(cpu.iy + 1), CHUNK_XY);
		cpu.cycles += 19 - 13 + 13 + 7 + 7;
		int gone = wx >= 0x20;
		if (!gone) {
			uint8_t wy = in_window(mem_read(cpu.iy + 2), CHUNK_XY + 1);
			cpu.cycles += 19 - 13 + 13 + 7 + 7;
			gone = wy >= 0x20;
			if (!gone) cpu.cycles += 5;
		} else cpu.cycles += 5;
		if (gone) {
			cpu.cycles += 19 + 19 + 10;
			mem_write(cpu.iy + 3, 0);
			mem_write(cpu.iy + 0, 0);
			goto next;
		}
		// $BA1C: which way the party is, and how far
		cpu.cycles += 13 + 19 + 4;
		cpu.e = (uint8_t)(mem_read(PARTY_XY) - mem_read(cpu.iy + 1));
		cpu.l = sign_of(cpu.e);
		cpu.cycles += 4 + 13 + 19 + 4;
		cpu.d = (uint8_t)(mem_read(PARTY_XY + 1) - mem_read(cpu.iy + 2));
		cpu.h = sign_of(cpu.d);
		cpu.cycles += 4 + 4;
		cpu.c = abs8(cpu.e);
		cpu.cycles += 4 + 4;
		uint8_t ady = abs8(cpu.d);
		cpu.cycles += 4 + 4;
		cpu.c = (uint8_t)(cpu.c + ady);
		uint8_t dist = cpu.c;
		cpu.cycles += 19 + 7 + 7;
		g = mem_read(cpu.iy + 3);
		int twister = g == 0x8C;
		if (twister) cpu.cycles += 5;
		else { cpu.cycles += 7 + 7; twister = g == 0x8E; if (twister) cpu.cycles += 5; }
		if (!twister) {
			cpu.cycles += 4 + 7 + 7;
			if (dist < 2) { attacked(0xB5CC); return 1; }
			cpu.cycles += 5;
		}
		cpu.cycles += 19 + 7 + 7;      // $BA65
		if (g == 0x80) {               // a pirate ship
			cpu.cycles += 4 + 7 + 7;
			if (dist < 4) { cpu.cycles += 17; ship_near(0xBA71); cpu.cycles += 12; }
			else { cpu.cycles += 5 + 17; ship_moves(); cpu.cycles += 12; }
			goto next;
		}
		cpu.cycles += 5 + 7 + 7;
		int breather = g == 0x88;
		if (breather) cpu.cycles += 5;
		else {
			cpu.cycles += 7 + 7;
			breather = g == 0xE8;
			if (breather) cpu.cycles += 5;
			else { cpu.cycles += 7 + 7; breather = g >= 0xF4; if (!breather) cpu.cycles += 5; }
		}
		if (breather) {                // $BA87: within 5 cells, half the time it fires
			cpu.cycles += 4 + 7 + 7;
			if (cpu.e < 5) {
				cpu.cycles += 4 + 7 + 7;
				if (cpu.d < 5) {
					rnd();
					cpu.cycles += 7 + 7;
					if (cpu.a & 1) {
						cpu.cycles += 7;
						cpu.a = 9;
						cpu.cycles += 17;
						creature_shot();
						cpu.cycles += 12;
						goto next;
					}
					cpu.cycles += 5;
				} else cpu.cycles += 5;
			} else cpu.cycles += 5;
		}
		cpu.cycles += 19 + 7 + 7;      // $BA9F
		g = mem_read(cpu.iy + 3);
		int wander = g == 0x8C;
		if (wander) cpu.cycles += 5;
		else { cpu.cycles += 7 + 7; wander = g == 0x8E; if (wander) cpu.cycles += 5; }
		if (!wander) {
			cpu.a = g;
			call(0x430A);              // carry: it attacks
			cpu.cycles += 7;
			int chase = cpu.f & FC;
			if (!chase) {
				rnd();
				cpu.cycles += 7 + 7;
				chase = !(cpu.a & 3);
			}
			if (chase) {               // $BAC0: towards the party
				cpu.cycles += 5 + 7 + 17;
				cpu.c = 2;
				world_step();
				goto next;
			}
		}
		random_step();                 // $BAB6: somewhere at random
		cpu.cycles += 7 + 17;
		cpu.c = 1;
		world_step();
		cpu.cycles += 12;
	next:
		cpu.cycles += 10 + 15 + 4 + 10;   // $BAC5
		SET_DE(0x0008);
		cpu.iy += 8;
	} while (--cpu.b);
	cpu.cycles += 10;
	return 0;
}

// $B5B3: the creatures, once for each 256 units of time the turn took
int creatures_move(uint16_t back) {
	turn_back = back;
	tcall(0xBBE7, 0xB5B3);             // twisters and whirlpools at the party
	cpu.cycles += 16 + 20 + 11 + 16 + 4 + 4;
	uint16_t t = (uint16_t)(rd16(MOVE_TIME) + rd16(TIME_SPENT));
	wr16(TIME_SPENT, t);
	SET_HL(t);
	if (!(t >> 8)) { cpu.cycles += 11; return 0; }
	cpu.cycles += 5;
	for (;;) {
		cpu.cycles += 13 + 4 + 11;     // $B5C4
		uint8_t mode = mem_read(MAP_KIND);
		if (!mode) {
			cpu.cycles += 17;
			uint8_t b = cpu.b;
			if (world_monsters()) return 1;
			cpu.b = b;
			cpu.cycles += 10 + 10;
		} else {
			cpu.cycles += 10 + 10 + 17;
			if (town_folk()) return 1;
		}
		cpu.cycles += 10 + 11 + 10;
		uint8_t left = (uint8_t)(mem_read(TIME_SPENT + 1) - 1);
		mem_write(TIME_SPENT + 1, left);
		SET_HL(TIME_SPENT + 1);
		if (!left) {                   // jp z,$BBE7: it returns for $B5B3's caller
			task_call(0xBBE7, 0xB4DE);
			return 0;
		}
		call(0x041C);                  // wait_vblank
		call(0x129D);
		tcall(0xBBE7, 0xB5DD);
		cpu.cycles += 12;
	}
}

// $BB17: now and then a new monster appears somewhere out of view: on water
// a ship or a sea creature, on land one that grows stronger with the moves
void creatures_appear(void) {
	cpu.cycles += 13 + 4;
	if (mem_read(MAP_KIND)) { cpu.cycles += 11; return; }
	cpu.cycles += 5;
	rnd();
	cpu.cycles += 7;
	if (cpu.a & 0x0F) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 14 + 7;
	cpu.iy = PEOPLE;
	cpu.b = 4;
	do {
		cpu.cycles += 11 + 19 + 4 + 10;
		uint8_t bsave = cpu.b;
		if (mem_read(cpu.iy + 3)) goto next;
		rnd();
		cpu.cycles += 7 + 4;
		cpu.e = cpu.a & 0x1F;
		rnd();
		cpu.cycles += 7 + 4 + 13 + 4 + 7;
		cpu.d = cpu.a & 0x1F;
		uint8_t dx = (uint8_t)(mem_read(VIEW_XY) - cpu.e);
		if (mem_read(VIEW_XY) < cpu.e) { cpu.cycles += 8; dx = (uint8_t)-dx; } else cpu.cycles += 5;
		cpu.cycles += 7 + 7;
		if (dx < 6) { cpu.cycles += 5; goto next; }
		cpu.cycles += 13 + 4 + 7;
		uint8_t dy = (uint8_t)(mem_read(VIEW_XY + 1) - cpu.d);
		if (mem_read(VIEW_XY + 1) < cpu.d) { cpu.cycles += 8; dy = (uint8_t)-dy; } else cpu.cycles += 5;
		cpu.cycles += 7 + 7;
		if (dy < 6) { cpu.cycles += 5; goto next; }
		cpu.cycles += 11 + 17;
		uint8_t e = cpu.e, d = cpu.d;
		uint8_t tile = cell_body();
		cpu.e = e; cpu.d = d;
		cpu.cycles += 10 + 7 + 7;
		uint8_t g;
		if (tile < 2) {                // deep water
			rnd();
			cpu.cycles += 7 + 7;
			if (cpu.a & 7) { cpu.cycles += 5; goto next; }
			rnd();
			cpu.cycles += 7 + 4 + 7 + 7 + 7;
			g = (uint8_t)(2 * (cpu.a & 7) + 0x80);
			if (g == 0x82) { cpu.cycles += 7; g = 0x80; } else cpu.cycles += 5;
			cpu.cycles += 12;
		} else {
			cpu.cycles += 5 + 7 + 7;
			if (tile < 4) { cpu.cycles += 5; goto next; }
			cpu.cycles += 7 + 7;
			if (tile >= 8) { cpu.cycles += 5; goto next; }
			cpu.cycles += 10;          // land: the more moves so far, the stronger
			SET_HL(0xBBD7);
			call(0xBBC6);              // carry: fewer moves than 10000
			uint8_t mask = 3;
			cpu.cycles += 7 + 7;
			if (!(cpu.f & FC)) {
				cpu.cycles += 10;
				SET_HL(0xBBDF);
				call(0xBBC6);
				mask = 7;
				cpu.cycles += 7 + 7;
				if (!(cpu.f & FC)) { cpu.cycles += 7; mask = 0x0F; }
				else cpu.cycles += 5;
			} else cpu.cycles += 5;
			cpu.b = mask;
			rnd();
			cpu.cycles += 4;
			cpu.c = cpu.a;
			rnd();
			cpu.cycles += 4 + 4 + 4 + 4 + 7;
			g = (uint8_t)(((cpu.a & cpu.c & mask) << 2) + 0xC0);
		}
		cpu.cycles += 19 + 19 + 13 + 16 + 4 + 19 + 13 + 16 + 4 + 19;   // $BB9F
		cpu.a = g;
		mem_write(cpu.iy + 3, g);
		mem_write(cpu.iy + 0, g);
		mem_write(cpu.iy + 1, (uint8_t)(16 * mem_read(CHUNK_XY) + cpu.e));
		mem_write(cpu.iy + 2, (uint8_t)(16 * mem_read(CHUNK_XY + 1) + cpu.d));
	next:
		cpu.cycles += 10 + 15 + 10 + 4 + 10;   // $BBBB
		SET_BC(0x0008);
		cpu.iy += 8;
		cpu.b = bsave;
	} while (--cpu.b);
	cpu.cycles += 10;
}
