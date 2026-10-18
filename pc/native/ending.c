// The end of the game, as a C task (bank 3 $B127, game mode $18): before
// the Codex, the party (its leader back to life if need be) must hold the
// whole key, answer the Word of Passage, be eight and be of the highest
// virtue in all eight; then the eight virtues and the three principles are
// asked, and last the axiom. A wrong answer, three times, sends the party
// back to the world ($B40F). Then the Codex speaks, the screen fades
// away, and the end (mode $19). Cycle counts are the original's.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define SLOT        cpu.ix
#define PARTY_SIZE  0xC957
#define KEPT_SIZE   0xC64D          // the party's size, while the leader stands alone

static void call(uint16_t addr) { cpu.cycles += 17; call_original(addr); }

static void tcall(uint16_t addr, uint16_t site) {   // a routine that may wait for frames
	cpu.cycles += 17;
	task_call(addr, (uint16_t)(site + 3));
}

static void message(uint8_t n, uint16_t site) {   // ld a,n / call vec_print_message at site
	cpu.cycles += 7;
	cpu.a = n;
	tcall(0x7F65, site);
}

static void newline_v(uint16_t site) { tcall(0x7F6E, site); }   // call vec_print_newline

static void sleep_turns(uint8_t n, uint16_t site) {   // ld a,n / call $0B16 at site
	cpu.cycles += 7 + 17 + 19 + 58;
	cpu.a = n;
	mem_write(SLOT + 1, n);
	SET_HL((uint16_t)(site + 3));
	task_wait_turn((uint16_t)(site + 3));
}

static uint16_t rst20(uint16_t t, uint8_t a) {   // ld hl,t / rst $20: HL = the word t[A]
	cpu.cycles += 10 + (a < 0x80 ? 91 : 90);
	SET_HL(rd16((uint16_t)(t + 2 * a)));
	cpu.a = cpu.l;
	return HL;
}

static void rst30(uint16_t hl, uint8_t a) {      // rst $30: HL += A
	uint16_t r = (uint16_t)(hl + a);
	cpu.cycles += 11 + 4 + 4 + ((uint8_t)(hl + a) < a ? 5 + 4 + 10 : 11);
	SET_HL(r);
	cpu.a = (uint8_t)r;
}

static void ldir(void) {
	uint16_t n = BC;
	cpu.cycles += 21 * (n - 1) + 16;
	for (; n; n--) { mem_write(DE, mem_read(HL)); SET_HL(HL + 1); SET_DE(DE + 1); }
	SET_BC(0);
	cpu.f &= ~(FH | FN | FP);
}

// call $B48C at site: the cursor blinks, and the next turn (jp yield)
static void blink_turn(uint16_t site) {
	cpu.cycles += 17;
	tcall(0x7F5C, 0xB48C);
	cpu.cycles += 10 + 58;
	SET_HL((uint16_t)(site + 3));
	task_wait_turn((uint16_t)(site + 3));
}

// call $B476 at site: wait for a button
static void wait_button(uint16_t site) {
	cpu.cycles += 17 + 17 + 10 + 19 + 19 + 19 + 10;   // call $0AD5: its return kept in the slot
	wr16(SLOT + 0x2A, (uint16_t)(site + 3));
	SET_HL((uint16_t)(site + 3));
	for (;;) {
		blink_turn(0xB479);
		cpu.cycles += 13 + 7 + 7;
		cpu.a = mem_read(0xC0B0);
		op_and(0x30);
		if (cpu.a) break;
		cpu.cycles += 5;
	}
	tcall(0x7F14, 0xB483);
	tcall(0x7F71, 0xB486);
	cpu.cycles += 10 + 19 + 19 + 4;    // jp $0ADE
	SET_HL(rd16(SLOT + 0x2A));
}

static void say_wait(uint8_t n, uint16_t site) {   // message n, then a button
	message(n, site);
	wait_button((uint16_t)(site + 3));
}

