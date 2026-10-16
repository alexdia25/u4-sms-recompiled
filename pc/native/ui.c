// Prompts that print: messages, item and member names, the yes/no menu,
// number entry, buy/sell and picking a party member (bank 1 $4A66-$4C27,
// $5516-$55A0; bank 2 $AB55-$AB91). They can wait for frames when the text
// scrolls, so they are blocking natives (checked with whole runs, not call
// by call). Cycle counts are the original's.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define JOY_PRESSED   0xC0B0
#define CURSOR        0xC639
#define NUMBER_TEXT   0xC065          // number entry: the digits typed, as text
#define SLOT          cpu.ix
#define ANSWER        (SLOT + 0x07)   // yes/no: bit 0 set = no
#define MEMBER        (SLOT + 0x08)   // member pick: the one under the bar
#define DIGITS        (SLOT + 0x0E)   // number entry: how many digits at most
#define DIGIT_AT      (SLOT + 0x0F)   // number entry: the digit being changed
#define ENTRY_CURSOR  (SLOT + 0x10)   // number entry: where the number starts
#define MENU_LINE     (SLOT + 0x1A)

static void click(uint16_t which) { cpu.cycles += 17; call_original(which); }   // $40C9 / $40D1 / $40D9

// switch_on_a ($334A) over a key table: the matching entry, or n
static int key_entry(const uint8_t *keys, int n, uint8_t key) {
	int i;
	for (i = 0; i < n && keys[i] != key; i++) {}
	cpu.cycles += i < n ? 58 * i + 79 : 58 * n + 22;
	cpu.b = i < n ? (uint8_t)(keys[i] + 1) : 0;
	return i;
}

// ------------------------------------------------------------ messages --
// print_message ($5582): message A of the current message table ($C0CB, in
// bank $C0CA)
static void nat_print_message(void) {
	uint16_t bc = BC, hl = HL;
	uint8_t bank = mem_read(0xC0CA), saved = mem_read(0xDFFF);
	uint8_t n = cpu.a;
	cpu.cycles += 11 + 11 + 4 + 13 + 16 + 17 + 259 + 13 + 17 + 17 + 102 + 17;
	mem_write(0xFFFF, bank);
	uint16_t text = rd16((uint16_t)(rd16(0xC0CB) + 2 * n));
	wr16(0xC0C2, text);
	cpu.b = 0; cpu.c = n;
	text_print(text);
	mem_write(0xFFFF, saved);
	wr16(0xC0C2, HL);
	cpu.cycles += 10 + 77 + 10 + 10 + 10;
	SET_BC(bc); SET_HL(hl);
}
Native native_print_message = { "print_message", 0x5582, 1, nat_print_message, OUT_A, 0, 0, .blocking = 1 };

// print_item_name ($5516): item A's name (item_name, $340F), printed
static void print_item_name(void) {
	cpu.cycles += 17;
	call_original(0x340F);
	text_print(HL);
}
static void nat_print_item_name(void) { print_item_name(); }
static void nat_weapon_name(void) { cpu.cycles += 7 + 12 + 10; cpu.a += 0x25; print_item_name(); }   // $AB85
static void nat_armour_name(void) { cpu.cycles += 7 + 12 + 10; cpu.a += 0x35; print_item_name(); }   // $AB89
static void nat_reagent_name(void) { cpu.cycles += 7 + 10; cpu.a += 0x5D; print_item_name(); }       // $AB8D
Native native_print_item_name = { "print_item_name", 0x5516, 1, nat_print_item_name, OUT_A | OUT_HL, 0, 0, .blocking = 1 };
Native native_weapon_name = { "weapon_name", 0xAB85, 2, nat_weapon_name, OUT_A | OUT_HL, 0, 0, .blocking = 1 };
Native native_armour_name = { "armour_name", 0xAB89, 2, nat_armour_name, OUT_A | OUT_HL, 0, 0, .blocking = 1 };
Native native_reagent_name = { "reagent_name", 0xAB8D, 2, nat_reagent_name, OUT_A | OUT_HL, 0, 0, .blocking = 1 };

// -------------------------------------------------------------- yes/no --
static uint8_t yes_no_line(void) {     // $4BB5: A = the bar's line, 1 (yes) or 3 (no)
	cpu.cycles += 17 + 7 + 20;
	if (!(mem_read(ANSWER) & 1)) { cpu.cycles += 11; cpu.a = 1; }
	else { cpu.cycles += 5 + 7 + 10; cpu.a = 3; }
	return cpu.a;
}

