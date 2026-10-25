// What lies about on the world map (bank 3): 8-byte records of the people
// block, +0 figure (0: none), +1/+2 place, +3 figure again. The balloon
// ($C691), five ships ($C699), five horses ($C6C1) and thirteen chests
// ($C6E9). A full list makes room by dropping one: out of the party's
// sight if it can, else not where the party stands, else the first; never
// the one at $C973/$C974.
// - put_here ($B894): IY's record at the party's place, as the figure it
//   travels as ($C959).
// - in_view ($B8B7): carry when IY's record is within 5 of the party;
//   first_out_of_view ($B8A7): IY on the first of B that isn't (carry:
//   none).
// - at_party ($B8DD), at_c973 ($B94C): Z when IY's record is there;
//   first_not_at_party ($B8CD): IY on the first of B that isn't (Z: none).
// - room_for_ship ($B8EB), room_for_horse ($B95A), room_for_chest ($B994):
//   IY = the list's last record, cleared, after making room.
// - the entries: room for a ship and a horse ($B9CF); the balloon's record
//   from $B9F2 ($B9E0), or at the party ($B9FA); a ship at $C97A ($BA0B);
//   a ship or a horse left at the party ($BA2A, $BA3B); a chest at $C97A
//   ($BA4C).
// Cycle counts are the original's, instruction by instruction (I).
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define PARTY_X  0xC953
#define PARTY_Y  0xC954
#define FIGURE   0xC959
#define SHIPS    0xC699
#define HORSES   0xC6C1
#define CHESTS   0xC6E9

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static void push(uint16_t v) { push16(v); I(11); }
static uint8_t iy_at(uint8_t d) { return mem_read((uint16_t)(cpu.iy + d)); }
static void iy_set(uint8_t d, uint8_t v) { mem_write((uint16_t)(cpu.iy + d), v); I(19); }

static void compact(uint16_t list, uint8_t n) {   // ld hl,list / ld b,n / call compact_records
	SET_HL(list); I(10);
	cpu.b = n; I(7);
	I(17); call_original(0xB868);
}

static void put_here(void) {           // $B894
	SET_HL(rd16(PARTY_X)); I(16);
	iy_set(1, cpu.l);
	iy_set(2, cpu.h);
	cpu.a = mem_read(FIGURE); I(13);
	iy_set(3, cpu.a);
	iy_set(0, cpu.a);
	I(10);
}

static void in_view(void) {            // $B8B7
	cpu.a = mem_read(PARTY_X); I(13);
	op_sub8(iy_at(1), 0, 1); I(19);
	op_add8(0x05, 0); I(7);
	op_sub8(0x0B, 0, 0); I(7);
	if (!(cpu.f & FC)) { I(11); return; }
	I(5);
	cpu.a = mem_read(PARTY_Y); I(13);
	op_sub8(iy_at(2), 0, 1); I(19);
	op_add8(0x05, 0); I(7);
	op_sub8(0x0B, 0, 0); I(7);
	I(10);
}

static void at(uint16_t x, uint16_t y) {   // $B8DD / $B94C
	cpu.a = mem_read(x); I(13);
	op_sub8(iy_at(1), 0, 0); I(19);
	if (!(cpu.f & FZ)) { I(11); return; }
	I(5);
	cpu.a = mem_read(y); I(13);
	op_sub8(iy_at(2), 0, 0); I(19);
	I(10);
}
static void at_party(void) { at(PARTY_X, PARTY_Y); }
static void at_c973(void) { at(0xC973, 0xC974); }

static void first_out_of_view(void) {  // $B8A7
	push(DE);
	SET_DE(0x0008); I(10);
	for (;;) {
		I(17); in_view();
		if (!(cpu.f & FC)) { I(12); break; }
		I(7);
		cpu.iy = op_add16(cpu.iy, DE); I(15);
		if (--cpu.b) I(13); else { I(8); op_scf(); I(4); break; }
	}
	SET_DE(pop16()); I(10);
	I(10);
}

static void first_not_at_party(void) { // $B8CD
	push(DE);
	SET_DE(0x0008); I(10);
	for (;;) {
		I(17); at_party();
		if (!(cpu.f & FZ)) { I(12); break; }
		I(7);
		cpu.iy = op_add16(cpu.iy, DE); I(15);
		if (--cpu.b) I(13); else { I(8); op_xor(cpu.a); I(4); break; }
	}
	SET_DE(pop16()); I(10);
	I(10);
}

static void clear_last(uint16_t last, int jump) {   // ld iy,last (/ jr or jp $B93B)
	cpu.iy = last; I(14);
	I(jump);
	iy_set(3, 0x00);
	iy_set(0, 0x00);
	iy_set(6, 0x00);
	iy_set(7, 0x00);
	I(10);
}

