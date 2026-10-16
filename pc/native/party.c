// Gold, numbers on screen and who in the party can act, rewritten from
// bank 0 ($2AC8, $2B3F) and bank 1 ($43DA-$447F, $55CF-$564F).
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define GOLD         0xC909
#define FOOD         0xC907
#define PARTY_SIZE   0xC957
#define PARTY        0xC751          // 29 bytes a member; +$1C status
#define CURSOR       0xC639          // text cursor column, row
#define NUM_STARTED  0xC0A6          // print_number: a non-zero digit was printed
#define NUM_FILL     0xC0A7          // print_number: the character for leading zeros

// ------------------------------------------------------------ maths --
// multiply_16 ($2AC8): HL = HL * DE (the low 16 bits), shift-and-add over
// H's bits (skipped when H is 0), then L's. A ends as L.
static void nat_multiply_16(void) {
	uint8_t h = cpu.h, l = cpu.l;
	uint16_t de = DE, hl = 0;
	cpu.cycles += 26;
	uint8_t bytes[2] = { h, l };
	for (int k = h ? 0 : 1; k < 2; k++) {
		uint8_t a = bytes[k];
		for (int i = 0; i < 8; i++) {
			hl = (uint16_t)(hl << 1);
			if (a & 0x80) { hl = (uint16_t)(hl + de); cpu.cycles += 33; }
			else cpu.cycles += 27;
			a = (uint8_t)((a << 1) | (a >> 7));
		}
		cpu.cycles += 10;              // ret
	}
	cpu.cycles += h ? 17 + 4 : 10 + 4;   // call nz (taken or not), ex af,af'
	SET_HL(hl);
	cpu.a = l;
}
Native native_multiply_16 = { "multiply_16", 0x2AC8, 0, nat_multiply_16, OUT_A | OUT_HL, 0, 0 };

// divide_16 ($2B3F): HL = HL / DE, DE = the remainder (HL = 0 if DE is 0),
// restoring division one bit at a time
static void nat_divide_16(void) {
	uint16_t de = DE;
	if (de == 0) { SET_HL(0); cpu.a = 0; cpu.cycles += 4 + 4 + 12 + 10 + 10; return; }
	cpu.cycles += 4 + 4 + 7 + 11 + 4 + 4 + 10;
	uint16_t rem = 0;
	uint8_t q[2], in[2] = { cpu.h, cpu.l };
	for (int k = 0; k < 2; k++) {
		uint8_t a = in[k];
		cpu.cycles += 17;
		for (int i = 0; i < 8; i++) {
			int c = a >> 7;
			a = (uint8_t)(a << 1);
			uint32_t t = (uint32_t)rem * 2 + c;          // adc hl,hl
			int c1 = t > 0xFFFF;
			rem = (uint16_t)t;
			a++;
			int borrow = rem < (uint32_t)de + c1;          // sbc hl,de
			rem = (uint16_t)(rem - de - c1);
			if (borrow) {                                  // add hl,de / dec a: undo
				rem = (uint16_t)(rem + de);
				a--;
				cpu.cycles += 4 + 15 + 4 + 15 + 7 + 11 + 4;
			} else {
				cpu.cycles += 4 + 15 + 4 + 15 + 12;
			}
		}
		cpu.cycles += 10;
		q[k] = a;
		cpu.cycles += k == 0 ? 4 + 4 : 0;   // ld c,a / ld a,b
	}
	cpu.cycles += 4 + 4 + 4 + 10 + 10;     // ex de,hl / ld h,c / ld l,a / pop bc / ret
	SET_DE(rem);
	SET_HL((uint16_t)(q[0] << 8 | q[1]));
	cpu.a = q[1];
}
Native native_divide_16 = { "divide_16", 0x2B3F, 0, nat_divide_16, OUT_A | OUT_DE | OUT_HL, 0, 0 };

// ---------------------------------------------------------- numbers --
// print_number ($55CF): print HL in B digits at the text cursor, leading
// zeros as character C (the last digit always shows). The native does the
// cases that stay on one line; the rest goes to the original.
static int number_fits(int chars) {
	return cpu.c >= 0x20 && mem_read(CURSOR) + chars < 0x1F;
}

