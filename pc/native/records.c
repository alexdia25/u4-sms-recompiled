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
//   (party_lost, $4483: the afterlife, then Lord British raises them);
//   else C = the first who can act, then on to the first able
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
// - is_foe ($430A): carry when figure A is a creature ($80 and up) but
//   not one of the six at $431E.
// - member_step_place ($4CA3): (L, H) = the current member's figure's
//   place one step in direction A.
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

// the most magic points of class `class_` ($47B3's routines, after the
// rst $18 that picks one): A from intelligence C
void class_magic_most(uint8_t class_) {
	switch (rd16((uint16_t)(0x47B3 + 2 * class_))) {
	case 0x47D0: cpu.a = cpu.c; I(4); op_add8(cpu.a, 0); I(4); break;
	case 0x47C9: cpu.a = cpu.c; I(4); break;
	case 0x47C3: op_xor(cpu.a); I(4); break;
	case 0x47CB: cpu.a = cpu.c; I(4); cpu.a = op_rot(7, cpu.a); I(8); op_add8(cpu.c, 0); I(4); break;
	default:     cpu.a = cpu.c; I(4); cpu.a = op_rot(7, cpu.a); I(8); break;   // $47C5
	}
	I(10);
}
static void class_magic(uint8_t class_) {   // rst $18 on the table at $47B3: A from C
	I(11 + 10 + 52);
	class_magic_most(class_);
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

// $4483: the party is lost. Everyone dead (status 'D', hit points 0),
// the screen dissolved and quiet, the six lines of the afterlife ("All is
// Dark...", "But wait...", ..., "You hear: <Avatar's name>", "I feel
// motion...") each followed by a wait of 120 frames; then Lord British's
// words, everyone good ('G') with full hit points, the 16 counts at $C91F
// (weapons owned) and the 8 at $C917 cleared, 200 food (and $63 of the
// next), 200 gold, and the party in location 2 at (19, 8), the way out
// to ($56, $6C) on the world
static void lost_line(uint16_t text) { // ld hl,text / call $4574: the text, then $4577
	I(10); I(17);
	SET_HL(text); I(17); call_original(0x5519);
	cpu.b = 0x14; I(7);
	I(10); call_original(0x042D);      // jp wait_frames_6b: 120 frames
}
static void party_lost(void) {
	cpu.a = mem_read(0xC9A0); I(13);
	op_or(cpu.a); I(4);
	if (cpu.f & FZ) { I(17); call_original(0x40E9); }   // on the world: people_save
	else I(10);
	cpu.iy = 0xC751; I(14);
	SET_DE(0x001D); I(10);
	cpu.a = mem_read(0xC957); I(13);
	cpu.b = cpu.a; I(4);
	do {
		mem_write((uint16_t)(cpu.iy + 0x1C), 0x44); I(19);
		mem_write(cpu.iy, 0); I(19);
		mem_write((uint16_t)(cpu.iy + 1), 0); I(19);
		cpu.iy += 0x1D; I(15);
		I(--cpu.b ? 13 : 8);
	} while (cpu.b);
	static const uint16_t first[] = { 0x4324, 0x041C, 0x0605, 0x4EAC, 0x05E3, 0x041C, 0x54CB, 0x54CB };
	for (unsigned i = 0; i < sizeof first / sizeof *first; i++) { I(17); call_original(first[i]); }
	static const uint16_t lines[] = { 0x457C, 0x458C, 0x459A, 0x45AA, 0x45B9 };
	for (unsigned i = 0; i < 5; i++) lost_line(lines[i]);
	SET_HL(0x45C8); I(10);
	I(17); call_original(0x5519);
	I(17); call_original(0x5650);      // print_avatar_name
	I(17); cpu.b = 0x14; I(7); I(10); call_original(0x042D);
	lost_line(0x45D9);
	SET_HL(0x8A1F); I(10); wr16(0xC959, HL); I(16);
	SET_HL(0x0813); I(10); wr16(0xC953, HL); I(16); wr16(0xC985, HL); I(16);
	SET_HL(0x6C56); I(10); wr16(0xC96D, HL); I(16);
	op_xor(cpu.a); I(4);
	mem_write(0xC976, 0); I(13);
	mem_write(0xC977, 0); I(13);
	mem_write(0xC9A4, 0); I(13);
	mem_write(0xC982, 0); I(13);
	cpu.a = 0xFF; I(7);
	mem_write(0xC98F, 0xFF); I(13);
	SET_HL(0x45EB); I(10);
	I(17); call_original(0x5519);      // Lord British's words
	I(17); call_original(0x54CE);
	cpu.iy = 0xC751; I(14);
	SET_DE(0x001D); I(10);
	cpu.a = mem_read(0xC957); I(13);
	cpu.b = cpu.a; I(4);
	do {
		mem_write((uint16_t)(cpu.iy + 0x1C), 0x47); I(19);
		cpu.a = mem_read((uint16_t)(cpu.iy + 2)); I(19);
		mem_write(cpu.iy, cpu.a); I(19);
		cpu.a = mem_read((uint16_t)(cpu.iy + 3)); I(19);
		mem_write((uint16_t)(cpu.iy + 1), cpu.a); I(19);
		cpu.iy += 0x1D; I(15);
		I(--cpu.b ? 13 : 8);
	} while (cpu.b);
	SET_HL(0xC91F); I(10);             // weapons_owned
	SET_BC(0x1000); I(10);
	I(17); call_original(0x333B);      // fill
	SET_HL(0xC917); I(10);
	SET_BC(0x0800); I(10);
	I(17); call_original(0x333B);
	cpu.a = 0x63; I(7);
	mem_write(0xC906, 0x63); I(13);    // food_fraction
	SET_HL(0x00C8); I(10);
	wr16(0xC907, HL); I(16);           // food
	wr16(0xC909, HL); I(16);           // gold
	I(17); call_original(0x4324);
	cpu.a = 0x02; I(7);
	mem_write(0xC9A0, 0x02); I(13);    // location
	cpu.a = 0x12; I(7);
	mem_write(0xC600, 0x12); I(13);    // game_mode
	cpu.a = 0xFF; I(7);
	mem_write(0xC99F, 0xFF); I(13);    // map_mode
	I(17); call_original(0x0E6C);      // load_location
	I(10); native_continue_at(0x0CD2); // jp main_loop
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

static void is_foe(void) {             // $430A
	push(BC); push(HL);
	op_sub8(0x80, 0, 0); I(7);
	int listed = 0;
	if (cpu.f & FC) I(12);
	else {
		I(7);
		SET_HL(0x431E); I(10);
		SET_BC(0x0006); I(10);
		while (op_block(6, 1)) I(21);  // cpir
		I(16);
		if (cpu.f & FZ) { I(12); listed = 1; } else I(7);
	}
	if (!listed) { op_ccf(); I(4); }
	SET_HL(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

static void member_step_place(void) {  // $4CA3
	I(17); call_original(0x40B9);      // HL = the step
	I(17); call_original(0x48E1);      // current_object
	cpu.a = mem_read(cpu.iy); I(19);
	op_add8(cpu.l, 0); I(4);
	cpu.l = cpu.a; I(4);
	cpu.a = mem_read((uint16_t)(cpu.iy + 1)); I(19);
	op_add8(cpu.h, 0); I(4);
	cpu.h = cpu.a; I(4);
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

// $42E3: person_at ($42B7) over the first 8 people on the world, 32 in a place
static void person_here(void) {
	push(BC);
	cpu.a = mem_read(0xC9A0); I(13);
	op_or(cpu.a); I(4);
	cpu.b = 0x08; I(7);
	if (cpu.f & FZ) I(12);
	else { I(7); cpu.b = 0x20; I(7); }
	I(17); call_original(0x42B7);
	SET_BC(pop16()); I(10);
	I(10);
}

// $62F8: Z for the figures $9C, $BC, $C4, $E4 (the undead and such)
static void undead(void) {
	static const uint8_t kinds[4] = { 0x9C, 0xBC, 0xC4, 0xE4 };
	for (int i = 0; i < 4; i++) {
		op_sub8(kinds[i], 0, 0); I(7);
		if (i == 3) break;
		if (cpu.f & FZ) { I(11); return; }
		I(5);
	}
	I(10);
}

#define RECORD(name, addr, out) Native native_##name = { #name, addr, 1, name, out, .long_calls = 1 };
RECORD(member_record, 0x48B6, OUT_F | OUT_IY)
Native native_party_lost = { "party_lost", 0x4483, 1, party_lost, 0, 0, 0, .blocking = 1 };
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
RECORD(person_here, 0x42E3, OUT_A | OUT_F)
RECORD(undead, 0x62F8, OUT_F)
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
RECORD(is_foe, 0x430A, OUT_A | OUT_F)
RECORD(member_step_place, 0x4CA3, OUT_A | OUT_F | OUT_HL | OUT_IY)

// The short messages of bank 1: sound $0D ($40D1, except $4719), then the
// text after the call to print_inline: "Pass" ($46B5), "Not Here!"
// ($46DF), "Disabled!" ($46F1), "Only on foot!" ($4703), "Slow progress!"
// ($4719), "Aborted!" ($472D), "Drift only!" ($473F, whose text runs
// over the code at $474E to the ret at $4752)
static void message(int sound, uint16_t text) {
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: printing may wait for frames
	if (sound) { I(17); call_original(0x40D1); }
	uint16_t hl = HL;
	I(17 + 19 + 17);                   // call print_inline
	text_print(text);
	I(19 + 10);
	SET_HL(hl);
	I(10);                             // ret
}
static void msg_pass(void) { message(1, 0x46BB); }
static void msg_not_here(void) { message(1, 0x46E5); }
static void msg_disabled(void) { message(1, 0x46F7); }
static void msg_only_on_foot(void) { message(1, 0x4709); }
static void msg_slow_progress(void) { message(0, 0x471C); }
static void msg_aborted(void) { message(1, 0x4733); }
static void msg_drift_only(void) { message(1, 0x4745); }
#define MESSAGE(name, addr) Native native_##name = { #name, addr, 1, name, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, .long_calls = 1 };
MESSAGE(msg_pass, 0x46B5)
MESSAGE(msg_not_here, 0x46DF)
MESSAGE(msg_disabled, 0x46F1)
MESSAGE(msg_only_on_foot, 0x4703)
MESSAGE(msg_slow_progress, 0x4719)
MESSAGE(msg_aborted, 0x472D)
MESSAGE(msg_drift_only, 0x473F)

// $55A0: word A of the table at $55AA printed (the directions, ...)
static void say_word(void) {
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: printing may wait for frames
	push(HL);
	SET_HL(0x55AA); I(10);
	I(11); call_original(0x0020);      // rst $20
	I(17); call_original(0x5519);      // print_text_hl
	SET_HL(pop16()); I(10);
	I(10);
}

// $4C28: "Direction: ", and the picker's cursor on 4 (none yet)
static void direction_prompt(void) {
	if (!native_on_fiber()) { native_decline(); return; }
	uint16_t hl = HL;
	I(17 + 19 + 17);                   // call print_inline
	text_print(0x4C2B);
	I(19 + 10);
	SET_HL(hl);
	mem_write((uint16_t)(cpu.ix + 0x1A), 0x04); I(19);
	I(10);
}

// $4C3C: the direction picker's step, on the keys newly pressed ($4C42)
static void direction_step(void) {
	cpu.a = mem_read(0xC0B0); I(13);
	SET_HL(0x4C51); I(10);
	native_continue_at(0x4C42);
}

// $548D: a quotation mark printed (print_char)
static void print_quote(void) {
	cpu.a = 0x22; I(7);
	native_continue_at(0x548F);
}

// $5DCE: after a spell: the status window, and in combat the caster's line
// highlighted again ($49A6) and a frame
static void spell_after(void) {
	if (!native_on_fiber()) { native_decline(); return; }
	I(17); call_original(0x4324);
	cpu.a = mem_read(0xC99F); I(13);
	op_sub8(0x03, 0, 0); I(7);
	if (cpu.f & FC) { I(11); return; }
	I(5);
	SET_BC(rd16(0xC97F)); I(20);
	I(17); call_original(0x49A6);      // member_flash
	I(10); native_continue_at(0x041C); // jp wait_vblank
}

Native native_say_word = { "say_word", 0x55A0, 1, say_word, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, .long_calls = 1 };
Native native_direction_prompt = { "direction_prompt", 0x4C28, 1, direction_prompt, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, .long_calls = 1 };
Native native_direction_step = { "direction_step", 0x4C3C, 1, direction_step, OUT_A | OUT_F | OUT_HL, .long_calls = 1 };
Native native_print_quote = { "print_quote", 0x548D, 1, print_quote, OUT_A, .long_calls = 1 };
Native native_spell_after = { "spell_after", 0x5DCE, 1, spell_after, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, .long_calls = 1 };

// $4887: every member's line of the status window back to normal ($4999),
// then a frame
static void party_unflash(void) {
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: it waits for a frame
	cpu.a = mem_read(0xC957); I(13);
	cpu.b = cpu.a; I(4);
	cpu.c = 0x00; I(7);
	do {
		I(17); call_original(0x4999);  // member_unflash
		cpu.c = op_inc8(cpu.c); I(4);
		I(--cpu.b ? 13 : 8);
	} while (cpu.b);
	I(10); native_continue_at(0x041C); // jp wait_vblank
}

// $4812: harm from fire or lava: the party flashes, a sound, the screen
// shakes; on a ship ($C959 below $14) the hull loses 10 ($C961), and
// below 0 "Thy Ship Sinks!": the party is lost ($4483); else each member
// alive, one time in two, loses 10-24 hit points
static void harm(void) {
	if (!native_on_fiber()) { native_decline(); return; }
	push(BC); push(DE); push(HL);
	push16(cpu.iy); I(15);
	I(17); call_original(0x4896);      // party_flash
	cpu.a = 0x13; I(7);
	I(17); call_original(0x05F1);      // play_sound
	I(17); call_original(0x4117);      // shake
	I(17); call_original(0x4887);      // party_unflash
	cpu.a = mem_read(0xC959); I(13);
	op_sub8(0x14, 0, 0); I(7);
	if (!(cpu.f & FC)) {
		I(7);
		cpu.iy = 0xC751; I(14);
		SET_DE(0x001D); I(10);
		cpu.a = mem_read(0xC957); I(13);
		cpu.b = cpu.a; I(4);
		cpu.c = 0x00; I(7);
		do {
			cpu.a = mem_read((uint16_t)(cpu.iy + 0x1C)); I(19);
			op_sub8(0x44, 0, 0); I(7);
			if (cpu.f & FZ) I(12);
			else {
				I(7);
				I(17); call_original(0x2BBF);   // random
				op_and(0x01); I(7);
				if (cpu.f & FZ) I(12);
				else {
					I(7);
					cpu.a = 0x0F; I(7);
					I(17); call_original(0x2BEB);   // random_below
					op_add8(0x0A, 0); I(7);
					cpu.l = cpu.a; I(4);
					cpu.h = 0x00; I(7);
					I(17); call_original(0x4679);   // member_hurt
				}
			}
			cpu.iy = op_add16(cpu.iy, DE); I(15);
			cpu.c = op_inc8(cpu.c); I(4);
			I(--cpu.b ? 13 : 8);
		} while (cpu.b);
		I(12);
	} else {
		I(12);
		SET_HL(rd16(0xC961)); I(16);
		SET_DE(0x000A); I(10);
		op_or(cpu.a); I(4);
		op_sbc16(DE); I(15);
		wr16(0xC961, HL); I(16);
		if (cpu.f & FC) {
			I(7);
			SET_HL(0x0000); I(10);
			wr16(0xC961, HL); I(16);
			I(17); call_original(0x4896);
			I(17); call_original(0x4324);
			SET_HL(0x48A5); I(10);     // "Thy Ship Sinks!"
			I(17); call_original(0x5519);
			I(10); native_continue_at(0x4483);
			return;
		}
		I(12);
	}
	I(17); call_original(0x4324);      // $487E: the status window
	cpu.iy = pop16(); I(14);
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}
Native native_party_unflash = { "party_unflash", 0x4887, 1, party_unflash, OUT_A | OUT_F | OUT_BC, .long_calls = 1 };
Native native_harm = { "harm", 0x4812, 1, harm, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, .long_calls = 1 };

// ------------------------------------------------------------- chests --
// open_chest ($5B13, the map; $5978, in combat): the chest under the party
// goes (a dungeon's cell $40; a town's cell $3C, which costs Honesty,
// Justice and Honor, or else the chest among the people; in combat the
// battlefield's) or "Not Here!"; then chest_opened
#define C(addr) do { I(17); call_original(addr); } while (0)

static void say_text(uint16_t text) {  // call print_inline / the text follows
	uint16_t hl = HL;
	I(17 + 19 + 17);
	text_print(text);
	I(19 + 10);
	SET_HL(hl);
}

// $59CB: "The Chest Holds: <10-98> Gold."
static void chest_gold(void) {
	say_text(0x59CE);
	cpu.a = 0x50; I(7);
	C(0x2BEB);                         // random_below
	cpu.b = cpu.a; I(4);
	cpu.a = 0x0A; I(7);
	C(0x2BEB);
	op_add8(cpu.b, 0); I(4);
	op_add8(0x0A, 0); I(7);
	cpu.l = cpu.a; I(4);
	cpu.h = 0x00; I(7);
	push(HL);
	C(0x5627);                         // print_number_auto
	say_text(0x59F8);
	SET_HL(pop16()); I(10);
	I(10); native_continue_at(0x4412); // jp receive_gold
}

static void trap_acid(void) {          // $59AC: the member hurt, up to 29
	cpu.a = 0x13; I(7);
	C(0x05F1);
	C(0x4324);
	C(0x49A6);                         // member_flash
	C(0x041C);
	cpu.a = 0x1E; I(7);
	C(0x2BEB);
	cpu.l = cpu.a; I(4);
	cpu.h = 0x00; I(7);
	C(0x4679);                         // member_hurt
	C(0x041C);
	I(10); call_original(0x4324);      // jp
}

static void trap_status(uint8_t status) {   // $5A6B sleep, $5A8D poison
	cpu.c = mem_read((uint16_t)(cpu.ix + 0x08)); I(19);
	C(0x48B6);                         // member_record
	mem_write((uint16_t)(cpu.iy + 0x1C), status); I(19);
	C(0x4324);
	if (status == 0x53) {
		cpu.a = mem_read(0xC99F); I(13);
		op_sub8(0x03, 0, 0); I(7);
		if (cpu.f & FC) I(12);
		else {                         // the figure lies down
			I(7);
			C(0x48D5);                 // member_object
			mem_write((uint16_t)(cpu.iy + 2), 0x38); I(19);
			mem_write((uint16_t)(cpu.iy + 3), 0x38); I(19);
		}
	}
	I(10); call_original(0x4247);      // jp
}

// $5A9D: a bomb: on the map as fire ($4812); in combat everyone on the
// field flashes, the screen shakes, and each who can act, one time in
// two, loses 10-24
static void trap_bomb(void) {
	cpu.a = mem_read(0xC99F); I(13);
	op_sub8(0x03, 0, 0); I(7);
	I(10);
	if (cpu.f & FC) { call_original(0x4812); return; }
	cpu.a = mem_read(0xC957); I(13);
	cpu.b = cpu.a; I(4);
	cpu.c = 0x00; I(7);
	do {
		C(0x48D5);
		cpu.a = mem_read((uint16_t)(cpu.iy + 2)); I(19);
		op_or(cpu.a); I(4);
		if (!(cpu.f & FZ)) C(0x49A6);
		else I(10);
		cpu.c = op_inc8(cpu.c); I(4);
		I(--cpu.b ? 13 : 8);
	} while (cpu.b);
	C(0x041C);
	cpu.a = 0x13; I(7);
	C(0x05F1);
	C(0x4117);                         // shake
	C(0x4887);                         // party_unflash
	cpu.a = mem_read(0xC957); I(13);
	cpu.b = cpu.a; I(4);
	cpu.c = 0x00; I(7);
	do {
		C(0x48D5);
		cpu.a = mem_read((uint16_t)(cpu.iy + 2)); I(19);
		op_or(cpu.a); I(4);
		if (cpu.f & FZ) I(12);
		else {
			I(7);
			C(0x4445);                 // member_can_act
			if (cpu.f & FC) I(12);
			else {
				I(7);
				C(0x2BBF);
				op_and(0x01); I(7);
				if (cpu.f & FZ) I(12);
				else {
					I(7);
					cpu.a = 0x0F; I(7);
					C(0x2BEB);
					op_add8(0x0A, 0); I(7);
					cpu.l = cpu.a; I(4);
					cpu.h = 0x00; I(7);
					C(0x4679);
				}
			}
		}
		cpu.c = op_inc8(cpu.c); I(4);
		I(--cpu.b ? 13 : 8);
	} while (cpu.b);
	I(10); call_original(0x4324);      // jp
}

// $5A04: half the chests are trapped: "Acid Trap!" (rarely the others:
// random 0-3 and random 0-3): evaded when no member is opening ($FF) or
// by dexterity + 25 in 100 ("Evaded!"); then the gold
static void chest_opened(void) {
	C(0x2BBF);                         // random
	op_and(0x01); I(7);
	if (cpu.f & FZ) { I(12); chest_gold(); return; }
	I(7);
	C(0x2BBF);
	op_and(0x03); I(7);
	cpu.b = cpu.a; I(4);
	C(0x2BBF);
	op_and(cpu.b); I(4);
	mem_write((uint16_t)(cpu.ix + 0x1A), cpu.a); I(19);
	SET_HL(0x5AF4); I(10);
	I(11); call_original(0x0020);      // rst $20: its name
	C(0x5519);
	say_text(0x5A22);                  // " Trap!"
	cpu.a = mem_read((uint16_t)(cpu.ix + 0x08)); I(19);
	op_sub8(0xFF, 0, 0); I(7);
	int evaded = 1;
	if (cpu.f & FZ) I(12);
	else {
		I(7);
		cpu.c = cpu.a; I(4);
		C(0x48B6);
		cpu.a = mem_read((uint16_t)(cpu.iy + 7)); I(19);
		op_add8(0x19, 0); I(7);
		cpu.b = cpu.a; I(4);
		cpu.a = 0x64; I(7);
		C(0x2BEB);
		op_sub8(cpu.b, 0, 0); I(4);
		if (!(cpu.f & FC)) { I(12); evaded = 0; }
		else I(7);
	}
	if (evaded) {
		cpu.a = 0x19; I(7);
		C(0x05F1);
		say_text(0x5A4B);              // "Evaded!"
		I(12);
	} else {
		uint8_t trap = mem_read((uint16_t)(cpu.ix + 0x1A));
		cpu.a = trap; I(19);
		SET_HL(0x5A60); I(10);
		I(11 + 10 + 52);               // rst $18
		switch (trap) {
		case 0: I(10); trap_acid(); break;     // (jp $59AC)
		case 1: trap_status(0x53); break;
		case 2: trap_status(0x50); break;
		default: trap_bomb(); break;
		}
	}
	I(10);
	chest_gold();
}

static void open_chest(void) {         // $5B13
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: printing may wait for frames
	cpu.a = mem_read(0xC99F); I(13);
	op_sub8(0x02, 0, 0); I(7);
	if (cpu.f & FZ) {                  // a dungeon
		I(7);
		cpu.a = mem_read(0xC990); I(13);
		op_sub8(0x40, 0, 0); I(7);
		I(10);
		if (!(cpu.f & FZ)) { native_continue_at(0x46DF); return; }
		SET_HL(rd16(0xC953)); I(16);
		C(0x6DC0);                     // cell_address
		mem_write(HL, 0x00); I(10);
		I(10);
		chest_opened();
		return;
	}
	I(12);
	cpu.a = mem_read(0xC9A0); I(13);
	op_sub8(0x00, 0, 0); I(7);
	int on_ground = 0;
	if (cpu.f & FZ) I(12);
	else {
		I(7);
		SET_HL(rd16(0xC953)); I(16);
		C(0xB5A4);                     // town_cell
		cpu.a = mem_read(HL); I(7);
		op_sub8(0x3C, 0, 0); I(7);
		if (!(cpu.f & FZ)) I(12);
		else {
			I(7);
			on_ground = 1;
			mem_write(HL, 0x3E); I(10);
			cpu.l = op_inc8(cpu.l); I(4);
			mem_write(HL, 0x00); I(10);
			static const uint8_t virtue[3] = { 0, 3, 5 };
			for (int i = 0; i < 3; i++) {
				cpu.a = 0x01; I(7);
				cpu.c = virtue[i]; I(7);
				C(0x426F);             // karma_lose
			}
			I(12);
		}
	}
	if (!on_ground) {                  // $5B5B: among the people
		SET_HL(rd16(0xC953)); I(16);
		cpu.iy = 0xC651; I(14);
		SET_DE(0x0008); I(10);
		cpu.b = 0x20; I(7);
		int found = 0;
		do {
			cpu.a = mem_read((uint16_t)(cpu.iy + 3)); I(19);
			op_sub8(0x3C, 0, 0); I(7);
			if (!(cpu.f & FZ)) I(12);
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
			I(--cpu.b ? 13 : 8);
		} while (cpu.b);
		if (!found) { I(10); native_continue_at(0x46DF); return; }
		mem_write((uint16_t)(cpu.iy + 3), 0); I(19);
		mem_write((uint16_t)(cpu.iy + 0), 0); I(19);
	}
	I(10);
	chest_opened();
}

static void open_chest_combat(void) {  // $5978
	if (!native_on_fiber()) { native_decline(); return; }
	C(0x48E1);                         // current_object
	cpu.l = mem_read(cpu.iy); I(19);
	cpu.h = mem_read((uint16_t)(cpu.iy + 1)); I(19);
	C(0x38F2);                         // arena_cell
	cpu.a = mem_read(HL); I(7);
	op_sub8(0x3C, 0, 0); I(7);
	I(10);
	if (!(cpu.f & FZ)) { native_continue_at(0x46DF); return; }
	mem_write(HL, 0x16); I(10);
	cpu.l = op_inc8(cpu.l); I(4);
	mem_write(HL, 0x00); I(10);
	cpu.a = mem_read(0xC9A0); I(13);
	op_sub8(0x12, 0, 0); I(7);
	if (cpu.f & FC) I(12);
	else {                             // in a dungeon's room: its chest cell too
		I(7);
		SET_HL(rd16(0xC953)); I(16);
		C(0x6DC0);
		cpu.a = mem_read(HL); I(7);
		op_sub8(0x40, 0, 0); I(7);
		if (!(cpu.f & FZ)) I(12);
		else { I(7); mem_write(HL, 0x00); I(10); }
	}
	cpu.a = mem_read(0xC97F); I(13);
	mem_write((uint16_t)(cpu.ix + 0x08), cpu.a); I(19);
	I(10);
	chest_opened();
}
Native native_open_chest = { "open_chest", 0x5B13, 1, open_chest, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, .long_calls = 1 };
Native native_open_chest_combat = { "open_chest_combat", 0x5978, 1, open_chest_combat, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, .long_calls = 1 };

static void member_pick_start(void) {  // $4BCB: the pick starts on the first, and the status window
	if (!native_on_fiber()) { native_decline(); return; }
	mem_write((uint16_t)(cpu.ix + 0x08), 0x00); I(19);
	I(10); native_continue_at(0x4324);
}

// $4247: member C's line flashes with a sound, and back
static void member_flash_sound(void) {
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: it waits for a frame
	C(0x49A6);
	cpu.a = 0x13; I(7);
	C(0x05F1);
	C(0x041C);
	I(10); native_continue_at(0x4999);
}

// $ABD2 (bank 2): HL more food, up to 9999 ($63 of the next at the top)
static void food_add(void) {
	if (!native_on_fiber()) { native_decline(); return; }
	SET_DE(rd16(0xC907)); I(20);
	SET_HL(op_add16(HL, DE)); I(11);
	C(0x36DA);                         // cap_9999
	wr16(0xC907, HL); I(16);
	if (!(cpu.f & FC)) I(12);
	else {
		I(7);
		cpu.a = 0x63; I(7);
		mem_write(0xC906, 0x63); I(13);
	}
	I(10); native_continue_at(0x43F6);
}

// $9DE3 (bank 3): the status window, the current member's line again, a frame
static void member_shown(void) {
	if (!native_on_fiber()) { native_decline(); return; }
	C(0x7F29);
	SET_BC(rd16(0xC97F)); I(20);
	C(0x7E66);
	I(10); native_continue_at(0x041C);
}

Native native_member_pick_start = { "member_pick_start", 0x4BCB, 1, member_pick_start, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, .long_calls = 1 };
Native native_member_flash_sound = { "member_flash_sound", 0x4247, 1, member_flash_sound, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, .long_calls = 1 };
Native native_food_add = { "food_add", 0xABD2, 2, food_add, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, .long_calls = 1 };
Native native_member_shown = { "member_shown", 0x9DE3, 3, member_shown, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, .long_calls = 1 };
