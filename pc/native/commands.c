// The commands behind the command menu (bank 2), as C tasks. Each is entered
// from the menu with the turn loop's return address already kept in the
// slot, and ends at $8084 (refresh, wait a frame, back to the turn) or $8081
// (cancelled: "Aborted!" first) - both original code, reached by jumping
// there. Cycle counts are the original's.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define SLOT        cpu.ix
#define PARTY_XY    0xC953
#define MAP_KIND    0xC99F          // 1 in towns

static void call(uint16_t addr) { cpu.cycles += 17; call_original(addr); }
static void newline(void) { cpu.cycles += 17; text_newline(); }
static void done(void) { cpu.cycles += 10; native_continue_at(0x8084); }       // jp $8084
static void go(uint16_t addr) { cpu.cycles += 10; native_continue_at(addr); }  // jp addr

static void say_inline(uint16_t text) {   // call print_inline / the text follows
	uint16_t hl = HL;
	cpu.cycles += 17 + 19 + 17;
	text_print(text);
	cpu.cycles += 19 + 10;
	SET_HL(hl);
}

// a prompt's loop: "call frame_tick / call step / jr z,loop"; carry = cancelled
static int prompt(uint16_t site, uint16_t step) {
	for (;;) {
		task_frame_tick(site);
		call(step);
		cpu.cycles += 7;
		if (!(cpu.f & FZ)) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 10;                  // jp c,$8081
	return (cpu.f & FC) != 0;
}

// $8101: HL += DE (x and y separately); carry if still on the 32x32 map
static int step_on_map(void) {
	cpu.cycles += 17 + 4 + 4 + 4 + 7;
	cpu.l = (uint8_t)(cpu.l + cpu.e);
	cpu.a = cpu.l;
	if (cpu.l >= 0x20) { cpu.cycles += 11; cpu.f &= ~FC; return 0; }
	cpu.cycles += 5 + 4 + 4 + 4 + 7 + 10;
	cpu.h = (uint8_t)(cpu.h + cpu.d);
	cpu.a = cpu.h;
	op_sub8(0x20, 0, 0);
	return (cpu.f & FC) != 0;
}

// ---------------------------------------------------------------- talk --
// talk_command ($8091): "Direction:", then whoever is there. Across a shop
// counter (tiles $60-$7E) the shopkeeper's shop ($9652); a person with
// something to say: their conversation ($6DCE), or Lord British's ($812B);
// otherwise "Funny, no response!" ($810D).
static void talk_command(void) {
	call(0x4324);                      // the status window
	newline();
	call(0x4C28);                      // "Direction:"
	if (prompt(0x809A, 0x4C3C)) { native_continue_at(0x8081); return; }
	cpu.cycles += 13 + 7 + 10;
	if (mem_read(MAP_KIND) != 1) { native_continue_at(0x810D); return; }   // only in towns
	cpu.cycles += 19 + 17 + 100;       // $40B9: HL = the step that way
	cpu.a = mem_read(SLOT + 0x1A);
	SET_HL(rd16(0x40BF + 2 * cpu.a));
	cpu.cycles += 4 + 16;
	SET_DE(HL);
	SET_HL(rd16(PARTY_XY));
	cpu.cycles += 7;
	if (!step_on_map()) { cpu.cycles += 5; native_continue_at(0x810D); return; }
	cpu.cycles += 11;
	uint16_t at = HL;
	call(0xB5A4);                      // the map cell
	cpu.cycles += 7 + 10 + 7 + 7;
	uint8_t tile = mem_read(HL);
	SET_HL(at);
	cpu.a = tile;
	int counter = tile >= 0x60 && tile < 0x7F;
	if (tile < 0x60) cpu.cycles += 5;
	else { cpu.cycles += 7 + 7; if (!counter) cpu.cycles += 5; }
	if (counter) {                     // talk across the counter
		if (!step_on_map()) { go(0x810D); return; }
		cpu.cycles += 10;
		call(0x42E3);                  // whoever stands there
		cpu.cycles += 10;
		if (cpu.f & FC) { native_continue_at(0x810D); return; }
		call(0x48F7);                  // IY = them
		cpu.cycles += 19 + 7 + 10;
		if (mem_read(cpu.iy + 3) != 0x52) { native_continue_at(0x810D); return; }   // not a shopkeeper
		cpu.cycles += 4 + 4;
		cpu.a = (uint8_t)(cpu.h - cpu.d);   // the counter's row picks the shop
		go(0x9652);
		return;
	}
	call(0x42E3);                      // $80E6
	cpu.cycles += 10;
	if (cpu.f & FC) { native_continue_at(0x810D); return; }
	call(0x48F7);
	cpu.cycles += 19 + 4 + 10;
	if (mem_read(cpu.iy + 7) == 0) { native_continue_at(0x810D); return; }   // nothing to say
	cpu.cycles += 19 + 7 + 10;
	if (mem_read(cpu.iy + 3) == 0x5E) { native_continue_at(0x812B); return; }   // Lord British
	go(0x6DCE);
}
Native native_talk_command = { "talk_command", 0x8091, 2, talk_command, 0, 0, 0, .task = 1 };

// ----------------------------------------------------------- get chest --
// get_chest ($890D): "Who opens?", then that member opens the chest under
// the party ($5B13). In a dungeon ($957C) the dungeon's own Get ($8077).
static void get_chest(void) {
	newline();
	call(0x957C);
	cpu.cycles += 10;
	if (cpu.f & FZ) { native_continue_at(0x8077); return; }
	say_inline(0x8919);                // "Who opens? "
	call(0x4BCB);                      // member_pick_start
	if (prompt(0x8928, 0x4BD2)) { native_continue_at(0x8081); return; }
	cpu.cycles += 19;
	cpu.c = mem_read(SLOT + 0x08);
	call(0x4466);                      // can they?
	cpu.cycles += 10;
	if (cpu.f & FC) { native_continue_at(0x807C); return; }
	call(0x5B13);                      // open it
	done();
}
Native native_get_chest = { "get_chest", 0x890D, 2, get_chest, 0, 0, 0, .task = 1 };

// --------------------------------------------------------------- ready --
// ready ($8B28): Weapon or Armour, then the original's ready routines
// ($57D7, $5855: they wait for frames, so they run as original code).
static void ready(void) {
	cpu.cycles += 10;
	SET_HL(0x9613);
	call(0x2E7F);                      // the menu
	cpu.cycles += 7 + 17;
	text_char('-');
	cpu.cycles += 19;
	mem_write(SLOT + 0x1A, 0);
	for (;;) {
		task_frame_tick(0x8B37);
		cpu.cycles += 7 + 10;
		cpu.c = 2;
		SET_DE(0x0102);
		call(0x49F9);
		cpu.cycles += 7;
		if (!(cpu.f & FZ)) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 10;
	if (cpu.f & FC) { native_continue_at(0x8081); return; }
	cpu.cycles += 10 + 17 + 19 + 91 + 10;   // ld hl,$960F / call $806B: its name
	uint16_t name = rd16(0x960F + 2 * mem_read(SLOT + 0x1A));
	SET_HL(name);
	text_print(name);
	newline();
	cpu.cycles += 20 + 7;
	cpu.cycles += 17;                  // call $57D7 / $5855
	if (!(mem_read(SLOT + 0x1A) & 1)) { push16(0x8B59); native_continue_at(0x57D7); }
	else { cpu.cycles += 5; push16(0x8B5E); native_continue_at(0x5855); }
}
Native native_ready = { "ready", 0x8B28, 2, ready, 0, 0, 0, .task = 1 };

// ---------------------------------------------------------------- camp --
// camp_menu ($9254): Rest, New Order, Status, Save
static void rest(void);
static void new_order(void);

static void camp_menu(void) {
	cpu.cycles += 10;
	SET_HL(0x962E);
	call(0x2E7F);                      // the menu
	cpu.cycles += 7 + 17;
	text_char('-');
	cpu.cycles += 19;
	mem_write(SLOT + 0x1A, 0);
	for (;;) {
		task_frame_tick(0x9263);
		cpu.cycles += 7 + 10;
		cpu.c = 4;
		SET_DE(0x0102);
		call(0x49F9);
		cpu.cycles += 7;
		if (!(cpu.f & FZ)) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 10;
	if (cpu.f & FC) { native_continue_at(0x8081); return; }
	cpu.cycles += 10 + 17 + 19 + 91 + 10;   // ld hl,$9626 / call $806B: its name
	uint8_t choice = mem_read(SLOT + 0x1A);
	uint16_t name = rd16(0x9626 + 2 * choice);
	SET_HL(name);
	text_print(name);
	cpu.cycles += 19 + 10 + 10 + 10 + 52;   // the table at $9282
	cpu.a = choice;
	switch (choice) {
	case 0: rest(); break;
	case 1: new_order(); break;
	case 2: native_continue_at(0x9367); break;   // Status (original: it waits for frames)
	default: native_continue_at(0x9370); break;  // Save (original)
	}
}
Native native_camp_menu = { "camp_menu", 0x9254, 2, camp_menu, 0, 0, 0, .task = 1 };

// rest ($928A): only outdoors and on foot; camping is its own game mode ($15)
static void rest(void) {
	call(0x4324);
	newline();
	cpu.cycles += 13 + 7 + 7;
	uint8_t where = mem_read(MAP_KIND);
	if (where != 0) {
		cpu.cycles += 7 + 10;
		if (where != 2) { native_continue_at(0x8072); return; }   // not in a town
	} else cpu.cycles += 5;
	cpu.cycles += 13 + 7 + 7;
	if (mem_read(0xC959) != 0x1F) {    // not on a horse or ship
		call(0x4703);
		done();
		return;
	}
	cpu.cycles += 5 + 13 + 13 + 7 + 13 + 10;
	mem_write(0xC9B7, mem_read(0xC600));   // the mode to come back to
	mem_write(0xC600, 0x15);           // camping
	native_continue_at(0x0CD2);        // the main loop
}

// new_order ($92B7): swap two members (the Avatar stays first)
static void new_order(void) {
	newline();
	cpu.cycles += 13 + 7 + 7;
	if (mem_read(0xC957) < 2) {
		call(0x40D1);
		say_inline(0x92C7);            // "Unable to do that now."
		done();
		return;
	}
	cpu.cycles += 5;
	call(0x4BCB);
	say_inline(0x92E8);                // "Exchange "
	if (prompt(0x92F3, 0x4BD2)) { native_continue_at(0x8081); return; }
	cpu.cycles += 19 + 4 + 7;
	uint8_t first = mem_read(SLOT + 0x08);
	if (first == 0) goto lead;
	cpu.cycles += 19;
	mem_write(SLOT + 0x1B, first);
	say_inline(0x930A);                // " with "
	if (prompt(0x9315, 0x4BD2)) { native_continue_at(0x8081); return; }
	cpu.cycles += 19 + 4 + 7;
	uint8_t second = mem_read(SLOT + 0x08);
	if (second == 0) goto lead;
	cpu.cycles += 19 + 7;
	if (second == first) {
		call(0x46C2);                  // "the same one"
		done();
		return;
	}
	cpu.cycles += 5 + 4;
	cpu.c = second;
	call(0x48B6);                      // member_record
	cpu.cycles += 15 + 10 + 19;
	SET_HL(cpu.iy);
	cpu.c = first;
	call(0x48B6);
	cpu.cycles += 15 + 10 + 7;
	SET_DE(cpu.iy);
	cpu.b = 0x1D;
	call(0x3340);                      // swap the two records
	done();
	return;
lead:                                  // $9349: the Avatar must lead
	cpu.cycles += 5;
	call(0x40D1);
	call(0x5650);
	say_inline(0x9352);                // ", you must lead!"
	done();
}

// -------------------------------------------------------------- search --
// search ($8B61): look up the party's place (map, x, y) in the table of
// things to find ($8C07; in dungeons $8C76; 5 bytes each, ending with $FF)
// and go to its routine (original code); nothing there: "Nothing here!"
static void search(void) {
	call(0x4324);
	call(0x54CE);
	call(0x957C);
	cpu.cycles += 14 + 7;
	uint16_t iy = 0x8C76;
	if (!(cpu.f & FZ)) { cpu.cycles += 14; iy = 0x8C07; }
	else cpu.cycles += 5;
	cpu.cycles += 10 + 16 + 20;
	SET_DE(0x0005);
	SET_HL(rd16(PARTY_XY));
	SET_BC(rd16(0xC9A0));
	for (;; iy += 5) {
		cpu.iy = iy;
		cpu.cycles += 19 + 7 + 7;
		cpu.a = mem_read(iy);
		op_sub8(0xFF, 0, 0);
		if (cpu.a == 0xFF) break;
		const uint8_t want[3] = { cpu.c, cpu.l, cpu.h };
		int k;
		for (k = 0; k < 3; k++) {
			if (k) { cpu.cycles += 19; cpu.a = mem_read(iy + k); }
			cpu.cycles += 4 + 7;
			op_sub8(want[k], 0, 0);
			if (cpu.a != want[k]) break;
		}
		if (k == 3) {
			cpu.cycles += 19 + 19 + 4;
			SET_HL(rd16(iy + 3));
			native_continue_at(HL);
			return;
		}
		cpu.cycles += 5 + 15 + 12;
	}
	cpu.cycles += 5;
	call(0x40D1);
	cpu.cycles += 10;
	SET_HL(0x8E61);                    // "Nothing here!"
	call(0x5519);
	done();
}
Native native_search = { "search", 0x8B61, 2, search, 0, 0, 0, .task = 1 };

// ----------------------------------------------------------------- use --
// use_command ($8942): Item or Stone. Stones are their own routine ($651E);
// items are picked from a menu (the sextant only once owned, $C916) and
// each needs one in the pack ($C913 + item).
static void use_torch(void);
static void use_gem(void);
static void use_key(void);
static void use_sextant(void);

static void use_command(void) {
	cpu.cycles += 10;
	SET_HL(0x95FA);
	call(0x2E7F);                      // the menu
	cpu.cycles += 7 + 17;
	text_char('-');
	cpu.cycles += 19;
	mem_write(SLOT + 0x1A, 0);
	for (;;) {
		task_frame_tick(0x8951);
		cpu.cycles += 7 + 10;
		cpu.c = 2;
		SET_DE(0x0102);
		call(0x49F9);
		cpu.cycles += 7;
		if (!(cpu.f & FZ)) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 10;
	if (cpu.f & FC) { native_continue_at(0x8081); return; }
	cpu.cycles += 10 + 17 + 19 + 91 + 10;   // ld hl,$95F6 / call $806B: its name
	uint16_t name = rd16(0x95F6 + 2 * mem_read(SLOT + 0x1A));
	SET_HL(name);
	text_print(name);
	newline();
	cpu.cycles += 20 + 10;
	if (mem_read(SLOT + 0x1A) & 1) {   // a stone
		cpu.cycles += 10 + 17;         // $8B22 (it waits for frames: original code)
		push16(0x8B25);
		native_continue_at(0x651E);
		return;
	}
	cpu.cycles += 7;
	cpu.a = 0x0A;
	call(0x7552);
	say_inline(0x897C);                // "which: "
	cpu.cycles += 19;
	mem_write(SLOT + 0x1A, 0);
	for (;;) {
		task_frame_tick(0x8988);
		cpu.cycles += 7 + 13 + 4 + 7;
		cpu.c = 3;
		cpu.a = mem_read(0xC916);      // the sextant, once found
		op_or(cpu.a);
		if (cpu.a) { cpu.cycles += 4; cpu.c = 4; }
		else cpu.cycles += 5;
		cpu.cycles += 10;
		SET_DE(0x0202);
		call(0x49F9);
		cpu.cycles += 7;
		if (!(cpu.f & FZ)) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 10;
	if (cpu.f & FC) { native_continue_at(0x8081); return; }
	call(0x4324);
	cpu.cycles += 19 + 11;
	uint8_t item = mem_read(SLOT + 0x1A);
	cpu.a = item;
	push16((uint16_t)(cpu.a << 8 | cpu.f));
	cpu.cycles += 10 + 91 + 17;        // ld hl,$8AF8 / rst $20 / call print_text_hl
	name = rd16(0x8AF8 + 2 * item);
	SET_HL(name);
	text_print(name);
	newline();
	uint16_t af = pop16();
	cpu.a = af >> 8; cpu.f = (uint8_t)af;
	cpu.cycles += 10 + 10 + 30 + 7 + 4 + 7;   // ld hl,$C913 / rst $30 / ld a,(hl) / or a / jr nz
	SET_HL(0xC913 + item);
	cpu.a = mem_read(HL);
	op_or(cpu.a);
	if (!cpu.a) {
		call(0x40D1);
		say_inline(0x89BF);            // "...you don't have it."
		done();
		return;
	}
	cpu.cycles += 5 + 19 + 10 + 10 + 52;   // ld a,(ix+$1A) / ld hl,$89DF / jp $0018
	cpu.a = item;
	switch (item) {
	case 0: use_torch(); break;
	case 1: use_gem(); break;
	case 2: use_key(); break;
	default: use_sextant(); break;
	}
}
Native native_use_command = { "use_command", 0x8942, 2, use_command, 0, 0, 0, .task = 1 };

// is the map mode `mode`? (ld a,(map_mode) / cp mode / jp nz/nc,$8072)
static uint8_t map_mode(void) {
	cpu.cycles += 13 + 7 + 10;
	cpu.a = mem_read(MAP_KIND);
	return cpu.a;
}

// $89E7: a torch lights a dungeon for a while
static void use_torch(void) {
	if (map_mode() != 2) { native_continue_at(0x8072); return; }
	cpu.cycles += 10 + 11 + 13 + 7 + 7;
	mem_write(0xC913, mem_read(0xC913) - 1);
	unsigned light = mem_read(0xC95B) + 0xC8;
	if (light > 0xFF) { cpu.cycles += 7; light = 0xFF; }
	else cpu.cycles += 5;
	cpu.cycles += 13;
	cpu.a = (uint8_t)light;
	mem_write(0xC95B, cpu.a);
	say_inline(0x8A02);                // "Ignite Torch!"
	done();
}

// $8A14: a gem shows the map
static void use_gem(void) {
	cpu.cycles += 10 + 11;
	mem_write(0xC914, mem_read(0xC914) - 1);
	say_inline(0x8A1B);                // "Peer at a Gem!"
	cpu.cycles += 17;                  // the map (it waits for frames: original code)
	push16(0x8A2E);
	native_continue_at(0x61FC);
}

// $8A72: a locked door ($3A) at (L, H) on the town map opens ($3B)
static void unlock(void) {
	cpu.cycles += 17 + 4 + 7 + 5;
	if (cpu.l >= 0x20) { cpu.cycles += 6; return; }
	cpu.cycles += 4 + 7 + 5;
	if (cpu.h >= 0x20) { cpu.cycles += 6; return; }
	call(0xB5A4);                      // the map cell
	cpu.cycles += 7 + 7 + 5;
	cpu.a = mem_read(HL);
	op_sub8(0x3A, 0, 0);
	if (cpu.a != 0x3A) { cpu.cycles += 6; return; }
	cpu.cycles += 10 + 4 + 10 + 4 + 10;
	mem_write(HL, 0x3B);
	cpu.l++;
	mem_write(HL, 0x00);
	cpu.c++;
}

// $8A31: a key opens the locked doors next to the party (towns only)
static void use_key(void) {
	if (map_mode() != 1) { native_continue_at(0x8072); return; }
	cpu.cycles += 7;
	cpu.c = 1;
	static const int8_t dx[4] = { 1, -1, 0, 0 }, dy[4] = { 0, 0, 1, -1 };
	for (int i = 0; i < 4; i++) {
		cpu.cycles += 16 + 4;
		SET_HL(rd16(PARTY_XY));
		cpu.l = (uint8_t)(cpu.l + dx[i]);
		cpu.h = (uint8_t)(cpu.h + dy[i]);
		unlock();
	}
	cpu.cycles += 4 + 10;
	if (--cpu.c == 0) { native_continue_at(0x8072); return; }   // no door
	cpu.cycles += 10 + 11;
	mem_write(0xC915, mem_read(0xC915) - 1);
	say_inline(0x8A62);                // "Jimmy lock!"
	done();
}

// $8ADB: a coordinate as two letters, A' B"
static void say_coordinate(uint8_t a) {
	cpu.cycles += 17 + 11 + 16;
	cpu.cycles += 17 + 7 + 7 + 10;
	text_char((uint8_t)('A' + (a >> 4)));
	cpu.cycles += 7 + 17;
	text_char('\'');
	cpu.cycles += 10 + 17 + 7 + 7 + 10;
	text_char((uint8_t)('A' + (a & 0x0F)));
	cpu.cycles += 7 + 10;
	text_char('"');
}

// $8A88: the sextant tells where the party is (not in dungeons)
static void use_sextant(void) {
	if (map_mode() >= 2) { native_continue_at(0x8072); return; }
	say_inline(0x8A93);                // "Location is ... Latitude: "
	cpu.cycles += 13;
	say_coordinate(mem_read(PARTY_XY + 1));
	say_inline(0x8AC3);                // "Longitude: "
	cpu.cycles += 13;
	say_coordinate(mem_read(PARTY_XY));
	newline();
	done();
}
