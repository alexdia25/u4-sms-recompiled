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

// -------------------------------------------------------------- attack --
// attack ($848A): "Direction:", then whoever is there. Some can't be
// attacked (the table at $85CD). In towns everyone guarding (guards $50,
// Lord British $5E) turns hostile; elsewhere attacking the harmless costs
// karma. Then the fight ($3BD9). Nobody there: on a ship the broadsides
// ($8534, original code: the cannonballs fly frame by frame), else
// "Nothing to Attack!". In dungeons the dungeon's own ($8077).
static void karma_cost(void) {        // $85EE: compassion, justice, honour -5
	static const uint8_t virtues[3] = { 1, 3, 5 };
	for (int i = 0; i < 3; i++) {
		cpu.cycles += 7 + 7 + (i < 2 ? 17 : 10);
		cpu.a = 5;
		cpu.c = virtues[i];
		call_original(0x426F);         // karma_lose
	}
}

static void attack_nothing(void) {     // $85D2
	call(0x40D1);
	say_inline(0x85D8);                // "Nothing to Attack!"
	done();
}

static void attack(void) {
	call(0x4324);
	newline();
	call(0x957C);
	cpu.cycles += 10;
	if (cpu.f & FZ) { native_continue_at(0x8077); return; }
	call(0x4C28);                      // "Direction:"
	if (prompt(0x8499, 0x4C3C)) { native_continue_at(0x8081); return; }
	cpu.cycles += 19 + 17 + 100;       // $40B9: HL = the step that way
	cpu.a = mem_read(SLOT + 0x1A);
	SET_HL(rd16(0x40BF + 2 * cpu.a));
	cpu.cycles += 13 + 4 + 4 + 13 + 4 + 4;
	cpu.l = (uint8_t)(mem_read(PARTY_XY) + cpu.l);
	cpu.a = mem_read(PARTY_XY + 1);
	op_add8(cpu.h, 0);
	cpu.h = cpu.a;
	call(0x42E3);                      // whoever stands there
	cpu.cycles += 7;
	if (!(cpu.f & FC)) {
		call(0x48F7);                  // IY = them
		cpu.cycles += 19 + 10 + 10;
		cpu.a = mem_read(cpu.iy + 3);
		int n = 0, found = 0;          // cpir over the five that can't be attacked
		for (;;) {
			uint8_t v = mem_read(0x85CD + n++);
			if (v == cpu.a) { found = 1; break; }
			if (n == 5) break;
			cpu.cycles += 21;
		}
		cpu.cycles += 16;
		SET_HL(0x85CD + n);
		SET_BC(5 - n);
		cpu.cycles += 7;
		if (!found) {
			cpu.cycles += 13 + 7 + 7;
			if (mem_read(MAP_KIND) == 1) {   // in towns: the guards turn on the party
				cpu.cycles += 15 + 14 + 10 + 7;
				push16(cpu.iy);
				SET_DE(0x0008);
				cpu.b = 0x20;
				for (uint16_t p = 0xC651; ; p += 8) {
					cpu.iy = p;
					cpu.cycles += 19 + 7 + 7;
					uint8_t who = mem_read(p + 3);
					cpu.a = who;
					if (who == 0x50) cpu.cycles += 5;
					else { cpu.cycles += 7 + 7; if (who != 0x5E) cpu.cycles += 5; }
					if (who == 0x50 || who == 0x5E) {
						cpu.cycles += 19 + 19;
						mem_write(p + 6, 0xFF);
						mem_write(p + 7, 0x00);
					}
					cpu.cycles += 15;
					if (--cpu.b == 0) { cpu.cycles += 8; break; }
					cpu.cycles += 13;
				}
				cpu.cycles += 14 + 12;
				cpu.iy = pop16();
				cpu.cycles += 17;
				karma_cost();
			} else {
				cpu.cycles += 5 + 19 + 17;
				cpu.a = mem_read(cpu.iy + 3);
				call_original(0x430A);     // carry: hostile anyway
				cpu.cycles += 7;
				if (cpu.f & FC) cpu.cycles += 5;
				else { cpu.cycles += 17; karma_cost(); }
			}
			cpu.cycles += 10;
			native_continue_at(0x3BD9);    // the fight
			return;
		}
		cpu.cycles += 5;
	} else cpu.cycles += 5;
	cpu.cycles += 13 + 4 + 10;             // $8504: nobody to attack
	if (mem_read(MAP_KIND) != 0) { attack_nothing(); return; }
	cpu.cycles += 13 + 10 + 17;            // switch on the transport: ships fire
	uint8_t t = mem_read(0xC959);
	cpu.a = t;
	if (t < 0x10 || t > 0x13) {
		cpu.cycles += 58 * 4 + 22 + 10;
		cpu.b = 0;
		SET_HL(0x8523);
		attack_nothing();
		return;
	}
	cpu.cycles += 58 * (t - 0x10) + 79 + 20 + 10;
	cpu.b = (uint8_t)(t + 1);
	SET_HL((t & 1) ? 0x852D : 0x8524);
	cpu.a = 0x85;                      // (the switch pops its return address into AF)
	int odd = mem_read(SLOT + 0x1A) & 1;   // broadside: the direction across the ship
	if ((t & 1) ? odd : !odd) { attack_nothing(); return; }
	if (!(t & 1)) cpu.cycles += 12;
	native_continue_at(0x8534);
}
Native native_attack = { "attack", 0x848A, 2, attack, 0, 0, 0, .task = 1 };

