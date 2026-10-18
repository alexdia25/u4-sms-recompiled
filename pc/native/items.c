// Using the special things (bank 1 $651E, from Use): the stones, the bell,
// book and candle, the three-part key, the horn, the wheel and the skull.
// A menu of those the party has; each works in one place only: the stones
// at the Abyss's altars (the last one ends the game) and in the dungeons'
// altar rooms (the three parts of the key), the bell, book and candle in
// turn at the Abyss's entrance, the skull there or anywhere else. Called
// from Use's C task. Cycle counts are the original's.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define SLOT        cpu.ix
#define LIST        0xC63D          // the menu's entries: item numbers
#define COUNT       (SLOT + 0x1B)   // how many
#define STONES      0xC955          // a bit for each of the eight
#define RUNES       0xC951          // the key's parts (bits 5-7), and more
#define THINGS      0xC952          // bell, book, candle placed (bits 4, 3, 2), horn, wheel
#define LOCATION    0xC9A0
#define LEVEL       0xC970

static void call(uint16_t addr) { cpu.cycles += 17; call_original(addr); }

static void tcall(uint16_t addr, uint16_t site) {   // a routine that may wait for frames
	cpu.cycles += 17;
	task_call(addr, (uint16_t)(site + 3));
}

static void message(uint8_t n) { cpu.a = n; call(0x5582); }   // call print_message
static void newline(void) { cpu.cycles += 17; text_newline(); }

static uint8_t rst28(uint16_t t, uint8_t a) {   // ld hl,t / rst $28: A = t[A]
	cpu.cycles += 10 + 48;
	SET_HL((uint16_t)(t + a));
	cpu.a = mem_read(HL);
	return cpu.a;
}

static uint16_t rst20(uint16_t t, uint8_t a) {  // HL = t / rst $20: HL = the word t[A]
	cpu.cycles += a < 0x80 ? 91 : 90;
	SET_HL(rd16((uint16_t)(t + 2 * a)));
	cpu.a = cpu.l;
	return HL;
}

static void rst30(uint16_t hl, uint8_t a) {     // rst $30: HL += A
	uint16_t r = (uint16_t)(hl + a);
	cpu.cycles += 11 + 4 + 4 + ((uint8_t)(hl + a) < a ? 5 + 4 + 10 : 11);
	SET_HL(r);
	cpu.a = (uint8_t)r;
}

static void add_item(uint8_t n) {      // call $6838: n into the list
	cpu.cycles += 17 + 7 + 6 + 4 + 10;
	cpu.a = n;
	mem_write(HL, n);
	SET_HL(HL + 1);
	cpu.b++;
}

static int one_bit(uint8_t a) {        // call $683C: Z if A has just one bit set (cpir in $6849)
	cpu.cycles += 17 + 11 + 11 + 10 + 10 + 10 + 10 + 10;
	int i;
	for (i = 0; i < 8 && mem_read(0x6849 + i) != a; i++) {}
	int found = i < 8;
	cpu.cycles += found ? 21 * i + 16 : 21 * 7 + 16;
	op_sub8(found ? a : mem_read(0x6850), 0, 0);
	cpu.f = (uint8_t)((cpu.f & ~(FZ | FP)) | (found ? FZ : 0) | (found && i < 7 ? FP : 0));
	return found;
}

static void flash(void) {              // call $681F: a flash and its sound
	cpu.cycles += 17 + 7;
	cpu.a = 0x1C;
	call(0x05F1);
	cpu.cycles += 10;                  // jp $4117
	call_original(0x4117);
}

static void flashes(void) {            // call $6813
	cpu.cycles += 17;
	flash();
	call(0x41EC);
	flash();
	call(0x41EC);
	cpu.cycles -= 17;                  // the last one is fallen into, not called
	flash();
}

// call $6827: Z at the Abyss's entrance (out in the world, at $E9, $E9)
static int at_abyss(void) {
	cpu.cycles += 17 + 13 + 4;
	cpu.a = mem_read(LOCATION);
	op_or(cpu.a);
	if (cpu.a) { cpu.cycles += 11; return 0; }
	cpu.cycles += 5 + 13 + 7;
	cpu.a = mem_read(0xC953);
	op_sub8(0xE9, 0, 0);
	if (cpu.a != 0xE9) { cpu.cycles += 11; return 0; }
	cpu.cycles += 5 + 13 + 7 + 10;
	cpu.a = mem_read(0xC954);
	op_sub8(0xE9, 0, 0);
	return cpu.a == 0xE9;
}

