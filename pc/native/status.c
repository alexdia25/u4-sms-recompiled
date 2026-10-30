// The status pages (bank 1 $7552, A = which): 0-7 a member's (name, sex,
// class, status, level, strength, dexterity, intelligence, magic and its
// most, hit points and most, experience, weapon and armour), 8 the
// weapons, 9 the armours, $0A torches, gems, keys and the sextant, $0B the
// special things, $0C the reagents, $0D and $10 the mixtures; $0E, $0F
// other titles for the weapons and armours. Drawn into the text window
// with the menu routines. Cycle counts are the original's.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

static void call(uint16_t addr) { cpu.cycles += 17; call_original(addr); }
static void jp(uint16_t addr) { cpu.cycles += 10; call_original(addr); }   // jp addr: its ret ends the page

static void advance_de(uint16_t by) {  // ld hl,by / add hl,de / ex de,hl
	cpu.cycles += 10 + 11 + 4;
	uint16_t at = DE;
	SET_DE(op_add16(by, at));
	SET_HL(at);
}

static void title(uint16_t text) {     // ld hl,text / call $78C2: the window cleared, its title
	cpu.cycles += 10 + 17;
	SET_HL(text);
	call(0x2E1B);
	jp(0x2D65);
}

static void number(uint8_t a) {        // call $78C8: the number A at DE
	cpu.cycles += 17 + 4 + 7;
	cpu.l = a; cpu.h = 0;
	jp(0x2D87);
}

static uint8_t length(void) {          // call $78CE: A = the length of the text at HL
	cpu.cycles += 17 + 11 + 11 + 4 + 4;
	uint16_t hl = HL, bc = BC;
	cpu.a = 0;
	op_xor(0);
	cpu.b = 0;
	for (;;) {
		cpu.cycles += 7 + 7;
		op_sub8(mem_read(HL), 0, 0);
		if (cpu.f & FZ) { cpu.cycles += 5; break; }
		cpu.cycles += 6 + 4 + 10;
		SET_HL(HL + 1);
		cpu.b++;
	}
	cpu.cycles += 4 + 10 + 10 + 10;
	cpu.a = cpu.b;
	SET_HL(hl); SET_BC(bc);
	return cpu.a;
}

// call $78DE: anything among the B bytes after HL? If not, "None" and the
// page is over (pop af: the page's own return dropped); 0 then
static int any(void) {
	cpu.cycles += 17 + 4;
	cpu.a = 0;
	op_xor(0);
	do {
		cpu.cycles += 6 + 7;
		SET_HL(HL + 1);
		op_or(mem_read(HL));
		if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
	} while (1);
	if (cpu.a) { cpu.cycles += 11; return 1; }
	cpu.cycles += 5 + 10;
	return 0;
}

static void none(void) {               // $78E5: "None", the window shown
	cpu.cycles += 10;
	SET_HL(0x79CD);
	call(0x2D65);
	jp(0x2E54);
}

static void thing_name(uint8_t a, uint8_t d) {   // call $78EE: thing A's name, right-aligned on row D
	cpu.cycles += 17;
	cpu.a = a; cpu.d = d;
	call(0x340F);
	length();
	cpu.cycles += 8 + 7 + 4 + 10;
	op_neg();
	op_add8(0x1F, 0);
	cpu.e = cpu.a;
	call_original(0x2D69);
}

// call $78FC: entry C of the name table at HL, ":" and the count (IY) on
// the line at DE; DE on to the next column
static void entry(void) {
	cpu.cycles += 17 + 4 + 91 + 11 + 17;
	cpu.a = cpu.c;
	SET_HL(rd16((uint16_t)(HL + 2 * cpu.c)));
	push16(DE);
	call_original(0x2D6C);
	cpu.cycles += 10;
	SET_DE(pop16());
	advance_de(0x0012);
	cpu.cycles += 7;
	cpu.a = ':';
	call(0x2D9A);
	cpu.cycles += 19;
	number(mem_read(cpu.iy));
	advance_de(0x0028);
	cpu.cycles += 10;
}

