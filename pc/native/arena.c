// The battlefield (bank 0).
// - load_arena ($377E): arena A from bank 5 (the table of words at $A4DF):
//   the 16 foes' places (x, y) into their records ($C839, 8 bytes each,
//   their graphic and its frame cleared), the 8 members' places into their
//   figures ($C8B9, 5 bytes each, graphics cleared), then the arena's map
//   to $D800 (decode_location_map) and its edge bits (terrain_edges).
// - load_dungeon_room ($3D82): room C9B8 of the current dungeon (from the
//   dungeon's table, $2A6E): its four triggers (a byte and three packed
//   places each) to $C8E1; its 16 foes (graphic, $AC shown as $3C, place,
//   hit points from the strength table $3CFB: a random part and half the
//   rest); the members' places for the side the party came in by ($C9B9),
//   each member's graphic if it can act; the room's map to $D800 with its
//   edge bits, tiles $38 and $3C given the edges $90 and $92. In the
//   Abyss's last room (room 15, not the Abyss itself) the party is told
//   where it stands: $C984 = 0, 1 or 2 by its column, else $FF.
// - target_in_pattern ($A279, bank 3): whether the target (IX+$20, $21)
//   is one of the offsets (x, y pairs at HL, ended by $80) from the one
//   acting (IX+$1E, $1F): Z and no carry when it is, Z and carry at the
//   list's end.
// - damage_monster ($39B5): foe C takes A damage, told on a new line with
//   its name; when it dies (unless it is $5E, which can't be hurt) it is
//   "Killed!", and member B (unless $FF) gains experience by its strength
//   ($3CFB's high nibble + 1); else how it fares, by its hit points left
//   against its strength (the strings at $3A42).
// - foe_can_step ($A9E2, bank 3): whether foe C may step to (L, H): off
//   the battlefield only when it is weak enough to flee (hit points below
//   $18); not onto a hazard, nor (most of the time) where it stands, nor
//   onto another foe or a member; then by its kind (graphic below $80:
//   where people walk, $3EB7; below $8E: water; others by the table at
//   $AA93, left to the original). Carry: it may not.
// - compact_records ($B868, bank 3): of the B 8-byte records at HL, the
//   empty ones (first byte 0) taken out, the rest moved up, zeroes after.
// - missile_start ($38FC): the missile's glyphs queued ($30DA), its place
//   on screen from (L, H) (IX+$0B, $0D: $3991), its step towards (E, D)
//   (aim, four times over: IX+$0E/$0F, $10/$11), 8 steps (IX+9), and
//   sound $18.
// - times_11_b3 ($B856) and times_12_b3 ($B860): A times 11 or 12 (bank
//   3's copies of $36ED and $36F7).
// - divide_16_b3 ($B7EA, bank 3's own): HL = HL / DE, DE = the remainder
//   (HL = 0 when DE is 0), by two runs of divide_step ($B805: eight bits
//   of A into the remainder HL, A the quotient's bits).
// - east_wrap ($389F), west_wrap ($38A7), south_wrap ($38AE),
//   north_wrap ($38B6): E (x) or D (y) one step on the 11x11 battlefield,
//   round to the other side; carry when it stayed on.
// - arena_cell ($38F2): HL = the cell (L, H) of the battlefield's map at
//   $D800; arena_cell_at ($38EB): A = what is there.
// Cycle counts are the original's, instruction by instruction (I).
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define ROOM_MONSTERS  0xC839
#define PARTY_OBJECTS  0xC8B9

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static void push(uint16_t v) { push16(v); I(11); }
static void call(uint16_t addr) { I(17); call_original(addr); }

static void add_a_to_hl(void) {        // add a,l / ld l,a / adc a,h / sub l / ld h,a
	op_add8(cpu.l, 0); I(4);
	cpu.l = cpu.a; I(4);
	op_add8(cpu.h, cpu.f & FC); I(4);
	op_sub8(cpu.l, 0, 1); I(4);
	cpu.h = cpu.a; I(4);
}
static void rst20(void) {              // HL = the word HL[A]
	I(11); I(10);
	op_add8(cpu.a, 0); I(4);
	if (!(cpu.f & FC)) I(12);
	else { I(7); cpu.h = op_inc8(cpu.h); I(4); }
	add_a_to_hl();
	cpu.a = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	cpu.h = mem_read(HL); I(7);
	cpu.l = cpu.a; I(4);
	I(10);
}