static void shuffle(uint8_t n) {       // call $68D7: the first n entries of the list shuffled
	cpu.cycles += 17 + 11 + 11 + 11 + 10 + 4 + 4;
	push16(BC); push16(DE); push16(HL);
	SET_DE(LIST);
	cpu.b = n; cpu.c = n;
	do {
		cpu.cycles += 11 + 4;
		push16(BC);
		cpu.a = cpu.c;
		call(0x2BEB);                  // random below C
		cpu.cycles += 10;
		rst30(LIST, cpu.a);
		cpu.cycles += 7 + 7 + 7 + 4 + 7 + 10 + 6;
		uint8_t t = mem_read(HL);
		mem_write(HL, mem_read(DE));
		mem_write(DE, t);
		cpu.a = t;
		SET_BC(pop16());
		SET_DE(DE + 1);
		if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
	} while (1);
	cpu.cycles += 10 + 10 + 10 + 10;
	SET_HL(pop16()); SET_DE(pop16()); SET_BC(pop16());
}

static void stones_list(void) {        // call $68B8: the stones the party has, shuffled
	cpu.cycles += 17 + 10 + 10 + 7 + 13;
	SET_HL(LIST); SET_DE(0);
	cpu.b = 8;
	cpu.a = mem_read(STONES);
	for (;;) {
		cpu.cycles += 4 + 7;
		op_rrca();
		if (cpu.f & FC) {
			cpu.cycles += 7 + 6 + 4;
			mem_write(HL, cpu.e);
			SET_HL(HL + 1);
			cpu.d++;
		} else cpu.cycles += 5;
		cpu.cycles += 4;
		cpu.e++;
		if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
	}
	cpu.cycles += 19 + 4;
	mem_write(COUNT, cpu.d);
	cpu.a = cpu.d;
	shuffle(cpu.d);
	cpu.cycles += 10 + 10;
	SET_HL(0x6998);
}

// call $6851 at site (or $6855, `keep`: +$1A as it is): a menu of the list's
// entries named by the table at HL; carry if cancelled, else A = the entry
static int pick(uint16_t names, int keep, uint16_t site) {
	cpu.cycles += 17;
	if (!keep) { cpu.cycles += 19; mem_write(SLOT + 0x1A, 0); }
	cpu.cycles += 19 + 19;
	mem_write(SLOT + 0x1C, (uint8_t)names);
	mem_write(SLOT + 0x1D, (uint8_t)(names >> 8));
	cpu.cycles += 17 + 10 + 19 + 19 + 19 + 10;   // call $0AC5: its return kept in the slot
	wr16(SLOT + 0x28, (uint16_t)(site + 3));
	SET_HL((uint16_t)(site + 3));
	tcall(0x2E1B, 0x685E);             // the window cleared
	cpu.cycles += 19 + 7 + 10;
	cpu.b = mem_read(COUNT);
	cpu.c = 0;
	SET_DE(0xCAA6);
	for (;;) {                         // the names, a line each
		cpu.cycles += 11 + 11 + 4;
		push16(BC); push16(DE);
		rst28(LIST, cpu.c);
		cpu.cycles += 19 + 19;
		rst20(names, cpu.a);
		tcall(0x2D6C, 0x6877);
		cpu.cycles += 10 + 10 + 10 + 11 + 4 + 4;
		SET_DE(pop16()); SET_BC(pop16());
		uint16_t at = DE;
		SET_DE(op_add16(0x0080, at)); SET_HL(at);
		cpu.c++;
		if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
	}
	tcall(0x2E54, 0x6884);             // shown
	for (;;) {
		task_frame_tick(0x6887);
		cpu.cycles += 19 + 10;
		cpu.c = mem_read(COUNT);
		SET_DE(0x0102);
		call(0x49F9);                  // menu_step_wrap: Z nothing, carry cancelled
		cpu.cycles += 7;
		if (cpu.f & FZ) { cpu.cycles += 5; continue; }
		break;
	}
	cpu.cycles += 7;
	int r;
	if (cpu.f & FC) {
		call(0x472D);
		cpu.cycles += 4;
		op_scf();
		r = -1;
	} else {
		cpu.cycles += 5 + 19 + 11;
		uint8_t a = rst28(LIST, mem_read(SLOT + 0x1A));
		push16((uint16_t)(a << 8 | cpu.f));
		cpu.cycles += 19 + 19;
		rst20(rd16(SLOT + 0x1C), a);
		call(0x5519);                  // its name
		newline();
		cpu.cycles += 10 + 4;
		uint16_t af = pop16();
		cpu.a = (uint8_t)(af >> 8); cpu.f = (uint8_t)af;
		op_or(cpu.a);
		r = cpu.a;
	}
	cpu.cycles += 10 + 19 + 19 + 4;    // jp $0ACE
	SET_HL(rd16(SLOT + 0x28));
	return r;
}