static void member(void) {             // $7633
	call(0x48B6);                      // IY = member C's record
	cpu.cycles += 10;
	SET_HL(0x7919);
	call(0x2E79);
	cpu.cycles += 15 + 10 + 10 + 11;
	SET_HL(op_add16(cpu.iy, 0x000D));
	SET_BC(0x000D);
	uint8_t n = length();              // the name, in the middle
	cpu.cycles += 4 + 8 + 8 + 7 + 4 + 7;
	cpu.a = op_inc8(n);
	cpu.a = op_rot(7, cpu.a);
	op_neg();
	op_add8(0x19, 0);
	cpu.e = cpu.a; cpu.d = 1;
	call(0x2D69);
	cpu.cycles += 10 + 19;
	SET_DE(0xCAA6);
	cpu.a = mem_read(cpu.iy + 0x1A);   // sex
	call(0x2D9A);
	cpu.cycles += 4 + 4 + 19 + 7;
	cpu.e += 2;
	cpu.a = (uint8_t)(mem_read(cpu.iy + 0x1B) + 0x4D);   // class
	call(0x340F);
	call(0x2D6C);
	cpu.cycles += 19;                  // the status
	uint8_t s = mem_read(cpu.iy + 0x1C);
	static const uint8_t states[4] = { 'G', 'P', 'S', 'D' };
	static const uint16_t texts[4] = { 0x79D7, 0x79E3, 0x79EC, 0x79F4 };
	int i;
	for (i = 0; i < 4; i++) {
		cpu.cycles += 7 + 10 + 7;
		cpu.a = s;
		op_sub8(states[i], 0, 0);
		SET_HL(texts[i]);
		if (s == states[i]) { cpu.cycles += 5; break; }
	}
	if (i == 4) cpu.cycles += 5;       // (the last is jr nz)
	else {
		if (i == 3) cpu.cycles -= 5;
		cpu.cycles += 10;
		SET_DE(0xCAE6);
		call(0x2D6C);
	}
	cpu.cycles += 19 + 7 + 10;         // level
	cpu.a = (uint8_t)(mem_read(cpu.iy + 0x0A) + 0x30);
	SET_DE(0xCB34);
	call(0x2D9A);
	static const uint8_t stat[4] = { 6, 7, 8, 9 };
	static const uint16_t at[4] = { 0xCB72, 0xCBB2, 0xCBF2, 0xCC32 };
	for (int k = 0; k < 4; k++) {      // strength, dexterity, intelligence, magic
		cpu.cycles += 19 + 10;
		SET_DE(at[k]);
		number(mem_read(cpu.iy + stat[k]));
	}
	cpu.cycles += 19 + 19 + 10 + 11 + 10 + 52;   // the most magic, by class ($47B3)
	cpu.c = mem_read(cpu.iy + 8);
	uint8_t cls = mem_read(cpu.iy + 0x1B);
	SET_HL(0x47B3);
	uint16_t to = rd16((uint16_t)(0x47B3 + 2 * cls));
	SET_HL(to);
	cpu.a = (uint8_t)to;
	class_magic_most(cls);             // rst $18 / jp (hl)
	cpu.cycles += 7 + 7;
	op_sub8(0x64, 0, 0);
	if (cpu.f & FC) cpu.cycles += 5;
	else { cpu.cycles += 7; cpu.a = 0x63; }
	cpu.cycles += 10;
	SET_DE(0xCC72);
	number(cpu.a);
	static const uint8_t word[3] = { 0, 2, 4 };
	static const uint16_t wat[3] = { 0xCCB0, 0xCCF0, 0xCD2E };
	static const uint16_t how[3] = { 0x2D81, 0x2D81, 0x2D7B };
	for (int k = 0; k < 3; k++) {      // hit points, their most, experience
		cpu.cycles += 19 + 19 + 10;
		SET_HL(rd16(cpu.iy + word[k]));
		SET_DE(wat[k]);
		call(how[k]);
	}
	cpu.cycles += 19 + 7 + 7;
	thing_name((uint8_t)(mem_read(cpu.iy + 0x0B) + 0x25), 0x0E);   // weapon
	cpu.cycles += 19 + 7 + 7;
	thing_name((uint8_t)(mem_read(cpu.iy + 0x0C) + 0x35), 0x10);   // armour
	jp(0x2E54);
}

