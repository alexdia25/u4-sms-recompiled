// The screen's moving parts, refreshed every turn and every frame of waiting
// ($129D): by the map mode, a routine updates the wind, the moons, the
// animation frames of people and monsters, copies the 11 x 11 cells around
// the party into the view buffer ($D800), puts the creatures on it, works
// out what the party can see, and queues it for the screen.
//
// The world's and the towns' refresh are C; the dungeon's and combat's are
// the original's still. Cycle counts are the original's.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define GAME_MODE   0xC600
#define MAP_KIND    0xC99F
#define VIEW_XY     0xC985
#define WIND        0xC98A          // 0-3 a direction, 4 calm
#define WIND_SHOWN  0xC98B
#define WIND_TIMER  0xC989
#define PEOPLE      0xC651
#define FRAME_COUNT 0xC0C8

static void call(uint16_t addr) { cpu.cycles += 17; call_original(addr); }

static uint8_t rnd(void) { call(0x2BBF); return cpu.a; }

// $147F: the balloon drifts with the wind every 15th time (not in dungeons)
static void balloon_drift(void) {
	call(0x957C);                      // NZ: not ... (Z: in a dungeon)
	if (!(cpu.f & FZ)) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 10 + 11 + 7 + 7;
	SET_HL(0xC99E);
	uint8_t n = (uint8_t)(mem_read(0xC99E) + 1);
	mem_write(0xC99E, n);
	cpu.a = n;
	op_sub8(0x0F, 0, 0);
	if (n < 0x0F) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 10;
	mem_write(0xC99E, 0);
	call(0x372E);                      // one more move
	cpu.cycles += 13 + 10 + 10 + 10 + 52;   // ld a,(wind) / ld hl / jp $0018
	uint8_t w = mem_read(WIND);
	cpu.a = w;
	SET_HL(rd16(0x1499 + 2 * w));
	call_original(HL);                 // step_east etc., or nothing when calm
}

// $14A4: now and then the wind changes; a new wind is shown
static void wind(void) {
	cpu.cycles += 10 + 11 + 7;
	SET_HL(WIND_TIMER);
	uint8_t t = (uint8_t)(mem_read(WIND_TIMER) - 1);
	mem_write(WIND_TIMER, t);
	if (t) cpu.cycles += 5;
	else {
		uint8_t r = rnd();
		cpu.cycles += 4 + 7 + 7;
		cpu.b = r;
		if (!(r & 1)) cpu.cycles += 5;
		else {
			cpu.cycles += 10 + 7 + 7 + 7;
			SET_HL(WIND);
			uint8_t w = mem_read(WIND);
			if (w < 4) {
				cpu.cycles += 4 + 7 + 7;
				if (!(r & 0xE0)) {     // calm
					cpu.cycles += 10 + 12;
					mem_write(WIND, 4);
				} else {               // veers by one
					cpu.cycles += 5;
					rnd();
					cpu.cycles += 7 + 4 + 7 + 7 + 7 + 12;
					cpu.a = (uint8_t)(((cpu.a & 2) - 1 + w) & 3);
					mem_write(WIND, cpu.a);
				}
			} else {                   // calm: a wind rises, half the time
				cpu.cycles += 5;
				uint8_t r2 = rnd();
				cpu.cycles += 4 + 7 + 7;
				cpu.b = r2;
				if (r2 & 0x80) cpu.cycles += 5;
				else {
					cpu.cycles += 4 + 7 + 7;
					mem_write(WIND, r2 & 3);
				}
			}
		}
	}
	cpu.cycles += 13 + 10 + 7;         // $14DB
	uint8_t w = mem_read(WIND);
	SET_HL(WIND_SHOWN);
	cpu.a = w;
	op_sub8(mem_read(WIND_SHOWN), 0, 0);
	if (w == mem_read(WIND_SHOWN)) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 7 + 4 * 5 + 10 + 30 + 10 + 10 + 7 + 11;
	mem_write(WIND_SHOWN, w);
	uint16_t src = (uint16_t)(0xAF0F + (uint8_t)(w * 32));
	SET_HL(src);
	SET_BC(0x0010);
	SET_DE(0x1770);
	cpu.a = 0x13;
	call(0x0465);                      // queue_vram_copy: the wind's arrow
	cpu.cycles += 10 + 10 + 11 + 10 + 7 + 10;
	SET_HL(src + 0x10);
	SET_BC(0x0010);
	SET_DE(0x26C0);
	cpu.a = 0x13;
	call_original(0x0465);             // jp: its ret is this one's
}

