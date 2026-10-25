// IY on a record (bank 1):
// - member_record ($48B6): party member C's record ($C751 + 29 * C);
//   current_record ($48CB) for the member whose turn it is ($C97F).
// - member_object ($48D5): member C's figure on the battlefield ($C8B9 +
//   5 * C); current_object ($48E1).
// - foe_record ($48EB): foe C's ($C839 + 8 * C).
// - person_record ($48F7): person A of the place ($C651 + 8 * A).
// - person_with_graphic ($4906): the first person whose graphic is A (in
//   a place 32 of them from $C651, on the world 24 from $C691): A = its
//   number, or carry for none.
// And three small ones of the party's:
// - party_iter_setup ($4098): IY = the first member, DE = 29, B = the
//   party's size, C = 0, for a loop over the members.
// - member_hurt ($4679): member C takes DE damage (member_take_damage,
//   $467F), then the status window is drawn again ($4324).
// - print_avatar_name ($5650): the Avatar's name printed.
// - person_at ($42B7): which of the first B people of the place ($C651)
//   stands at (L, H): A = its number, or carry for none.
// Cycle counts are the original's, instruction by instruction (I).
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static void push(uint16_t v) { push16(v); I(11); }

static void nth(uint16_t base, uint16_t size, int jp) {   // pushes done: HL = base, DE = size; $48BF
	SET_HL(base); I(10);
	SET_DE(size); I(10);
	if (jp) I(10);
	cpu.b = cpu.c; I(4);
	cpu.b = op_inc8(cpu.b); I(4);
	do {
		SET_HL(op_add16(HL, DE)); I(11);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	push(HL);
	cpu.iy = pop16(); I(14);
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

static void member_record(void) { push(BC); push(DE); push(HL); nth(0xC734, 0x001D, 0); }
static void member_object(void) { push(BC); push(DE); push(HL); nth(0xC8B4, 0x0005, 1); }
static void foe_record(void) { push(BC); push(DE); push(HL); nth(0xC831, 0x0008, 1); }

static void current(void (*fn)(void)) {
	push(BC);
	SET_BC(rd16(0xC97F)); I(20);
	I(17); fn();
	SET_BC(pop16()); I(10);
	I(10);
}
static void current_record(void) { current(member_record); }
static void current_object(void) { current(member_object); }

static void person_record(void) {      // $48F7
	push(BC);
	for (int i = 0; i < 3; i++) { op_add8(cpu.a, 0); I(4); }
	cpu.c = cpu.a; I(4);
	cpu.b = 0; I(7);
	cpu.iy = 0xC651; I(14);
	cpu.iy = op_add16(cpu.iy, BC); I(15);
	SET_BC(pop16()); I(10);
	I(10);
}

static void person_with_graphic(void) {   // $4906
	push(BC); push(DE);
	cpu.c = cpu.a; I(4);
	cpu.a = mem_read(0xC9A0); I(13);
	op_or(cpu.a); I(4);
	if (cpu.f & FZ) {
		I(12);
		cpu.iy = 0xC691; I(14);
		cpu.b = 0x18; I(7);
	} else {
		I(7);
		cpu.iy = 0xC651; I(14);
		cpu.b = 0x20; I(7);
		I(12);
	}
	SET_DE(0x0008); I(10);
	cpu.a = cpu.c; I(4);
	int found = 0;
	do {
		op_sub8(mem_read((uint16_t)(cpu.iy + 3)), 0, 0); I(19);
		if (cpu.f & FZ) { I(12); found = 1; break; }
		I(7);
		cpu.iy = op_add16(cpu.iy, DE); I(15);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	if (found) {
		cpu.a = 0x20; I(7);
		op_sub8(cpu.b, 0, 1); I(4);
	} else {
		op_scf(); I(4);
		I(12);
	}
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

static void party_iter_setup(void) {   // $4098
	cpu.iy = 0xC751; I(14);
	SET_DE(0x001D); I(10);
	cpu.a = mem_read(0xC957); I(13);
	cpu.b = cpu.a; I(4);
	cpu.c = 0x00; I(7);
	I(10);
}

static void member_hurt(void) {        // $4679
	I(17); call_original(0x467F);      // member_take_damage
	I(10);
	native_continue_at(0x4324);        // jp status_window
}

static void print_avatar_name(void) {  // $5650
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: printing may wait for frames
	push(HL);
	SET_HL(0xC75E); I(10);
	I(17); call_original(0x5519);      // print_text_hl
	SET_HL(pop16()); I(10);
	I(10);
}

static void person_at(void) {          // $42B7
	push(BC); push(DE);
	push16(cpu.iy); I(15);
	cpu.iy = 0xC651; I(14);
	SET_DE(0x0008); I(10);
	cpu.c = cpu.b; I(4);
	int found = 0;
	do {
		cpu.a = mem_read((uint16_t)(cpu.iy + 3)); I(19);
		op_or(cpu.a); I(4);
		if (cpu.f & FZ) I(12);
		else {
			I(7);
			cpu.a = mem_read((uint16_t)(cpu.iy + 1)); I(19);
			op_sub8(cpu.l, 0, 0); I(4);
			if (!(cpu.f & FZ)) I(12);
			else {
				I(7);
				cpu.a = mem_read((uint16_t)(cpu.iy + 2)); I(19);
				op_sub8(cpu.h, 0, 0); I(4);
				if (cpu.f & FZ) { I(12); found = 1; break; }
				I(7);
			}
		}
		cpu.iy = op_add16(cpu.iy, DE); I(15);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	if (found) { cpu.a = cpu.c; I(4); op_sub8(cpu.b, 0, 1); I(4); }
	else { op_scf(); I(4); I(12); }
	cpu.iy = pop16(); I(14);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

#define RECORD(name, addr, out) Native native_##name = { #name, addr, 1, name, out, .long_calls = 1 };
RECORD(member_record, 0x48B6, OUT_F | OUT_IY)
RECORD(current_record, 0x48CB, OUT_F | OUT_IY)
RECORD(member_object, 0x48D5, OUT_F | OUT_IY)
RECORD(current_object, 0x48E1, OUT_F | OUT_IY)
RECORD(foe_record, 0x48EB, OUT_F | OUT_IY)
RECORD(person_record, 0x48F7, OUT_A | OUT_F | OUT_IY)
RECORD(person_with_graphic, 0x4906, OUT_A | OUT_F | OUT_IY)
RECORD(party_iter_setup, 0x4098, OUT_A | OUT_BC | OUT_DE | OUT_IY)
RECORD(member_hurt, 0x4679, OUT_A | OUT_F)
RECORD(print_avatar_name, 0x5650, OUT_A | OUT_F | OUT_BC | OUT_DE)
RECORD(person_at, 0x42B7, OUT_A | OUT_F)
