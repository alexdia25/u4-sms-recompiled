// Doors in towns, and the world's ways in (bank 2).
// - town_cell ($B5A4): HL = the cell (L, H) of the town's 32x32 map at
//   $D000 (2 bytes a cell).
// - door_ahead ($B464): the party's step (L, H) in a town: a locked door
//   ($3A) is "Locked!" and stops it; a door ($3B) opens ($3E) for two
//   turns ($C9A4, the one opened before shut again, its place at $C9A2),
//   with a sound, and stops this step too. Carry: the step is stopped.
// - world_step ($ACDC): after a step on the world (unless travel there is
//   off, $C9A1, or by balloon): the Abyss's isle ($AEDA, left to the
//   original); on an open moongate ($C98C = $43 at $C98D/$C98E) the party
//   is sent, with a sound and two flashes, to the gate of the other moon
//   ($C95F, places at $7CA3), or, both moons new ($C95D = $C95F = 4), to
//   location $20 (the rest in the original, $ADFF); else a place among
//   the 32 at $AE60 is entered: location = its number + 2 (with the
//   carry), the way back kept, and what happens by the figure the party
//   travels as ($C9BB, the table at $AD6D, in the original).
// - town_step ($AFAA): after a step in a town (unless travel is off,
//   $C9A1): off the 32x32 map the party leaves for the world ($AFC9); in
//   the Abyss ($C959 = $1F) a ladder down ($1B) or up ($1C) under it
//   ($C990) takes it on ($AFEF, $B02A): those, which print and wait, are
//   the original's.
// - tile_at ($BACF): A = the tile at (E, D): in a town ($BAF3) the cell
//   of the 32x32 map at $D000 (DE its address); on the world ($BAD5)
//   (E, D) made relative to the loaded 32x32 chunks ($C987, $C988), carry
//   when off them.
// - view_step ($B3DF west, $B3E8 north, $B3F1 east, $B3FA south): the
//   party's place in the loaded window ($C985/$C986) and on the map
//   ($C953/$C954) one step on.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define MAP_MODE   0xC99F
#define PARTY_X    0xC953
#define PARTY_Y    0xC954
#define DOOR_OPEN  0xC9A4
#define DOOR_AT    0xC9A2

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static void push(uint16_t v) { push16(v); I(11); }
static void call(uint16_t addr) { I(17); call_original(addr); }

static void town_cell(void) {          // $B5A4
	op_xor(cpu.a); I(4);
	cpu.h = op_rot(7, cpu.h); I(8);    // srl h / rra, twice: H's low bits into A
	op_rra(); I(4);
	cpu.h = op_rot(7, cpu.h); I(8);
	op_rra(); I(4);
	op_add8(cpu.l, 0); I(4);
	op_add8(cpu.l, 0); I(4);
	cpu.l = cpu.a; I(4);
	cpu.a = cpu.h; I(4);
	op_or(0xD0); I(7);
	cpu.h = cpu.a; I(4);
	I(10);
}

static int ret_if(int cond) { if (cond) { I(11); return 1; } I(5); return 0; }

