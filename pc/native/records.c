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
// - eat ($4753): A hundredths of food eaten ($C906, then the 2-byte food
//   at $C907); none left: all three bytes 0.
// - magic_regain ($4776): each member not dead ($1C != $44) gains a magic
//   point, up to its class's most (the jump table at $47B3, from its
//   intelligence: x2, x1, 0, x1.5 or x0.5), 99 at most.
// - party_alive ($465D): when no member can act the party is lost
//   ($4483); else C = the first who can act, then on to the first able
//   one ($4466).
// - member_row ($49B3): D, E, B, C = member C's line of the status
//   window (row C * 2 + 1, column 18, 2 x 14); member_flash ($49A6): that
//   line highlighted; party_flash ($4896): every member's, then a wait
//   for the vertical blank.
// - member_graphic ($42F3): A = member C's figure: $38 asleep, else by
//   its class ($20 + class * 2).
// - member_unflash ($4999): member C's line of the status window back to
//   normal (highlight_off).
// - people_save ($40E9) / people_load ($4100): the place's 256 bytes of
//   people ($C651) to or from the battery RAM at $8200.
// - figure_name ($47D3): the name of figure A, by item_name ($340F):
//   foes $90 and up by fours ($05 + ...), $80-$8F by twos ($01 + ...), $38
//   ($9F), people $20-$2F and $50-$5F by twos ($4D + ...), others $14.
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