// yes_no_step ($4B69): one frame of the YES / NO menu. Up or down swaps the
// answer; button 1 means no; button 2 takes it. When answered, YES or NO is
// printed and carry is clear; until then carry is set.
static void nat_yes_no_step(void) {
	static const uint8_t keys[4] = { 0x01, 0x02, 0x10, 0x20 };
	cpu.cycles += 13 + 10 + 17;
	uint8_t key = mem_read(JOY_PRESSED);
	cpu.a = key;
	int i = key_entry(keys, 4, key);
	if (i == 4) cpu.cycles += 12;       // jr to the bar
	if (i < 2) {                        // swap
		click(0x40D9);
		yes_no_line();
		cpu.cycles += 17;
		call_original(0x49E0);          // bar off
		cpu.cycles += 23;
		mem_write(ANSWER, mem_read(ANSWER) + 1);
	}
	if (i < 2 || i == 4) {              // $4B8D: the bar, keep waiting
		yes_no_line();
		cpu.cycles += 17;
		call_original(0x49D5);
		cpu.cycles += 4 + 10;
		cpu.f |= FC;
		return;
	}
	if (i == 2) {                       // no
		click(0x40D1);
		cpu.cycles += 19 + 12;
		mem_write(ANSWER, 1);
	} else click(0x40C9);
	cpu.cycles += 10 + 20 + 7;          // $4BA1
	uint16_t text = 0x4BC1;
	if (mem_read(ANSWER) & 1) { cpu.cycles += 10; text = 0x4BC7; }
	else cpu.cycles += 5;
	cpu.cycles += 17;
	text_print(text);
	cpu.cycles += 17;
	text_newline();
	cpu.cycles += 4 + 10;
	op_or(cpu.a);
}
Native native_yes_no_step = { "yes_no_step", 0x4B69, 1, nat_yes_no_step, OUT_A | OUT_F, 0, 0, .blocking = 1 };

// --------------------------------------------------------- number entry --
// number_entry_start ($4A66): room for A digits (underlined), the cursor back
// at the start, "0" shown
static void nat_number_entry_start(void) {
	uint8_t n = cpu.a;
	mem_write(DIGITS, n);
	mem_write(DIGIT_AT, 0);
	cpu.cycles += 19 + 19 + 4 + 7 + 10 + 4 + 15 + 17;
	SET_DE(n);
	text_print((uint16_t)(0x4A9E - n));   // n underscores
	cpu.cycles += 19;
	cpu.b = n;
	do {
		cpu.cycles += 7 + 17;
		text_char(0x02);                // back
		cpu.cycles += 8;
		if (--cpu.b) cpu.cycles += 5;
	} while (cpu.b);
	cpu.cycles += 16 + 19 + 19 + 7 + 17;
	uint16_t cursor = rd16(CURSOR);
	mem_write(ENTRY_CURSOR, (uint8_t)cursor);
	mem_write(ENTRY_CURSOR + 1, cursor >> 8);
	text_char('0');
	cpu.cycles += 10 + 16 + 10;
	wr16(NUMBER_TEXT, 0x0030);
	SET_HL(0x0030);
}
Native native_number_entry_start = { "number_entry_start", 0x4A66, 1, nat_number_entry_start, OUT_A | OUT_DE | OUT_HL, 0, 0, .blocking = 1 };

