// The start of the game, as C tasks (bank 3): each game mode loads its
// graphics and starts one task ($0B33), which runs the screen. Original
// routines that wait for frames are called with task_call. Cycle counts are
// the original's.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define SLOT        cpu.ix
#define GAME_MODE   0xC600
#define JOY_PRESSED 0xC0B0

static void call(uint16_t addr) { cpu.cycles += 17; call_original(addr); }

// call an original routine that may wait for frames, from `site`
static void tcall(uint16_t addr, uint16_t site) {
	cpu.cycles += 17;
	task_call(addr, (uint16_t)(site + 3));
}

static void yield_at(uint16_t site) {  // "call yield" at site: the next turn goes on after it
	cpu.cycles += 17 + 58;
	SET_HL((uint16_t)(site + 3));
	task_wait_turn((uint16_t)(site + 3));
}

// $8220 (game mode 2): the opening words, for 120 turns or until a button,
// faded in and out; then the title (mode 3)
static void opening(void) {
	tcall(0x7EED, 0x8220);
	cpu.cycles += 10;
	SET_HL(0x8262);
	tcall(0x7EFF, 0x8226);             // the frame
	cpu.cycles += 10 + 7;
	SET_HL(0x826C);
	cpu.b = 9;
	tcall(0x7E51, 0x822E);             // the nine lines
	tcall(0x0458, 0x8231);
	cpu.cycles += 10 + 10;
	SET_HL(0x9C18);
	SET_BC(0x0301);
	tcall(0x0A22, 0x823A);             // fade in
	tcall(0x0A82, 0x823D);
	cpu.cycles += 19;
	mem_write(SLOT + 0x16, 120);
	for (;;) {
		yield_at(0x8244);
		cpu.cycles += 13 + 7 + 7;
		cpu.a = mem_read(JOY_PRESSED);
		op_and(0x30);
		if (cpu.a) { cpu.cycles += 5; break; }
		cpu.cycles += 23 + 7;
		uint8_t n = (uint8_t)(mem_read(SLOT + 0x16) - 1);
		mem_write(SLOT + 0x16, n);
		op_dec8((uint8_t)(n + 1));
		if (!n) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 10;
	SET_BC(0x8301);
	tcall(0x0A22, 0x8256);             // fade out
	tcall(0x0A82, 0x8259);
	cpu.cycles += 7 + 13 + 10;
	cpu.a = 3;
	mem_write(GAME_MODE, 3);
}
Native native_opening = { "opening", 0x8220, 3, opening, 0, 0, 0, .task = 1 };

// ------------------------------------------------------------ the title --
// $8305 (game mode 3): the field drawn in from the middle, "ULTIMA" drawn
// line by line, the rest of the logo slid in, the colours faded up (a
// button makes all that instant: bit 0 of +$06), then the demo: people
// walking about on the field by a script ($929D) until a button - the
// Options menu - or the script's end (mode 4).

#define DEMO_WAIT   0xC650          // turns until the script's next step
#define DEMO_AT     0xC64E          // the script's position (16-bit)
#define PEOPLE      0xC651          // 32 people, 8 bytes each: +0 tile, +1 x, +2 y, +3 kind

static void ldir(void) {
	uint16_t n = BC;
	cpu.cycles += 21 * (n - 1) + 16;
	for (; n; n--) { mem_write(DE, mem_read(HL)); SET_HL(HL + 1); SET_DE(DE + 1); }
	SET_BC(0);
	cpu.f &= ~(FH | FN | FP);
}

static void push(uint16_t v) { cpu.cycles += 11; push16(v); }
static uint16_t pop(void) { cpu.cycles += 10; return pop16(); }

// call $859F at site: wait for the next turn - unless a button has been
// pressed (now or before): from then on, no waiting
static void step_at(uint16_t site) {
	cpu.cycles += 17 + 20;
	if (mem_read(SLOT + 6) & 1) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 13 + 7;
	cpu.a = mem_read(JOY_PRESSED);
	op_and(0x30);
	if (cpu.a) {
		cpu.cycles += 7 + 23 + 10;
		mem_write(SLOT + 6, mem_read(SLOT + 6) | 1);
		return;
	}
	cpu.cycles += 12 + 58;             // its own yield
	SET_HL((uint16_t)(site + 3));
	task_wait_turn((uint16_t)(site + 3));
}

static void field_copy(void) {         // call $8691: the field's map, $D800 -> $D900
	cpu.cycles += 17 + 10 + 10 + 10 + 10;
	SET_HL(0xD800); SET_DE(0xD900); SET_BC(0x00BE);
	ldir();
}

// call $85B8 at site: two characters of the line being drawn (+$1C, then
// +$1D) at +$1A/+$1B
static void two_chars(uint16_t site) {
	cpu.cycles += 17 + 19 + 19 + 19 + 23;
	cpu.e = mem_read(SLOT + 0x1A);
	cpu.d = mem_read(SLOT + 0x1B);
	cpu.a = mem_read(SLOT + 0x1C);
	mem_write(SLOT + 0x1C, (uint8_t)(cpu.a + 1));
	tcall(0x0210, 0x85C4);
	cpu.cycles += 4 + 19 + 10;
	cpu.e++;
	cpu.a = mem_read(SLOT + 0x1D);
	task_call(0x0210, (uint16_t)(site + 3));   // jp $0210
}

// call $85DC: B rows of +$1C bytes from HL to the screen at DE; then +$1A/+$1B
// back 4 and +$1C 4 more
static void rows(void) {
	cpu.cycles += 17 + 19;
	cpu.c = mem_read(SLOT + 0x1C);
	for (;;) {
		push(BC); push(HL); push(DE);
		cpu.cycles += 7 + 7;
		cpu.b = 0;
		cpu.a = 0x14;
		tcall(0x07F4, 0x85E6);
		cpu.cycles += 10 + 11 + 4 + 11;
		SET_BC(0x0020);
		SET_HL(pop());
		SET_HL(op_add16(HL, BC));
		uint16_t t = HL; SET_HL(DE); SET_DE(t);
		SET_HL(pop());
		SET_HL(op_add16(HL, BC));
		SET_BC(pop());
		if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
	}
	cpu.cycles += 19 + 19 + 10 + 11 + 19 + 19 + 19 + 7 + 19 + 10;
	SET_HL((uint16_t)(mem_read(SLOT + 0x1A) | mem_read(SLOT + 0x1B) << 8));
	SET_DE(0xFFFC);
	SET_HL(op_add16(HL, DE));
	mem_write(SLOT + 0x1A, cpu.l);
	mem_write(SLOT + 0x1B, cpu.h);
	cpu.a = mem_read(SLOT + 0x1C);
	op_add8(4, 0);
	mem_write(SLOT + 0x1C, cpu.a);
}

// call $869D: the demo's people step through their frames, at random
static void people_animate(void) {
	cpu.cycles += 17 + 14 + 10 + 7;
	cpu.iy = PEOPLE;
	SET_DE(0x0008);
	cpu.b = 0x20;
	for (;;) {
		cpu.cycles += 19 + 7 + 7;
		uint8_t k = mem_read(cpu.iy + 3);
		cpu.a = k;
		int mask = 0;                  // 1: $01FE, 2: $03FC
		if (k >= 0x84) {
			cpu.cycles += 7 + 7;
			if (k < 0x8F) { cpu.cycles += 5; mask = 1; }
		} else cpu.cycles += 5;
		if (!mask) {                   // $86B1
			cpu.cycles += 7 + 7;
			if (k >= 0x20) {
				cpu.cycles += 7 + 7;
				if (k < 0x2F) { cpu.cycles += 5; mask = 1; }
			} else cpu.cycles += 5;
		}
		if (!mask) {                   // $86B9
			cpu.cycles += 7 + 7;
			if (k < 0x50) cpu.cycles += 5;
			else {
				cpu.cycles += 7 + 7;
				if (k >= 0x5F) cpu.cycles += 5; else mask = 1;
			}
		}
		if (!mask) {                   // $86C6
			cpu.cycles += 7 + 7;
			op_sub8(0x90, 0, 0);
			if (k < 0x90) cpu.cycles += 5; else mask = 2;
		}
		if (mask) {
			if (mask == 1) { cpu.cycles += 10 + 12; SET_HL(0x01FE); }
			else { cpu.cycles += 10; SET_HL(0x03FC); }
			call(0x7ED8);              // random
			cpu.cycles += 7 + 7;
			op_sub8(0xC0, 0, 0);
			if (!(cpu.f & FC)) cpu.cycles += 5;
			else {
				cpu.cycles += 4 + 4 + 19 + 4 + 4 + 19;
				op_and(cpu.h);
				cpu.h = cpu.a;
				cpu.a = mem_read(cpu.iy + 0);
				op_and(cpu.l);
				op_or(cpu.h);
				mem_write(cpu.iy + 0, cpu.a);
			}
		}
		cpu.cycles += 15;              // $86DE
		cpu.iy += 8;
		if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
	}
	cpu.cycles += 10;
}

static uint8_t demo_byte(void) {       // call $8788: the script's next byte
	cpu.cycles += 17 + 11 + 11 + 16 + 6 + 16 + 10 + 11 + 7;
	push16(DE); push16(HL);
	uint16_t at = (uint16_t)(rd16(DEMO_AT) + 1);
	wr16(DEMO_AT, at);
	SET_DE(0x929D);
	SET_HL(op_add16(at, 0x929D));
	cpu.a = 0x15;
	call(0x0815);                      // A = the byte, from bank $15
	cpu.cycles += 10 + 10 + 10;
	SET_HL(pop16()); SET_DE(pop16());
	return cpu.a;
}

static uint8_t demo_kind(uint8_t a) {  // call $879D: the kind of person n (table $928E)
	cpu.cycles += 17 + 11 + 11 + 7 + 4 + 7 + 10;
	push16(BC); push16(HL);
	cpu.a = a;
	op_and(0x0F);
	cpu.c = cpu.a;
	cpu.a = 0x15;
	SET_HL(0x928E);
	call(0x07FD);
	cpu.cycles += 10 + 10 + 10;
	SET_HL(pop16()); SET_BC(pop16());
	return cpu.a;
}

// call $86E3: the script: $0n..$6n puts person n (a kind and a frame) at
// a place, $7n takes them away, $8n waits n turns (the field redrawn with
// everyone on it); anything else is the end
static void demo_step(void) {
	cpu.cycles += 17 + 13 + 4 + 7;
	uint8_t w = mem_read(DEMO_WAIT);
	cpu.a = w;
	op_or(w);
	if (w) {
		cpu.cycles += 4 + 13 + 10;
		cpu.a = op_dec8(w);
		mem_write(DEMO_WAIT, cpu.a);
		return;
	}
	cpu.cycles += 5;
	for (;;) {
		uint8_t op = demo_byte();      // $86EE
		cpu.cycles += 7 + 10;
		op_sub8(0x70, 0, 0);
		if (op < 0x70) {               // $8707: a person, placed
			cpu.cycles += 4 + 7;
			cpu.b = op;
			cpu.a = op;
			op_and(0x0F);
			call(0x7E93);              // IY = person n
			cpu.cycles += 4;
			uint8_t c = demo_kind(op);
			cpu.cycles += 4 + 4 + 4 * 4 + 7 + 19;
			cpu.c = c;
			cpu.a = (uint8_t)(op >> 4);
			op_and(0x0F);
			mem_write(cpu.iy + 2, cpu.a);
			uint8_t d = demo_byte();
			cpu.cycles += 4 + 7 + 19 + 4 + 4 * 3 + 7 + 4 + 19 + 4 + 4 + 7 + 4 + 7;
			cpu.d = d;
			mem_write(cpu.iy + 1, d & 0x1F);
			uint8_t t = (uint8_t)(((d >> 5) & 7) + c);
			mem_write(cpu.iy + 0, t);
			cpu.e = t;
			cpu.a = op_dec8(op & 0x0F);
			if (cpu.a == 0) { cpu.cycles += 19 + 10; mem_write(cpu.iy + 3, c); }
			else { cpu.cycles += 5 + 19 + 10; mem_write(cpu.iy + 3, t); }
			continue;
		}
		cpu.cycles += 7 + 10;
		cpu.a = op;
		op_sub8(0x80, 0, 0);
		if (op < 0x80) {               // $8742: a person taken away
			cpu.cycles += 7 + 19 + 19 + 10;
			cpu.a = op;
			op_and(0x0F);
			call(0x7E93);
			mem_write(cpu.iy + 0, 0);
			mem_write(cpu.iy + 3, 0);
			continue;
		}
		cpu.cycles += 7 + 10;
		cpu.a = op;
		op_sub8(0x90, 0, 0);
		if (op < 0x90) {               // $8752: wait, everyone drawn on the field
			cpu.cycles += 7 + 13;
			cpu.a = op;
			op_and(0x0F);
			mem_write(DEMO_WAIT, cpu.a);
			field_copy();
			cpu.cycles += 14 + 10 + 7;
			cpu.iy = PEOPLE;
			SET_DE(0x0008);
			cpu.b = 0x20;
			for (;;) {
				cpu.cycles += 19 + 4 + 7;
				uint8_t tile = mem_read(cpu.iy + 0);
				cpu.a = tile;
				op_or(tile);
				if (tile) {            // at $D900 + 2 * (19 * y + x)
					cpu.cycles += 4 + 19 + 4 * 10 + 19 + 4 + 4 + 4 + 7 + 7 + 4 + 10;
					uint8_t y2 = (uint8_t)(mem_read(cpu.iy + 2) * 2);
					uint8_t l = (uint8_t)(y2 * 19);
					cpu.a = (uint8_t)(mem_read(cpu.iy + 1) * 2);
					op_add8(l, 0);
					cpu.l = cpu.a;
					cpu.h = 0xD9;
					cpu.c = tile;
					mem_write(HL, tile);
					cpu.l++;
					mem_write(HL, 0);
				} else cpu.cycles += 5;
				cpu.cycles += 15;
				cpu.iy += 8;
				if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
			}
			cpu.cycles += 10;
			return;
		}
		cpu.cycles += 10 + 16 + 10;    // the end
		SET_HL(0xFFFF);
		wr16(DEMO_AT, 0xFFFF);
		return;
	}
}

static void far_copy(uint8_t bank, uint16_t from, uint16_t to, uint16_t n, uint16_t routine, uint16_t site) {
	cpu.cycles += 7 + 10 + 10 + 10;    // ld a / ld hl / ld de / ld bc
	cpu.a = bank;
	SET_HL(from); SET_DE(to); SET_BC(n);
	tcall(routine, site);
}

static void title(void) {
	far_copy(0x15, 0x833E, 0xCD80, 0x0280, 0x07EC, 0x8310);
	far_copy(0x15, 0x8000, 0xD800, 0x00BE, 0x07EC, 0x831E);
	field_copy();
	tcall(0x7E42, 0x8324);
	cpu.cycles += 19 + 19 + 19;
	mem_write(SLOT + 0x1A, 0xA0);
	mem_write(SLOT + 0x1B, 0xA0);
	mem_write(SLOT + 0x16, 0x10);
	for (;;) {                         // $8333: the field, from the middle out
		cpu.cycles += 7 + 19 + 10;
		cpu.d = 0xCD;
		cpu.e = mem_read(SLOT + 0x1A);
		SET_BC(0x0A01);
		tcall(0x0499, 0x8338);
		cpu.cycles += 4 + 4 + 19 + 19 + 10;
		cpu.e += 2;
		mem_write(SLOT + 0x1A, cpu.e);
		cpu.e = mem_read(SLOT + 0x1B);
		SET_BC(0x0A01);
		tcall(0x0499, 0x8349);
		cpu.cycles += 4 + 4 + 19;
		cpu.e -= 2;
		op_dec8((uint8_t)(cpu.e + 1));
		mem_write(SLOT + 0x1B, cpu.e);
		yield_at(0x8351);
		cpu.cycles += 23 + 7;
		uint8_t n = op_dec8(mem_read(SLOT + 0x16));
		mem_write(SLOT + 0x16, n);
		if (!n) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 19 * 4;              // $8359: "ULTIMA", line by line (the lines at $8380)
	mem_write(SLOT + 0x08, 0x14);
	mem_write(SLOT + 0x12, 0x80);
	mem_write(SLOT + 0x13, 0x83);
	mem_write(SLOT + 0x16, 0x02);
	for (;;) {
		call(0x0C19);                  // a line's start, or 0: done
		cpu.cycles += 4 + 7;
		op_or(cpu.a);
		if (!cpu.a) { cpu.cycles += 5; break; }
		cpu.cycles += 19;
		mem_write(SLOT + 0x0B, cpu.a);
		call(0x0C19);
		cpu.cycles += 19;
		mem_write(SLOT + 0x0D, cpu.a);
		for (;;) {                     // $8378: a point of it
			cpu.cycles += 19 + 19 + 7;
			cpu.l = mem_read(SLOT + 0x0B);
			cpu.h = mem_read(SLOT + 0x0D);
			cpu.a = 1;
			call(0x8B16);
			cpu.cycles += 23 + 7;
			uint8_t n = op_dec8(mem_read(SLOT + 0x16));
			mem_write(SLOT + 0x16, n);
			if (!n) {
				cpu.cycles += 19;
				mem_write(SLOT + 0x16, 2);
				step_at(0x838C);
			} else cpu.cycles += 5;
			call(0x0BEE);              // the next point; carry: the line's end
			cpu.cycles += 7;
			if (!(cpu.f & FC)) { cpu.cycles += 5; continue; }
			break;
		}
		cpu.cycles += 12;
	}
	far_copy(0x14, 0x8500, 0x21C0, 0x0060, 0x07F4, 0x83A1);   // $8396
	step_at(0x83A4);
	far_copy(0x14, 0x8560, 0x3E40, 0x0080, 0x07F4, 0x83B2);
	cpu.cycles += 19 * 3;
	mem_write(SLOT + 0x1A, 0x0E);
	mem_write(SLOT + 0x1B, 0x39);
	mem_write(SLOT + 0x16, 0x12);
	for (;;) {                         // $83C1
		cpu.cycles += 19 + 19 + 19;
		mem_write(SLOT + 0x1C, 0xF2);
		mem_write(SLOT + 0x1D, 0x09);
		mem_write(SLOT + 0x17, 0x04);
		for (;;) {
			two_chars(0x83CD);
			step_at(0x83D0);
			cpu.cycles += 23 + 7;
			uint8_t n = op_dec8(mem_read(SLOT + 0x17));
			mem_write(SLOT + 0x17, n);
			if (!n) break;
			cpu.cycles += 5;
		}
		cpu.cycles += 23 + 23 + 23 + 7;
		mem_write(SLOT + 0x1A, (uint8_t)(mem_read(SLOT + 0x1A) + 2));
		uint8_t n = op_dec8(mem_read(SLOT + 0x16));
		mem_write(SLOT + 0x16, n);
		if (!n) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 19 + 19;             // $83E3
	mem_write(SLOT + 0x1C, 0xF2);
	mem_write(SLOT + 0x1D, 0x09);
	two_chars(0x83EB);
	step_at(0x83EE);
	two_chars(0x83F1);
	step_at(0x83F4);
	cpu.cycles += 10 + 7;              // $83F7
	SET_DE(0x23C0);
	cpu.b = 6;
	for (;;) {
		push(BC); push(DE);
		cpu.cycles -= 10;              // (no ld de here)
		far_copy(0x14, 0x85C0, DE, 0x0020, 0x07F4, 0x8406);
		SET_HL(pop());
		cpu.cycles += 10 + 11 + 4;
		SET_BC(0x0020);
		SET_HL(op_add16(HL, BC));
		uint16_t t = HL; SET_HL(DE); SET_DE(t);
		SET_BC(pop());
		if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
	}
	cpu.cycles += 10 + 10 + 10;
	SET_HL(0x85CE); SET_DE(0x391A); SET_BC(0x000E);
	tcall(0x024C, 0x841B);
	step_at(0x841E);
	cpu.cycles += 19 * 4;              // $8421
	mem_write(SLOT + 0x1A, 0x3C);
	mem_write(SLOT + 0x1B, 0x22);
	mem_write(SLOT + 0x1C, 0x04);
	mem_write(SLOT + 0x16, 0x08);
	for (;;) {                         // $8431
		cpu.cycles += 10 + 19 + 19 + 7;
		SET_HL(0x85E0);
		cpu.e = mem_read(SLOT + 0x1A);
		cpu.d = mem_read(SLOT + 0x1B);
		cpu.b = 0x0D;
		rows();
		step_at(0x843F);
		step_at(0x8442);
		step_at(0x8445);
		cpu.cycles += 23 + 10;
		uint8_t n = op_dec8(mem_read(SLOT + 0x16));
		mem_write(SLOT + 0x16, n);
		if (!n) break;
	}
	cpu.cycles += 19 * 4;              // $844E
	mem_write(SLOT + 0x1A, 0x90);
	mem_write(SLOT + 0x1B, 0x87);
	mem_write(SLOT + 0x1C, 0x04);
	mem_write(SLOT + 0x16, 0x05);
	for (;;) {                         // $845E
		cpu.cycles += 19 + 19 + 10 + 7;
		cpu.l = mem_read(SLOT + 0x1A);
		cpu.h = mem_read(SLOT + 0x1B);
		SET_DE(0x23CC);
		cpu.b = 6;
		rows();
		step_at(0x846C);
		step_at(0x846F);
		step_at(0x8472);
		cpu.cycles += 23 + 10;
		uint8_t n = op_dec8(mem_read(SLOT + 0x16));
		mem_write(SLOT + 0x16, n);
		if (!n) break;
	}
	cpu.cycles += 19 + 19;             // $847B: the colours, up
	mem_write(SLOT + 0x1A, 0x00);
	mem_write(SLOT + 0x16, 0x40);
	for (;;) {
		cpu.cycles += 20 + 7;
		if (mem_read(SLOT + 6) & 1) {
			cpu.cycles += 19 + 19;
			mem_write(SLOT + 0x1A, 0x3F);
			mem_write(SLOT + 0x16, 0x01);
		} else cpu.cycles += 5;
		cpu.cycles += 19;
		cpu.a = mem_read(SLOT + 0x1A);
		tcall(0x7F3E, 0x8494);
		step_at(0x8497);
		cpu.cycles += 23 + 23 + 7;
		mem_write(SLOT + 0x1A, (uint8_t)(mem_read(SLOT + 0x1A) + 1));
		uint8_t n = op_dec8(mem_read(SLOT + 0x16));
		mem_write(SLOT + 0x16, n);
		if (!n) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 7 + 13 + 7;          // $84A2: the music, the other task, the demo
	cpu.a = 1;
	mem_write(0xC00C, 1);
	call(0x0621);
	cpu.cycles += 7 + 10;
	cpu.c = 0x40;
	SET_DE(0x87AD);
	call(0x0B35);                      // start the task at $87AD
	cpu.cycles += 4 + 13 + 10 + 16 + 10 + 10 + 10 + 10;
	cpu.a = 0;
	op_xor(0);
	mem_write(DEMO_WAIT, 0);
	wr16(DEMO_AT, 0xFFFF);
	SET_HL(PEOPLE); SET_DE(PEOPLE + 1); SET_BC(0x00FF);
	mem_write(PEOPLE, 0);
	ldir();
	for (;;) {
		cpu.cycles += 19;              // $84CB
		mem_write(SLOT + 0x16, 2);
		int pressed = 0;
		for (;;) {
			cpu.cycles += 13 + 7 + 7;
			cpu.a = mem_read(JOY_PRESSED);
			op_and(0x30);
			if (cpu.a) { cpu.cycles += 5; pressed = 1; break; }
			tcall(0x7E3F, 0x84D6);
			yield_at(0x84D9);
			cpu.cycles += 23 + 7;
			uint8_t n = op_dec8(mem_read(SLOT + 0x16));
			mem_write(SLOT + 0x16, n);
			if (!n) break;
			cpu.cycles += 5;
		}
		if (!pressed) {
			people_animate();
			demo_step();
			cpu.cycles += 16 + 6 + 4 + 4 + 7;
			uint16_t at = (uint16_t)(rd16(DEMO_AT) + 1);
			SET_HL(at);
			cpu.a = cpu.h;
			op_or(cpu.l);
			if (at) { cpu.cycles += 5; continue; }
			cpu.cycles += 10;          // $84EF: the end of the demo
			SET_BC(0x8301);
			tcall(0x0A22, 0x84F2);
			tcall(0x0A82, 0x84F5);
			cpu.cycles += 7 + 13 + 10;
			cpu.a = 4;
			mem_write(GAME_MODE, 4);
			return;
		}
		tcall(0x8A97, 0x84FE);         // $84FE: the Options menu
		cpu.cycles += 10 + 16 + 10 + 16 + 10;
		wr16(0xC608, 0x0030);
		wr16(0xC60A, 0x0100);
		SET_HL(0x8610);
		tcall(0x7E4E, 0x8510);
		cpu.cycles += 10 + 7;
		SET_HL(0x864C);
		cpu.b = 4;
		tcall(0x7E51, 0x8518);
		cpu.cycles += 19;
		mem_write(SLOT + 0x1A, 0);
		for (;;) {
			cpu.cycles += 19 + 10 + 48 + 4 + 7 + 16;   // $851F: the hand at the choice
			uint8_t row = mem_read(0x860D + mem_read(SLOT + 0x1A));
			SET_HL((uint16_t)(row << 8 | 0x34));
			cpu.a = row;
			wr16(0xC604, HL);
			tcall(0x7F5F, 0x852C);
			yield_at(0x852F);
			cpu.cycles += 13 + 8 + 7;
			uint8_t joy = mem_read(JOY_PRESSED);
			cpu.a = joy;
			if (joy & 1) {
				cpu.cycles += 23 + 12;
				mem_write(SLOT + 0x1A, (uint8_t)(mem_read(SLOT + 0x1A) - 1));
			} else {
				cpu.cycles += 5 + 8 + 7;
				if (!(joy & 2)) {
					cpu.cycles += 5 + 7 + 7;   // $8557
					op_and(0x30);
					if (!cpu.a) { cpu.cycles += 5; continue; }
					break;
				}
				cpu.cycles += 23;
				mem_write(SLOT + 0x1A, (uint8_t)(mem_read(SLOT + 0x1A) + 1));
			}
			tcall(0x7F14, 0x8545);     // $8545: the click; the choice, 0-2
			cpu.cycles += 19 + 7 + 4 + 4;
			cpu.a = mem_read(SLOT + 0x1A);
			cpu.h = 3;
			op_add8(3, 0);
			cpu.l = cpu.a;
			call(0x7E3C);
			cpu.cycles += 19 + 12;
			mem_write(SLOT + 0x1A, cpu.h);
		}
		tcall(0x7F17, 0x855B);         // chosen
		cpu.cycles += 19 + 7 + 7;
		cpu.a = mem_read(SLOT + 0x1A);
		op_sub8(2, 0, 0);
		if (cpu.f & FC) break;
		yield_at(0x8565);              // "Return to the view"
		tcall(0x7E42, 0x8568);
		cpu.cycles += 10;
	}
	cpu.cycles += 5;                   // $856E: Journey Onward (mode 5) or a new game (6)
	tcall(0x0605, 0x856E);
	cpu.cycles += 10;
	SET_BC(0x8201);
	tcall(0x0A22, 0x8574);
	tcall(0x0A82, 0x8577);
	cpu.cycles += 10 + 10 + 10 + 10;
	SET_HL(0xCA00); SET_DE(0xCA01); SET_BC(0x037F);
	mem_write(0xCA00, 0);
	ldir();
	cpu.cycles += 10 + 10 + 10;
	SET_HL(0xCA00); SET_DE(0x3800); SET_BC(0x0380);
	tcall(0x0463, 0x8590);
	tcall(0x041C, 0x8593);
	cpu.cycles += 19 + 7 + 13 + 10;
	cpu.a = mem_read(SLOT + 0x1A);
	op_add8(5, 0);
	mem_write(GAME_MODE, cpu.a);
}
Native native_title = { "title", 0x8305, 3, title, 0, 0, 0, .task = 1 };

// $87AD: the title's second task (started at $84AE): the two figures at the
// sides come down into place (+$0D, a row a turn, to $18; a button puts them
// there at once), then stay, their frames cycling (+$1A, 0-$37) through the
// tables at $889D, $88D5 and $890D. It runs until the title ends.

static uint8_t table_byte(uint16_t t) {   // ld a,(ix+$1A) / ld hl,t / rst $28
	cpu.cycles += 19 + 10 + 48;
	SET_HL((uint16_t)(t + mem_read(SLOT + 0x1A)));
	cpu.a = mem_read(HL);
	return cpu.a;
}

static void figures(uint16_t site) {   // call $883D: both figures' sprites at row +$0D
	cpu.cycles += 17 + 19 + 7 + 10;
	cpu.d = mem_read(SLOT + 0x0D);
	cpu.e = 0x08;
	SET_HL(0x8853);
	tcall(0x09C7, 0x8845);
	cpu.cycles += 19 + 7 + 10 + 10;
	cpu.d = mem_read(SLOT + 0x0D);
	cpu.e = 0xD8;
	SET_HL(0x8878);
	task_call(0x09C7, (uint16_t)(site + 3));   // jp $09C7
}

static void title_figures(void) {
	for (;;) {
		table_byte(0x889D);
		cpu.cycles += 10;
		SET_DE(0x2480);
		tcall(0x7EE7, 0x87B7);
		figures(0x87BA);
		yield_at(0x87BD);
		table_byte(0x88D5);
		cpu.cycles += 10;
		SET_DE(0x2600);
		tcall(0x7EE7, 0x87CA);
		figures(0x87CD);
		yield_at(0x87D0);
		uint8_t t = table_byte(0x890D);
		cpu.cycles += 4 + 7;
		if (t != 0xFF) {               // a frame's pixels: $AC88 + t * $80
			cpu.cycles += 4 + 4 + 7 + 8 + 8 + 10 + 11 + 10 + 10 + 7;
			cpu.a = t;
			SET_HL(op_add16((uint16_t)(t << 7), 0xAC88));
			SET_DE(0x2780);
			SET_BC(0x0080);
			cpu.a = 0x14;
			tcall(0x07F4, 0x87F1);
		} else {
			cpu.cycles += 5;
			op_inc8(t);
			cpu.a = 0;
		}
		figures(0x87F4);
		yield_at(0x87F7);
		figures(0x87FA);
		yield_at(0x87FD);
		cpu.cycles += 23 + 19 + 7 + 7;
		uint8_t f = (uint8_t)(mem_read(SLOT + 0x1A) + 1);
		mem_write(SLOT + 0x1A, f);
		cpu.a = f;
		op_sub8(0x38, 0, 0);
		if (f >= 0x38) { cpu.cycles += 19; mem_write(SLOT + 0x1A, 0); }
		else cpu.cycles += 5;
		cpu.cycles += 20 + 7;          // $880E
		if (mem_read(SLOT + 6) & 1) { cpu.cycles += 5; continue; }
		cpu.cycles += 23 + 19 + 7 + 7;
		uint8_t row = (uint8_t)(mem_read(SLOT + 0x0D) + 1);
		mem_write(SLOT + 0x0D, row);
		cpu.a = row;
		op_sub8(0x18, 0, 0);
		if (row < 0x18) { cpu.cycles += 5; continue; }
		cpu.cycles += 10 + 10 + 10;    // in place: the rest of them drawn
		SET_HL(0x8945); SET_DE(0x3872); SET_BC(0x0004);
		tcall(0x024C, 0x8827);
		cpu.cycles += 10 + 10 + 10;
		SET_HL(0x8949); SET_DE(0x38B2); SET_BC(0x0004);
		tcall(0x024C, 0x8833);
		cpu.cycles += 23 + 10;
		mem_write(SLOT + 6, mem_read(SLOT + 6) | 1);
	}
}
Native native_title_figures = { "title_figures", 0x87AD, 3, title_figures, 0, 0, 0, .task = 1 };

// ------------------------------------------------------- the interlude --
// $894D (game mode 4, after the demo): "In another world, in a time to
// come." on the world map, which ripples in ($89A4) and out again, for ten
// seconds or until a button ($8A61: back to the title); then the opening
// again (mode 1).

static void start_task(uint16_t entry) {   // ld de,entry / call $0B33: IY = its slot
	cpu.cycles += 10;
	SET_DE(entry);
	call(0x0B33);
}

static void interlude(void) {
	start_task(0x8A61);
	cpu.cycles += 10 + 10;
	SET_HL(0x9C38);
	SET_BC(0x0303);
	tcall(0x0A22, 0x8959);             // fade in
	start_task(0x89A4);                // the ripple, in
	cpu.cycles += 19 + 19 + 19;
	mem_write(cpu.iy + 0x1A, 4);
	mem_write(cpu.iy + 0x1B, 0xFF);
	mem_write(SLOT + 0x16, 10);
	for (;;) {
		cpu.cycles += 19;
		mem_write(SLOT + 0x17, 60);
		for (;;) {
			yield_at(0x8972);
			cpu.cycles += 23 + 7;
			uint8_t n = op_dec8(mem_read(SLOT + 0x17));
			mem_write(SLOT + 0x17, n);
			if (!n) break;
			cpu.cycles += 5;
		}
		cpu.cycles += 23 + 7;
		uint8_t n = op_dec8(mem_read(SLOT + 0x16));
		mem_write(SLOT + 0x16, n);
		if (!n) break;
		cpu.cycles += 5;
	}
	start_task(0x89A4);                // and out
	cpu.cycles += 19 + 19 + 7 + 17 + 19 + 58;
	mem_write(cpu.iy + 0x1A, 0);
	mem_write(cpu.iy + 0x1B, 1);
	cpu.a = 0x32;
	mem_write(SLOT + 1, 0x32);         // call $0B16: sleep 50 turns
	SET_HL(0x8992);
	task_wait_turn(0x8992);
	tcall(0x0605, 0x8992);
	cpu.cycles += 10;
	SET_BC(0x8303);
	tcall(0x0A22, 0x8998);             // fade out
	tcall(0x0A82, 0x899B);
	cpu.cycles += 7 + 13 + 10;
	cpu.a = 1;
	mem_write(GAME_MODE, 1);
}
Native native_interlude = { "interlude", 0x894D, 3, interlude, 0, 0, 0, .task = 1 };

static void clear_lines(void) {        // call $8A03: the ripple's offsets ($D000-$D0BF), none
	cpu.cycles += 17 + 10 + 10 + 10 + 10 + 10;
	SET_HL(0xD000); SET_DE(0xD001); SET_BC(0x00BF);
	mem_write(0xD000, 0);
	ldir();
}

// $89A4: the ripple: line offsets from the table at $8A11 (+$1A: which
// row of it, stepping by +$1B five times), 16 turns each
static void ripple(void) {
	clear_lines();
	cpu.cycles += 10;
	SET_BC(0x000A);
	tcall(0x01A9, 0x89AA);
	tcall(0x0201, 0x89AD);
	cpu.cycles += 19;
	mem_write(SLOT + 0x16, 5);
	for (;;) {
		cpu.cycles += 19 + 19;         // $89B4
		mem_write(SLOT + 0x1C, 0x18);
		mem_write(SLOT + 0x17, 0x10);
		for (;;) {
			cpu.cycles += 19 + 4 * 4 + 4 + 7 + 10 + 11 + 19 + 7 + 7;
			uint8_t a = (uint8_t)(mem_read(SLOT + 0x1A) << 4);
			SET_DE(a);
			SET_HL(op_add16(0x8A11, a));
			cpu.e = mem_read(SLOT + 0x1C);
			cpu.d = 0xD0;
			cpu.a = 8;
			for (;;) {
				push(HL);
				cpu.cycles += 10;
				SET_BC(0x0010);
				ldir();
				SET_HL(pop());
				cpu.cycles += 4 + 7;
				cpu.a = op_dec8(cpu.a);
				if (!cpu.a) break;
				cpu.cycles += 5;
			}
			tcall(0x01F1, 0x89DB);
			yield_at(0x89DE);
			cpu.cycles += 23 + 23 + 7;
			mem_write(SLOT + 0x1C, (uint8_t)(mem_read(SLOT + 0x1C) + 1));
			uint8_t n = op_dec8(mem_read(SLOT + 0x17));
			mem_write(SLOT + 0x17, n);
			if (!n) break;
			cpu.cycles += 5;
		}
		cpu.cycles += 19 + 19 + 19 + 23 + 7;
		cpu.a = mem_read(SLOT + 0x1A);
		op_add8(mem_read(SLOT + 0x1B), 0);
		mem_write(SLOT + 0x1A, cpu.a);
		uint8_t n = op_dec8(mem_read(SLOT + 0x16));
		mem_write(SLOT + 0x16, n);
		if (!n) break;
		cpu.cycles += 5;
	}
	clear_lines();
	yield_at(0x89FA);
	tcall(0x01E3, 0x89FD);
	cpu.cycles += 10;                  // jp end_task
	native_continue_at(0x0B5A);
}
Native native_ripple = { "ripple", 0x89A4, 3, ripple, 0, 0, 0, .task = 1 };

// $8A61: each turn: a button goes back to the title
static void interlude_button(void) {
	cpu.cycles += 13 + 7;
	cpu.a = mem_read(JOY_PRESSED);
	op_and(0x30);
	if (!cpu.a) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 7 + 13 + 10;
	cpu.a = 3;
	mem_write(GAME_MODE, 3);
}
Native native_interlude_button = { "interlude_button", 0x8A61, 3, interlude_button, 0, 0, 0, .task = 1 };

// ----------------------------------------------------------- a new game --
// $8EBD (game mode 6): the Avatar's name, picked letter by letter from a
// grid (13 x 5 letters, $928F; "End" at the bottom right), then he or she
// ($C622: $1E or $1F); then the story (mode 7). The name "SOUND" opens the
// sound test (mode 9) instead.

#define NAME        0xC614          // its length, then up to 12 letters
#define SEX         0xC622
#define COLUMN      (SLOT + 0x0A)
#define ROW         (SLOT + 0x0C)
#define ON_END      (SLOT + 0x07)   // 1: the hand is on "End" (or on the second choice)

// call $9104 at site: C steps of the palette at $85BE, from D by E, a turn each
static void fade_steps(uint8_t n, uint8_t from, uint8_t by, uint16_t site) {
	cpu.cycles += 7 + 10;              // ld c / ld de
	cpu.c = n; cpu.d = from; cpu.e = by;
	cpu.cycles += 17 + 19 + 19 + 19;
	mem_write(SLOT + 0x16, n);
	mem_write(SLOT + 0x1A, from);
	mem_write(SLOT + 0x1B, by);
	cpu.cycles += 17 + 10 + 19 + 19 + 19 + 10;   // call $0AA5: its return kept in the slot
	wr16(SLOT + 0x24, (uint16_t)(site + 3));
	SET_HL((uint16_t)(site + 3));
	for (;;) {
		cpu.cycles += 19 + 7 + 10;
		cpu.c = mem_read(SLOT + 0x1A);
		cpu.a = 0x15;
		SET_HL(0x85BE);
		tcall(0x0807, 0x9118);
		cpu.cycles += 7;
		cpu.a = 0x15;
		tcall(0x7F77, 0x911D);
		yield_at(0x9120);
		cpu.cycles += 19 + 19 + 19 + 23 + 7;
		cpu.a = mem_read(SLOT + 0x1A);
		op_add8(mem_read(SLOT + 0x1B), 0);
		mem_write(SLOT + 0x1A, cpu.a);
		uint8_t k = op_dec8(mem_read(SLOT + 0x16));
		mem_write(SLOT + 0x16, k);
		if (!k) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 10 + 19 + 19 + 4;    // jp $0AAE: back there
	SET_HL(rd16(SLOT + 0x24));
}

static uint16_t rst30(uint16_t hl, uint8_t a) {   // rst $30: HL += A
	uint16_t r = (uint16_t)(hl + a);
	cpu.cycles += 11 + 4 + 4 + ((uint8_t)(hl + a) < a ? 5 + 4 + 10 : 11);
	SET_HL(r);
	cpu.a = (uint8_t)r;
	return r;
}

static void hand_colour(void) {        // call $9134: the hand's colours cycle (+$1A)
	cpu.cycles += 17 + 19 + 4 + 7 + 10;
	cpu.a = mem_read(SLOT + 0x1A);
	op_rrca();
	op_and(7);
	rst30(0x914C, cpu.a);
	cpu.cycles += 10 + 10;
	SET_DE(0xC03B); SET_BC(0x0008);
	ldir();
	cpu.cycles += 7 + 13 + 10;
	cpu.a = 0xFF;
	mem_write(0xC009, 0xFF);
}

// "call switch_on_a" with the table at t (n entries) through its vector, from
// `back`: the index of the key that matched, or -1
static int switch_on(uint16_t t, int n, uint16_t back) {
	cpu.cycles += 17 + 10;
	uint8_t a = cpu.a;
	for (int i = 0; i < n; i++)
		if (mem_read(t + 3 * i) == a) {
			cpu.cycles += 58 * i + 79;
			cpu.b = (uint8_t)(a + 1);
			SET_HL(rd16(t + 3 * i + 1));
			cpu.a = cpu.l;
			cpu.f = (uint8_t)back;
			return i;
		}
	cpu.cycles += 58 * n + 22;
	cpu.b = 0;
	SET_HL(t + 3 * n);
	op_inc8(0xFF);
	return -1;
}

static void new_game(void) {
	cpu.cycles += 7;
	cpu.a = 4;
	call(0x0621);                      // the music
	cpu.cycles += 10 + 10 + 10 + 10;
	SET_HL(0xC500); SET_DE(0xC501); SET_BC(0x00FF);
	mem_write(0xC500, 0);
	ldir();
	cpu.cycles += 10 + 7 * 10 + 6 * 4;
	for (int i = 0; i < 7; i++) mem_write(0xC5E0 + i, (uint8_t)(i + 1));
	SET_HL(0xC5E6);
	cpu.cycles += 10 + 10 + 10 + 10;
	SET_HL(0xC651); SET_DE(0xC652); SET_BC(0x036B);
	mem_write(0xC651, 0);
	ldir();
	cpu.cycles += 7 + 13 + 7 + 13;
	mem_write(0xC0CD, 0xFF);
	mem_write(0xC0CE, 0x20);
	cpu.a = 0x20;
	cpu.cycles += 10 + 10;
	SET_HL(NAME); SET_BC(0x0E00);
	tcall(0x7E57, 0x8F03);
	cpu.cycles += 10 + 10;
	SET_HL(NAME + 1); SET_BC(0x0C20);
	tcall(0x7E57, 0x8F0C);
	cpu.cycles += 7 + 13 + 10;
	cpu.a = 0x1E;
	mem_write(SEX, 0x1E);
	SET_HL(0x9C78);
	tcall(0x083B, 0x8F17);
	tcall(0x8A97, 0x8F1A);
	cpu.cycles += 10;
	SET_HL(0x915B);
	tcall(0x7E4E, 0x8F20);
	cpu.cycles += 10;
	SET_HL(0x9191);
	tcall(0x7E4E, 0x8F26);
	fade_steps(6, 0x01, 0x01, 0x8F2E);
	cpu.cycles += 19 * 3 + 10;
	mem_write(SLOT + 0x1A, 0);
	mem_write(COLUMN, 0);
	mem_write(ROW, 0);
	SET_HL(0x201C);
	tcall(0x06DA, 0x8F40);
	for (;;) {                         // $8F43: the name so far, the hand on a letter
		cpu.cycles += 13 + 7 + 4 + 7 + 16;
		cpu.a = mem_read(NAME);
		op_add8(0x0B, 0);
		cpu.l = cpu.a; cpu.h = 0x15;
		wr16(0xC639, HL);
		tcall(0x7F5C, 0x8F4E);
		cpu.cycles += 10 + 10;
		SET_HL(NAME + 1); SET_DE(0x150B);
		tcall(0x7EB4, 0x8F57);
		cpu.cycles += 19 + 19 + 7 + 4 + 7 + 7;
		cpu.e = mem_read(COLUMN);
		cpu.d = mem_read(ROW);
		cpu.b = 0;
		cpu.a = cpu.d;
		op_sub8(4, 0, 0);
		int end = 0;
		if (cpu.d >= 4) {
			cpu.cycles += 4 + 7 + 7;
			cpu.a = cpu.e;
			op_sub8(0x0B, 0, 0);
			if (cpu.e >= 0x0B) end = 1; else cpu.cycles += 5;
		} else cpu.cycles += 5;
		if (end) {                     // on "End"
			cpu.cycles += 4 + 7 + 10 + 23 + 12;
			cpu.b = 1;
			cpu.e = 0x0B;
			SET_HL(0x9208);
			mem_write(SLOT + 0x1A, (uint8_t)(mem_read(SLOT + 0x1A) + 1));
		} else {
			cpu.cycles += 10 + 23;
			SET_HL(0x91EF);
			mem_write(SLOT + 0x1A, (uint8_t)(mem_read(SLOT + 0x1A) - 1));
		}
		cpu.cycles += 19 + 4 + 4 * 4 + 7 + 4 + 4 + 4 * 4 + 7 + 4;
		mem_write(ON_END, cpu.b);
		cpu.a = (uint8_t)(cpu.e * 16);
		op_add8(0x20, 0);
		cpu.e = cpu.a;
		cpu.a = (uint8_t)(cpu.d * 16);
		op_add8(0x10, 0);
		cpu.d = cpu.a;
		tcall(0x09C7, 0x8F90);         // the hand
		hand_colour();
		yield_at(0x8F96);
		cpu.cycles += 13 + 7 + 10;
		cpu.a = mem_read(JOY_PRESSED);
		op_and(0x3F);
		int k = switch_on(0x8FA6, 6, 0x8FA4);
		switch (k) {
		case 0:                        // up
			cpu.cycles += 23 + 10;
			{
				uint8_t r = op_dec8(mem_read(ROW));
				mem_write(ROW, r);
				if (r & 0x80) { cpu.cycles += 19; mem_write(ROW, 4); }
			}
			break;
		case 1:                        // down
			cpu.cycles += 23 + 19 + 7 + 7;
			{
				uint8_t r = (uint8_t)(mem_read(ROW) + 1);
				mem_write(ROW, r);
				cpu.a = r;
				op_sub8(5, 0, 0);
				if (r < 5) cpu.cycles += 5;
				else { cpu.cycles += 19 + 12; mem_write(ROW, 0); }
			}
			break;
		case 2:                        // left ("End": to the letters)
			cpu.cycles += 20 + 7;
			if (mem_read(ON_END) & 1) { cpu.cycles += 19 + 12; mem_write(COLUMN, 0x0A); }
			else {
				cpu.cycles += 5 + 23 + 10;
				uint8_t c = op_dec8(mem_read(COLUMN));
				mem_write(COLUMN, c);
				if (c & 0x80) { cpu.cycles += 19 + 12; mem_write(COLUMN, 0x0C); }
			}
			break;
		case 3:                        // right
			cpu.cycles += 20 + 7;
			if (mem_read(ON_END) & 1) { cpu.cycles += 5 + 19 + 12; mem_write(COLUMN, 0); }
			else {
				cpu.cycles += 23 + 19 + 7 + 7;
				uint8_t c = (uint8_t)(mem_read(COLUMN) + 1);
				mem_write(COLUMN, c);
				cpu.a = c;
				op_sub8(0x0D, 0, 0);
				if (c < 0x0D) cpu.cycles += 5;
				else { cpu.cycles += 19 + 12; mem_write(COLUMN, 0); }
			}
			break;
		case 4:                        // button 1: the last letter off
			tcall(0x7F11, 0x9007);
			cpu.cycles += 10 + 7 + 4 + 7;
			SET_HL(NAME);
			cpu.a = mem_read(NAME);
			op_or(cpu.a);
			if (cpu.a) {
				cpu.cycles += 11;
				mem_write(NAME, op_dec8(cpu.a));
				rst30(NAME, cpu.a);
				cpu.cycles += 10;
				mem_write(HL, ' ');
			} else cpu.cycles += 5;
			cpu.cycles += 10;
			continue;
		case 5:                        // button 2: the letter added, or "End"
			tcall(0x7F17, 0x9018);
			cpu.cycles += 20 + 7;
			if (mem_read(ON_END) & 1) goto done;
			cpu.cycles += 10 + 7 + 7 + 7;
			SET_HL(NAME);
			cpu.a = mem_read(NAME);
			op_sub8(0x0C, 0, 0);
			if (cpu.a < 0x0C) {
				cpu.cycles += 11 + 4;
				uint8_t len = (uint8_t)(cpu.a + 1);
				mem_write(NAME, len);
				cpu.a = op_inc8(cpu.a);
				rst30(NAME, len);
				cpu.cycles += 4 + 19 + 7;
				SET_DE(HL);
				cpu.l = mem_read(ROW);
				cpu.h = 0x0D;
				call(0x7EC3);          // HL = row * 13
				cpu.cycles += 19 + 4 + 10 + 48 + 7;
				cpu.a = mem_read(COLUMN);
				op_add8(cpu.l, 0);
				SET_HL((uint16_t)(0x928F + cpu.a));
				cpu.a = mem_read(HL);
				mem_write(DE, cpu.a);
			} else cpu.cycles += 5;
			cpu.cycles += 10;
			continue;
		default:
			cpu.cycles += 12;          // jr $8F43
			continue;
		}
		tcall(0x7F14, 0x8FC3);         // $8FC3: the click
		cpu.cycles += 10;
		continue;
	done:
		cpu.cycles += 5 + 13 + 4 + 10;    // $9041
		cpu.a = mem_read(NAME);
		op_or(cpu.a);
		if (!cpu.a) continue;          // jp z,$8F43
		break;
	}
	tcall(0x06D4, 0x9048);
	fade_steps(6, 0x05, 0xFF, 0x9050);
	cpu.cycles += 10 + 10 + 7;
	SET_HL(NAME); SET_DE(0x92CE);
	cpu.b = 6;
	int sound = 1;
	for (;;) {                         // "SOUND"?
		cpu.cycles += 7 + 7 + 7;
		cpu.a = mem_read(DE);
		op_sub8(mem_read(HL), 0, 0);
		if (!(cpu.f & FZ)) { cpu.cycles += 5; sound = 0; break; }
		cpu.cycles += 6 + 6;
		SET_HL(HL + 1); SET_DE(DE + 1);
		if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
	}
	if (sound) {
		cpu.cycles += 10;
		SET_BC(0x8301);
		tcall(0x0A22, 0x9066);
		tcall(0x0A82, 0x9069);
		cpu.cycles += 7 + 13 + 10;
		cpu.a = 9;
		mem_write(GAME_MODE, 9);
		return;
	}
	tcall(0x8A97, 0x9072);             // $9072: he or she
	cpu.cycles += 10;
	SET_HL(0x91B9);
	tcall(0x7E4E, 0x9078);
	fade_steps(3, 0x08, 0x01, 0x9080);
	for (;;) {
		cpu.cycles += 10 + 16;         // $9083
		SET_HL(0x140E);
		wr16(0xC639, HL);
		tcall(0x7F5C, 0x9089);
		cpu.cycles += 13 + 7 + 7 + 7;
		cpu.a = mem_read(SEX);
		cpu.b = 0;
		op_sub8(0x1E, 0, 0);
		if (cpu.a == 0x1E) {
			cpu.cycles += 23 + 10 + 10 + 12;
			mem_write(SLOT + 0x1A, (uint8_t)(mem_read(SLOT + 0x1A) - 1));
			SET_HL(0x922D); SET_DE(0x2870);
		} else {
			cpu.cycles += 5 + 4 + 23 + 10 + 10;
			cpu.b = 1;
			mem_write(SLOT + 0x1A, (uint8_t)(mem_read(SLOT + 0x1A) + 1));
			SET_HL(0x9258); SET_DE(0x3868);
		}
		cpu.cycles += 19;
		mem_write(ON_END, cpu.b);
		tcall(0x09C7, 0x90AD);
		hand_colour();
		yield_at(0x90B3);
		cpu.cycles += 13 + 4 + 7 + 7;
		uint8_t joy = mem_read(JOY_PRESSED);
		cpu.b = joy;
		cpu.a = joy;
		op_and(0x03);
		if (cpu.a) {                   // up or down: the other one
			cpu.cycles += 20 + 7 + 7;
			if (mem_read(ON_END) & 1) { cpu.cycles += 5; cpu.a = 0x1E; }
			else { cpu.cycles += 7; cpu.a = 0x1F; }
			cpu.cycles += 13;
			mem_write(SEX, cpu.a);
			tcall(0x7F14, 0x90CB);
			cpu.cycles += 12;
			continue;
		}
		cpu.cycles += 5 + 4 + 7 + 7;   // $90D0
		cpu.a = joy;
		op_and(0x30);
		if (!cpu.a) { cpu.cycles += 5; continue; }
		break;
	}
	tcall(0x7F17, 0x90D5);
	cpu.cycles += 20 + 10 + 7;
	if (mem_read(ON_END) & 1) { cpu.cycles += 10; SET_HL(0x91E8); }
	else { cpu.cycles += 5; SET_HL(0x91E3); }
	cpu.cycles += 10;
	SET_DE(0x140E);
	tcall(0x7EB4, 0x90E7);             // the choice, shown
	fade_steps(3, 0x09, 0xFF, 0x90EF);
	tcall(0x0605, 0x90F2);
	cpu.cycles += 10;
	SET_BC(0x8301);
	tcall(0x0A22, 0x90F8);
	tcall(0x0A82, 0x90FB);
	cpu.cycles += 7 + 13 + 10;
	cpu.a = 7;
	mem_write(GAME_MODE, 7);
}
Native native_new_game = { "new_game", 0x8EBD, 3, new_game, 0, 0, 0, .task = 1 };