// --------------------------------------------------------------- magic --
// magic ($863F): Cast (cast_spell $5B8C, original code: it waits for frames)
// or Mix: pick a spell (two pages of 13), tick reagents, say how many, and
// the reagents are used up one mixture at a time. A mixture comes out when
// the reagents ticked are exactly the spell's recipe ($88F3) and all there.
#define REAGENTS   0xC92F
#define MIXTURES   0xC937
#define TEXT_COL   0xC639
#define JOY        0xC0B0

// call switch_on_a ($334A) on a table of n keys: the index matched, or -1.
// Matched, it pops its return address (`back`) into AF.
static int switch_on(const uint8_t *keys, int n, uint8_t a, uint16_t back) {
	cpu.cycles += 17;
	cpu.a = a;
	for (int i = 0; i < n; i++)
		if (keys[i] == a) {
			cpu.cycles += 58 * i + 79;
			cpu.a = back >> 8;
			cpu.f = (uint8_t)back;
			return i;
		}
	cpu.cycles += 58 * n + 22;
	return -1;
}

static uint8_t reagent_bar(void) {     // $87B1: the cursor line of the reagent picked
	cpu.cycles += 17 + 19 + 7 + 7;
	uint8_t r = mem_read(SLOT + 0x1E);
	if (r >= 8) { cpu.cycles += 4 + 10; cpu.a = 0; }
	else { cpu.cycles += 5 + 4 + 4 + 10; cpu.a = (uint8_t)(2 * r + 1); }
	return cpu.a;
}

static void mix_reagents(void);