// number_entry_step ($4A9F): one frame. Up/down change the digit (wrapping
// 0-9), left takes the last digit back, right adds one (not after a leading
// 0, at most DIGITS), button 1 clears, button 2 is done: HL = the number,
// carry clear. Otherwise the number is shown again and carry is set.
static void nat_number_entry_step(void) {
	static const uint8_t keys[6] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20 };
	cpu.cycles += 13 + 7 + 10 + 17;
	uint8_t key = mem_read(JOY_PRESSED) & 0x7F;
	cpu.a = key;
	int i = key_entry(keys, 6, key);
	switch (i) {
	case 0: case 1: {                   // digit up / down
		int8_t d = i == 0 ? 1 : -1;
		cpu.cycles += 7 + (i == 0 ? 12 : 0) + 19 + 10;
		uint8_t at = mem_read(DIGIT_AT);
		uint16_t p = (uint16_t)(NUMBER_TEXT + at);
		cpu.cycles += 11 + 4 + 4 + ((uint8_t)(NUMBER_TEXT + at) < at ? 5 + 4 + 10 : 11);
		cpu.cycles += 7 + 4 + 7 + 7;
		uint8_t v = (uint8_t)(mem_read(p) + d);
		if (v == 0x2F) { cpu.cycles += 7; v = '9'; } else cpu.cycles += 5;
		cpu.cycles += 7 + 7;
		if (v == 0x3A) { cpu.cycles += 7; v = '0'; } else cpu.cycles += 5;
		cpu.cycles += 7;
		mem_write(p, v);
		click(0x40D9);
		cpu.cycles += 12;
		break;
	}
	case 2: {                           // back one digit
		cpu.cycles += 19 + 4 + 10;
		int8_t at = (int8_t)(mem_read(DIGIT_AT) - 1);
		if (at < 0) break;
		cpu.cycles += 19 + 10;
		mem_write(DIGIT_AT, (uint8_t)at);
		uint16_t p = (uint16_t)(0xC066 + at);
		cpu.cycles += 11 + 4 + 4 + ((uint8_t)(0xC066 + at) < (uint8_t)at ? 5 + 4 + 10 : 11);
		cpu.cycles += 10;
		mem_write(p, 0);
		cpu.cycles += 7 + 17;
		text_char(0x02);
		click(0x40D9);
		cpu.cycles += 12;
		break;
	}
	case 3: {                           // one more digit
		cpu.cycles += 13 + 7 + 7;
		if (mem_read(NUMBER_TEXT) == '0') { cpu.cycles += 5; break; }
		cpu.cycles += 19 + 4 + 19 + 7;
		uint8_t at = (uint8_t)(mem_read(DIGIT_AT) + 1);
		if (at >= mem_read(DIGITS)) { cpu.cycles += 5; break; }
		cpu.cycles += 19 + 10;
		mem_write(DIGIT_AT, at);
		uint16_t p = (uint16_t)(NUMBER_TEXT + at);
		cpu.cycles += 11 + 4 + 4 + ((uint8_t)(NUMBER_TEXT + at) < at ? 5 + 4 + 10 : 11);
		cpu.cycles += 10 + 6 + 10;
		mem_write(p, '0');
		mem_write(p + 1, 0);
		click(0x40D9);
		cpu.cycles += 12;
		break;
	}
	case 4: {                           // clear
		for (;;) {
			click(0x40D1);
			cpu.cycles += 19 + 4 + 7;
			if (mem_read(DIGIT_AT) == 0) { cpu.cycles += 5; break; }
			cpu.cycles += 7 + 17;
			text_char(0x02);
			cpu.cycles += 23 + 12;
			mem_write(DIGIT_AT, mem_read(DIGIT_AT) - 1);
		}
		cpu.cycles += 10 + 16 + 10;
		wr16(NUMBER_TEXT, 0x0030);
		break;
	}
	case 5: {                           // done: the number
		click(0x40C9);
		cpu.cycles += 10 + 10;
		uint16_t hl = 0, de = NUMBER_TEXT;
		for (;;) {
			uint8_t ch = mem_read(de++);
			cpu.cycles += 7 + 6 + 7 + 4;
			if (ch < '0') { cpu.cycles += 11; break; }
			cpu.cycles += 5 + 11 + 4 + 4 + 11 + 11 + 11;
			uint16_t bc = hl * 2;
			hl = (uint16_t)(bc * 4 + bc);   // * 10
			cpu.b = bc >> 8; cpu.c = (uint8_t)bc;
			uint8_t a = (uint8_t)(ch - '0');
			cpu.cycles += 11 + 4 + 4 + ((uint8_t)(hl + a) < a ? 5 + 4 + 10 : 11);
			hl = (uint16_t)(hl + a);
			cpu.cycles += 12;
		}
		SET_HL(hl);
		SET_DE(de);
		cpu.a = 0;   // (sub $30 of the terminator, then ccf)
		cpu.a = (uint8_t)(mem_read(de - 1) - '0');
		cpu.f &= ~FC;
		return;
	}
	default: break;
	}
	// $4AAA: show the number again; carry set
	cpu.cycles += 19 + 19 + 16 + 10 + 17;
	wr16(CURSOR, (uint16_t)(mem_read(ENTRY_CURSOR) | mem_read(ENTRY_CURSOR + 1) << 8));
	text_print(NUMBER_TEXT);
	cpu.cycles += 4 + 10;
	cpu.f |= FC;
}
Native native_number_entry_step = { "number_entry_step", 0x4A9F, 1, nat_number_entry_step, OUT_A | OUT_F | OUT_HL, 0, 0, .blocking = 1 };