static void door_ahead(void) {         // $B464
	if (!native_on_fiber() && mem_read(MAP_MODE)) {   // shadow mode: a door prints, which may wait for frames
		uint8_t x = (uint8_t)(mem_read(PARTY_X) + cpu.l), y = (uint8_t)(mem_read(PARTY_Y) + cpu.h);
		if (x < 0x20 && y < 0x20) {
			uint8_t t = mem_read((uint16_t)(0xD000 | (y << 6) | (x << 1)));
			if (t == 0x3A || t == 0x3B) { native_decline(); return; }
		}
	}
	cpu.a = mem_read(MAP_MODE); I(13);
	op_or(cpu.a); I(4);
	if (ret_if(cpu.f & FZ)) return;
	cpu.a = mem_read(PARTY_X); I(13);
	op_add8(cpu.l, 0); I(4);
	cpu.l = cpu.a; I(4);
	cpu.e = cpu.a; I(4);
	op_sub8(0x20, 0, 0); I(7);
	if (ret_if(!(cpu.f & FC))) return;
	cpu.a = mem_read(PARTY_Y); I(13);
	op_add8(cpu.h, 0); I(4);
	cpu.h = cpu.a; I(4);
	cpu.d = cpu.a; I(4);
	op_sub8(0x20, 0, 0); I(7);
	if (ret_if(!(cpu.f & FC))) return;
	I(17); town_cell();
	cpu.a = mem_read(HL); I(7);
	op_sub8(0x3A, 0, 0); I(7);
	if (cpu.f & FZ) {                  // $B489: locked
		I(12);
		SET_HL(0xBD0E); I(10);
		call(0x5519);                  // print_text_hl
		cpu.a = 0x10; I(7);
		call(0x05F1);                  // play_sound
		SET_HL(0xBD19); I(10);
		call(0x5519);
		op_scf(); I(4);
		I(10);
		return;
	}
	I(7);
	op_sub8(0x3B, 0, 0); I(7);
	if (!(cpu.f & FZ)) { I(7); op_or(cpu.a); I(4); I(10); return; }
	I(12);
	cpu.a = mem_read(DOOR_OPEN); I(13);    // $B49C: a door opens
	op_or(cpu.a); I(4);
	if (cpu.f & FZ) I(12);
	else {                             // the one open before shuts
		I(7);
		push(HL);
		SET_HL(rd16(DOOR_AT)); I(16);
		I(17); town_cell();
		mem_write(HL, 0x3B); I(10);
		cpu.l = op_inc8(cpu.l); I(4);
		mem_write(HL, 0x00); I(10);
		SET_HL(pop16()); I(10);
	}
	mem_write(HL, 0x3E); I(10);
	cpu.l = op_inc8(cpu.l); I(4);
	mem_write(HL, 0x00); I(10);
	cpu.a = 0x02; I(7);
	mem_write(DOOR_OPEN, cpu.a); I(13);
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
	wr16(DOOR_AT, HL); I(16);
	SET_HL(0xBD0E); I(10);
	call(0x5519);
	cpu.a = 0x0E; I(7);
	call(0x05F1);
	op_scf(); I(4);
	I(10);
}

// $AEDA: come in from the north ($C9B9 = 2) onto ($DC, $E0), the Abyss's
// isle: its people set up (the original's, $AEEC)
static void abyss_isle(void) {
	static const struct { uint16_t at; uint8_t v; } need[3] = { { 0xC9B9, 0x02 }, { PARTY_X, 0xDC }, { PARTY_Y, 0xE0 } };
	for (int i = 0; i < 3; i++) {
		cpu.a = mem_read(need[i].at); I(13);
		op_sub8(need[i].v, 0, 0); I(7);
		if (ret_if(!(cpu.f & FZ))) return;
	}
	native_continue_at(0xAEEC);
}

static int world_event_here(void) {    // a moongate, or a place's entrance, under the party
	uint8_t x = mem_read(PARTY_X), y = mem_read(PARTY_Y);
	if (x == mem_read(0xC98D) && y == mem_read(0xC98E) && mem_read(0xC98C) == 0x43) return 1;
	for (int i = 0; i < 0x20; i++)
		if (mem_read((uint16_t)(0xAE60 + 2 * i)) == x && mem_read((uint16_t)(0xAE61 + 2 * i)) == y) return 1;
	return 0;
}

// call switch_on_a ($334A) from `back` - 3 on the table at HL: the
// address it jumps to (its return popped into AF), or 0 (it returns, Z)
static uint16_t switch_on(uint16_t back) {
	I(17);
	for (;;) {
		cpu.b = mem_read(HL); I(7);
		cpu.b = op_inc8(cpu.b); I(4);
		if (cpu.f & FZ) { I(11); return 0; }
		I(5);
		op_sub8(mem_read(HL), 0, 0); I(7);
		SET_HL(HL + 1); I(6);
		if (cpu.f & FZ) break;
		I(7);
		SET_HL(HL + 1); I(6);
		SET_HL(HL + 1); I(6);
		I(10);
	}
	I(12);
	cpu.a = back >> 8; cpu.f = (uint8_t)back; I(10);
	cpu.a = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	cpu.h = mem_read(HL); I(7);
	cpu.l = cpu.a; I(4);
	I(4);
	return HL;
}

static void place_name(uint8_t base) {   // add a,base / call print_item_name / call print_newline
	op_add8(base, 0); I(7);
	call(0x5516);
	call(0x54CE);
}