// a list of counts ($C920 weapons, $C918 armours...) with names from the table at t
static void counts(uint16_t list, int n, uint16_t names, int skip_empty_lines) {
	cpu.cycles += 10 + 14 + 10;
	SET_DE(skip_empty_lines ? 0xCAE6 : 0xCAA6);
	cpu.iy = list;
	SET_BC((uint16_t)(n << 8));
	do {
		cpu.cycles += 11 + 10 + 19 + 4;
		push16(BC);
		SET_HL(names);
		cpu.a = mem_read(cpu.iy);
		op_or(cpu.a);
		if (!skip_empty_lines) {
			if (cpu.a) entry(); else cpu.cycles += 10;
		} else {
			cpu.cycles += 7;
			if (cpu.a) { entry(); advance_de(0x0040); }
			else cpu.cycles += 5;
		}
		cpu.cycles += 10 + 10 + 4;
		SET_BC(pop16());
		cpu.iy++;
		cpu.c++;
		if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
	} while (1);
	jp(0x2E54);
}

static void weapons(uint16_t t) {      // $770E (or $773A)
	title(t);
	if (t == 0x796D) {
		cpu.cycles += 10 + 7;
		SET_HL(0xC91F);
		cpu.b = 0x0F;
		if (!any()) { none(); return; }
	} else cpu.cycles += 12;
	counts(0xC920, 15, 0x7A33, 0);
}

static void armours(uint16_t t) {      // $7742 (or $7775)
	title(t);
	if (t == 0x7976) {
		cpu.cycles += 10 + 7;
		SET_HL(0xC917);
		cpu.b = 7;
		if (!any()) { none(); return; }
	} else cpu.cycles += 12;
	counts(0xC918, 7, 0x7AC4, 1);
}

static void items(void) {              // $777D: torches, gems, keys; the sextant if any
	title(0x797F);
	cpu.cycles += 10 + 14 + 10;
	SET_DE(0xCAE6);
	cpu.iy = 0xC913;
	SET_BC(0x0300);
	do {
		cpu.cycles += 11 + 10;
		push16(BC);
		SET_HL(0x7B0F);
		entry();
		advance_de(0x0040);
		cpu.cycles += 10 + 10 + 4;
		SET_BC(pop16());
		cpu.iy++;
		cpu.c++;
		if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
	} while (1);
	cpu.cycles += 10 + 19 + 4;
	SET_HL(0x7B0F);
	cpu.a = mem_read(cpu.iy);
	op_or(cpu.a);
	if (cpu.a) entry(); else cpu.cycles += 10;
	jp(0x2E54);
}

// call $781A: if A, the text at HL and then, for each of A's 8 bits, the
// letter of the table at BC
static void bits_line(uint8_t a, uint16_t text, uint16_t letters) {
	cpu.cycles += 17 + 4;
	cpu.a = a;
	op_or(a);
	if (!a) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 11 + 11 + 11 + 17;
	SET_HL(text); SET_BC(letters);
	push16(DE);
	call_original(0x2D6C);
	cpu.cycles += 10;
	SET_DE(pop16());
	advance_de(0x0042);
	cpu.cycles += 10 + 10 + 4 + 7 + 11;
	SET_HL(letters);
	cpu.c = a;
	cpu.b = 8;
	push16(DE);
	do {
		cpu.cycles += 7 + 6 + 8;
		cpu.a = mem_read(HL);
		SET_HL(HL + 1);
		cpu.c = op_rot(3, cpu.c);      // rr c
		if (cpu.f & FC) call(0x2D9A); else cpu.cycles += 10;
		if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
	} while (1);
	cpu.cycles += 10;
	SET_DE(pop16());
	advance_de(0x003E);
	cpu.cycles += 10;
}