static void magic(void) {
	cpu.cycles += 10;
	SET_HL(0x95D9);
	call(0x2E7F);                      // the menu
	cpu.cycles += 7 + 17;
	text_char('-');
	cpu.cycles += 19;
	mem_write(SLOT + 0x1A, 0);
	for (;;) {
		task_frame_tick(0x864E);
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
	cpu.cycles += 10 + 17 + 19 + 91 + 10;   // ld hl,$95D5 / call $806B: its name
	uint16_t name = rd16(0x95D5 + 2 * mem_read(SLOT + 0x1A));
	SET_HL(name);
	text_print(name);
	newline();
	cpu.cycles += 20 + 10;
	if (!(mem_read(SLOT + 0x1A) & 1)) {   // Cast
		cpu.cycles += 17;
		push16(0x8671);
		native_continue_at(0x5B8C);
		return;
	}
	cpu.cycles += 10 + 4 + 7;          // $8674: any reagents at all?
	uint8_t any = 0;
	for (int i = 0; i < 8; i++) {
		any |= mem_read(REAGENTS + i);
		cpu.cycles += 7 + 6 + (i < 7 ? 13 : 8);
	}
	cpu.a = any;
	op_or(any);
	SET_HL(REAGENTS + 8);
	cpu.b = 0;
	cpu.cycles += 7;
	if (!any) {
		say_inline(0x8683);            // "You have no reagents!"
		done();
		return;
	}
	cpu.cycles += 5;
	say_inline(0x86A0);                // "For what spell: "
	cpu.cycles += 19 + 19 + 7;
	mem_write(SLOT + 0x1C, 0);         // the spell on the page
	mem_write(SLOT + 0x1D, 0);         // the page
	cpu.a = 0x0D;
	call(0x7552);                      // the first page of spells
	cpu.cycles += 12;
	static const uint8_t spell_keys[6] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20 };
	for (;;) {
		cpu.cycles += 19 + 7;          // $8701: the cursor on the spell
		cpu.a = (uint8_t)(mem_read(SLOT + 0x1C) + 2);
		call(0x49D5);
		cpu.cycles += 12;
		int k;
		do {                           // $86C0: wait for a key
			task_frame_tick(0x86C0);
			cpu.cycles += 13 + 7 + 10;
			uint8_t a = mem_read(JOY) & 0x7F;
			SET_HL(0x86D0);
			k = switch_on(spell_keys, 6, a, 0x86CE);
			if (k < 0) cpu.cycles += 12;
		} while (k < 0);
		if (k == 4) { native_continue_at(0x8081); return; }   // button 1
		if (k == 5) break;             // button 2: this spell
		if (k < 2) {                   // up / down, around the page of 13
			cpu.cycles += 7 + (k == 0 ? 12 : 0);
			cpu.b = k == 0 ? 0x0C : 0x0E;
			cpu.cycles += 19 + 7;
			cpu.a = (uint8_t)(mem_read(SLOT + 0x1C) + 2);
			call(0x49E0);
			cpu.cycles += 19 + 4 + 4 + 7;
			cpu.l = (uint8_t)(mem_read(SLOT + 0x1C) + cpu.b);
			cpu.a = cpu.l;
			cpu.h = 0x0D;
			call(0x2AFC);              // divide_8: H = the remainder
			cpu.cycles += 19;
			mem_write(SLOT + 0x1C, cpu.h);
		} else {                       // left / right: the other page
			cpu.cycles += 19 + 7 + 19 + 7;
			uint8_t page = mem_read(SLOT + 0x1D) ^ 1;
			mem_write(SLOT + 0x1D, page);
			cpu.a = (uint8_t)(page + 0x0D);
			call(0x7552);
			cpu.cycles += 12;
		}
		call(0x40D9);                  // $86FE: beep
	}
	call(0x40C9);                      // $871A: click
	cpu.cycles += 4 + 20 + 7;
	uint8_t spell = 0;
	if (mem_read(SLOT + 0x1D) & 1) { cpu.cycles += 7; spell = 0x0D; }
	else cpu.cycles += 5;
	cpu.cycles += 19 + 19 + 7;
	spell = (uint8_t)(spell + mem_read(SLOT + 0x1C));
	mem_write(SLOT + 0x1C, spell);     // the spell, 0-25
	cpu.a = (uint8_t)(spell + 0x65);
	cpu.cycles += 17 + 17;             // call print_item_name / its call item_name
	call_original(0x340F);
	text_print(HL);
	cpu.cycles += 19 + 19 + 7;
	mem_write(SLOT + 0x1E, 0);         // the reagent picked
	mem_write(SLOT + 0x1F, 0);         // the reagents ticked, one bit each
	cpu.a = 0x11;
	call(0x7552);                      // the reagents
	cpu.cycles += 12;
	mix_reagents();
}
Native native_magic = { "magic", 0x863F, 2, magic, 0, 0, 0, .task = 1 };