// $AD6D, by the figure the party travels as ($C9BB): how it goes in.
// Dungeons ($AD89; $ADE9 for the Abyss: all eight runes' parts first,
// $C951/$C952) on foot only (else $4703, and no way in); towns, castles
// and the rest ($ADB2..: "Welcome to" the place, its start by size);
// shrines ($ADFF). Then $AE39 (the original from $AE3D: the window wiped,
// the people kept, the new game mode); no way in: location 0 ($AE5B).
static void enter_place(void) {
	uint16_t to = switch_on(0xAD6C);
	uint16_t welcome = 0;
	uint8_t mode;
	switch (to) {
	case 0: I(10); return;             // ret at $AD6C
	case 0xADE9:                       // the Abyss
		cpu.a = mem_read(0xC951); I(13);
		op_and(0x1C); I(7);
		op_sub8(0x1C, 0, 0); I(7);
		if (!(cpu.f & FZ)) { I(12); goto no_way; }
		I(7);
		cpu.a = mem_read(0xC952); I(13);
		op_and(0x1C); I(7);
		op_sub8(0x1C, 0, 0); I(7);
		if (!(cpu.f & FZ)) { I(10); goto no_way; }
		I(10); I(10);
		/* fall through */
	case 0xAD89:                       // a dungeon
		SET_HL(0xBCD1); I(10);
		call(0xAE20);                  // say_welcome
		cpu.a = mem_read(0xC9A0); I(13);
		place_name(0x7D);
		cpu.a = mem_read(0xC959); I(13);
		op_sub8(0x1F, 0, 0); I(7);
		if (!(cpu.f & FZ)) {
			I(7);
			call(0x4703);
			I(10);
			goto no_way;
		}
		I(12);
		SET_HL(0x0202); I(10);
		wr16(PARTY_X, HL); I(16);
		cpu.a = mode = 0x13; I(7);
		I(10);
		break;
	case 0xADFF:                       // a shrine
		SET_HL(0xBCC6); I(10); call(0x5519);
		SET_HL(0xBCF3); I(10); call(0x5519);
		SET_HL(0xBCCD); I(10); call(0x5519);
		cpu.a = mem_read(0xC9A0); I(13);
		place_name(0x7D);
		cpu.a = mode = 0x17; I(7);
		I(12);
		break;
	case 0xADB2: welcome = 0xBCD9; break;
	case 0xADDA: welcome = 0xBCDF; break;
	case 0xADDF: welcome = 0xBCE6; break;
	case 0xADE4: welcome = 0xBCEE; break;
	default: native_continue_at(to); return;
	}
	if (welcome) {                     // $ADB5: a town, castle, village
		SET_HL(welcome); I(10);
		if (to != 0xADB2) I(12);       // jr $ADB5
		call(0xAE20);
		call(0x33EE);                  // place_index
		place_name(0x7F);
		cpu.a = mem_read(0xC9A0); I(13);
		op_sub8(0x06, 0, 0); I(7);
		SET_HL(0x1E0F); I(10);
		if (cpu.f & FC) I(12);
		else { I(7); SET_HL(0x0F01); I(10); }
		wr16(PARTY_X, HL); I(16);
		wr16(0xC985, HL); I(16);
		cpu.a = mode = 0x12; I(7);
		I(12);
	}
	(void)mode;
	SET_HL(pop16()); I(10);            // $AE39: no return to the turn
	mem_write((uint16_t)(cpu.ix + 0x08), cpu.a); I(19);
	native_continue_at(0xAE3D);
	return;
no_way:                                // $AE5B
	op_xor(cpu.a); I(4);
	mem_write(0xC9A0, cpu.a); I(13);
	I(10);
}