// $155A: a moon's phase moves on by a; a new phase is shown at DE
static void moon(uint16_t at, uint8_t by, uint16_t vram) {
	cpu.cycles += 17 + 7 + 7 + 4 + 4 + 4 + 7 + 6 + 7;
	uint8_t v = (uint8_t)(mem_read(at) + by);
	mem_write(at, v);
	uint8_t phase = (uint8_t)(v >> 5);
	cpu.a = phase;
	SET_HL(at - 1);
	op_sub8(mem_read(at - 1), 0, 0);
	if (phase == mem_read(at - 1)) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 7 + 4 * 5 + 4 + 7 + 10 + 11 + 10 + 7 + 10;
	mem_write(at - 1, phase);
	SET_HL(0xAFAF + (uint8_t)(phase * 32));
	SET_BC(0x0020);
	SET_DE(vram);
	cpu.a = 0x13;
	call_original(0x0465);
}

// $1506: the moons: every 8th frame they wax; where the moongate opens
static void moons(void) {
	cpu.cycles += 13 + 7 + 10 + 7;
	uint8_t a = mem_read(FRAME_COUNT) & 7;
	uint8_t bphase = 0, cphase = 0;
	if (!a) { cpu.cycles += 5; bphase = 3; cphase = 1; }
	else cpu.cycles += 10;
	cpu.cycles += 11 + 10 + 10 + 4;
	moon(0xC95E, cphase, 0x25A0);
	cpu.cycles += 10 + 10 + 10 + 4;
	moon(0xC960, bphase, 0x25C0);
	cpu.cycles += 13 + 10 + 91 + 16 + 13 + 7 + 7 + 7 + 7;
	SET_HL(rd16(0x7CA3 + 2 * mem_read(0xC95D)));
	wr16(0xC98D, HL);                  // the moongate's place
	uint8_t p = mem_read(0xC95E) & 0x1F;
	uint8_t b = 0x40;                  // the gate's size by the phase
	static const uint8_t lo[3] = { 2, 4, 6 }, hi[3] = { 0x1E, 0x1C, 0x1A };
	int i;
	for (i = 0; i < 3; i++) {
		if (p < lo[i]) { cpu.cycles += 5; break; }
		cpu.cycles += 7 + 7;
		if (p >= hi[i]) { cpu.cycles += 5; break; }
		cpu.cycles += 4;
		b++;
		if (i < 2) cpu.cycles += 7 + 7;
	}
	cpu.cycles += 4 + 13 + 10;
	cpu.a = b;
	cpu.b = b;
	mem_write(0xC98C, b);
}

// $1581: the people's and monsters' animation frames
static void animate(void) {
	cpu.cycles += 14 + 10 + 7;
	cpu.iy = PEOPLE;
	SET_DE(0x0008);
	cpu.b = 0x20;
	do {
		native_sync();                 // (the C runs ahead: let the machine keep up)
		cpu.cycles += 19 + 4 + 7 + 7;
		uint8_t g = mem_read(cpu.iy + 3);
		cpu.c = g;
		int two = 0;
		if (g >= 0x84) {
			cpu.cycles += 7 + 7;
			if (g < 0x8F) { cpu.cycles += 5; two = 1; }
		} else cpu.cycles += 5;
		if (!two) {
			cpu.cycles += 7 + 7;
			if (g < 0x20) cpu.cycles += 5;
			else {
				cpu.cycles += 7 + 7;
				if (g < 0x2F) { cpu.cycles += 5; two = 1; }
			}
		}
		if (!two) {
			cpu.cycles += 7 + 7;
			if (g < 0x50) cpu.cycles += 5;
			else {
				cpu.cycles += 7 + 7;
				if (g >= 0x5F) cpu.cycles += 5;
				else two = 1;
			}
		}
		if (two) {                     // $15A6: two frames, flipped 1 time in 4
			rnd();
			cpu.cycles += 7 + 7;
			if (cpu.a & 3) cpu.cycles += 5;
			else {
				cpu.cycles += 4 + 7 + 4 + 19 + 4 + 7 + 4 + 19 + 12;
				cpu.c = g & 0xFE;
				mem_write(cpu.iy + 0, (uint8_t)(((mem_read(cpu.iy + 0) + 1) & 1) | cpu.c));
			}
		} else {                       // $15BD
			cpu.cycles += 7 + 7;
			if (g >= 0x90) {           // monsters: four frames, 1 time in 8
				rnd();
				cpu.cycles += 7 + 7;
				if (cpu.a & 7) cpu.cycles += 5;
				else {
					cpu.cycles += 4 + 7 + 4 + 19 + 4 + 7 + 4 + 19 + 12;
					cpu.c = g & 0xFC;
					mem_write(cpu.iy + 0, (uint8_t)(((mem_read(cpu.iy + 0) + 1) & 3) | cpu.c));
				}
			} else {                   // $15D8: as it is (ships keep their heading)
				cpu.cycles += 5 + 7 + 7;
				if (g == 0x80) cpu.cycles += 5;
				else { cpu.cycles += 19; mem_write(cpu.iy + 0, g); }
			}
		}
		cpu.cycles += 15;
		cpu.iy += 8;
		cpu.cycles += --cpu.b ? 13 : 8;
	} while (cpu.b);
	cpu.cycles += 10;
}