static void thing_line(uint16_t flags, uint8_t bit, uint16_t text) {   // call $7851 / $7856
	cpu.cycles += 7 + 10 + 17 + 13 + (flags == 0xC951 ? 12 : 0) + 4;
	cpu.b = bit;
	SET_HL(text);
	cpu.a = mem_read(flags);
	op_and(bit);
	if (!cpu.a) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 11 + 17;
	push16(DE);
	call_original(0x2D6C);
	cpu.cycles += 10;
	SET_DE(pop16());
	advance_de(0x0040);
	cpu.cycles += 10;
}

static void special(void) {            // $77AC: the stones, runes, bell, book, candle, key, horn, wheel, skull
	title(0x798B);
	cpu.cycles += 10 + 13 + 10 + 10;
	SET_DE(0xCAE6);
	bits_line(mem_read(0xC955), 0x79F9, 0x783E);   // stones
	cpu.cycles += 13 + 10 + 10;
	bits_line(mem_read(0xC956), 0x7A00, 0x7846);   // runes
	thing_line(0xC951, 0x04, 0x7A06);
	thing_line(0xC951, 0x08, 0x7A0B);
	thing_line(0xC951, 0x10, 0x7A10);
	cpu.cycles += 13 + 4 + 4 + 4 + 7 + 10 + 10;
	cpu.a = (uint8_t)((mem_read(0xC951) << 3 | mem_read(0xC951) >> 5) & 7);
	bits_line(cpu.a, 0x7A17, 0x784E);              // the key's parts
	thing_line(0xC952, 0x01, 0x7A22);
	thing_line(0xC952, 0x02, 0x7A27);
	thing_line(0xC951, 0x01, 0x7A2D);
	cpu.cycles += 10 + 4 + 15 + 10;
	SET_HL(0xCAE6);
	op_or(cpu.a);
	op_sbc16(DE);
	if (cpu.f & FZ) { none(); return; }
	jp(0x2E54);
}

static void reagents(uint16_t t) {     // $7866 (or $788B)
	cpu.cycles += 10;
	if (t == 0x79C7) cpu.cycles += 12;
	SET_HL(t);
	cpu.cycles += 17;
	call(0x2E1B);
	jp(0x2D65);
	cpu.cycles += 10 + 14 + 10;
	SET_DE(0xCAA6);
	cpu.iy = 0xC92F;
	SET_BC(0x0800);
	do {
		cpu.cycles += 11 + 10;
		push16(BC);
		SET_HL(0x7B31);
		entry();
		advance_de(0x0040);
		cpu.cycles += 10 + 10 + 4;
		SET_BC(pop16());
		cpu.iy++;
		cpu.c++;
		if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
	} while (1);
	jp(0x2E54);
}

static void mixtures(void) {           // $7890: the first 13, or (C even) the other 13
	cpu.cycles += 8 + 7;
	if (cpu.c & 1) {
		cpu.cycles += 10 + 14 + 7 + 12;
		SET_HL(0x799E);
		cpu.iy = 0xC937;
		cpu.c = 0;
	} else {
		cpu.cycles += 5 + 10 + 14 + 7;
		SET_HL(0x79AB);
		cpu.iy = 0xC944;
		cpu.c = 0x0D;
	}
	cpu.cycles += 11;
	push16(BC);
	cpu.cycles += 17;
	call(0x2E1B);
	jp(0x2D65);
	cpu.cycles += 10 + 10 + 7;
	SET_BC(pop16());
	SET_DE(0xCAE6);
	cpu.b = 0x0D;
	do {
		cpu.cycles += 11 + 10;
		push16(BC);
		SET_HL(0x7B84);
		entry();
		cpu.cycles += 10 + 10 + 4;
		SET_BC(pop16());
		cpu.iy++;
		cpu.c++;
		if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
	} while (1);
	jp(0x2E54);
}

