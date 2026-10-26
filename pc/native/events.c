// Tasks of game modes that interrupt the turns (bank 3): the start of a
// fight ($9D1E, mode $14; $9D16, mode $16 for a dungeon room's), camping
// ($AD0F, mode $15: rest, and maybe an ambush) and meditating at a shrine
// ($AE63, mode $17). Cycle counts are the original's.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define SLOT        cpu.ix

static void call(uint16_t addr) { cpu.cycles += 17; call_original(addr); }

static void tcall(uint16_t addr, uint16_t site) {   // a routine that may wait for frames
	if (window_anim_call(addr, site)) return;
	cpu.cycles += 17;
	task_call(addr, (uint16_t)(site + 3));
}

static void say_inline_v(uint16_t text) {   // "call vec_print_inline" with the text after it
	uint16_t hl = HL;
	cpu.cycles += 17 + 10 + 19 + 17;
	text_print(text);
	cpu.cycles += 19 + 10;
	SET_HL(hl);
}

static void message(uint8_t n, uint16_t site) {   // ld a,n / call vec_print_message at site
	cpu.cycles += 7;
	cpu.a = n;
	tcall(0x7F65, site);
}

static void yield_at(uint16_t site) {  // "call yield" at site
	cpu.cycles += 17 + 58;
	SET_HL((uint16_t)(site + 3));
	task_wait_turn((uint16_t)(site + 3));
}

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static void vram_write_at_de(void) {   // rst $10 / jp $0078: di, the address DE to write, ei, ret
	I(11); I(10);
	cpu.iff1 = cpu.iff2 = 0; I(4);
	cpu.a = cpu.e; I(4);
	io_out((uint16_t)(cpu.a << 8 | 0xBF), cpu.a); I(11);
	cpu.a = cpu.d; I(4);
	op_or(0x40); I(7);
	io_out((uint16_t)(cpu.a << 8 | 0xBF), cpu.a); I(11);
	cpu.iff1 = cpu.iff2 = 1; cpu.cycles += 4 + 10;   // ei / ret: an interrupt pending comes after the ret
	native_irq_point();
}