// a pass over the five ships, skipping the one at $C973: the first for
// which `pick` holds; 1 when found
static int ships_pass(int out_of_view) {
	cpu.iy = SHIPS; I(14);
	SET_DE(0x0008); I(10);
	cpu.b = 0x05; I(7);
	do {
		I(17); at_c973();
		if (cpu.f & FZ) I(12);
		else {
			I(7);
			if (out_of_view) {
				I(17); in_view();
				if (!(cpu.f & FC)) { I(12); return 1; }
			} else {
				I(17); at_party();
				if (!(cpu.f & FZ)) { I(12); return 1; }
			}
			I(7);
		}
		cpu.iy = op_add16(cpu.iy, DE); I(15);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	return 0;
}

static void room_for_ship(void) {      // $B8EB
	compact(SHIPS, 0x05);
	cpu.a = mem_read(SHIPS + 0x20); I(13);
	op_or(cpu.a); I(4);
	if (cpu.f & FZ) I(12);
	else {
		I(7);
		if (!ships_pass(1) && !ships_pass(0)) { cpu.iy = SHIPS; I(14); }
		iy_set(0, 0x00);               // $B92B: that one dropped
		compact(SHIPS, 0x05);
	}
	clear_last(SHIPS + 0x20, 0);
}

// room for a horse or a chest: out of view first, then not at the party
static void make_room(uint16_t list, uint8_t n) {
	compact(list, n);
	uint16_t last = (uint16_t)(list + 8 * (n - 1));
	cpu.a = mem_read(last); I(13);
	op_or(cpu.a); I(4);
	if (cpu.f & FZ) { I(12); return; }
	I(7);
	cpu.iy = list; I(14);
	cpu.b = n; I(7);
	I(17); first_out_of_view();
	if (!(cpu.f & FC)) I(12);
	else {
		I(7);
		cpu.iy = list; I(14);
		cpu.b = n; I(7);
		I(17); first_not_at_party();
		if (!(cpu.f & FZ)) I(12);
		else { I(7); cpu.iy = list; I(14); }
	}
	iy_set(0, 0x00);
	compact(list, n);
}

static void room_for_horse(void) {     // $B95A
	make_room(HORSES, 0x05);
	clear_last(HORSES + 0x20, 12);
}

static void room_for_chest(void) {     // $B994
	make_room(CHESTS, 0x0D);
	clear_last(CHESTS + 0x60, 10);
}

static void push_all(void) { push(BC); push(DE); push(HL); push16(cpu.iy); I(15); }
static void pop_all(void) {
	cpu.iy = pop16(); I(14);
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

static void put_at_c97a(uint8_t figure) {
	SET_HL(rd16(0xC97A)); I(16);
	iy_set(1, cpu.l);
	iy_set(2, cpu.h);
	iy_set(3, figure);
	iy_set(0, figure);
}

static void room_for_both(void) {      // $B9CF
	push_all();
	I(17); room_for_ship();
	I(17); room_for_horse();
	pop_all();
}

static void balloon_reset(void) {      // $B9E0
	push(BC); push(DE); push(HL);
	SET_HL(0xB9F2); I(10);
	SET_DE(0xC691); I(10);
	SET_BC(0x0008); I(10);
	while (op_block(6, 0)) I(21);      // ldir
	I(16);
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

static void balloon_here(void) {       // $B9FA
	push(HL);
	cpu.a = 0x18; I(7);
	mem_write(0xC691, cpu.a); I(13);
	mem_write(0xC694, cpu.a); I(13);
	SET_HL(rd16(PARTY_X)); I(16);
	wr16(0xC692, HL); I(16);
	SET_HL(pop16()); I(10);
	I(10);
}

static void ship_at_c97a(void) {       // $BA0B
	push_all();
	I(17); room_for_ship();
	put_at_c97a(0x10);
	pop_all();
}

static void ship_left_here(void) {     // $BA2A
	push_all();
	I(17); room_for_ship();
	I(17); put_here();
	pop_all();
}

static void horse_left_here(void) {    // $BA3B
	push_all();
	I(17); room_for_horse();
	I(17); put_here();
	pop_all();
}

static void chest_at_c97a(void) {      // $BA4C
	push_all();
	I(17); room_for_chest();
	put_at_c97a(0x3C);
	pop_all();
}

#define LYING(name, addr, out) Native native_##name = { #name, addr, 3, name, out, .long_calls = 1 };
LYING(put_here, 0xB894, OUT_A | OUT_F | OUT_HL)
LYING(first_out_of_view, 0xB8A7, OUT_A | OUT_F | OUT_BC | OUT_IY)
LYING(in_view, 0xB8B7, OUT_A | OUT_F)
LYING(first_not_at_party, 0xB8CD, OUT_A | OUT_F | OUT_BC | OUT_IY)
LYING(at_party, 0xB8DD, OUT_A | OUT_F)
LYING(room_for_ship, 0xB8EB, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL | OUT_IY)
LYING(at_c973, 0xB94C, OUT_A | OUT_F)
LYING(room_for_horse, 0xB95A, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL | OUT_IY)
LYING(room_for_chest, 0xB994, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL | OUT_IY)
LYING(room_for_both, 0xB9CF, OUT_A | OUT_F)
LYING(balloon_reset, 0xB9E0, OUT_A | OUT_F)
LYING(balloon_here, 0xB9FA, OUT_A | OUT_F)
LYING(ship_at_c97a, 0xBA0B, OUT_A | OUT_F)
LYING(ship_left_here, 0xBA2A, OUT_A | OUT_F)
LYING(horse_left_here, 0xBA3B, OUT_A | OUT_F)
LYING(chest_at_c97a, 0xBA4C, OUT_A | OUT_F)