// ------------------------------------------------------------- buy/sell --
// buy_or_sell_step ($AB55): one frame of the BUY / SELL menu; when chosen,
// the choice is printed and A = 0 (buy) or 1 (sell), Z clear
static void nat_buy_or_sell_step(void) {
	cpu.cycles += 7 + 10 + 17;
	cpu.c = 2;
	SET_DE(0x0102);
	call_original(0x49F9);              // menu_step_wrap
	if (cpu.f & FZ) { cpu.cycles += 11; return; }
	cpu.cycles += 5;
	if (cpu.f & FC) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 20 + 10 + 7;
	uint16_t text = 0xAB79;
	if (mem_read(MENU_LINE) & 1) { cpu.cycles += 10; text = 0xAB7F; }
	else cpu.cycles += 5;
	cpu.cycles += 17;
	text_print(text);
	cpu.cycles += 17;
	text_newline();
	cpu.cycles += 4 + 4 + 19 + 10;
	op_xor(cpu.a);
	cpu.a = op_dec8(cpu.a);
	cpu.a = mem_read(MENU_LINE);
}
Native native_buy_or_sell_step = { "buy_or_sell_step", 0xAB55, 2, nat_buy_or_sell_step, OUT_A | OUT_F, 0, 0, .blocking = 1 };

// ---------------------------------------------------------- member pick --
// member_name ($49BD): print member C's name
static void nat_member_name(void) {
	uint16_t bc = BC, hl = HL, iy = cpu.iy;
	uint8_t a = cpu.a, f = cpu.f;
	cpu.cycles += 11 + 11 + 11 + 15 + 17;
	call_original(0x48B6);              // member_record -> IY
	cpu.cycles += 15 + 10 + 10 + 11 + 17;
	text_print((uint16_t)(cpu.iy + 0x0D));
	cpu.cycles += 14 + 10 + 10 + 10 + 10;
	cpu.iy = iy; SET_BC(bc); SET_HL(hl); cpu.a = a; cpu.f = f;
}
Native native_member_name = { "member_name", 0x49BD, 1, nat_member_name, 0, 0, 0, .blocking = 1 };

// member_pick_step ($4BD2): one frame of picking a party member: up/down
// move the bar (wrapping), button 1 cancels (carry), button 2 picks (the
// name is printed, Z clear); otherwise Z set
static void nat_member_pick_step(void) {
	static const uint8_t keys[4] = { 0x01, 0x02, 0x10, 0x20 };
	cpu.cycles += 13 + 7 + 10 + 17;
	uint8_t key = mem_read(JOY_PRESSED) & 0x7F;
	cpu.a = key;
	int i = key_entry(keys, 4, key);
	if (i == 2) {                       // cancel
		click(0x40D1);
		cpu.cycles += 7 + 4 + 10;
		cpu.a = 0xFF;
		op_add8(0xFF, 0);
		return;
	}
	if (i == 3) {                       // picked
		click(0x40C9);
		cpu.cycles += 19 + 17;
		cpu.c = mem_read(MEMBER);
		nat_member_name();
		cpu.cycles += 17;
		text_newline();
		cpu.cycles += 4 + 4 + 10;
		op_xor(cpu.a);
		cpu.a = op_dec8(cpu.a);
		return;
	}
	if (i < 2) {                        // move the bar
		int8_t step = i == 0 ? -1 : 1;
		cpu.b = (uint8_t)step;
		cpu.cycles += 7 + (i == 0 ? 12 : 0);
		click(0x40D9);
		cpu.cycles += 19 + 17;
		cpu.c = mem_read(MEMBER);
		call_original(0x4999);          // bar off
		uint8_t size = mem_read(0xC957);
		uint8_t l = (uint8_t)(cpu.c + step + size);
		cpu.cycles += 4 + 4 + 10 + 7 + 7 + 4 + 17;
		uint8_t q = l / size, r = l % size;
		int pc = 0; for (uint8_t v = q; v; v >>= 1) pc += v & 1;
		cpu.cycles += 242 + 3 * pc;     // divide_8
		cpu.h = r; cpu.l = q;
		cpu.cycles += 19 + 4;
		mem_write(MEMBER, r);
		cpu.c = r;
	} else {
		cpu.cycles += 19 + 12;
		cpu.c = mem_read(MEMBER);
	}
	cpu.cycles += 17;                   // $4C0D: the bar
	call_original(0x49A6);
	cpu.cycles += 4 + 10;
	op_xor(cpu.a);
}
Native native_member_pick_step = { "member_pick_step", 0x4BD2, 1, nat_member_pick_step, OUT_A | OUT_F, 0, 0, .blocking = 1 };