// $B039 (called at $B006): the screen dissolved before the vision, in 12
// frames: each, 11 times, 16 cells of the name table cleared a $220 step
// apart (with wait_vdp, $0262, between the writes), then 4 bytes from
// $B081; the start moves on each time.
static void vision_dissolve(void) {
	I(17);                             // call $B039
	SET_DE(0x0020); I(10);
	cpu.b = 0x0C; I(7);
	do {
		push16(BC); I(11);
		tcall(0x041C, 0xB03F);         // wait_vblank
		cpu.b = 0x0B; I(7);
		do {
			push16(BC); I(11);
			SET_HL(0x0220); I(10);
			cpu.b = 0x10; I(7);
			do {
				vram_write_at_de();
				op_xor(cpu.a); I(4);
				for (int i = 0; i < 4; i++) {
					I(17);             // call wait_vdp: push af / pop af / ret
					push16((uint16_t)(cpu.a << 8 | cpu.f)); I(11);
					pop16(); I(10);
					I(10);
					io_out((uint16_t)(cpu.a << 8 | 0xBE), cpu.a); I(11);
				}
				{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
				SET_HL(op_add16(HL, DE)); I(11);
				{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
				if (--cpu.b) I(13); else I(8);
			} while (cpu.b);
			vram_write_at_de();
			SET_HL(0xB081); I(10);
			SET_BC(0x04BE); I(10);
			do {
				uint8_t v = mem_read(HL);
				cpu.b--;
				io_out(BC, v);
				SET_HL(HL + 1);
				cpu.f = z80_sz[cpu.b] | FN;
				I(16);
				I(cpu.b ? 12 : 7);
			} while (cpu.b);
			SET_HL(0xDE30); I(10);
			SET_HL(op_add16(HL, DE)); I(11);
			{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
			SET_BC(pop16()); I(10);
			if (--cpu.b) I(13); else I(8);
		} while (cpu.b);
		SET_HL(0xFDF4); I(10);
		SET_HL(op_add16(HL, DE)); I(11);
		{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
		SET_BC(pop16()); I(10);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	I(10);
}

static uint8_t random_v(void) { call(0x7ED8); return cpu.a; }

// ------------------------------------------------------------ the fight --
static void fight_begins(int room) {   // $9D1E / $9D16, on into the first member's turn ($9D3B)
	tcall(0x0605, room ? 0x9D16 : 0x9D1E);
	tcall(room ? 0x7EA5 : 0x7EA2, room ? 0x9D19 : 0x9D21);
	if (room) cpu.cycles += 12;
	tcall(0x7F4D, 0x9D24);             // the field drawn
	tcall(0x7F29, 0x9D27);
	tcall(0x7EAB, 0x9D2A);
	cpu.cycles += 7;
	cpu.a = 2;
	tcall(0x0621, 0x9D2F);             // the fight's music
	cpu.cycles += 7 + 13 + 4 + 13;
	mem_write(0xC9BA, 0xFF);
	cpu.a = 0;
	op_xor(0);
	mem_write(0xC97F, 0);
	native_continue_at(0x9D3B);
}
static void fight_begins_map(void) { fight_begins(0); }
static void fight_begins_room(void) { fight_begins(1); }
Native native_fight_begins = { "fight_begins", 0x9D1E, 3, fight_begins_map, 0, 0, 0, .task = 1 };
Native native_room_fight_begins = { "room_fight_begins", 0x9D16, 3, fight_begins_room, 0, 0, 0, .task = 1 };

// ------------------------------------------------------------- camping --
// $AD0F: the camp drawn (in a dungeon, its own picture), "Resting" and
// three dots; one time in eight an ambush (a fight with a foe of the
// table at $AE4F, the party asleep); else, if food can be had for the
// time ($C8FD / 100 from the food), the party healed and woken: "Players
// Healed"; then back to the map
static void camp(void) {
	tcall(0x0605, 0xAD0F);
	tcall(0x7EA2, 0xAD12);
	cpu.cycles += 13 + 7 + 7 + 7 + 7;
	cpu.a = mem_read(0xC9B7) & 0x7F;
	op_sub8(0x13, 0, 0);
	uint8_t pic = 0x0D;
	if (cpu.a == 0x13) { cpu.cycles += 4; pic = 0x0E; } else cpu.cycles += 5;
	cpu.a = pic;
	tcall(0x7E9C, 0xAD21);             // the camp's battlefield
	cpu.cycles += 10 + 10 + 10;
	SET_HL(0xC83E); SET_DE(0x0008); SET_BC(0x1000);
	do {                               // no foes
		cpu.cycles += 7 + 11;
		mem_write(HL, 0);
		SET_HL(HL + 8);
		if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
	} while (1);
	cpu.cycles += 14 + 10 + 13 + 4 + 7;
	cpu.iy = 0xC8B9;
	SET_DE(0x0005);
	cpu.b = mem_read(0xC957);
	cpu.c = 0;
	do {                               // the members lying down (those who can)
		call(0x7ED2);
		cpu.cycles += 7 + 7;
		cpu.a = 0x38;
		if (cpu.f & FC) { cpu.cycles += 4; cpu.a = 0; op_xor(0); } else cpu.cycles += 5;
		cpu.cycles += 19 + 19 + 15 + 4;
		mem_write(cpu.iy + 2, cpu.a);
		mem_write(cpu.iy + 3, cpu.a);
		cpu.iy += 5;
		cpu.c++;
		if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
	} while (1);
	tcall(0x7F4D, 0xAD51);
	tcall(0x7F29, 0xAD54);
	tcall(0x7EAB, 0xAD57);
	cpu.cycles += 7;
	cpu.a = 9;
	tcall(0x0621, 0xAD5C);
	say_inline_v(0xAD62);              // "Resting"
	cpu.cycles += 19;
	mem_write(SLOT + 0x16, 3);
	for (;;) {                         // ". . ."
		cpu.cycles += 7;
		cpu.a = 0x46;
		tcall(0x7F53, 0xAD70);
		cpu.cycles += 7;
		cpu.a = '.';
		tcall(0x7E33, 0xAD75);
		cpu.cycles += 23 + 7;
		uint8_t n = op_dec8(mem_read(SLOT + 0x16));
		mem_write(SLOT + 0x16, n);
		if (!n) break;
		cpu.cycles += 5;
	}
	tcall(0x7F6E, 0xAD7D);
	random_v();
	cpu.cycles += 7 + 10;
	op_and(7);
	if (!cpu.a) {                      // $AE0C: ambushed
		say_inline_v(0xAE0F);
		cpu.cycles += 16 + 16;
		SET_HL(rd16(0xC953));
		wr16(0xC97A, HL);
		random_v();
		cpu.cycles += 7 + 10 + 48 + 13 + 13;
		op_and(7);
		SET_HL((uint16_t)(0xAE4F + cpu.a));
		cpu.a = mem_read(HL);
		mem_write(0xC97D, cpu.a);
		mem_write(0xC97E, cpu.a);
		cpu.cycles += 14 + 10 + 13 + 4;
		cpu.iy = 0xC751;
		SET_DE(0x001D);
		cpu.b = mem_read(0xC957);
		do {                           // all asleep
			cpu.cycles += 19 + 7 + 7;
			cpu.a = mem_read(cpu.iy + 0x1C);
			op_sub8('G', 0, 0);
			if (cpu.a == 'G') { cpu.cycles += 19; mem_write(cpu.iy + 0x1C, 'S'); }
			else cpu.cycles += 5;
			cpu.cycles += 15;
			cpu.iy += 0x1D;
			if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
		} while (1);
		tcall(0x7F29, 0xAE49);
		cpu.cycles += 10;              // jp $7E30: the fight
		native_continue_at(0x7E30);
		return;
	}
	cpu.cycles += 16 + 10;             // food for the rest: $C8FD / 100
	SET_HL(rd16(0xC8FD));
	SET_DE(0x0064);
	call(0xB7EA);
	cpu.cycles += 4 + 16 + 4 + 15 + 4 + 16 + 7;
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); }
	SET_HL(rd16(0xC965));
	op_or(cpu.a);
	op_sbc16(DE);
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); }
	wr16(0xC965, HL);
	if (cpu.f & FZ) {                  // "No effect."
		say_inline_v(0xADA1);
		cpu.cycles += 10;
	} else {
		cpu.cycles += 5 + 14 + 10 + 13 + 4 + 7;   // healed and woken, each who can
		cpu.iy = 0xC751;
		SET_DE(0x001D);
		cpu.b = mem_read(0xC957);
		cpu.c = 0;
		do {
			call(0x7ED2);
			cpu.cycles += 7;
			if (!(cpu.f & FC)) {
				cpu.cycles += 19 + 7 + 7;
				cpu.a = mem_read(cpu.iy + 0x1C);
				op_sub8('S', 0, 0);
				if (cpu.a == 'S') { cpu.cycles += 19; mem_write(cpu.iy + 0x1C, 'G'); }
				else cpu.cycles += 5;
				random_v();
				cpu.cycles += 7 + 7 + 4 + 7;
				op_and(0x77);
				op_add8(0x63, 0);
				cpu.l = cpu.a; cpu.h = 0;
				call(0x7E03);          // healed by 99-218
				cpu.cycles += 19;
				mem_write(cpu.iy + 9, 0x63);
			} else cpu.cycles += 5;
			cpu.cycles += 15 + 4;
			cpu.iy += 0x1D;
			cpu.c++;
			if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
		} while (1);
		tcall(0x7EE1, 0xADE3);
		say_inline_v(0xADE9);          // "Players Healed"
	}
	tcall(0x7F29, 0xADFA);             // $ADFA: back to the map
	tcall(0x0605, 0xADFD);
	tcall(0x7EA2, 0xAE00);
	cpu.cycles += 13 + 7 + 13 + 10;
	cpu.a = mem_read(0xC9B7) & 0x7F;
	op_and(0x7F);
	mem_write(0xC600, cpu.a);
}
Native native_camp = { "camp", 0xAD0F, 3, camp, 0, 0, 0, .task = 1 };

// ------------------------------------------------------------ a shrine --
// $AE63: without its rune, nothing. Else the shrine's picture, a virtue
// asked (the menu of eight), the cycles (1-3) to meditate, the food for
// them, and for each cycle the mantra typed; the right virtue and three
// cycles at the most virtue (99) bring a vision and partial avatarhood
// (the virtue back to 0); else the quest's words for the cycles
static void back_to_map(void) {       // $B02A
	tcall(0x7F6E, 0xB02A);
	tcall(0x0605, 0xB02D);
	tcall(0x7EA2, 0xB030);
	cpu.cycles += 7 + 13 + 10;
	cpu.a = 0x11;
	mem_write(0xC600, 0x11);
}

static void shrine(void) {
	cpu.cycles += 7 + 13 + 10 + 16 + 10;
	cpu.a = 0x1A;
	mem_write(0xC0CA, 0x1A);
	wr16(0xC0CB, 0x8096);
	SET_HL(0x0B0A);
	tcall(0x06DA, 0xAE71);
	cpu.cycles += 13 + 7 + 4 + 7;      // the rune of this shrine?
	cpu.a = (uint8_t)(mem_read(0xC9A0) + 0xE7);
	cpu.b = cpu.a;
	cpu.a = 0x80;
	do {
		cpu.cycles += 4;
		op_rlca();
		if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
	} while (1);
	cpu.cycles += 10 + 7 + 7;
	SET_HL(0xC956);
	op_and(mem_read(HL));
	if (!cpu.a) {
		message(0x20, 0xAE87);
		cpu.cycles += 10;
		back_to_map();
		return;
	}
	cpu.cycles += 5 + 7;
	cpu.a = 0x17;
	tcall(0x7E9C, 0xAE8F);             // the shrine's picture
	tcall(0x7F4D, 0xAE92);
	tcall(0x7F29, 0xAE95);
	tcall(0x7EAB, 0xAE98);
	cpu.cycles += 7;
	cpu.a = 3;
	call(0x05F1);
	message(0x21, 0xAEA2);             // "Upon which virtue..."
	tcall(0xB0AE, 0xAEA5);             // the eight
	cpu.cycles += 19;
	mem_write(SLOT + 0x1A, 0);
	for (;;) {
		tcall(0x7F56, 0xAEAC);
		cpu.cycles += 7 + 10;
		cpu.c = 8;
		SET_DE(0x0102);
		tcall(0x7EB1, 0xAEB4);
		cpu.cycles += 7;
		if (!(cpu.f & FZ)) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 10;
	if (cpu.f & FC) { tcall(0x7F29, 0xB027); back_to_map(); return; }
	cpu.cycles += 19 + 19 + 7;
	cpu.a = mem_read(SLOT + 0x1A);
	mem_write(SLOT + 0x1B, cpu.a);
	op_add8(0x97, 0);
	tcall(0x7F6B, 0xAEC4);
	tcall(0x7F71, 0xAEC7);
	message(0x22, 0xAECC);             // "For how many cycles?"
	cpu.cycles += 10;
	SET_HL(0xB0CA);
	tcall(0x7E60, 0xAED2);
	cpu.cycles += 19;
	mem_write(SLOT + 0x1A, 0);
	for (;;) {
		tcall(0x7F56, 0xAED9);
		cpu.cycles += 7 + 10;
		cpu.c = 4;
		SET_DE(0x0102);
		tcall(0x7EB1, 0xAEE1);
		cpu.cycles += 7;
		if (!(cpu.f & FZ)) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 10;
	if (cpu.f & FC) { tcall(0x7F29, 0xB027); back_to_map(); return; }
	cpu.cycles += 19 + 19 + 7;
	cpu.a = mem_read(SLOT + 0x1A);
	mem_write(SLOT + 0x1C, cpu.a);
	op_add8('0', 0);
	tcall(0x7E33, 0xAEF1);
	tcall(0x7F71, 0xAEF4);
	tcall(0x7F29, 0xAEF7);
	cpu.cycles += 19 + 4 + 7;
	cpu.a = mem_read(SLOT + 0x1C);
	op_or(cpu.a);
	int wrong = 1;
	if (cpu.a) {                       // the shrine of that virtue?
		cpu.cycles += 13 + 7 + 19 + 7;
		cpu.a = (uint8_t)(mem_read(0xC9A0) - 0x1A);
		op_sub8(mem_read(SLOT + 0x1B), 0, 0);
		if (cpu.f & FZ) { cpu.cycles += 5; wrong = 0; }
	} else cpu.cycles += 5;
	if (wrong) {
		message(0x23, 0xAF0C);
		cpu.cycles += 10;
		back_to_map();
		return;
	}
	cpu.cycles += 16 + 10;             // $AF12: food for it
	SET_HL(rd16(0xC8FD));
	SET_DE(0x0064);
	call(0xB7EA);
	cpu.cycles += 4 + 16 + 4 + 15 + 20 + 7;
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); }
	SET_HL(rd16(0xC969));
	op_or(cpu.a);
	op_sbc16(DE);
	wr16(0xC969, DE);
	if (cpu.f & FZ) {
		message(0x24, 0xAF2A);
		cpu.cycles += 10;
		back_to_map();
		return;
	}
	cpu.cycles += 5;
	message(0x25, 0xAF32);             // "Begin meditation"
	tcall(0x7F6E, 0xAF35);
	cpu.cycles += 19 + 19;
	cpu.a = mem_read(SLOT + 0x1C);
	mem_write(SLOT + 0x16, cpu.a);
	for (;;) {                         // $AF3E: each cycle
		cpu.cycles += 19;
		mem_write(SLOT + 0x17, 0x10);
		for (;;) {
			cpu.cycles += 7;
			cpu.a = 8;
			tcall(0x7F53, 0xAF44);
			cpu.cycles += 7;
			cpu.a = '.';
			tcall(0x7E33, 0xAF49);
			cpu.cycles += 23 + 7;
			uint8_t n = op_dec8(mem_read(SLOT + 0x17));
			mem_write(SLOT + 0x17, n);
			if (!n) break;
			cpu.cycles += 5;
		}
		tcall(0x7F6E, 0xAF51);
		message(0x26, 0xAF56);         // "Mantra:"
		cpu.cycles += 7;
		cpu.a = 5;
		tcall(0x7E72, 0xAF5B);
		for (;;) {
			tcall(0x7F56, 0xAF5E);
			tcall(0x7E6F, 0xAF61);
			cpu.cycles += 7;
			if (!(cpu.f & FZ)) break;
			cpu.cycles += 5;
		}
		tcall(0x7F29, 0xAF66);
		cpu.cycles += 7;
		int bad = 1;
		if (!(cpu.f & FC)) {
			tcall(0x7F6E, 0xAF6B);
			cpu.cycles += 19 + 10 + 91 + 10;
			cpu.a = mem_read(SLOT + 0x1B);
			SET_HL(rd16((uint16_t)(0xB0DB + 2 * cpu.a)));
			cpu.a = cpu.l;
			SET_DE(0xC065);
			tcall(0x7F2C, 0xAF78);
			cpu.cycles += 7;
			if (!(cpu.f & FC)) { cpu.cycles += 5; bad = 0; }
		} else cpu.cycles += 5;
		if (bad) {                     // "Thou art not able to focus..."
			message(0x27, 0xAF7F);
			cpu.cycles += 7 + 7;
			cpu.a = 3; cpu.c = 6;
			tcall(0x7F2F, 0xAF86);
			cpu.cycles += 10;
			back_to_map();
			return;
		}
		cpu.cycles += 23 + 7;          // $AF8C
		uint8_t n = op_dec8(mem_read(SLOT + 0x16));
		mem_write(SLOT + 0x16, n);
		if (!n) break;
		cpu.cycles += 5;
	}
	tcall(0x7F6E, 0xAF91);
	cpu.cycles += 19 + 7 + 7;
	cpu.a = mem_read(SLOT + 0x1C);
	op_sub8(3, 0, 0);
	int vision = 0;
	if (cpu.a == 3) {
		cpu.cycles += 19 + 10 + 48 + 7 + 7;
		cpu.a = mem_read((uint16_t)(0xC90B + mem_read(SLOT + 0x1B)));
		SET_HL((uint16_t)(0xC90B + mem_read(SLOT + 0x1B)));
		op_sub8(0x63, 0, 0);
		if (cpu.a == 0x63) { cpu.cycles += 5; vision = 1; }
	} else cpu.cycles += 5;
	if (!vision) {                     // the quest's words, by cycles; some virtue for it
		message(0x28, 0xAFA8);
		tcall(0x7F50, 0xAFAB);
		tcall(0x7F71, 0xAFAE);
		cpu.cycles += 19 + 4 + 4 + 4 + 7;
		cpu.b = mem_read(SLOT + 0x1C);
		cpu.a = (uint8_t)(cpu.b * 3);
		cpu.c = 6;
		call(0x7E00);
		cpu.cycles += 19 + 4 + 4 + 4 + 19 + 7;
		cpu.b = mem_read(SLOT + 0x1B);
		cpu.a = (uint8_t)(cpu.b * 3 + mem_read(SLOT + 0x1C) + 7);
		tcall(0x7F65, 0xAFC7);
		tcall(0x7F50, 0xAFCA);
		tcall(0x7F6E, 0xAFCD);
		cpu.cycles += 10;
		back_to_map();
		return;
	}
	message(0x29, 0xAFD5);             // a vision
	cpu.cycles += 19 + 7;
	cpu.a = (uint8_t)(mem_read(SLOT + 0x1B) + 0x97);
	tcall(0x7F6B, 0xAFDD);
	cpu.cycles += 7;
	cpu.a = 0x15;
	call(0x05F1);
	call(0x041C);
	tcall(0x7E69, 0xAFE8);
	call(0x041C);
	tcall(0x7E69, 0xAFEE);
	call(0x041C);
	cpu.cycles += 19 + 10 + 10;
	{
		uint8_t v = mem_read(SLOT + 0x1B);
		cpu.cycles += 11 + 4 + 4 + 11;   // rst $30
		SET_HL((uint16_t)(0xC90B + v));
		cpu.a = (uint8_t)HL;
		mem_write(HL, 0);              // partial avatarhood
	}
	tcall(0x7F29, 0xAFFD);
	tcall(0x7F50, 0xB000);
	tcall(0x7F71, 0xB003);
	vision_dissolve();                 // call $B039 at $B006: the screen dissolved
	cpu.cycles += 19;
	cpu.a = mem_read(SLOT + 0x1B);
	tcall(0xB085, 0xB00C);             // the vision's picture
	message(0x2A, 0xB011);
	for (;;) {
		tcall(0x7F5C, 0xB014);
		yield_at(0xB017);
		cpu.cycles += 13 + 7 + 7;
		cpu.a = mem_read(0xC0B0);
		op_and(0x30);
		if (cpu.a) break;
		cpu.cycles += 5;
	}
	tcall(0x7F6E, 0xB021);
	cpu.cycles += 10;
	back_to_map();
}
Native native_shrine = { "shrine", 0xAE63, 3, shrine, 0, 0, 0, .task = 1 };