static void eat(void) {                // $4753
	push(BC); push(HL);
	cpu.b = cpu.a; I(4);
	SET_HL(0xC906); I(10);
	cpu.a = mem_read(HL); I(7);
	op_sub8(cpu.b, 0, 1); I(4);
	mem_write(HL, cpu.a); I(7);
	if (!(cpu.f & FC)) I(12);
	else {
		I(7);
		op_add8(0x64, 0); I(7);
		mem_write(HL, cpu.a); I(7);
		op_scf(); I(4);
		SET_BC(0x0200); I(10);
		do {                           // the borrow through the food
			SET_HL(HL + 1); I(6);
			cpu.a = mem_read(HL); I(7);
			op_sub8(cpu.c, cpu.f & FC, 1); I(4);
			mem_write(HL, cpu.a); I(7);
			if (--cpu.b) I(13); else I(8);
		} while (cpu.b);
		if (!(cpu.f & FC)) I(12);
		else {
			I(7);
			cpu.b = 0x03; I(7);
			do {                       // none left
				mem_write(HL, cpu.c); I(7);
				SET_HL(HL - 1); I(6);
				if (--cpu.b) I(13); else I(8);
			} while (cpu.b);
		}
	}
	SET_HL(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

static void class_magic(uint8_t class_) {   // rst $18 on the table at $47B3: A from C
	I(11 + 10 + 52);
	switch (rd16((uint16_t)(0x47B3 + 2 * class_))) {
	case 0x47D0: cpu.a = cpu.c; I(4); op_add8(cpu.a, 0); I(4); break;
	case 0x47C9: cpu.a = cpu.c; I(4); break;
	case 0x47C3: op_xor(cpu.a); I(4); break;
	case 0x47CB: cpu.a = cpu.c; I(4); cpu.a = op_rot(7, cpu.a); I(8); op_add8(cpu.c, 0); I(4); break;
	default:     cpu.a = cpu.c; I(4); cpu.a = op_rot(7, cpu.a); I(8); break;   // $47C5
	}
	I(10);
}

static void magic_regain(void) {       // $4776
	push(BC); push(DE); push(HL);
	push16(cpu.iy); I(15);
	cpu.iy = 0xC751; I(14);
	SET_DE(0x001D); I(10);
	cpu.a = mem_read(0xC957); I(13);
	cpu.b = cpu.a; I(4);
	do {
		cpu.a = mem_read((uint16_t)(cpu.iy + 0x1C)); I(19);
		op_sub8(0x44, 0, 0); I(7);
		if (cpu.f & FZ) I(12);
		else {
			I(7);
			cpu.c = mem_read((uint16_t)(cpu.iy + 0x08)); I(19);
			cpu.a = mem_read((uint16_t)(cpu.iy + 0x1B)); I(19);
			SET_HL(0x47B3); I(10);
			uint8_t class_ = cpu.a;
			SET_HL(rd16((uint16_t)(0x47B3 + 2 * class_)));   // as rst $18 leaves it
			class_magic(class_);
			op_sub8(0x64, 0, 0); I(7);
			if (cpu.f & FC) I(12);
			else { I(7); cpu.a = 0x63; I(7); }
			cpu.c = cpu.a; I(4);
			cpu.a = mem_read((uint16_t)(cpu.iy + 0x09)); I(19);
			cpu.a = op_inc8(cpu.a); I(4);
			op_sub8(cpu.c, 0, 0); I(4);
			if (cpu.f & FC) I(12);
			else { I(7); cpu.a = cpu.c; I(4); }
			mem_write((uint16_t)(cpu.iy + 0x09), cpu.a); I(19);
		}
		cpu.iy = op_add16(cpu.iy, DE); I(15);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	cpu.iy = pop16(); I(14);
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

static void party_alive(void) {        // $465D
	push(BC);
	cpu.a = mem_read(0xC957); I(13);
	cpu.b = cpu.a; I(4);
	cpu.c = 0x00; I(7);
	for (;;) {
		I(17); call_original(0x4445);  // member_can_act
		if (!(cpu.f & FC)) { I(12); break; }
		I(7);
		cpu.c = op_inc8(cpu.c); I(4);
		if (--cpu.b) I(13);
		else { I(8); I(10); native_continue_at(0x4483); return; }   // nobody: the party is lost
	}
	for (;;) {
		I(17); call_original(0x4466);  // member_able
		if (!(cpu.f & FC)) { I(12); break; }
		I(7);
		cpu.c = op_inc8(cpu.c); I(4);
		if (--cpu.b) I(13); else { I(8); break; }
	}
	SET_BC(pop16()); I(10);
	I(10);
}

static void member_row(void) {         // $49B3
	cpu.a = cpu.c; I(4);
	op_add8(cpu.a, 0); I(4);
	cpu.a = op_inc8(cpu.a); I(4);
	cpu.d = cpu.a; I(4);
	cpu.e = 0x12; I(7);
	SET_BC(0x020E); I(10);
	I(10);
}

static void member_flash(void) {       // $49A6
	push(BC); push(DE); push(HL);
	I(17); member_row();
	I(17); call_original(0x2DE3);      // highlight_on
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

static void party_flash(void) {        // $4896
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: it waits for a frame
	cpu.a = mem_read(0xC957); I(13);
	cpu.b = cpu.a; I(4);
	cpu.c = 0x00; I(7);
	do {
		I(17); member_flash();
		cpu.c = op_inc8(cpu.c); I(4);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	I(10);
	native_continue_at(0x041C);        // jp wait_vblank
}

static void member_graphic(void) {     // $42F3
	push16(cpu.iy); I(15);
	I(17); call_original(0x48B6);      // member_record
	cpu.a = mem_read((uint16_t)(cpu.iy + 0x1C)); I(19);
	op_sub8(0x53, 0, 0); I(7);
	cpu.a = 0x38; I(7);
	if (cpu.f & FZ) I(12);
	else {
		I(7);
		cpu.a = mem_read((uint16_t)(cpu.iy + 0x1B)); I(19);
		op_add8(cpu.a, 0); I(4);
		op_add8(0x20, 0); I(7);
	}
	cpu.iy = pop16(); I(14);
	I(10);
}

static void member_unflash(void) {     // $4999
	push(BC); push(DE); push(HL);
	I(17); member_row();
	I(17); call_original(0x2DC4);      // highlight_off
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

static void people_copy(uint16_t from, uint16_t to) {   // $40E9 / $4100
	I(17); call_original(0x0C2D);      // sram_on
	push(BC); push(DE); push(HL);
	SET_HL(from); I(10);
	SET_DE(to); I(10);
	SET_BC(0x0100); I(10);
	while (op_block(6, 0)) I(21);      // ldir
	I(16);
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
	native_continue_at(0x0C35);        // jp sram_off
}
static void people_save(void) { people_copy(0xC651, 0x8200); }
static void people_load(void) { people_copy(0x8200, 0xC651); }

static void figure_name(void) {        // $47D3
	op_sub8(0x80, 0, 0); I(7);
	if (!(cpu.f & FC)) {
		I(7);
		op_sub8(0x90, 0, 0); I(7);
		if (!(cpu.f & FC)) {
			I(7);
			op_sub8(0x80, 0, 1); I(7);
			cpu.a = op_rot(7, cpu.a); I(8);
			cpu.a = op_rot(7, cpu.a); I(8);
			op_add8(0x05, 0); I(7);
		} else {
			I(12);
			op_sub8(0x80, 0, 1); I(7);
			cpu.a = op_rot(7, cpu.a); I(8);
			op_add8(0x01, 0); I(7);
		}
		I(12);
	} else {
		I(12);
		op_sub8(0x38, 0, 0); I(7);
		if (cpu.f & FZ) { I(7); cpu.a = 0x9F; I(7); I(12); }
		else {
			I(12);
			int other = 0;
			op_sub8(0x20, 0, 0); I(7);
			if (cpu.f & FC) { I(12); other = 1; }
			else {
				I(7);
				op_sub8(0x60, 0, 0); I(7);
				if (!(cpu.f & FC)) { I(12); other = 1; }
				else {
					I(7);
					op_sub8(0x30, 0, 0); I(7);
					if (cpu.f & FC) I(12);
					else {
						I(7);
						op_sub8(0x50, 0, 0); I(7);
						if (cpu.f & FC) { I(12); other = 1; } else I(7);
					}
				}
			}
			if (other) { cpu.a = 0x14; I(7); }
			else {
				op_and(0x1F); I(7);
				cpu.a = op_rot(7, cpu.a); I(8);
				op_add8(0x4D, 0); I(7);
				I(12);
			}
		}
	}
	I(10);
	native_continue_at(0x340F);        // jp item_name
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
RECORD(eat, 0x4753, OUT_A | OUT_F)
RECORD(magic_regain, 0x4776, OUT_A | OUT_F)
RECORD(party_alive, 0x465D, OUT_A | OUT_F)
RECORD(member_row, 0x49B3, OUT_A | OUT_F | OUT_BC | OUT_DE)
RECORD(member_flash, 0x49A6, OUT_A | OUT_F)
RECORD(party_flash, 0x4896, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
RECORD(member_graphic, 0x42F3, OUT_A | OUT_F)
RECORD(member_unflash, 0x4999, OUT_A | OUT_F)
RECORD(people_save, 0x40E9, OUT_F)
RECORD(people_load, 0x4100, OUT_F)
RECORD(figure_name, 0x47D3, OUT_A | OUT_F | OUT_HL)