static void world_step(void) {         // $ACDC
	// shadow mode: what follows a moongate or an entrance waits for frames
	if (!native_on_fiber() && world_event_here()) { native_decline(); return; }
	cpu.a = mem_read(0xC9A1); I(13);
	op_or(cpu.a); I(4);
	if (ret_if(cpu.f & FZ)) return;
	call(0x957C);                      // in_balloon
	if (ret_if(cpu.f & FZ)) return;
	call(0xAEDA);                      // the Abyss's isle
	int gate = 0;
	cpu.a = mem_read(PARTY_X); I(13);
	SET_HL(0xC98D); I(10);
	op_sub8(mem_read(HL), 0, 0); I(7);
	if (!(cpu.f & FZ)) I(12);
	else {
		I(7);
		cpu.a = mem_read(PARTY_Y); I(13);
		SET_HL(0xC98E); I(10);
		op_sub8(mem_read(HL), 0, 0); I(7);
		if (!(cpu.f & FZ)) I(12);
		else {
			I(7);
			cpu.a = mem_read(0xC98C); I(13);
			op_sub8(0x43, 0, 0); I(7);
			if (!(cpu.f & FZ)) I(12); else { I(7); gate = 1; }
		}
	}
	if (gate) {                        // a moongate
		cpu.a = 0x15; I(7);
		call(0x05F1);                  // play_sound
		call(0x41A9);                  // invert: a flash
		call(0x41A9);
		cpu.a = mem_read(0xC95D); I(13);
		op_sub8(0x04, 0, 0); I(7);
		int both_new = 0;
		if (!(cpu.f & FZ)) I(12);
		else {
			I(7);
			cpu.a = mem_read(0xC95F); I(13);
			op_sub8(0x04, 0, 0); I(7);
			if (!(cpu.f & FZ)) I(12); else { I(7); both_new = 1; }
		}
		if (both_new) {
			cpu.a = 0x20; I(7);
			mem_write(0xC9A0, cpu.a); I(13);
			SET_HL(rd16(PARTY_X)); I(16);
			wr16(0xC96D, HL); I(16);
			I(10);
			native_continue_at(0xADFF);
			return;
		}
		cpu.a = mem_read(0xC95F); I(13);   // $AD28: the other moon's gate
		SET_HL(0x7CA3); I(10);
		I(11); I(10);                  // rst $20: HL = the word HL[A]
		op_add8(cpu.a, 0); I(4);
		if (!(cpu.f & FC)) I(12);
		else { I(7); cpu.h = op_inc8(cpu.h); I(4); }
		op_add8(cpu.l, 0); I(4);
		cpu.l = cpu.a; I(4);
		op_add8(cpu.h, cpu.f & FC); I(4);
		op_sub8(cpu.l, 0, 1); I(4);
		cpu.h = cpu.a; I(4);
		cpu.a = mem_read(HL); I(7);
		SET_HL(HL + 1); I(6);
		cpu.h = mem_read(HL); I(7);
		cpu.l = cpu.a; I(4);
		I(10);
		wr16(PARTY_X, HL); I(16);
		call(0x0E22);                  // recenter_map_view
		call(0x276D);                  // forget_screen
		call(0x129D);                  // refresh
		call(0x41A9);
		I(10);
		native_continue_at(0x41A9);    // jp invert
		return;
	}
	SET_HL(0xAE60); I(10);             // $AD41: a place entered?
	SET_DE(rd16(PARTY_X)); I(20);
	cpu.b = 0x20; I(7);
	for (;;) {
		cpu.a = cpu.e; I(4);
		op_sub8(mem_read(HL), 0, 0); I(7);
		SET_HL(HL + 1); I(6);
		if (cpu.f & FZ) {
			I(7);
			cpu.a = cpu.d; I(4);
			op_sub8(mem_read(HL), 0, 0); I(7);
			if (cpu.f & FZ) { I(12); break; }
			I(7);
		} else I(12);
		SET_HL(HL + 1); I(6);
		if (--cpu.b) I(13);
		else { I(8); I(10); return; }  // none
	}
	cpu.a = 0x1F; I(7);                // $AD57
	op_sub8(cpu.b, 0, 1); I(4);
	op_sub8(0xFD, cpu.f & FC, 1); I(7);
	mem_write(0xC9A0, cpu.a); I(13);
	wr16(0xC96D, DE); I(20);
	cpu.a = mem_read(0xC9BB); I(13);
	SET_HL(0xAD6D); I(10);
	enter_place();
}

static int town_event_here(void) {     // town_step would go on into the original
	if (mem_read(PARTY_X) >= 0x20 || mem_read(PARTY_Y) >= 0x20) return 1;
	uint8_t t = mem_read(0xC990);
	return (t == 0x1B || t == 0x1C) && mem_read(0xC959) == 0x1F;
}

static void town_step(void) {          // $AFAA
	if (mem_read(0xC9A1) && !native_on_fiber() && town_event_here()) { native_decline(); return; }
	cpu.a = mem_read(0xC9A1); I(13);
	op_or(cpu.a); I(4);
	if (ret_if(cpu.f & FZ)) return;
	cpu.a = mem_read(PARTY_X); I(13);
	op_sub8(0x20, 0, 0); I(7);
	int off = !(cpu.f & FC);
	if (!off) {
		I(7);
		cpu.a = mem_read(PARTY_Y); I(13);
		op_sub8(0x20, 0, 0); I(7);
		off = !(cpu.f & FC);
	}
	if (off) { I(12); native_continue_at(0xAFC9); return; }   // off the map: to the world
	I(7);
	cpu.a = mem_read(0xC990); I(13);
	op_sub8(0x1B, 0, 0); I(7);
	uint16_t on = 0;
	if (cpu.f & FZ) { I(12); on = 0xAFEF; }
	else {
		I(7);
		op_sub8(0x1C, 0, 0); I(7);
		if (cpu.f & FZ) { I(12); on = 0xB02A; }
		else { I(7); I(10); return; }
	}
	cpu.a = mem_read(0xC959); I(13);   // a ladder: in the Abyss only
	op_sub8(0x1F, 0, 0); I(7);
	if (ret_if(!(cpu.f & FZ))) return;
	native_continue_at(on);
}