static void print_digit_place(uint16_t *hl, uint16_t place) {   // $5606
	uint8_t d = 0;
	while (*hl >= place) { *hl -= place; d++; }
	cpu.cycles += 17 + 4 + 31 * (d + 1) - 5 + 11 + 4 + 7;
	if (d) {
		mem_write(NUM_STARTED, d);
		cpu.cycles += 13 + 7 + 10;
		text_char((uint8_t)('0' + d));
	} else if (mem_read(NUM_STARTED)) {
		cpu.cycles += 5 + 13 + 4 + 7 + 10;
		text_char('0');
	} else {
		cpu.cycles += 5 + 13 + 4 + 7 + 10 + 13 + 10;
		text_char(mem_read(NUM_FILL));
	}
}

static void print_number(void) {
	uint16_t bc = BC, hl = HL;
	mem_write(NUM_FILL, cpu.c);
	mem_write(NUM_STARTED, 0);
	cpu.cycles += 11 + 11 + 4 + 13 + 4 + 13;
	static const uint16_t places[4] = { 10000, 1000, 100, 10 };
	int digits = cpu.b;
	int skip = 5 - digits;             // places not printed
	cpu.cycles += (digits < 5 ? digits - 1 : 4) * (4 + 7);   // dec b / jr z not taken
	if (digits < 5) cpu.cycles += 4 + 12;                  // and the one that jumps
	uint16_t v = hl;
	for (int i = skip; i < 4; i++) {
		cpu.cycles += 10;              // ld bc,place
		print_digit_place(&v, places[i]);
	}
	cpu.cycles += 4 + 7 + 17;
	text_char((uint8_t)('0' + (v & 0xFF)));
	cpu.cycles += 10 + 10 + 10;
	SET_BC(bc); SET_HL(hl);
}

static void nat_print_number(void) {
	if (cpu.b < 1 || cpu.b > 5 || !number_fits(cpu.b)) { native_decline(); return; }
	print_number();
}
Native native_print_number = { "print_number", 0x55CF, 1, nat_print_number, OUT_A, 0, 0 };

// print_number_auto ($5627): print HL with as many digits as it needs
static void nat_print_number_auto(void) {
	uint16_t v = HL;
	uint8_t digits = v >= 10000 ? 5 : v >= 1000 ? 4 : v >= 100 ? 3 : v >= 10 ? 2 : 1;
	if (mem_read(CURSOR) + digits >= 0x1F) { native_decline(); return; }
	uint16_t bc = BC, de = DE;
	cpu.c = 0;
	cpu.cycles += 11 + 11 + 4 + 10;
	static const uint16_t steps[4] = { 10000, 1000, 100, 10 };
	for (int i = 0; i < 4; i++) {
		cpu.cycles += 10 + 11 + 7;
		if (v >= steps[i]) { cpu.cycles += 5; break; }
		cpu.cycles += 4;
	}
	cpu.cycles += 4 + 17;
	cpu.b = digits;
	SET_DE(de);
	print_number();
	cpu.cycles += 10 + 10 + 10;
	SET_BC(bc); SET_DE(de);
}
Native native_print_number_auto = { "print_number_auto", 0x5627, 1, nat_print_number_auto, OUT_A, 0, 0 };

// --------------------------------------------------------------- gold --
// show_gold ($43DA) / show_food ($43F6): the 4-digit counter in the top line
static void show_counter(uint16_t value_at, uint16_t where) {
	uint16_t cursor = rd16(CURSOR), bc = BC, hl = HL;
	cpu.cycles += 11 + 11 + 16 + 11 + 10 + 16 + 16 + 10 + 17;
	wr16(CURSOR, where);
	SET_HL(rd16(value_at));
	SET_BC(0x0430);
	print_number();
	wr16(CURSOR, cursor);
	cpu.cycles += 10 + 16 + 10 + 10 + 10;
	SET_BC(bc); SET_HL(hl);
}
static void nat_show_gold(void) { show_counter(GOLD, 0x000C); }
static void nat_show_food(void) { show_counter(FOOD, 0x0016); }
Native native_show_gold = { "show_gold", 0x43DA, 1, nat_show_gold, OUT_A, 0, 0 };
Native native_show_food = { "show_food", 0x43F6, 1, nat_show_food, OUT_A, 0, 0 };