enum { DONE, NO_EFFECT, SAY, SAY_A };   // how a use ends: $65CA, $65C0, $65C4, $65C1

// the stones at one of the Abyss's altars ($65DE): the colour asked for,
// then the stone; at the last altar, the end of the game
static int abyss_altar(void) {
	cpu.cycles += 13 + 7 + 10;
	cpu.a = mem_read(LOCATION);
	op_sub8(0x19, 0, 0);
	if (cpu.a != 0x19) return NO_EFFECT;
	cpu.cycles += 13 + 7 + 10;
	cpu.a = mem_read(0xC990);
	op_sub8(0xB0, 0, 0);
	if (cpu.a != 0xB0) return NO_EFFECT;
	cpu.cycles += 13 + 7 + 7;
	uint8_t level = mem_read(LEVEL);
	cpu.a = level;
	op_sub8(7, 0, 0);
	cpu.a = 0x0E;
	if (level != 7) message(0x0E); else cpu.cycles += 10;
	cpu.cycles += 13 + 7;
	cpu.a = mem_read(LEVEL);
	op_add8(5, 0);
	message(cpu.a);
	cpu.cycles += 7;
	message(0x0F);
	newline();
	cpu.cycles += 10 + 10;             // the eight colours, shuffled
	SET_HL(LIST); SET_BC(0x0800);
	do {
		cpu.cycles += 7 + 6 + 4;
		mem_write(HL, cpu.c);
		SET_HL(HL + 1);
		cpu.c++;
		if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
	} while (1);
	cpu.cycles += 7;
	cpu.a = 8;
	shuffle(8);
	cpu.cycles += 19 + 10;
	mem_write(COUNT, 8);
	int r = pick(0x6941, 0, 0x661F);   // "What colour?"
	cpu.cycles += 10;
	if (r < 0) return DONE;
	cpu.cycles += 10 + 7 + 10;
	op_sub8(mem_read(LEVEL), 0, 0);
	if (cpu.a != mem_read(LEVEL)) return NO_EFFECT;
	cpu.cycles += 7;
	message(0x10);                     // "Use which stone?"
	stones_list();
	r = pick(HL, 0, 0x6634);
	cpu.cycles += 10;
	if (r < 0) return DONE;
	cpu.cycles += 10 + 7 + 10;
	op_sub8(mem_read(LEVEL), 0, 0);
	if (cpu.a != mem_read(LEVEL)) return NO_EFFECT;
	cpu.cycles += 13 + 7 + 7;
	cpu.a = mem_read(LEVEL);
	op_sub8(7, 0, 0);
	if (mem_read(LEVEL) == 7) {        // the Codex: the end
		cpu.cycles += 5 + 7 + 13 + 10;
		cpu.a = 0x18;
		mem_write(0xC600, 0x18);
		native_continue_at(0x0CD2);
		return -1;
	}
	cpu.cycles += 16;
	SET_HL(rd16(0xC953));
	call(0x6DC0);                      // the altar's cell
	cpu.cycles += 10 + 7 + 13 + 7 + 10;
	mem_write(HL, 0x20);               // a ladder down
	mem_write(0xC9A1, 0xFF);
	cpu.a = 0x12;
	return SAY;
}