static void map_cell_de(void) {        // $BAF3
	op_xor(cpu.a); I(4);
	cpu.d = op_rot(7, cpu.d); I(8);    // srl d / rra, twice
	op_rra(); I(4);
	cpu.d = op_rot(7, cpu.d); I(8);
	op_rra(); I(4);
	op_add8(cpu.e, 0); I(4);
	op_add8(cpu.e, 0); I(4);
	cpu.e = cpu.a; I(4);
	cpu.a = cpu.d; I(4);
	op_or(0xD0); I(7);
	cpu.d = cpu.a; I(4);
	cpu.a = mem_read(DE); I(7);
	I(10);
}

static int chunk_relative(uint16_t chunk, uint8_t *v) {   // $BAD5 / $BAE4: carry when off the map
	cpu.a = mem_read(chunk); I(13);
	for (int i = 0; i < 4; i++) { op_add8(cpu.a, 0); I(4); }
	op_neg(); I(8);
	op_add8(*v, 0); I(4);
	*v = cpu.a; I(4);
	op_sub8(0x20, 0, 0); I(7);
	op_ccf(); I(4);
	return ret_if(cpu.f & FC);
}

static void world_tile_at(void) {      // $BAD5
	if (chunk_relative(0xC987, &cpu.e)) return;
	if (chunk_relative(0xC988, &cpu.d)) return;
	map_cell_de();
}

static void tile_at(void) {            // $BACF
	cpu.a = mem_read(0xC9A0); I(13);
	op_or(cpu.a); I(4);
	if (cpu.f & FZ) { I(7); world_tile_at(); }
	else { I(12); map_cell_de(); }
}

static void view_step(uint16_t view, uint16_t party, int d) {   // ld hl / inc or dec (hl), twice / ret
	SET_HL(view); I(10);
	mem_write(HL, d > 0 ? op_inc8(mem_read(HL)) : op_dec8(mem_read(HL))); I(11);
	SET_HL(party); I(10);
	mem_write(HL, d > 0 ? op_inc8(mem_read(HL)) : op_dec8(mem_read(HL))); I(11);
	I(10);
}
static void view_west(void) { view_step(0xC985, PARTY_X, -1); }   // $B3DF
static void view_north(void) { view_step(0xC986, PARTY_Y, -1); }  // $B3E8
static void view_east(void) { view_step(0xC985, PARTY_X, 1); }    // $B3F1
static void view_south(void) { view_step(0xC986, PARTY_Y, 1); }   // $B3FA

Native native_town_cell = { "town_cell", 0xB5A4, 2, town_cell, OUT_A | OUT_F | OUT_HL, .long_calls = 1 };
Native native_door_ahead = { "door_ahead", 0xB464, 2, door_ahead, OUT_A | OUT_F | OUT_DE | OUT_HL, .long_calls = 1 };
Native native_world_step = { "world_step", 0xACDC, 2, world_step, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, .long_calls = 1 };
Native native_town_step = { "town_step", 0xAFAA, 2, town_step, OUT_A | OUT_F, .long_calls = 1 };
Native native_tile_at = { "tile_at", 0xBACF, 2, tile_at, OUT_A | OUT_F | OUT_DE, .long_calls = 1 };
Native native_world_tile_at = { "world_tile_at", 0xBAD5, 2, world_tile_at, OUT_A | OUT_F | OUT_DE, .long_calls = 1 };
Native native_map_cell_de = { "map_cell_de", 0xBAF3, 2, map_cell_de, OUT_A | OUT_F | OUT_DE, .long_calls = 1 };
Native native_view_west = { "view_west", 0xB3DF, 2, view_west, OUT_A | OUT_F | OUT_HL, .long_calls = 1 };
Native native_view_north = { "view_north", 0xB3E8, 2, view_north, OUT_A | OUT_F | OUT_HL, .long_calls = 1 };
Native native_view_east = { "view_east", 0xB3F1, 2, view_east, OUT_A | OUT_F | OUT_HL, .long_calls = 1 };
Native native_view_south = { "view_south", 0xB3FA, 2, view_south, OUT_A | OUT_F | OUT_HL, .long_calls = 1 };
Native native_abyss_isle = { "abyss_isle", 0xAEDA, 2, abyss_isle, OUT_A | OUT_F, .long_calls = 1 };