// receive_gold ($4412): gold += HL, at most 9999
static void nat_receive_gold(void) {
	uint16_t hl = HL, de = DE;
	uint32_t sum = (uint32_t)rd16(GOLD) + hl;
	uint16_t s16 = (uint16_t)sum;
	cpu.cycles += 11 + 11 + 20 + 11 + 17 + (s16 < 9999 ? 87 : 94) + 16 + 10 + 10 + 12;
	wr16(GOLD, s16 > 9999 ? 9999 : s16);
	SET_HL(hl); SET_DE(de);
	show_counter(GOLD, 0x000C);
}
Native native_receive_gold = { "receive_gold", 0x4412, 1, nat_receive_gold, OUT_A, 0, 0 };

// pay ($4423): gold -= HL if there is that much (carry clear), else carry set
static void nat_pay(void) {
	uint16_t price = HL, gold = rd16(GOLD);
	cpu.cycles += 11 + 11 + 4 + 16 + 4 + 15 + 7;
	if (gold < price) {
		cpu.cycles += 5 + 10 + 10 + 11;
		cpu.f |= FC;
		return;
	}
	wr16(GOLD, (uint16_t)(gold - price));
	cpu.cycles += 16 + 10 + 10 + 5 + 17;
	show_counter(GOLD, 0x000C);
	cpu.cycles += 4 + 10;
	op_or(cpu.a);
}
Native native_pay = { "pay", 0x4423, 1, nat_pay, OUT_A | OUT_CARRY, 0, 0 };

// can_afford ($4439): carry set if the party has less than HL gold
static void nat_can_afford(void) {
	uint16_t price = HL;
	cpu.cycles += 11 + 11 + 4 + 16 + 4 + 15 + 10 + 10 + 10;
	if (rd16(GOLD) < price) cpu.f |= FC; else cpu.f &= ~FC;
}
Native native_can_afford = { "can_afford", 0x4439, 1, nat_can_afford, OUT_CARRY, 0, 0 };

// ------------------------------------------------------------- party --
// member_can_act ($4445): carry set unless member C exists and is Good,
// Sleeping or Poisoned. member_able ($4466): the same without Sleeping.
static void member_check(int sleeping_ok) {
	uint8_t c = cpu.c;
	cpu.cycles += 11 + 15 + 13 + 4 + 4 + 7;
	cpu.a = (uint8_t)(mem_read(PARTY_SIZE) - 1);
	if (cpu.a < c) {
		cpu.cycles += 5 + 14 + 10 + 10;
		cpu.f |= FC;
		return;
	}
	cpu.cycles += 17 + 145 + 24 * c + 19 + 7 + 7;
	uint8_t s = mem_read(PARTY + c * 29 + 0x1C);
	cpu.a = s;
	int ok;
	if (s == 'G') { ok = 1; cpu.cycles += 5; }
	else if (sleeping_ok) {
		cpu.cycles += 7 + 7;
		if (s == 'S') { ok = 1; cpu.cycles += 5; }
		else {
			cpu.cycles += 7 + 7;
			if (s == 'P') { ok = 1; cpu.cycles += 5; }
			else { ok = 0; cpu.cycles += 4; }
		}
	} else {
		cpu.cycles += 7 + 7;
		if (s == 'P') { ok = 1; cpu.cycles += 5; }
		else { ok = 0; cpu.cycles += 4; }
	}
	cpu.cycles += 14 + 10 + 10;
	if (ok) cpu.f &= ~FC; else cpu.f |= FC;
}
static void nat_member_can_act(void) { member_check(1); }
static void nat_member_able(void) { member_check(0); }
Native native_member_can_act = { "member_can_act", 0x4445, 1, nat_member_can_act, OUT_CARRY, 0, 0 };
Native native_member_able = { "member_able", 0x4466, 1, nat_member_able, OUT_CARRY, 0, 0 };