// the word typed (up to $10 letters) into $C065: carry if given up
static void type_word(uint16_t start, uint16_t step, uint16_t back) {
	cpu.cycles += 7;
	cpu.a = 0x10;
	tcall(0x7E72, start);
	for (;;) {
		blink_turn((uint16_t)(start + 3));
		tcall(0x7E6F, (uint16_t)(start + 6));
		cpu.cycles += 7;
		if (!(cpu.f & FZ)) break;
		cpu.cycles += 5;
	}
	tcall(0x7F29, (uint16_t)(start + 11));
	newline_v((uint16_t)(start + 14));
	(void)step; (void)back;
}

// call $B3BA at site: question A, the answer at HL; three tries. Carry: failed
static int ask(uint8_t q, uint16_t answer, uint16_t site) {
	cpu.cycles += 17 + 19 + 19 + 19 + 19;
	mem_write(SLOT + 0x1C, q);
	mem_write(SLOT + 0x1D, 3);
	mem_write(SLOT + 0x1E, (uint8_t)answer);
	mem_write(SLOT + 0x1F, (uint8_t)(answer >> 8));
	cpu.cycles += 17 + 10 + 19 + 19 + 19 + 10;   // call $0AB5: its return kept in the slot
	wr16(SLOT + 0x26, (uint16_t)(site + 3));
	SET_HL((uint16_t)(site + 3));
	int failed;
	for (;;) {
		cpu.cycles += 19;              // $B3CA
		cpu.a = mem_read(SLOT + 0x1C);
		tcall(0x7F65, 0xB3CD);
		newline_v(0xB3D0);
		type_word(0xB3D5, 0, 0);
		cpu.cycles += 7;
		if (!(cpu.f & FC)) {
			newline_v(0xB3E8);
			cpu.cycles += 10 + 19 + 19;
			SET_HL(0xC065);
			cpu.e = mem_read(SLOT + 0x1E);
			cpu.d = mem_read(SLOT + 0x1F);
			tcall(0x7F2C, 0xB3F4);     // carry: not the answer
			cpu.cycles += 7;
			if (!(cpu.f & FC)) { cpu.cycles += 5; failed = 0; break; }
		} else cpu.cycles += 5;
		cpu.cycles += 23 + 4 + 7;      // $B3F9: a try less
		uint8_t n = op_dec8(mem_read(SLOT + 0x1D));
		mem_write(SLOT + 0x1D, n);
		op_scf();
		if (!n) { cpu.cycles += 5; failed = 1; break; }
		tcall(0x7F11, 0xB3FF);
		say_wait(0x18, 0xB404);
		cpu.cycles += 12;
	}
	cpu.cycles += 10 + 19 + 19 + 4;    // jp $0ABE
	SET_HL(rd16(SLOT + 0x26));
	if (failed) cpu.f |= FC; else cpu.f &= ~FC;
	return failed;
}

// $B41C (with A, $B40F with "Thou art not ready", $0C): back to the world,
// to the place of the table at $B45C
static void sent_back(uint8_t where) {
	rst20(0xB45C, where);
	cpu.cycles += 16;
	wr16(0xC953, HL);
	newline_v(0xB423);
	tcall(0x7EA8, 0xB426);
	cpu.cycles += 10;
	SET_HL(0x9CBE);
	tcall(0x0830, 0xB42C);
	tcall(0x041C, 0xB42F);
	cpu.cycles += 13 + 13 + 4 + 13 + 7 + 13 + 4 + 13 + 13 + 7 + 13;
	mem_write(PARTY_SIZE, mem_read(KEPT_SIZE));
	mem_write(0xC9A0, 0);
	mem_write(0xC600, 0x11);
	mem_write(0xC977, 0);
	mem_write(0xC976, 0);
	mem_write(0xC98F, 0xFF);
	cpu.a = 0xFF;
	tcall(0x7F29, 0xB44D);
	cpu.cycles += 7 + 13 + 13;
	mem_write(0xC970, 0xFF);
	mem_write(0xC99F, 0xFF);
	cpu.a = 0xFF;
	tcall(0x7E9F, 0xB458);
	cpu.cycles += 10;                  // ret: the task ends
}