// $1A5D: (L, H) as a cell of the 11 x 11 view; carry when inside it
static int view_cell(void) {
	cpu.cycles += 17 + 11 + 20 + 4 + 7 + 4 + 7 + 7;
	uint8_t y = (uint8_t)(cpu.h + 5 - mem_read(0xC954));
	cpu.a = y;
	int in = 0;
	if (y < 0x0B) {
		cpu.cycles += 17 + 4 + 4 + 7 + 4 + 7 + 7;   // call $36ED (y * 11), ...
		cpu.cycles += 11 + 4 * 7 + 10 + 10;   // $36ED's own
		uint8_t row = (uint8_t)(y * 11);
		cpu.d = row;
		uint8_t x = (uint8_t)(cpu.l + 5 - mem_read(0xC953));
		cpu.a = x;
		if (x < 0x0B) {
			cpu.cycles += 4 + 4 + 4 + 4;
			cpu.a = (uint8_t)(2 * (uint8_t)(x + row));
			cpu.l = cpu.a;
			in = 1;
		} else cpu.cycles += 5;
	} else cpu.cycles += 5;
	cpu.cycles += 10 + 10;
	if (in) cpu.f |= FC; else cpu.f &= ~FC;
	return in;
}

// $15E4: the moongate, the line of sight, the creatures on the view; what
// is underfoot and around; then queued for the screen ($1A7C)
static void compose(void) {
	native_sync();
	cpu.cycles += 13 + 4 + 7;
	uint8_t gate = mem_read(0xC98C);
	if (gate) {
		cpu.cycles += 4 + 16 + 17;     // ld b,a / ld hl / call $1A4B
		cpu.b = gate;
		SET_HL(rd16(0xC98D));
		uint16_t de = DE;
		int in = view_cell();
		SET_DE(de);
		cpu.cycles += 7 + 10;          // ld h,$D8 / ret
		cpu.h = 0xD8;
		cpu.cycles += 7;
		if (in) {
			cpu.cycles += 7 + 4 + 10;
			mem_write(HL, gate);
			cpu.l++;
			mem_write(HL, 0);
		} else cpu.cycles += 5;
	} else cpu.cycles += 5;
	call(0x1691);                      // the view buffer: what can be seen
	cpu.cycles += 14 + 10 + 7;
	cpu.iy = 0xC749;                   // the people, last first
	SET_DE(0xFFF8);
	cpu.b = 0x20;
	do {
		native_sync();
		cpu.cycles += 19 + 4 + 7;
		uint8_t g = mem_read(cpu.iy + 0);
		if (!g) { cpu.cycles += 5; goto next; }
		cpu.cycles += 4 + 19 + 19 + 17;
		cpu.c = g;
		cpu.l = mem_read(cpu.iy + 1);
		cpu.h = mem_read(cpu.iy + 2);
		uint16_t de = DE;
		int in = view_cell();          // $1A51: and not hidden ($7E)
		SET_DE(de);
		if (!in) { cpu.cycles += 11; }
		else {
			cpu.cycles += 5 + 7 + 7 + 7;
			cpu.h = 0xD9;
			cpu.a = mem_read(HL);
			if (cpu.a == 0x7E) { cpu.cycles += 11; in = 0; cpu.f &= ~FC; }
			else { cpu.cycles += 5 + 4 + 10; cpu.f |= FC; }
		}
		cpu.cycles += 7;
		if (!in) { cpu.cycles += 5; goto next; }
		cpu.cycles += 4 + 7 + 7;
		cpu.a = g;
		if (g == 0x14) {               // the horse faces west
			cpu.cycles += 5 + 7 + 4 + 10 + 12;
			mem_write(HL, g); cpu.l++; mem_write(HL, 0x84);
		} else {
			cpu.cycles += 7 + 7;
			if (g == 0x15) {
				cpu.cycles += 5 + 7 + 4 + 10 + 12;
				mem_write(HL, g); cpu.l++; mem_write(HL, 0x85);
			} else {
				cpu.cycles += 7 + 4 + 10;
				mem_write(HL, g); cpu.l++; mem_write(HL, 0);
			}
		}
	next:
		cpu.cycles += 15;
		cpu.iy -= 8;
		cpu.cycles += --cpu.b ? 13 : 8;
	} while (cpu.b);
	native_sync();
	cpu.cycles += 16 * 14 + 10;        // the cells around the party, and the party itself
	wr16(0xC9BB, rd16(0xD878));
	wr16(0xC990, rd16(0xD978));
	wr16(0xC992, rd16(0xD962));
	wr16(0xC994, rd16(0xD98E));
	wr16(0xC996, rd16(0xD97A));
	wr16(0xC998, rd16(0xD976));
	SET_HL(rd16(0xC959));
	wr16(0xD978, HL);
	call_original(0x1A7C);             // jp: queued for the screen
}