// the key's parts at an altar room's altar ($6662): Truth, Love and
// Courage in the altar's order, each stone named once
static int altar_room(void) {
	cpu.cycles += 13 + 7 + 10;
	cpu.a = mem_read(0xC984);
	op_sub8(0xFF, 0, 0);
	if (cpu.a == 0xFF) return NO_EFFECT;
	call(0x48E1);                      // IY = the party's figure
	cpu.cycles += 19 + 19 + 10;
	cpu.a = mem_read(cpu.iy + 0);
	op_sub8(mem_read(cpu.iy + 1), 0, 0);
	if (cpu.a != mem_read(cpu.iy + 1)) return NO_EFFECT;
	cpu.cycles += 7 + 10;
	op_sub8(5, 0, 0);
	if (cpu.a != 5) return NO_EFFECT;
	cpu.cycles += 7;
	message(0x02);
	newline();
	stones_list();
	cpu.cycles += 19 * 3;
	mem_write(SLOT + 0x16, 4);
	mem_write(SLOT + 0x17, 0);
	mem_write(SLOT + 0x1A, 0);
	for (;;) {
		cpu.cycles += 7 + 19;          // $6692: "A : " ... "D : "
		cpu.a = (uint8_t)(0x45 - mem_read(SLOT + 0x16));
		op_sub8(mem_read(SLOT + 0x16), 0, 0);
		cpu.a = (uint8_t)(0x45 - mem_read(SLOT + 0x16));
		call(0x548F);
		{
			uint16_t hl = HL;
			cpu.cycles += 17 + 19 + 17;
			text_print(0x669D);
			cpu.cycles += 19 + 10;
			SET_HL(hl);
		}
		cpu.cycles += 10;
		int r = pick(0x6998, 1, 0x66A4);
		cpu.cycles += 10;
		if (r < 0) return DONE;
		uint8_t bit = rst28(0x6849, (uint8_t)r);
		cpu.cycles += 4 + 19 + 7 + 10;
		cpu.b = bit;
		op_and(mem_read(SLOT + 0x17));
		uint8_t used = cpu.a;
		cpu.a = 3;
		if (used) return SAY_A;        // that one again
		cpu.cycles += 4 + 19 + 19 + 23 + 7;
		cpu.a = bit;
		op_or(mem_read(SLOT + 0x17));
		mem_write(SLOT + 0x17, cpu.a);
		uint8_t n = op_dec8(mem_read(SLOT + 0x16));
		mem_write(SLOT + 0x16, n);
		if (!n) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 13 + 19 + 10;        // the room's order?
	uint8_t want = rst28(0x66F1, mem_read(0xC984));
	op_sub8(mem_read(SLOT + 0x17), 0, 0);
	if (want != mem_read(SLOT + 0x17)) return NO_EFFECT;
	cpu.cycles += 13 + 4 + 10 + 7 + 4 + 10;
	uint8_t part = rst28(0x684E, mem_read(0xC984));
	cpu.b = part;
	SET_HL(RUNES);
	cpu.a = mem_read(RUNES);
	op_and(part);
	if (cpu.a) return NO_EFFECT;       // that part already
	cpu.cycles += 7 + 4 + 7 + 7;
	cpu.a = mem_read(RUNES);
	op_or(part);
	mem_write(RUNES, cpu.a);
	cpu.a = 4;
	message(0x04);
	task_wait_button(0x66E8);
	newline();
	cpu.cycles += 10;
	return DONE;
}

// the bell, book and candle, in turn at the Abyss's entrance
static int bell_book_candle(int which) {
	if (!at_abyss()) { cpu.cycles += 10; return NO_EFFECT; }
	cpu.cycles += 10;
	if (which > 0) {                   // the one before it first
		cpu.cycles += 13 + 7 + 10;
		cpu.a = mem_read(THINGS);
		op_and(which == 1 ? 0x10 : 0x08);
		if (!cpu.a) return NO_EFFECT;
	}
	cpu.cycles += 10 + 15 + 7;
	mem_write(THINGS, mem_read(THINGS) | (which == 0 ? 0x10 : which == 1 ? 0x08 : 0x04));
	SET_HL(THINGS);
	if (which < 2) { cpu.cycles += 10; cpu.a = (uint8_t)(0x13 + which); return SAY; }
	message(0x15);                     // the candle: the way opens
	cpu.cycles += 7;
	cpu.a = 0x1C;
	call(0x05F1);
	call(0x4117);
	cpu.cycles += 7 + 13;
	cpu.a = 0xFF;
	mem_write(0xC9A1, 0xFF);
	newline();
	cpu.cycles += 10;
	return DONE;
}

// the skull: at the Abyss's entrance it is destroyed (and the party gains
// in every virtue); anywhere else every monster about dies (and the party
// loses in every virtue)
static int skull(void) {
	int there = at_abyss();
	cpu.cycles += 7;
	if (!there) {
		cpu.cycles += 7;
		message(0x18);
		flashes();
		cpu.cycles += 14 + 10 + 13 + 4 + 7 + 7;
		cpu.iy = 0xC651;
		SET_DE(0x0008);
		cpu.a = mem_read(LOCATION);
		op_or(cpu.a);
		cpu.b = 0x20;
		if (cpu.a) cpu.cycles += 5;
		else { cpu.cycles += 7; cpu.b = 0x08; }
		for (;;) {                     // the people on the map: monsters gone
			cpu.cycles += 19 + 7 + 7;
			cpu.a = mem_read(cpu.iy + 3);
			op_sub8(0x5E, 0, 0);
			if (cpu.a == 0x5E) {
				cpu.cycles += 19 + 19 + 12;
				mem_write(cpu.iy + 6, 0xFF);
				mem_write(cpu.iy + 7, 0x00);
			} else {
				cpu.cycles += 5 + 19 + 19;
				mem_write(cpu.iy + 3, 0);
				mem_write(cpu.iy + 0, 0);
			}
			cpu.cycles += 15;
			cpu.iy += 8;
			if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
		}
		cpu.cycles += 14 + 10 + 7;
		cpu.iy = 0xC839;
		SET_DE(0x0008);
		cpu.b = 0x10;
		for (;;) {                     // the foes in a fight too
			cpu.cycles += 19 + 7 + 7;
			cpu.a = mem_read(cpu.iy + 5);
			op_sub8(0x5E, 0, 0);
			if (cpu.a == 0x5E) cpu.cycles += 5;
			else {
				cpu.cycles += 19 + 19;
				mem_write(cpu.iy + 5, 0);
				mem_write(cpu.iy + 6, 0);
			}
			cpu.cycles += 15;
			cpu.iy += 8;
			if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
		}
		tcall(0x129D, 0x67E0);
		cpu.cycles += 7 + 10;
		cpu.a = 5;
		SET_BC(0x0800);
		for (;;) {
			call(0x426F);              // karma lost, each virtue
			cpu.cycles += 4;
			cpu.c++;
			if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
		}
		cpu.cycles += 12;
	} else {
		cpu.cycles += 5 + 7;
		message(0x19);
		flashes();
		cpu.cycles += 10 + 15 + 15 + 7 + 10;
		mem_write(RUNES, (uint8_t)((mem_read(RUNES) & ~0x01) | 0x02));
		SET_HL(RUNES);
		cpu.a = 0x0A;
		SET_BC(0x0800);
		for (;;) {
			call(0x4255);              // karma gained, each virtue
			cpu.cycles += 4;
			cpu.c++;
			if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
		}
	}
	call(0x4324);                      // $680A: the status window
	newline();
	cpu.cycles += 10;
	return DONE;
}

// call $651E at site; 1 when the game has ended (original code goes on)
int use_things(uint16_t site) {
	cpu.cycles += 17 + 17 + 10 + 19 + 19 + 19 + 10;   // call $0AB5: its return kept in the slot
	wr16(SLOT + 0x26, (uint16_t)(site + 3));
	SET_HL((uint16_t)(site + 3));
	cpu.cycles += 7 + 13 + 10 + 16 + 7 + 10;
	cpu.a = 0x1A;
	mem_write(0xC0CA, 0x1A);
	wr16(0xC0CB, 0x8000);
	cpu.b = 1;
	SET_HL(LIST);
	cpu.cycles += 13 + 4 + 7;          // the list: what the party has
	cpu.a = mem_read(STONES);
	op_or(cpu.a);
	if (cpu.a) {
		int one = one_bit(cpu.a);
		cpu.cycles += 7 + 7;
		if (one) { cpu.cycles += 5; add_item(0); }
		else { cpu.cycles += 4; add_item(1); }
	} else cpu.cycles += 5;
	static const struct { uint16_t at; uint8_t mask, item; } has[3] = {
		{ RUNES, 0x04, 2 }, { RUNES, 0x08, 3 }, { RUNES, 0x10, 4 } };
	for (int i = 0; i < 3; i++) {      // the bell, book, candle
		cpu.cycles += 13 + 7 + 7;
		cpu.a = mem_read(has[i].at);
		op_and(has[i].mask);
		int got = cpu.a != 0;
		cpu.a = has[i].item;
		if (got) add_item(has[i].item); else cpu.cycles += 10;
	}
	cpu.cycles += 13 + 7 + 7;          // the key's parts
	cpu.a = mem_read(RUNES);
	op_and(0xE0);
	if (cpu.a) {
		int one = one_bit(cpu.a);
		cpu.cycles += 7 + 7;
		if (one) { cpu.cycles += 5; add_item(5); }
		else { cpu.cycles += 4; add_item(6); }
	} else cpu.cycles += 5;
	static const struct { uint16_t at; uint8_t mask, item; } has2[3] = {
		{ THINGS, 0x01, 7 }, { THINGS, 0x02, 8 }, { RUNES, 0x01, 9 } };
	for (int i = 0; i < 3; i++) {      // the horn, the wheel, the skull
		cpu.cycles += 13 + 7 + 7;
		cpu.a = mem_read(has2[i].at);
		op_and(has2[i].mask);
		int got = cpu.a != 0;
		cpu.a = has2[i].item;
		if (got) add_item(has2[i].item); else cpu.cycles += 10;
	}
	int how;
	if (!--cpu.b) {                    // none
		cpu.cycles += 8 + 7 + 12;
		cpu.a = 1;
		how = SAY_A;
	} else {
		cpu.cycles += 13 + 19 + 7;
		mem_write(COUNT, cpu.b);
		message(0x1A);                 // "Use which item?"
		cpu.cycles += 10;
		int r = pick(0x68F5, 0, 0x65A1);
		cpu.cycles += 7;
		if (r < 0) { cpu.cycles += 5; how = DONE; }
		else {
			cpu.cycles += 10 + 10 + 10 + 52;   // the table at $65AC
			SET_HL(rd16(0x65AC + 2 * r));
			cpu.a = cpu.l;
			switch (r) {
			case 0: case 1:            // the stones
				cpu.cycles += 13 + 7 + 7;
				cpu.a = mem_read(0xC99F);
				op_sub8(2, 0, 0);
				if (cpu.a == 2) { cpu.cycles += 5; how = abyss_altar(); }
				else {
					cpu.cycles += 7 + 10;
					op_sub8(5, 0, 0);
					if (cpu.a == 5) how = altar_room();
					else { cpu.cycles += 7 + 10; cpu.a = 0x0D; how = SAY_A; }
				}
				break;
			case 2: case 3: case 4:
				how = bell_book_candle(r - 2);
				break;
			case 5: case 6:            // the key: nothing on its own
				cpu.cycles += 7 + 10;
				cpu.a = 0x0D;
				how = SAY_A;
				break;
			case 7:                    // the horn: the Abyss's guardians hold off
				cpu.cycles += 13 + 4 + 10;
				cpu.a = mem_read(LOCATION);
				op_or(cpu.a);
				if (cpu.a) { how = NO_EFFECT; break; }
				cpu.cycles += 7 + 13 + 7 + 13;
				mem_write(0xC976, 1);
				mem_write(0xC977, 0x0A);
				call(0x4324);
				cpu.cycles += 7 + 10;
				cpu.a = 0x16;
				how = SAY;
				break;
			case 8:                    // the wheel: the ship's hull, on board
				cpu.cycles += 13 + 4 + 10;
				cpu.a = mem_read(LOCATION);
				op_or(cpu.a);
				if (cpu.a) { how = NO_EFFECT; break; }
				cpu.cycles += 13 + 7 + 10;
				cpu.a = mem_read(0xC959);
				op_sub8(0x14, 0, 0);
				if (!(cpu.f & FC)) { how = NO_EFFECT; break; }
				cpu.cycles += 13 + 7 + 10;
				cpu.a = mem_read(0xC961);
				op_sub8(0x32, 0, 0);
				if (cpu.a != 0x32) { how = NO_EFFECT; break; }
				cpu.cycles += 7 + 13;
				cpu.a = 0x63;
				mem_write(0xC961, 0x63);
				call(0x4324);
				cpu.cycles += 7 + 10;
				cpu.a = 0x17;
				how = SAY;
				break;
			default:
				how = skull();
				break;
			}
		}
	}
	if (how < 0) return 1;             // the end of the game
	if (how == NO_EFFECT) { cpu.cycles += 4; cpu.a = 0; op_xor(0); how = SAY_A; }   // $65C0: "No effect!"
	if (how == SAY_A) { call(0x40D1); how = SAY; }
	if (how == SAY) { call(0x5582); newline(); }
	cpu.cycles += 10 + 19 + 19 + 4;    // $65CA: jp $0ABE
	SET_HL(rd16(SLOT + 0x26));
	return 0;
}