static void not_ready(void) {          // $B40F
	tcall(0x7F11, 0xB40F);
	message(0x0B, 0xB414);
	newline_v(0xB417);
	cpu.cycles += 7;
	sent_back(0x0C);
}

static void flash3(uint16_t site) {   // ld a,$1C / call sound / call vec_flash, at site
	cpu.cycles += 7;
	cpu.a = 0x1C;
	call(0x05F1);
	tcall(0x7F44, (uint16_t)(site + 5));
}

// call $B492 at site: the screen dissolves, from the middle out
static void dissolve(uint16_t site) {
	cpu.cycles += 17 + 17 + 10 + 19 + 19 + 19 + 10;   // call $0AB5
	wr16(SLOT + 0x26, (uint16_t)(site + 3));
	SET_HL((uint16_t)(site + 3));
	cpu.cycles += 19 * 5;
	mem_write(SLOT + 0x0A, 0x42);
	mem_write(SLOT + 0x0B, 0x43);
	mem_write(SLOT + 0x0C, 0x41);
	mem_write(SLOT + 0x0D, 0x40);
	mem_write(SLOT + 0x16, 0x21);
	for (;;) {
		tcall(0x7F59, 0xB4A9);
		cpu.cycles += 19 + 23 + 23;
		cpu.a = mem_read(SLOT + 0x0A);
		mem_write(SLOT + 0x0A, (uint8_t)(cpu.a + 2));
		call(0xB4FB);
		tcall(0x7F59, 0xB4B8);
		cpu.cycles += 19 + 23 + 23;
		cpu.a = mem_read(SLOT + 0x0C);
		mem_write(SLOT + 0x0C, (uint8_t)(cpu.a - 2));
		call(0xB4FB);
		tcall(0x7F59, 0xB4C7);
		cpu.cycles += 19 + 7 + 7;
		cpu.a = mem_read(SLOT + 0x16);
		op_sub8(0x20, 0, 0);
		if (cpu.f & FC) {
			cpu.cycles += 19 + 23 + 23;
			cpu.a = mem_read(SLOT + 0x0B);
			mem_write(SLOT + 0x0B, (uint8_t)(cpu.a + 2));
			call(0xB4FB);
		} else cpu.cycles += 5;
		tcall(0x7F59, 0xB4DD);
		cpu.cycles += 19 + 7 + 7;
		cpu.a = mem_read(SLOT + 0x16);
		op_sub8(0x20, 0, 0);
		if (cpu.f & FC) {
			cpu.cycles += 19 + 23 + 23;
			cpu.a = mem_read(SLOT + 0x0D);
			mem_write(SLOT + 0x0D, (uint8_t)(cpu.a - 2));
			call(0xB4FB);
		} else cpu.cycles += 5;
		cpu.cycles += 23 + 7;
		uint8_t n = op_dec8(mem_read(SLOT + 0x16));
		mem_write(SLOT + 0x16, n);
		if (!n) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 10 + 19 + 19 + 4;    // jp $0ABE
	SET_HL(rd16(SLOT + 0x26));
}

static void picture(uint16_t pal, uint8_t bank, uint16_t at, uint16_t vec, uint16_t site) {
	cpu.cycles += 10;                  // ld hl,pal / call $0830 / call wait_vblank
	SET_HL(pal);
	tcall(0x0830, site);
	tcall(0x041C, (uint16_t)(site + 3));
	cpu.cycles += 7 + 10 + 10;         // ld a / ld hl / ld de / call vec
	cpu.a = bank;
	SET_HL(at); SET_DE(0x0020);
	tcall(vec, (uint16_t)(site + 14));
}

static void ending(void) {
	cpu.cycles += 7 + 13 + 10 + 16 + 10;
	cpu.a = 0x1A;
	mem_write(0xC0CA, 0x1A);
	wr16(0xC0CB, 0x8036);
	SET_HL(0x0B0A);
	tcall(0x06DA, 0xB135);
	cpu.cycles += 7 + 13;
	cpu.a = 2;
	mem_write(0xC00C, 2);
	tcall(0x0605, 0xB13D);
	tcall(0xB55B, 0xB140);             // the screen wiped
	cpu.cycles += 10 + 7 + 7 + 7;
	SET_HL(0xC76D);
	cpu.a = mem_read(0xC76D);
	op_sub8(0x44, 0, 0);
	if (cpu.a == 0x44) {               // the leader dead: back to life
		cpu.cycles += 10 + 7;
		mem_write(0xC76D, 0x47);
		cpu.a = 0x15;
		call(0x05F1);
		tcall(0x7E69, 0xB152);
		tcall(0x7E69, 0xB155);
	} else cpu.cycles += 5;
	cpu.cycles += 10 + 7 + 13 + 10;    // $B158: the leader alone
	SET_HL(PARTY_SIZE);
	cpu.a = mem_read(PARTY_SIZE);
	mem_write(KEPT_SIZE, cpu.a);
	mem_write(PARTY_SIZE, 1);
	tcall(0x7F29, 0xB161);
	sleep_turns(0x1E, 0xB166);
	call(0x05E3);
	newline_v(0xB16C);
	say_wait(0x1C, 0xB171);
	cpu.cycles += 13 + 7 + 7 + 7;      // the whole key?
	cpu.a = mem_read(0xC951);
	op_and(0xE0);
	op_sub8(0xE0, 0, 0);
	if (cpu.a != 0xE0) {
		tcall(0x7F11, 0xB180);
		message(0x1D, 0xB185);
		newline_v(0xB188);
		cpu.cycles += 7 + 10;
		sent_back(0x0C);
		return;
	}
	cpu.cycles += 5;
	say_wait(0x1E, 0xB192);
	cpu.cycles += 7 + 10;              // the Word of Passage
	int failed = ask(0x1F, 0xB5BC, 0xB19D);
	cpu.cycles += 10;
	if (failed) { not_ready(); return; }
	cpu.cycles += 13 + 7 + 7;          // eight in the party?
	cpu.a = mem_read(KEPT_SIZE);
	op_sub8(8, 0, 0);
	if (cpu.a != 8) {
		say_wait(0x21, 0xB1AC);
		cpu.cycles += 10;
		not_ready();
		return;
	}
	cpu.cycles += 5 + 10 + 7 + 4;      // $B1B5: the highest virtue in all eight
	SET_HL(0xC90B);
	cpu.b = 8;
	cpu.a = 0;
	for (;;) {
		cpu.cycles += 7 + 6;
		op_or(mem_read(HL));
		SET_HL(HL + 1);
		if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
	}
	cpu.cycles += 7;
	if (cpu.a) {
		message(0x22, 0xB1C3);
		newline_v(0xB1C6);
		cpu.cycles += 10;
		not_ready();
		return;
	}
	cpu.cycles += 5;
	say_wait(0x23, 0xB1CE);
	picture(0x9CCE, 0x13, 0x8BEE, 0x7E78, 0xB1D7);   // the Codex
	cpu.cycles += 19 + 19;
	mem_write(SLOT + 0x16, 0x0B);
	mem_write(SLOT + 0x17, 0x00);
	for (;;) {                         // $B1F0: the eight virtues, the three principles
		message(0x24, 0xB1F2);
		newline_v(0xB1F5);
		cpu.cycles += 19 + 11 + 10;
		uint8_t q = mem_read(SLOT + 0x17);
		rst20(0xB5CF, q);
		cpu.cycles -= 10;              // (ld hl counted above)
		cpu.cycles += 10;              // pop af
		cpu.a = q;
		failed = ask(q, HL, 0xB201);
		cpu.cycles += 7;
		if (failed) {
			newline_v(0xB206);
			message(0x25, 0xB20B);
			newline_v(0xB20E);
			cpu.cycles += 19 + 10;
			sent_back(mem_read(SLOT + 0x17));
			return;
		}
		cpu.cycles += 5 + 10 + 10 + 10;   // $B217: a colour of the Codex lit
		SET_HL(0xC023); SET_DE(0xC043); SET_BC(0x0010);
		ldir();
		cpu.cycles += 19 + 10;
		rst30(0xC048, mem_read(SLOT + 0x17));
		cpu.cycles += 10 + 10 + 10;
		mem_write(HL, 0x3F);
		SET_HL(0xC043); SET_BC(0x0101);
		tcall(0x0A22, 0xB231);
		tcall(0x0A82, 0xB234);
		cpu.cycles += 19 + 7 + 7;
		cpu.a = mem_read(SLOT + 0x17);
		op_sub8(7, 0, 0);
		if (cpu.a == 7) say_wait(0x26, 0xB240);   // after the eight virtues
		else cpu.cycles += 5;
		cpu.cycles += 23 + 23 + 10;
		mem_write(SLOT + 0x17, (uint8_t)(mem_read(SLOT + 0x17) + 1));
		uint8_t n = op_dec8(mem_read(SLOT + 0x16));
		mem_write(SLOT + 0x16, n);
		if (!n) break;
	}
	message(0x27, 0xB251);
	flash3(0xB254);
	flash3(0xB25C);
	wait_button(0xB264);
	say_wait(0x28, 0xB269);
	say_wait(0x29, 0xB271);
	cpu.cycles += 19;
	mem_write(SLOT + 0x16, 3);
	for (;;) {                         // $B27B: the axiom, three tries
		cpu.cycles += 19 + 7 + 7 + 7;
		uint8_t t = mem_read(SLOT + 0x16);
		op_sub8(3, 0, 0);
		cpu.a = 0x2A;
		if (t == 3) cpu.cycles += 5; else { cpu.cycles += 4; cpu.a = 0x2B; }
		tcall(0x7F65, 0xB285);
		message(0x2C, 0xB28A);
		cpu.cycles += 7;
		cpu.a = 0x10;
		tcall(0x7E72, 0xB28F);
		for (;;) {
			blink_turn(0xB292);
			tcall(0x7E6F, 0xB295);
			cpu.cycles += 7;
			if (!(cpu.f & FZ)) break;
			cpu.cycles += 5;
		}
		tcall(0x7F29, 0xB29A);
		newline_v(0xB29D);
		cpu.cycles += 7;
		int right = 0;
		if (!(cpu.f & FC)) {
			newline_v(0xB2A2);
			cpu.cycles += 10 + 10;
			SET_HL(0xB5C6); SET_DE(0xC065);
			tcall(0x7F2C, 0xB2AB);
			cpu.cycles += 7;
			if (!(cpu.f & FC)) { cpu.cycles += 5; right = 1; }
		} else cpu.cycles += 5;
		if (right) break;
		cpu.cycles += 23 + 7;          // $B2B0
		uint8_t n = op_dec8(mem_read(SLOT + 0x16));
		mem_write(SLOT + 0x16, n);
		if (!n) {
			cpu.cycles += 5;
			message(0x2F, 0xB2C2);
			newline_v(0xB2C5);
			cpu.cycles += 7 + 10;
			sent_back(0x0B);
			return;
		}
		say_wait(0x2E, 0xB2B7);
		cpu.cycles += 10;
	}
	cpu.cycles += 13 + 4 + 10;         // $B2CD: the game won, in its save slot
	cpu.a = mem_read(0xC0CD);
	op_or(cpu.a);
	if (!(cpu.a & 0x80)) {
		rst20(0xB3B4, cpu.a);
		cpu.cycles += 7;
		cpu.a = 0x1D;
		tcall(0x0C43, 0xB2DA);
		cpu.cycles += 16 + 11 + 10 + 16;
		uint16_t at = rd16(0xC639);
		push16(at);
		wr16(0xC639, 0x0004);
		SET_HL(0x0004);
		{                              // call vec_print_inline: two spaces
			uint16_t hl = HL;
			cpu.cycles += 17 + 10 + 19 + 17;
			text_print(0xB2EA);
			cpu.cycles += 19 + 10;
			SET_HL(hl);
		}
		cpu.cycles += 10 + 16;
		SET_HL(pop16());
		wr16(0xC639, HL);
	}
	sleep_turns(0x1E, 0xB2F3);
	flash3(0xB2F6);
	flash3(0xB2FE);
	flash3(0xB306);
	sleep_turns(0x1E, 0xB310);
	cpu.cycles += 7;
	cpu.a = 0x17;
	call(0x05F1);
	dissolve(0xB318);
	picture(0x9CBE, 0x10, 0x9E59, 0x7E7B, 0xB31E);
	sleep_turns(0x1E, 0xB331);
	for (uint8_t m = 0x0C; m <= 0x11; m++) say_wait(m, (uint16_t)(0xB336 + 8 * (m - 0x0C)));
	tcall(0xB55B, 0xB364);
	say_wait(0x12, 0xB369);
	picture(0x9CDE, 0x15, 0x94C1, 0x7E7B, 0xB372);
	for (uint8_t m = 0x13; m <= 0x15; m++) say_wait(m, (uint16_t)(0xB385 + 8 * (m - 0x13)));
	message(0x16, 0xB39D);
	cpu.cycles += 10;
	SET_HL(0xC901);
	tcall(0x7E09, 0xB3A3);             // the turns it took
	tcall(0x7F62, 0xB3A6);
	message(0x17, 0xB3AB);
	cpu.cycles += 7 + 13 + 10;
	cpu.a = 0x19;
	mem_write(0xC600, 0x19);
}
Native native_ending = { "ending", 0xB127, 3, ending, 0, 0, 0, .task = 1 };

// ---------------------------------------------------------- the credits --
// $B63F (game mode $19): when the music has started ($DD00), the pages of
// the end: the Avatar's class first, then the other seven, then the rest;
// each scrolled in, shown for a while and scrolled away; last the Codex's
// picture, and the game stops there. $B7CF draws the ankh meanwhile.

static void yield_at(uint16_t site) {  // "call yield" at site
	cpu.cycles += 17 + 58;
	SET_HL((uint16_t)(site + 3));
	task_wait_turn((uint16_t)(site + 3));
}

static void credits_page(uint8_t n, uint16_t site) {   // ld a,n / call $B6BF at site
	cpu.cycles += 7 + 17 + 19;
	cpu.a = n;
	mem_write(SLOT + 0x09, n);
	cpu.cycles += 17 + 10 + 19 + 19 + 19 + 10;   // call $0AA5: its return kept in the slot
	wr16(SLOT + 0x24, (uint16_t)(site + 3));
	SET_HL((uint16_t)(site + 3));
	tcall(0xB709, 0xB6C5);             // the last page scrolled away
	cpu.cycles += 7 + 10 + 19;
	cpu.a = 0x1F;
	SET_HL(0xAA00);
	cpu.c = mem_read(SLOT + 0x09);
	tcall(0x0807, 0xB6D0);             // HL = the page
	cpu.cycles += 7 + 10;
	cpu.a = 0x1F;
	SET_DE(0xD000);
	tcall(0x071A, 0xB6D8);             // unpacked
	cpu.cycles += 10 + 10;
	SET_HL(0xD000); SET_DE(0xD400);
	tcall(0x0723, 0xB6E1);
	cpu.cycles += 10 + 10;
	SET_HL(0xD400); SET_DE(0xD800);
	tcall(0x0723, 0xB6EA);
	tcall(0x7E12, 0xB6ED);
	tcall(0x7E45, 0xB6F0);
	cpu.cycles += 10;
	SET_HL(0x9CEE);
	tcall(0x0830, 0xB6F6);
	tcall(0xB77D, 0xB6F9);             // scrolled in
	sleep_turns(0xB4, 0xB6FE);
	sleep_turns(0xB4, 0xB703);
	cpu.cycles += 10 + 19 + 19 + 4;    // jp $0AAE
	SET_HL(rd16(SLOT + 0x24));
}

static void credits(void) {
	sleep_turns(0x3C, 0xB641);
	cpu.cycles += 7;
	cpu.a = 1;
	call(0x0621);                      // the music
	cpu.cycles += 4 + 13;
	cpu.a = 0;
	op_xor(0);
	mem_write(0xDD00, 0);
	for (;;) {
		yield_at(0xB64D);
		cpu.cycles += 13 + 4 + 7;
		cpu.a = mem_read(0xDD00);
		op_or(cpu.a);
		if (cpu.a) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 13 - 7;              // ld a,(class) (the ld a,n of the page counted in it)
	credits_page(mem_read(0xC76C), 0xB659);
	credits_page(0x10, 0xB65E);
	cpu.cycles += 19;
	mem_write(SLOT + 0x16, 0);
	for (;;) {                         // the other seven classes
		cpu.cycles += 19 + 10 + 7 + 7;
		uint8_t c = mem_read(SLOT + 0x16);
		cpu.a = c;
		SET_HL(0xC76C);
		op_sub8(mem_read(0xC76C), 0, 0);
		if (c != mem_read(0xC76C)) credits_page((uint8_t)(c + 8), 0xB670);
		else cpu.cycles += 5;
		cpu.cycles += 23 + 19 + 7 + 7;
		c = (uint8_t)(mem_read(SLOT + 0x16) + 1);
		mem_write(SLOT + 0x16, c);
		cpu.a = c;
		op_sub8(8, 0, 0);
		if (c >= 8) break;
		cpu.cycles += 5;
	}
	static const uint8_t pages[8] = { 0x15, 0x11, 0x12, 0x13, 0x16, 0x14, 0x17, 0x18 };
	for (int i = 0; i < 8; i++) credits_page(pages[i], (uint16_t)(0xB67F + 5 * i));
	tcall(0xB55B, 0xB6A5);
	cpu.cycles += 10;
	SET_HL(0x9CDE);
	tcall(0x0830, 0xB6AB);
	yield_at(0xB6AE);
	cpu.cycles += 7 + 10 + 10;
	cpu.a = 0x15;
	SET_HL(0x94C1); SET_DE(0x0020);
	tcall(0x7E7B, 0xB6B9);             // the Codex
	native_continue_at(0xB6BC);        // jp $B6BC: here the game stops
}
Native native_credits = { "credits", 0xB63F, 3, credits, 0, 0, 0, .task = 1 };

// $B7CF: the ankh, drawn each turn
static void ankh(void) {
	cpu.cycles += 7 + 10 + 10;
	cpu.a = 0x05;
	SET_HL(0xB8E4); SET_DE(0x3E80);
	tcall(0x0788, 0xB7D7);
	for (;;) {
		cpu.cycles += 7 + 10 + 10;
		cpu.a = 0x05;
		SET_HL(0xB8F0); SET_DE(0x0888);
		tcall(0x09BE, 0xB7E2);
		yield_at(0xB7E5);
		cpu.cycles += 12;
	}
}
Native native_ankh = { "ankh", 0xB7CF, 3, ankh, 0, 0, 0, .task = 1 };