// $108B: the 11 x 11 cells around the party, from the map, into $D800
static void copy_view(void) {
	cpu.cycles += 17 + 13 + 4 + 10;
	if (mem_read(MAP_KIND)) { call_original(0x10E4); return; }   // towns: the original's (bounds)
	cpu.cycles += 13 + 7 + 4 + 4 + 4 + 4 + 7 + 11 + 11 + 11 + 13 + 7 + 4 + 4 + 7 + 11 + 10 + 7;
	uint16_t src = (uint16_t)(((uint8_t)((mem_read(VIEW_XY + 1) + 0xFB) * 8)) * 8 + 0xD000
		+ (uint8_t)((mem_read(VIEW_XY) + 0xFB) * 2));
	uint16_t dst = 0xD800;
	for (int row = 0; row < 11; row++) {
		native_sync();
		for (int i = 0; i < 22; i++) mem_write(dst++, mem_read(src++));
		src += 0x2A;
		cpu.cycles += 22 * 16 + 10 + 11 + 4 + 10;
	}
	SET_HL(src);
	SET_DE(dst);
	SET_BC(0x002A);
	cpu.a = 0;
	cpu.cycles += 10;
}

// $129D
static void refresh(void) {
	cpu.cycles += 13 + 7 + 7;
	uint8_t a = mem_read(GAME_MODE) & 0x7F;
	cpu.a = a;
	op_sub8(0x11, 0, 1);
	if (a < 0x11) { cpu.cycles += 11; return; }
	uint16_t bc = BC, de = DE, hl = HL, ix = cpu.ix, iy = cpu.iy;
	uint8_t kind = cpu.a;              // 0 world, 1 town, 2 dungeon, 3... combat
	cpu.cycles += 5 + 11 + 11 + 11 + 15 + 15 + 10 + 7 + 7 + 10 + 7;
	int changed = mem_read(MAP_KIND) != kind;
	mem_write(MAP_KIND, kind);
	if (!changed) cpu.cycles += 5;
	else { call(0x276D); cpu.cycles += 10; }
	cpu.cycles += 11 + 10 + 52;        // rst $18 into the table
	cpu.a = kind;
	if (kind == 0 || kind == 1) {
		if (changed) call(0x141B);
		if (kind == 0) { cpu.cycles += 17; balloon_drift(); }
		cpu.cycles += 17; wind();
		if (kind == 0) { cpu.cycles += 17; moons(); }
		cpu.cycles += 17; animate();
		copy_view();
		cpu.cycles += 10;              // jp $15E4
		compose();
	} else {
		uint16_t entry = rd16((changed ? 0x12D3 : 0x12C5) + 2 * kind);
		SET_HL(entry);
		call_original(entry);
	}
	cpu.cycles += 14 + 14 + 10 + 10 + 10 + 10;
	cpu.iy = iy; cpu.ix = ix;
	SET_HL(hl); SET_DE(de); SET_BC(bc);
}
Native native_refresh = { "refresh", 0x129D, 0, refresh, OUT_A | OUT_F, 0, 0, .long_calls = 1 };