static void status_page(void) {
	cpu.cycles += 11 * 4 + 15 + 4 + 10 + 11 + 10 + 52;   // the registers kept; rst $18 by the table at $7564
	uint16_t af = (uint16_t)(cpu.a << 8 | cpu.f);
	push16(af); push16(BC); push16(DE); push16(HL); push16(cpu.iy);
	uint8_t page = cpu.a;
	cpu.c = page;
	push16(0x755D);
	uint16_t to = rd16((uint16_t)(0x7564 + 2 * page));
	SET_HL(to);
	cpu.a = (uint8_t)to;
	switch (to) {
	case 0x7633: member(); break;
	case 0x770E: weapons(0x796D); break;
	case 0x773A: weapons(0x79B8); break;
	case 0x7742: armours(0x7976); break;
	case 0x7775: armours(0x79C0); break;
	case 0x777D: items(); break;
	case 0x77AC: special(); break;
	case 0x7866: reagents(0x7993); break;
	case 0x788B: reagents(0x79C7); break;
	case 0x7890: mixtures(); break;
	default: native_decline(); return;
	}
	pop16();                           // (its return, to $755D)
	cpu.cycles += 14 + 10 * 4 + 10;
	cpu.iy = pop16(); SET_HL(pop16()); SET_DE(pop16()); SET_BC(pop16());
	af = pop16();
	cpu.a = (uint8_t)(af >> 8); cpu.f = (uint8_t)af;
}
Native native_status_page = { "status_page", 0x7552, 1, status_page, 0, .long_calls = 1 };

// ------------------------------------------------------ the status window --
// Instruction by instruction from here (I): these run every turn.
static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }
static void push(uint16_t v) { push16(v); I(11); }
static void icall(uint16_t addr) { I(17); call_original(addr); }

// $342B: the glyph shown in the corner, by $C976: 0 the eight virtues'
// marks (a row lit for each one with karma left), 1 the ankh (bank $13),
// otherwise that character of the font
static void status_glyph(void) {
	cpu.a = mem_read(0xC976); I(13);
	op_or(cpu.a); I(4);
	if (cpu.f & FZ) {                  // $3454
		I(12);
		push(BC); push(DE); push(HL);
		SET_HL(0x34B3); I(10);
		SET_DE(0xC1C0); I(10);
		SET_BC(0x0008); I(10);
		while (op_block(6, 0)) I(21);
		I(16);
		SET_HL(0xC1C0); I(10);
		SET_DE(0xC90B); I(10);
		SET_BC(0x0800); I(10);
		do {
			cpu.a = mem_read(DE); I(7);
			op_or(cpu.a); I(4);
			if (cpu.f & FZ) I(12);
			else { I(7); mem_write(HL, cpu.c); I(7); }
			cpu.l = op_inc8(cpu.l); I(4);
			SET_DE(DE + 1); I(6);
			if (--cpu.b) I(13); else I(8);
		} while (cpu.b);
		SET_HL(0xC1C0); I(10);
		icall(0x3493);                 // cursor_glyph
		SET_HL(pop16()); I(10);
		SET_DE(pop16()); I(10);
		SET_BC(pop16()); I(10);
		I(10);
		return;
	}
	I(7);
	cpu.a = op_dec8(cpu.a); I(4);
	if (cpu.f & FZ) {                  // $347E: the ankh
		I(12);
		push(BC); push(DE); push(HL);
		cpu.a = 0x13; I(7);
		SET_HL(0xB12F); I(10);
		SET_DE(0x25F0); I(10);
		SET_BC(0x0020); I(10);
		icall(0x07F4);                 // far_vram_write
		SET_HL(pop16()); I(10);
		SET_DE(pop16()); I(10);
		SET_BC(pop16()); I(10);
		I(10);
		return;
	}
	I(7);
	cpu.a = 0x0F; I(7);
	vram_bank_push(0x3436);
	push(BC); push(DE); push(HL);
	cpu.a = mem_read(0xC976); I(13);
	op_sub8(0x20, 0, 1); I(7);
	cpu.l = cpu.a; I(4);
	cpu.h = 0; I(7);
	for (int i = 0; i < 3; i++) { SET_HL(op_add16(HL, HL)); I(11); }
	SET_DE(0xBB58); I(10);
	SET_HL(op_add16(HL, DE)); I(11);
	icall(0x3493);
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	vram_bank_pop();
}