static void rst28(void) {              // A = (HL + A)
	I(11);
	add_a_to_hl();
	cpu.a = mem_read(HL); I(7);
	I(10);
}
static void rst30(void) {              // HL += A
	I(11);
	op_add8(cpu.l, 0); I(4);
	cpu.l = cpu.a; I(4);
	if (!(cpu.f & FC)) { I(11); return; }
	I(5);
	cpu.h = op_inc8(cpu.h); I(4);
	I(10);
}

static void say_inline(uint16_t text) {   // call print_inline / the text follows
	uint16_t hl = HL;
	uint8_t b = cpu.b, c = cpu.c;
	cpu.cycles += 17 + 19 + 17;
	text_print(text);
	cpu.cycles += 19 + 10;
	cpu.b = b; cpu.c = c;
	cpu.a = 0;
	SET_HL(hl);
}

// n records of IY's from the pairs at HL, the two bytes after the pair cleared
static void places(uint16_t records, uint16_t size, uint8_t n, int8_t cleared) {
	cpu.iy = records; I(14);
	SET_DE(size); I(10);
	cpu.b = n; I(7);
	do {
		cpu.a = mem_read(HL); I(7);
		SET_HL(HL + 1); I(6);
		mem_write(cpu.iy, cpu.a); I(19);
		cpu.a = mem_read(HL); I(7);
		SET_HL(HL + 1); I(6);
		mem_write((uint16_t)(cpu.iy + 1), cpu.a); I(19);
		mem_write((uint16_t)(cpu.iy + cleared), 0x00); I(19);
		mem_write((uint16_t)(cpu.iy + cleared + 1), 0x00); I(19);
		cpu.iy = op_add16(cpu.iy, DE); I(15);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
}

static void load_arena(void) {         // $377E
	op_ex_af(); I(4);
	cpu.a = 0x05; I(7);
	vram_bank_push(0x3781);
	op_ex_af(); I(4);
	push(BC); push(DE); push(HL);
	push16(cpu.iy); I(15);
	SET_HL(0xA4DF); I(10);
	rst20();
	places(ROOM_MONSTERS, 0x0008, 0x10, 5);
	places(PARTY_OBJECTS, 0x0005, 0x08, 2);
	SET_DE(0xD800); I(10);
	call(0x116B);                      // decode_location_map
	call(0x1204);                      // terrain_edges
	cpu.iy = pop16(); I(14);
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	vram_bank_pop();
}

static void load_dungeon_room(void) {  // $3D82
	if (!native_on_fiber() && mem_read(0xC9A0) < 0x19 && mem_read(0xC9B8) == 0x0F) {
		native_decline();              // shadow mode: printing may wait for frames
		return;
	}
	call(0x2A6E);                      // dungeon_header
	vram_bank_push(0x3D85);
	cpu.a = mem_read(0xC9B8); I(13);
	cpu.a = op_inc8(cpu.a); I(4);
	rst20();
	SET_DE(0xC8E1); I(10);
	cpu.b = 0x04; I(7);
	do {                               // the triggers
		push(BC);
		op_block(4, 0); I(16);         // ldi
		cpu.b = 0x03; I(7);
		do {
			call(0x3EAA);              // unpack_position
			op_ex_af(); I(4);
			mem_write(DE, cpu.a); I(7);
			SET_DE(DE + 1); I(6);
			op_ex_af(); I(4);
			mem_write(DE, cpu.a); I(7);
			SET_DE(DE + 1); I(6);
			if (--cpu.b) I(13); else I(8);
		} while (cpu.b);
		SET_BC(pop16()); I(10);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	cpu.iy = ROOM_MONSTERS; I(14);
	SET_DE(0x0008); I(10);
	cpu.b = 0x10; I(7);
	do {                               // the foes
		cpu.a = mem_read(HL); I(7);
		SET_HL(HL + 1); I(6);
		mem_write((uint16_t)(cpu.iy + 5), cpu.a); I(19);
		op_sub8(0xAC, 0, 0); I(7);
		if (!(cpu.f & FZ)) I(12);
		else { I(7); cpu.a = 0x3C; I(7); }
		mem_write((uint16_t)(cpu.iy + 6), cpu.a); I(19);
		call(0x3EAA);
		mem_write(cpu.iy, cpu.a); I(19);
		mem_write((uint16_t)(cpu.iy + 2), cpu.a); I(19);
		op_ex_af(); I(4);
		mem_write((uint16_t)(cpu.iy + 1), cpu.a); I(19);
		mem_write((uint16_t)(cpu.iy + 3), cpu.a); I(19);
		mem_write((uint16_t)(cpu.iy + 7), 0x00); I(19);
		push(HL);
		cpu.a = mem_read((uint16_t)(cpu.iy + 5)); I(19);
		call(0x3ACE);                  // combat_strength_index
		SET_HL(0x3CFB); I(10);
		rst28();
		cpu.c = cpu.a; I(4);
		call(0x2BEB);                  // random_below
		cpu.c = op_rot(7, cpu.c); I(8);   // srl c
		op_or(cpu.c); I(4);
		mem_write((uint16_t)(cpu.iy + 4), cpu.a); I(19);
		SET_HL(pop16()); I(10);
		cpu.iy = op_add16(cpu.iy, DE); I(15);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	push(HL);
	cpu.a = mem_read(0xC9B9); I(13);   // the side the party came in by
	cpu.a = op_inc8(cpu.a); I(4);
	op_and(0x03); I(7);
	for (int i = 0; i < 3; i++) { op_add8(cpu.a, 0); I(4); }
	rst30();
	cpu.iy = PARTY_OBJECTS; I(14);
	SET_DE(0x0005); I(10);
	SET_BC(0x0800); I(10);
	do {                               // the members
		call(0x3EAA);
		mem_write(cpu.iy, cpu.a); I(19);
		op_ex_af(); I(4);
		mem_write((uint16_t)(cpu.iy + 1), cpu.a); I(19);
		call(0x4445);                  // member_can_act
		cpu.a = 0x00; I(7);
		if (cpu.f & FC) I(12);
		else { I(7); call(0x42F3); }   // its graphic
		mem_write((uint16_t)(cpu.iy + 2), cpu.a); I(19);
		mem_write((uint16_t)(cpu.iy + 3), cpu.a); I(19);
		cpu.iy = op_add16(cpu.iy, DE); I(15);
		cpu.c = op_inc8(cpu.c); I(4);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	SET_HL(pop16()); I(10);
	SET_DE(0x0020); I(10);
	SET_HL(op_add16(HL, DE)); I(11);
	SET_DE(0xD800); I(10);
	call(0x116B);                      // decode_location_map
	call(0x1204);                      // terrain_edges
	SET_HL(0xD800); I(10);
	SET_BC(0x9038); I(10);
	SET_DE(0x923C); I(10);
	cpu.a = 0x79; I(7);
	do {                               // tiles $38 and $3C: edges $90 and $92
		op_ex_af(); I(4);
		cpu.a = mem_read(HL); I(7);
		SET_HL(HL + 1); I(6);
		op_sub8(cpu.c, 0, 0); I(4);
		if (!(cpu.f & FZ)) I(12);
		else { I(7); mem_write(HL, cpu.b); I(7); }
		op_sub8(cpu.e, 0, 0); I(4);
		if (!(cpu.f & FZ)) I(12);
		else { I(7); mem_write(HL, cpu.d); I(7); }
		SET_HL(HL + 1); I(6);
		op_ex_af(); I(4);
		cpu.a = op_dec8(cpu.a); I(4);
		I(10);
	} while (!(cpu.f & FZ));
	cpu.a = 0xFF; I(7);
	mem_write(0xC984, cpu.a); I(13);
	cpu.a = mem_read(0xC9A0); I(13);
	op_sub8(0x19, 0, 0); I(7);
	if (!(cpu.f & FC)) { I(12); vram_bank_pop(); return; }
	I(7);
	cpu.a = mem_read(0xC9B8); I(13);
	op_sub8(0x0F, 0, 0); I(7);
	if (!(cpu.f & FZ)) { I(12); vram_bank_pop(); return; }
	I(7);
	say_inline(0x3E5E);
	cpu.a = mem_read(0xC953); I(13);   // the party's column
	op_sub8(0x03, 0, 0); I(7);
	if (cpu.f & FC) {
		I(7);
		say_inline(0x3E7B);
		op_xor(cpu.a); I(4);
		I(12);
	} else {
		I(12);
		op_sub8(0x09, 0, 0); I(7);
		if (cpu.f & FC) {
			I(7);
			say_inline(0x3E8B);
			cpu.a = 0x01; I(7);
			I(12);
		} else {
			I(12);
			say_inline(0x3E97);
			cpu.a = 0x02; I(7);
		}
	}
	mem_write(0xC984, cpu.a); I(13);
	I(17); text_newline();
	vram_bank_pop();
}

static void cp_iy(uint8_t d) { op_sub8(mem_read((uint16_t)(cpu.iy + d)), 0, 0); I(19); }

// a figure at (L, H) among n records from `records` whose byte `alive` is
// set: Z when one is found
static int someone_at(uint16_t records, uint16_t size, uint8_t n, uint8_t alive) {
	cpu.iy = records; I(14);
	SET_DE(size); I(10);
	cpu.b = n; I(7);
	do {
		cpu.a = mem_read((uint16_t)(cpu.iy + alive)); I(19);
		op_or(cpu.a); I(4);
		if (cpu.f & FZ) I(12);
		else {
			I(7);
			cpu.a = cpu.l; I(4);
			cp_iy(0);
			if (!(cpu.f & FZ)) I(12);
			else {
				I(7);
				cpu.a = cpu.h; I(4);
				cp_iy(1);
				if (cpu.f & FZ) { I(12); return 1; }
				I(7);
			}
		}
		cpu.iy = op_add16(cpu.iy, DE); I(15);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	return 0;
}

static void foe_can_step(void) {       // $A9E2
	push(BC); push(DE); push(HL);
	push16(cpu.iy); I(15);
	mem_write((uint16_t)(cpu.ix + 0x22), cpu.c); I(19);
	call(0x7E8D);                      // foe_record
	int blocked = -1;                  // -1: carry as it stands
	cpu.a = cpu.l; I(4);
	op_sub8(0x0B, 0, 0); I(7);
	int off = !(cpu.f & FC);
	if (off) I(12);
	else {
		I(7);
		cpu.a = cpu.h; I(4);
		op_sub8(0x0B, 0, 0); I(7);
		off = !(cpu.f & FC);
		I(off ? 7 : 12);
	}
	if (off) {                         // $A9F7: only a weak foe flees
		cpu.a = mem_read((uint16_t)(cpu.iy + 4)); I(19);
		op_sub8(0x18, 0, 0); I(7);
		op_ccf(); I(4);
		I(10);
	} else {
		call(0xA460);                  // A = the cell's tile
		mem_write((uint16_t)(cpu.ix + 0x23), cpu.a); I(19);
		call(0x7F47);                  // hazard
		I(10);
		if (!(cpu.f & FC)) {
			int here = 0;
			cpu.a = cpu.l; I(4);
			cp_iy(0);
			if (!(cpu.f & FZ)) I(12);
			else {
				I(7);
				cpu.a = cpu.h; I(4);
				cp_iy(1);
				if (!(cpu.f & FZ)) I(12);
				else {
					I(7);
					call(0x7ED8);      // random
					op_and(0x03); I(7);
					if (!(cpu.f & FZ)) { I(12); here = 1; }
					else I(7);
				}
			}
			if (here || someone_at(0xC839, 0x0008, 0x10, 5) || someone_at(0xC8B9, 0x0005, 0x08, 2))
				blocked = 1;
			else {
				cpu.c = mem_read((uint16_t)(cpu.ix + 0x22)); I(19);
				call(0x7E8D);
				cpu.a = mem_read((uint16_t)(cpu.iy + 5)); I(19);
				op_sub8(0x80, 0, 0); I(7);
				if (cpu.f & FC) {
					I(7);
					cpu.a = mem_read((uint16_t)(cpu.ix + 0x23)); I(19);
					call(0x7E99);      // people_ground
					I(12);
				} else {
					I(12);
					op_sub8(0x8E, 0, 0); I(7);
					if (cpu.f & FC) {
						I(7);
						cpu.a = mem_read((uint16_t)(cpu.ix + 0x23)); I(19);
						op_sub8(0x03, 0, 0); I(7);
						op_ccf(); I(4);
						I(12);
					} else {           // $AA7E: by the table, in the original
						I(12);
						SET_HL(0xAA93); I(10);
						I(17);
						push16(0xAA84);
						native_continue_at(0x7E18);   // switch_on_a
						return;
					}
				}
			}
		}
	}
	if (blocked == 1) { op_scf(); I(4); }  // $AA8C
	cpu.iy = pop16(); I(14);           // $AA8D
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

static void damage_monster(void) {     // $39B5
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: printing may wait for frames
	call(0x54CE);                      // print_newline
	call(0x48EB);                      // foe_record
	cpu.c = cpu.a; I(4);
	cpu.a = mem_read((uint16_t)(cpu.iy + 5)); I(19);
	call(0x47D3);                      // its name
	call(0x5519);                      // print_text_hl
	cpu.a = 0x20; I(7);
	call(0x548F);                      // print_char
	cpu.a = mem_read((uint16_t)(cpu.iy + 5)); I(19);
	op_sub8(0x5E, 0, 0); I(7);
	int dies = 0;
	if (cpu.f & FZ) I(12);
	else {
		I(7);
		cpu.a = mem_read((uint16_t)(cpu.iy + 4)); I(19);
		op_sub8(cpu.c, 0, 1); I(4);
		mem_write((uint16_t)(cpu.iy + 4), cpu.a); I(19);
		if (!(cpu.f & FC)) I(12);
		else { I(7); dies = 1; }
	}
	if (dies) {
		say_inline(0x39DD);
		cpu.a = cpu.b; I(4);
		cpu.a = op_inc8(cpu.a); I(4);
		if (cpu.f & FZ) I(12);
		else {
			I(7);
			say_inline(0x39EB);
			cpu.a = mem_read((uint16_t)(cpu.iy + 5)); I(19);
			call(0x3ACE);              // combat_strength_index
			SET_HL(0x3CFB); I(10);
			rst28();
			for (int i = 0; i < 4; i++) { op_rrca(); I(4); }
			op_and(0x0F); I(7);
			cpu.a = op_inc8(cpu.a); I(4);
			cpu.l = cpu.a; I(4);
			cpu.h = 0x00; I(7);
			cpu.c = cpu.b; I(4);
			call(0x4205);              // member_gain_xp
			call(0x5627);              // print_number_auto
		}
		mem_write((uint16_t)(cpu.iy + 5), 0x00); I(19);
		I(10);
		return;
	}
	cpu.b = 0x00; I(7);                // $3A13: how it fares
	cpu.c = cpu.a; I(4);
	op_sub8(0x18, 0, 0); I(7);
	if (cpu.f & FC) I(12);
	else {
		I(7);
		cpu.a = mem_read((uint16_t)(cpu.iy + 5)); I(19);
		call(0x3ACE);
		SET_HL(0x3CFB); I(10);
		rst28();
		cpu.a = op_rot(7, cpu.a); I(8);    // srl a
		cpu.e = cpu.a; I(4);
		cpu.a = op_rot(7, cpu.a); I(8);
		cpu.d = cpu.a; I(4);
		op_add8(cpu.e, 0); I(4);
		cpu.h = cpu.a; I(4);
		cpu.a = cpu.c; I(4);
		const uint8_t limits[3] = { cpu.d, cpu.e, cpu.h };
		int k = 0;
		for (; k < 3; k++) {
			cpu.b = op_inc8(cpu.b); I(4);
			op_sub8(limits[k], 0, 0); I(4);
			if (cpu.f & FC) { I(12); break; }
			I(7);
		}
		if (k == 3) { cpu.b = op_inc8(cpu.b); I(4); }
	}
	cpu.a = cpu.b; I(4);
	SET_HL(0x3A42); I(10);
	rst20();
	I(10);
	native_continue_at(0x5519);        // jp print_text_hl
}

static void compact_records(void) {    // $B868
	push(BC); push(DE); push(HL);
	cpu.b = op_dec8(cpu.b); I(4);
	do {
		cpu.a = mem_read(HL); I(7);
		op_or(cpu.a); I(4);
		if (!(cpu.f & FZ)) {
			I(7);
			SET_DE(0x0008); I(10);
			SET_HL(op_add16(HL, DE)); I(11);
			I(12);
		} else {
			I(12);
			push(BC); push(HL);
			cpu.d = cpu.h; I(4);
			cpu.e = cpu.l; I(4);
			cpu.a = cpu.b; I(4);
			SET_BC(0x0008); I(10);
			SET_HL(op_add16(HL, BC)); I(11);
			for (int i = 0; i < 3; i++) { op_add8(cpu.a, 0); I(4); }
			cpu.c = cpu.a; I(4);
			while (op_block(6, 0)) I(21);   // ldir: the rest moved up
			I(16);
			op_xor(cpu.a); I(4);
			cpu.b = 0x08; I(7);
			do {
				mem_write(DE, cpu.a); I(7);
				SET_DE(DE + 1); I(6);
				if (--cpu.b) I(13); else I(8);
			} while (cpu.b);
			SET_HL(pop16()); I(10);
			SET_BC(pop16()); I(10);
		}
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

static void ix_set(uint8_t d, uint8_t v) { mem_write((uint16_t)(cpu.ix + d), v); I(19); }

static void missile_start(void) {      // $38FC
	// shadow mode: queueing the glyphs may wait for room
	if (!native_on_fiber() && mem_read(0xC063) >= 0x20) { native_decline(); return; }
	call(0x30DA);                      // queue_glyphs
	push(HL);
	call(0x3991);                      // the place on screen
	ix_set(0x0B, cpu.l);
	ix_set(0x0D, cpu.h);
	ix_set(0x0A, 0x00);
	ix_set(0x0C, 0x00);
	SET_HL(pop16()); I(10);
	call(0x2BFE);                      // aim
	SET_HL(op_add16(HL, HL)); I(11);
	SET_HL(op_add16(HL, HL)); I(11);
	ix_set(0x0E, cpu.l);
	ix_set(0x0F, cpu.h);
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
	SET_HL(op_add16(HL, HL)); I(11);
	SET_HL(op_add16(HL, HL)); I(11);
	ix_set(0x10, cpu.l);
	ix_set(0x11, cpu.h);
	ix_set(0x09, 0x08);
	cpu.a = 0x18; I(7);
	I(10);
	native_continue_at(0x05F1);        // jp play_sound
}

static void times_11_b3(void) {        // $B856
	push(BC);
	cpu.b = cpu.a; I(4);
	op_add8(cpu.a, 0); I(4);
	cpu.c = cpu.a; I(4);
	op_add8(cpu.a, 0); I(4);
	op_add8(cpu.a, 0); I(4);
	op_add8(cpu.b, 0); I(4);
	op_add8(cpu.c, 0); I(4);
	SET_BC(pop16()); I(10);
	I(10);
}

static void times_12_b3(void) {        // $B860
	push(BC);
	op_add8(cpu.a, 0); I(4);
	op_add8(cpu.a, 0); I(4);
	cpu.b = cpu.a; I(4);
	op_add8(cpu.a, 0); I(4);
	op_add8(cpu.b, 0); I(4);
	SET_BC(pop16()); I(10);
	I(10);
}

static void divide_step(void) {        // $B805
	for (int i = 0; i < 8; i++) {
		op_add8(cpu.a, 0); I(4);
		op_adc16(HL); I(15);           // adc hl,hl
		cpu.a = op_inc8(cpu.a); I(4);
		op_sbc16(DE); I(15);
		if (!(cpu.f & FC)) I(12);
		else {
			I(7);
			SET_HL(op_add16(HL, DE)); I(11);
			cpu.a = op_dec8(cpu.a); I(4);
		}
	}
	I(10);
}

static void divide_16(void) {          // $B7EA
	cpu.a = cpu.d; I(4);
	op_or(cpu.e); I(4);
	if (cpu.f & FZ) { I(12); SET_HL(0x0000); I(10); I(10); return; }
	I(7);
	push(BC);
	cpu.a = cpu.h; I(4);
	cpu.b = cpu.l; I(4);
	SET_HL(0x0000); I(10);
	I(17); divide_step();
	cpu.c = cpu.a; I(4);
	cpu.a = cpu.b; I(4);
	I(17); divide_step();
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
	cpu.h = cpu.c; I(4);
	cpu.l = cpu.a; I(4);
	SET_BC(pop16()); I(10);
	I(10);
}

static void up_wrap(uint8_t *r) {      // inc r / ld a,r / cp $0B / ret c / ld r,0 / ret
	*r = op_inc8(*r); I(4);
	cpu.a = *r; I(4);
	op_sub8(0x0B, 0, 0); I(7);
	if (cpu.f & FC) { I(11); return; }
	I(5);
	*r = 0x00; I(7);
	I(10);
}
static void down_wrap(uint8_t *r) {    // dec r / scf / ret p / ld r,$0A / or a / ret
	*r = op_dec8(*r); I(4);
	op_scf(); I(4);
	if (!(cpu.f & FS)) { I(11); return; }
	I(5);
	*r = 0x0A; I(7);
	op_or(cpu.a); I(4);
	I(10);
}
static void east_wrap(void) { up_wrap(&cpu.e); }     // $389F
static void west_wrap(void) { down_wrap(&cpu.e); }   // $38A7
static void south_wrap(void) { up_wrap(&cpu.d); }    // $38AE
static void north_wrap(void) { down_wrap(&cpu.d); }  // $38B6

static void arena_cell(void) {         // $38F2
	cpu.a = cpu.h; I(4);
	call(0x36ED);                      // times_11
	op_add8(cpu.l, 0); I(4);
	op_add8(cpu.a, 0); I(4);
	cpu.l = cpu.a; I(4);
	cpu.h = 0xD8; I(7);
	I(10);
}

static void arena_cell_at(void) {      // $38EB
	push(HL);
	I(17); arena_cell();
	cpu.a = mem_read(HL); I(7);
	SET_HL(pop16()); I(10);
	I(10);
}

static void target_in_pattern(void) {  // $A279
	for (;;) {
		cpu.a = mem_read(HL); I(7);
		SET_HL(HL + 1); I(6);
		op_sub8(0x80, 0, 0); I(7);
		op_scf(); I(4);
		if (cpu.f & FZ) { I(11); return; }
		I(5);
		op_add8(mem_read((uint16_t)(cpu.ix + 0x1E)), 0); I(19);
		op_sub8(mem_read((uint16_t)(cpu.ix + 0x20)), 0, 0); I(19);
		if (cpu.f & FZ) {
			I(7);
			cpu.a = mem_read(HL); I(7);
			op_add8(mem_read((uint16_t)(cpu.ix + 0x1F)), 0); I(19);
			op_sub8(mem_read((uint16_t)(cpu.ix + 0x21)), 0, 0); I(19);
			if (cpu.f & FZ) { I(11); return; }
			I(5);
		} else I(12);
		SET_HL(HL + 1); I(6);
		I(12);
	}
}

Native native_foe_can_step = { "foe_can_step", 0xA9E2, 3, foe_can_step, OUT_A | OUT_F, .long_calls = 1 };
Native native_damage_monster = { "damage_monster", 0x39B5, 0, damage_monster, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL | OUT_IY, .long_calls = 1 };
Native native_compact_records = { "compact_records", 0xB868, 3, compact_records, OUT_A | OUT_F, .long_calls = 1 };
Native native_missile_start = { "missile_start", 0x38FC, 0, missile_start, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, .long_calls = 1 };
Native native_times_11_b3 = { "times_11_b3", 0xB856, 3, times_11_b3, OUT_A | OUT_F, .long_calls = 1 };
Native native_times_12_b3 = { "times_12_b3", 0xB860, 3, times_12_b3, OUT_A | OUT_F, .long_calls = 1 };
Native native_divide_step = { "divide_step", 0xB805, 3, divide_step, OUT_A | OUT_F | OUT_HL, .long_calls = 1 };
Native native_divide_16_b3 = { "divide_16_b3", 0xB7EA, 3, divide_16, OUT_A | OUT_F | OUT_DE | OUT_HL, .long_calls = 1 };
Native native_east_wrap = { "east_wrap", 0x389F, 0, east_wrap, OUT_A | OUT_F | OUT_DE, .long_calls = 1 };
Native native_west_wrap = { "west_wrap", 0x38A7, 0, west_wrap, OUT_A | OUT_F | OUT_DE, .long_calls = 1 };
Native native_south_wrap = { "south_wrap", 0x38AE, 0, south_wrap, OUT_A | OUT_F | OUT_DE, .long_calls = 1 };
Native native_north_wrap = { "north_wrap", 0x38B6, 0, north_wrap, OUT_A | OUT_F | OUT_DE, .long_calls = 1 };
Native native_arena_cell = { "arena_cell", 0x38F2, 0, arena_cell, OUT_A | OUT_F | OUT_HL, .long_calls = 1 };
Native native_arena_cell_at = { "arena_cell_at", 0x38EB, 0, arena_cell_at, OUT_A | OUT_F, .long_calls = 1 };
Native native_target_in_pattern = { "target_in_pattern", 0xA279, 3, target_in_pattern, OUT_A | OUT_F | OUT_HL, .long_calls = 1 };
Native native_load_dungeon_room = { "load_dungeon_room", 0x3D82, 0, load_dungeon_room, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL | OUT_IY, .long_calls = 1 };
Native native_load_arena = { "load_arena", 0x377E, 0, load_arena, OUT_A | OUT_F, .long_calls = 1 };