static void mix_reagents(void) {
	static const uint8_t reagent_keys[4] = { 0x01, 0x02, 0x10, 0x20 };
	for (;;) {
		// $8781: a mark by each reagent, ticked ($40) or not ($3A)
		cpu.cycles += 16 + 11 + 10 + 16 + 10 + 10;
		uint16_t col = rd16(TEXT_COL);
		push16(col);
		wr16(TEXT_COL, 0x021C);
		SET_HL(TEXT_COL + 1);
		cpu.b = 8;
		cpu.c = 0x80;
		do {
			cpu.cycles += 19 + 4 + 7 + 7;
			int ticked = mem_read(SLOT + 0x1F) & cpu.c;
			cpu.a = 0x3A;
			if (ticked) { cpu.cycles += 7; cpu.a = 0x40; }
			else cpu.cycles += 5;
			cpu.cycles += 17;
			call_original(0x5460);     // put_glyph
			cpu.cycles += 11 + 11 + 8;
			mem_write(HL, (uint8_t)(mem_read(HL) + 2));
			cpu.c = (uint8_t)(cpu.c >> 1 | cpu.c << 7);
			cpu.cycles += --cpu.b ? 13 : 8;
		} while (cpu.b);
		cpu.cycles += 10 + 16;
		col = pop16();
		wr16(TEXT_COL, col);
		SET_HL(col);
		reagent_bar();
		call(0x49D5);                  // the cursor on it
		cpu.cycles += 10;
		for (;;) {                     // $8740: wait for a key
			int k;
			do {
				task_frame_tick(0x8740);
				cpu.cycles += 13 + 7 + 10;
				uint8_t a = mem_read(JOY) & 0x7F;
				SET_HL(0x8750);
				k = switch_on(reagent_keys, 4, a, 0x874E);
				if (k < 0) cpu.cycles += 12;
			} while (k < 0);
			if (k == 2) { native_continue_at(0x8081); return; }   // button 1
			if (k < 2) {               // up / down, around the 8 reagents and Done
				cpu.cycles += 7 + (k == 0 ? 12 : 0);
				cpu.b = k == 0 ? 0x08 : 0x0A;
				reagent_bar();
				call(0x49E0);
				cpu.cycles += 19 + 4 + 4 + 7;
				cpu.l = (uint8_t)(mem_read(SLOT + 0x1E) + cpu.b);
				cpu.a = cpu.l;
				cpu.h = 0x09;
				call(0x2AFC);          // divide_8
				cpu.cycles += 19;
				mem_write(SLOT + 0x1E, cpu.h);
				reagent_bar();
				call(0x49D5);
				call(0x40D9);
				cpu.cycles += 12;
				continue;
			}
			// button 2 ($87BD): tick or untick the reagent, or Done
			cpu.cycles += 19 + 7 + 7;
			uint8_t r = mem_read(SLOT + 0x1E);
			if (r == 8) { cpu.cycles += 5; goto chosen; }
			cpu.cycles += 10 + 48 + 4 + 10;
			SET_HL(REAGENTS + r);
			cpu.a = mem_read(HL);
			op_or(cpu.a);
			if (!cpu.a) continue;      // none of it
			cpu.cycles += 19 + 4 + 7;
			uint8_t bit = 1;
			for (int i = 0; i <= r; i++) {
				bit = (uint8_t)(bit >> 1 | bit << 7);
				cpu.cycles += 4 + (i < r ? 13 : 8);
			}
			cpu.cycles += 4 + 19 + 19 + 4 + 11;
			cpu.b = bit;
			uint8_t ticks = mem_read(SLOT + 0x1F) ^ bit;
			mem_write(SLOT + 0x1F, ticks);
			cpu.a = ticks;
			op_and(bit);
			uint8_t a = cpu.a, f = cpu.f;
			if (a) { cpu.cycles += 17; call_original(0x40C9); }   // ticked: click
			else cpu.cycles += 10;
			cpu.cycles += 10;
			cpu.a = a; cpu.f = f;
			if (!a) { cpu.cycles += 17; call_original(0x40D1); }  // unticked
			else cpu.cycles += 10;
			cpu.cycles += 10;
			break;                     // jp $8781
		}
	}
chosen:                                // $87E8: Done
	call(0x40C9);
	newline();
	cpu.cycles += 19 + 4 + 7;
	uint8_t ticks = mem_read(SLOT + 0x1F);
	cpu.a = ticks;
	op_or(ticks);
	if (!ticks) {
		say_inline(0x87F7);            // "None!"
		done();
		return;
	}
	cpu.cycles += 5;
	say_inline(0x8804);                // "How many mixtures? "
	cpu.cycles += 7;
	cpu.a = 2;
	call(0x4A66);                      // number_entry_start
	for (;;) {
		task_frame_tick(0x881D);
		call(0x4A9F);                  // number_entry_step
		cpu.cycles += 7;
		if (!(cpu.f & FC)) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 19;
	uint8_t want = cpu.l;
	mem_write(SLOT + 0x20, want);
	newline();
	cpu.cycles += 4 + 4 + 10;
	if (!want) { native_continue_at(0x8084); return; }
	say_inline(0x8833);                // "Mixing"
	cpu.cycles += 19;
	mem_write(SLOT + 0x1A, 3);
	for (;;) {                         // "..." a dot every 10 turns
		task_wait_frames(0x8840, 10);
		cpu.cycles += 7 + 17;
		text_char('.');
		cpu.cycles += 23 + 7;
		uint8_t left = (uint8_t)(mem_read(SLOT + 0x1A) - 1);
		mem_write(SLOT + 0x1A, left);
		if (!left) break;
		cpu.cycles += 5;
	}
	newline();
	cpu.cycles += 19 + 19 + 19;
	mem_write(SLOT + 0x16, want);
	mem_write(SLOT + 0x21, 0);         // how many came out
	uint8_t spell = mem_read(SLOT + 0x1C);
	for (;;) {                         // $885A: one mixture
		cpu.cycles += 10 + 19 + 10;
		uint8_t d = ticks;
		uint8_t missing = 1;
		for (int i = 0; i < 8; i++) {
			cpu.cycles += 8 + 7;
			int on = d & 0x80;
			d = (uint8_t)(d << 1 | d >> 7);
			if (on) {
				cpu.cycles += 4 + 7 + 4 + 7;
				missing++;
				uint8_t have = mem_read(REAGENTS + i);
				cpu.a = have;
				if (have) { cpu.cycles += 4 + 11; missing--; mem_write(REAGENTS + i, have - 1); }
				else cpu.cycles += 5;
			} else cpu.cycles += 5;
			cpu.cycles += 6 + (i < 7 ? 13 : 8);
		}
		cpu.d = d;
		cpu.b = 0;
		SET_HL(REAGENTS + 8);
		cpu.cycles += 4 + 7;
		cpu.c = (uint8_t)(missing - 1);
		if (missing == 1) {            // everything ticked was there
			cpu.cycles += 19 + 10 + 48 + 19 + 7;
			cpu.a = mem_read(0x88F3 + spell);
			if (cpu.a == ticks) {      // the spell's recipe
				cpu.cycles += 23 + 19 + 10 + 30 + 7 + 7 + 7;
				mem_write(SLOT + 0x21, mem_read(SLOT + 0x21) + 1);
				SET_HL(MIXTURES + spell);
				uint8_t n = mem_read(HL);
				cpu.a = n;
				if (n < 0x63) { cpu.cycles += 11; mem_write(HL, n + 1); }
				else cpu.cycles += 5;
			} else {
				cpu.cycles += 5;
				SET_HL(0x88F3 + spell);
			}
		} else cpu.cycles += 5;
		cpu.cycles += 23 + 7;          // dec (ix+$16) / jr nz
		uint8_t left = (uint8_t)(mem_read(SLOT + 0x16) - 1);
		mem_write(SLOT + 0x16, left);
		if (!left) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 19 + 4 + 7;
	uint8_t made = mem_read(SLOT + 0x21);
	cpu.a = made;
	if (!made) {
		say_inline(0x889E);            // "It fizzles!"
		cpu.cycles += 12;
	} else {
		cpu.cycles += 5 + 19 + 7;
		if (made == want) {
			say_inline(0x88B4);        // "Success!"
			cpu.cycles += 12;
		} else {
			cpu.cycles += 5;
			say_inline(0x88C2);        // "You have only enough reagents for "
			cpu.cycles += 19 + 7 + 17;
			SET_HL(made);
			call_original(0x5627);     // print_number_auto
		}
	}
	newline();
	done();
}

// ---------------------------------------------------------- cast_spell --
// cast_spell ($5B8C, bank 1): who casts (outside combat), then a list of the
// spells with mixtures that work here (by map mode, $5D72), eight at a time,
// each with its mixtures and M.P. ($5DE1). Casting uses a mixture; with M.P.
// enough the spell's own routine runs ($5DFB, original code). Called with
// the return address kept in the slot (task_enter_2); returns carry clear
// when a spell ran.
#define CASTER     0xC97F
#define SPELL_MP   0x5DE1

// $5D54: from spell C, step by B (+1 / -1) to the next spell with mixtures
// that works here; carry when there is none
static int next_spell(void) {
	cpu.cycles += 17 + 13 + 10 + 91 + 4;
	uint16_t allowed = rd16(0x5D72 + 2 * mem_read(MAP_KIND));
	SET_DE(allowed);
	for (;;) {
		cpu.cycles += 4 + 4 + 4 + 7 + 4;
		cpu.c = (uint8_t)(cpu.c + cpu.b);
		cpu.a = cpu.c;
		if (cpu.c >= 0x1A) { cpu.cycles += 11; cpu.f |= FC; return 1; }
		cpu.cycles += 5 + 4 + 4 + 48 + 4 + 7;
		SET_HL(allowed + cpu.c);
		cpu.a = mem_read(HL);
		op_or(cpu.a);
		if (!cpu.a) { cpu.cycles += 5; continue; }
		cpu.cycles += 4 + 10 + 48 + 4 + 7;
		SET_HL(MIXTURES + cpu.c);
		cpu.a = mem_read(HL);
		op_or(cpu.a);
		if (!cpu.a) { cpu.cycles += 5; continue; }
		cpu.cycles += 10;
		return 0;
	}
}

static void cast_end(int ran) {        // $5D49 / $5D50: jp task_return_2
	cpu.cycles += 4 + 10 + 42;
	if (ran) cpu.f &= ~FC; else cpu.f |= FC;
	SET_HL(rd16(SLOT + 0x26));
	native_continue_at(HL);
}

static void cast_cancel(void) {        // $5D4D: "Aborted!"
	call(0x472D);
	cast_end(0);
}

// $5C01: the page of up to eight spells from (ix+$1C)
static void spell_page(void) {
	call(0x2E1B);                      // menu_window_clear
	cpu.cycles += 10 + 19 + 4 + 7;
	SET_DE(0xCA66);
	cpu.c = (uint8_t)(mem_read(SLOT + 0x1C) - 1);
	cpu.b = 8;
	for (;;) {
		cpu.cycles += 4 + 4 + 7 + 7;
		cpu.c++;
		cpu.a = cpu.c;
		if (cpu.c >= 0x1A) { cpu.cycles += 5; break; }
		cpu.cycles += 13 + 10 + 91 + 4 + 48 + 4 + 7;
		uint16_t allowed = rd16(0x5D72 + 2 * mem_read(MAP_KIND));
		SET_HL(allowed + cpu.c);
		cpu.a = mem_read(HL);
		op_or(cpu.a);
		if (!cpu.a) { cpu.cycles += 5; continue; }
		cpu.cycles += 4 + 10 + 48 + 4 + 7;
		SET_HL(MIXTURES + cpu.c);
		uint8_t have = mem_read(HL);
		cpu.a = have;
		op_or(have);
		if (!have) { cpu.cycles += 5; continue; }
		cpu.cycles += 11 + 4 + 7;
		push16((uint16_t)(cpu.a << 8 | cpu.f));
		cpu.a = (uint8_t)(cpu.c + 0x65);
		call(0x340F);                  // item_name: the spell's name
		cpu.cycles += 11;
		push16(DE);
		call(0x2D6C);                  // into the window
		SET_DE(pop16());
		cpu.cycles += 10 + 10 + 11 + 4 + 10 + 4 + 7 + 11;
		SET_HL(DE + 0x004C);
		SET_DE(HL);
		uint16_t af = pop16();
		cpu.a = af >> 8; cpu.f = (uint8_t)af;
		SET_HL(cpu.a);
		push16(BC);
		call(0x2D87);                  // mixtures
		SET_BC(pop16());
		cpu.cycles += 10 + 7;
		cpu.a = 0x28;
		call(0x2D9A);                  // "/"
		cpu.cycles += 4 + 10 + 48 + 4 + 7 + 11;
		cpu.a = mem_read(SPELL_MP + cpu.c);
		SET_HL(cpu.a);
		push16(BC);
		call(0x2D87);                  // M.P.
		SET_BC(pop16());
		cpu.cycles += 10 + 7;
		cpu.a = 0x29;
		call(0x2D9A);
		cpu.cycles += 10 + 11 + 4;
		uint16_t at = DE;
		SET_DE(at + 0x0028);
		SET_HL(at);
		if (--cpu.b == 0) { cpu.cycles += 8; break; }
		cpu.cycles += 13;
	}
	cpu.cycles += 19;                  // $5C5F
	cpu.c = mem_read(SLOT + 0x1D);
	call(0x49A6);                      // the bar on the line picked
	call(0x2E54);                      // menu_window_show
}

static void cast_spell(void) {
	uint16_t back = pop16();
	cpu.cycles += 17 + 77;             // task_enter_2
	wr16(SLOT + 0x26, back);
	SET_HL(back);
	cpu.cycles += 10;
	SET_BC(0x01FF);
	int none = next_spell();           // the first spell there is
	cpu.cycles += 19 + 19 + 19 + 7;
	mem_write(SLOT + 0x1C, cpu.c);     // the first spell on the page
	mem_write(SLOT + 0x1D, 0);         // the line picked
	mem_write(SLOT + 0x1E, cpu.c);     // the spell picked
	if (none) {
		say_inline(0x5BA4);            // "You have no mixtures!"
		cpu.cycles += 10;
		cast_end(0);
		return;
	}
	cpu.cycles += 5 + 13 + 7 + 7;
	if (mem_read(MAP_KIND) < 3) {      // outside combat: who casts
		say_inline(0x5BC8);            // "Player: "
		call(0x4BCB);
		if (prompt(0x5BD4, 0x4BD2)) { cast_cancel(); return; }
		cpu.cycles += 19 + 13 + 4;
		uint8_t who = mem_read(SLOT + 0x08);
		mem_write(CASTER, who);
		cpu.a = cpu.c = who;
		call(0x4466);                  // member_able
		cpu.cycles += 7;
		if (cpu.f & FC) {
			call(0x46D0);
			cpu.cycles += 10;
			cast_end(0);
			return;
		}
		cpu.cycles += 5;
	} else cpu.cycles += 5;
	say_inline(0x5BF4);                // "Spell name: "
	spell_page();
	static const uint8_t keys[4] = { 0x01, 0x02, 0x10, 0x20 };
	for (;;) {
		int k;
		do {                           // $5C68: wait for a key
			task_frame_tick(0x5C68);
			cpu.cycles += 13 + 7 + 10;
			uint8_t a = mem_read(JOY) & 0x7F;
			SET_HL(0x5C78);
			k = switch_on(keys, 4, a, 0x5C76);
			if (k < 0) cpu.cycles += 12;
		} while (k < 0);
		if (k == 2) { cast_cancel(); return; }   // button 1
		if (k == 3) break;             // button 2: cast it
		int up = k == 0;               // $5C85 up / $5CB6 down
		cpu.cycles += 7 + 19;
		cpu.b = up ? 0xFF : 0x01;
		cpu.c = mem_read(SLOT + 0x1E);
		next_spell();
		cpu.cycles += 7;
		if (cpu.f & FC) { cpu.cycles += 5; continue; }   // no more that way
		cpu.cycles += 19;
		mem_write(SLOT + 0x1E, cpu.c);
		call(0x40D9);                  // beep
		cpu.cycles += 19 + (up ? 4 : 7) + 7;   // ld a,(ix+$1D) / or a or cp 7 / jr z
		uint8_t line = mem_read(SLOT + 0x1D);
		cpu.a = line;
		if (up ? line != 0 : line != 7) {   // move the bar
			cpu.cycles += 23 + 4;
			mem_write(SLOT + 0x1D, up ? line - 1 : line + 1);
			cpu.c = line;
			call(0x4999);              // off
			cpu.cycles += 4;
			cpu.c = up ? line - 1 : line + 1;
			call(0x49A6);              // on
			cpu.cycles += 12;
			continue;
		}
		cpu.cycles += 5 + 7 + 19;      // scroll the page by one spell
		cpu.b = up ? 0xFF : 0x01;
		cpu.c = mem_read(SLOT + 0x1C);
		next_spell();
		cpu.cycles += 19 + 10;
		mem_write(SLOT + 0x1C, cpu.c);
		spell_page();
	}
	call(0x40C9);                      // $5CE8: click
	cpu.cycles += 19 + 7;
	uint8_t spell = mem_read(SLOT + 0x1E);
	cpu.a = (uint8_t)(spell + 0x65);
	cpu.cycles += 17 + 17;             // call print_item_name / its call item_name
	call_original(0x340F);
	text_print(HL);
	cpu.cycles += 7 + 17;
	text_char('!');
	newline();
	call(0x5DCE);                      // the caster's status
	cpu.cycles += 19 + 10 + 30 + 11;
	SET_HL(MIXTURES + spell);
	mem_write(HL, mem_read(HL) - 1);   // a mixture used
	cpu.cycles += 19 + 10 + 48 + 19 + 20;
	uint8_t mp = mem_read(SPELL_MP + spell);
	mem_write(SLOT + 0x1F, mp);
	SET_HL(SPELL_MP + spell);
	SET_BC(rd16(CASTER));
	call(0x48B6);                      // member_record: IY
	cpu.cycles += 19 + 19 + 7;
	cpu.a = mem_read(cpu.iy + 9);
	op_sub8(mp, 0, 0);
	if (!(cpu.f & FC)) {               // M.P. enough: the spell
		cpu.cycles += 19 + 10 + 10 + 52;
		cpu.a = spell;
		SET_HL(rd16(0x5DFB + 2 * spell));
		native_continue_at(HL);
		return;
	}
	cpu.cycles += 5;
	say_inline(0x5D2B);                // "M.P. too low!"
	call(0x40D1);
	say_inline(0x5D40);                // "Failed!"
	op_or(cpu.a);
	cast_end(1);
}
Native native_cast_spell = { "cast_spell", 0x5B8C, 1, cast_spell, 0, 0, 0, .task = 1 };