// $4324: the status window: each member's name, hit points, status and
// magic points, then food, and the hull ($C961) on a ship or else the gold
static void status_window(void) {
	push((uint16_t)(cpu.a << 8 | cpu.f)); push(BC); push(DE); push(HL);
	push16(cpu.iy); I(15);
	SET_HL(rd16(0xC639)); I(16);       // the text position, kept
	push(HL);
	I(17); status_glyph();
	icall(0x2E1B);                     // menu_window_clear
	cpu.iy = 0xC751; I(14);
	SET_DE(0xCA66); I(10);
	cpu.a = mem_read(0xC957); I(13);   // party_size
	cpu.b = cpu.a; I(4);
	do {
		push(BC);
		push16(cpu.iy); I(15);
		SET_HL(pop16()); I(10);
		SET_BC(0x000D); I(10);
		SET_HL(op_add16(HL, BC)); I(11);
		push(DE);
		icall(0x2D6C);                 // the name
		SET_DE(pop16()); I(10);
		SET_HL(0x0048); I(10);
		SET_HL(op_add16(HL, DE)); I(11);
		{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
		cpu.l = mem_read(cpu.iy); I(19);
		cpu.h = mem_read((uint16_t)(cpu.iy + 1)); I(19);
		icall(0x2D81);                 // hit points
		cpu.e = op_inc8(cpu.e); I(4);
		cpu.e = op_inc8(cpu.e); I(4);
		cpu.a = mem_read((uint16_t)(cpu.iy + 0x1C)); I(19);   // the status letter
		op_add8(0x40, 0); I(7);
		mem_write(DE, cpu.a); I(7);
		cpu.e = op_inc8(cpu.e); I(4);
		cpu.a = 0x01; I(7);
		mem_write(DE, cpu.a); I(7);
		for (int i = 0; i < 3; i++) { cpu.e = op_inc8(cpu.e); I(4); }
		cpu.l = mem_read((uint16_t)(cpu.iy + 9)); I(19);
		cpu.h = 0; I(7);
		icall(0x2D87);                 // magic points
		SET_HL(0x0028); I(10);
		SET_HL(op_add16(HL, DE)); I(11);
		{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
		SET_BC(0x001D); I(10);
		cpu.iy = op_add16(cpu.iy, BC); I(15);
		SET_BC(pop16()); I(10);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	icall(0x2E54);                     // menu_window_show
	SET_HL(0x0014); I(10);
	wr16(0xC639, HL); I(16);
	cpu.a = 'F'; I(7);
	icall(0x548F);                     // print_char
	cpu.a = ':'; I(7);
	icall(0x548F);
	SET_HL(rd16(0xC907)); I(16);       // food
	SET_BC(0x0430); I(10);
	icall(0x55CF);                     // print_number
	SET_HL(0x000A); I(10);
	wr16(0xC639, HL); I(16);
	cpu.a = mem_read(0xC959); I(13);
	op_sub8(0x14, 0, 0); I(7);
	if (cpu.f & FC) {                  // on a ship: its hull
		I(7);
		SET_HL(0x43D2); I(10);
		icall(0x5519);                 // print_text_hl
		SET_HL(rd16(0xC961)); I(16);
		SET_BC(0x0230); I(10);
		icall(0x55CF);
		I(12);
	} else {
		I(12);
		SET_HL(0x43D7); I(10);
		icall(0x5519);
		SET_HL(rd16(0xC909)); I(16);   // gold
		SET_BC(0x0430); I(10);
		icall(0x55CF);
	}
	SET_HL(pop16()); I(10);
	wr16(0xC639, HL); I(16);
	cpu.iy = pop16(); I(14);
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	{ uint16_t af = pop16(); cpu.a = af >> 8; cpu.f = (uint8_t)af; } I(10);
	I(10);
}
Native native_status_glyph = { "status_glyph", 0x342B, 0, status_glyph, OUT_A | OUT_F, .long_calls = 1 };
Native native_status_window = { "status_window", 0x4324, 1, status_window, 0, .long_calls = 1 };
