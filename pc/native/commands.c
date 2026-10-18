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
static void tcall(uint16_t addr, uint16_t site);
static void yield_at(uint16_t site);
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
// call $7588 at site (bank 1): the status pages: 0 a member's (left and
// right pick the member), 1-5 the party's things, 6 the reagents or the
// mixtures (left and right); up and down turn the pages, button 2 starts
// over, button 1 leaves
static void status(uint16_t site) {
	cpu.cycles += 17 + 17 + 10 + 19 + 19 + 19 + 10;   // call $0AB5: its return kept in the slot
	wr16(SLOT + 0x26, (uint16_t)(site + 3));
	SET_HL((uint16_t)(site + 3));
start:
	cpu.cycles += 19 * 3 + 13 + 7 + 7;   // $758B
	mem_write(SLOT + 0x1A, 0);
	mem_write(SLOT + 0x1B, 0);
	mem_write(SLOT + 0x1C, 0);
	cpu.a = mem_read(0xC99F);
	op_sub8(3, 0, 0);
	if (cpu.a < 3) cpu.cycles += 5;
	else {                             // in combat: the member whose turn it is
		cpu.cycles += 13 + 19;
		cpu.a = mem_read(0xC97F);
		mem_write(SLOT + 0x1B, cpu.a);
	}
page:
	cpu.cycles += 19 + 4 + 7;          // $75A4: the page drawn
	{
		uint8_t pg = mem_read(SLOT + 0x1A);
		cpu.a = pg;
		op_or(pg);
		if (!pg) { cpu.cycles += 5 + 19 + 12; cpu.a = mem_read(SLOT + 0x1B); }
		else {
			cpu.cycles += 7 + 7;
			op_sub8(6, 0, 0);
			if (pg == 6) {
				cpu.cycles += 5 + 19 + 7;
				cpu.a = mem_read(SLOT + 0x1C);
				op_add8(0x0D, 0);
			} else {
				cpu.cycles += 7 + 12;
				cpu.a = pg;
				op_add8(0x07, 0);
			}
		}
	}
	tcall(0x7552, 0x75BC);
	for (;;) {
		task_frame_tick(0x75BF);
		cpu.cycles += 13 + 10 + 17;    // ld a / ld hl / call switch_on_a
		uint8_t joy = mem_read(0xC0B0);
		cpu.a = joy;
		static const uint8_t keys[6] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20 };
		int k;
		for (k = 0; k < 6 && keys[k] != joy; k++) {}
		if (k == 6) {
			cpu.cycles += 58 * 6 + 22 + 12;
			cpu.b = 0;
			SET_HL(0x75DF);
			op_inc8(0xFF);
			continue;
		}
		cpu.cycles += 58 * k + 79;
		cpu.b = (uint8_t)(joy + 1);
		SET_HL(rd16(0x75CE + 3 * k));
		cpu.a = cpu.l;
		cpu.f = 0xCB;                  // (the switch pops its return address into AF)
		switch (k) {
		case 0: case 1:                // up, down: the page
			cpu.cycles += 7 + (k == 0 ? 12 : 0) + 19 + 4 + 4 + 7;
			cpu.b = k == 0 ? 6 : 8;
			cpu.a = mem_read(SLOT + 0x1A);
			op_add8(cpu.b, 0);
			cpu.l = cpu.a; cpu.h = 7;
			call(0x2AFC);              // H = it mod 7
			cpu.cycles += 19;
			mem_write(SLOT + 0x1A, cpu.h);
			call(0x40D9);
			cpu.cycles += 12;
			goto page;
		case 2: case 3: {              // left, right
			cpu.cycles += 7 + (k == 2 ? 12 : 0) + 19 + 7 + 7;
			uint8_t b = k == 2 ? 0xFF : 0x01;
			cpu.b = b;
			uint8_t pg = mem_read(SLOT + 0x1A);
			cpu.a = pg;
			op_sub8(6, 0, 0);
			if (pg == 6) {             // reagents or mixtures
				cpu.cycles += 5 + 19 + 7 + 19;
				cpu.a = mem_read(SLOT + 0x1C);
				op_xor(1);
				mem_write(SLOT + 0x1C, cpu.a);
			} else {
				cpu.cycles += 4 + 7;
				cpu.a = pg;
				op_or(pg);
				if (pg) { cpu.cycles += 5; continue; }
				cpu.cycles += 13 + 4 + 4 + 19 + 4;   // the member
				uint8_t n = mem_read(0xC957);
				cpu.h = n;
				cpu.a = (uint8_t)(n + b);
				op_add8(mem_read(SLOT + 0x1B), 0);
				cpu.l = cpu.a;
				call(0x2AFC);          // H = L mod the party's size
				cpu.cycles += 19 + 12;
				mem_write(SLOT + 0x1B, cpu.h);
			}
			call(0x40D9);
			cpu.cycles += 10;
			goto page;
		}
		case 4:                        // button 1: back
			call(0x40D1);
			cpu.cycles += 10 + 19 + 19 + 4;   // jp $0ABE
			SET_HL(rd16(SLOT + 0x26));
			return;
		default:                       // button 2: from the start
			call(0x40C9);
			cpu.cycles += 10;
			goto start;
		}
	}
}

static void ldir(void) {
	uint16_t n = BC;
	cpu.cycles += 21 * (n - 1) + 16;
	for (; n; n--) { mem_write(DE, mem_read(HL)); SET_HL(HL + 1); SET_DE(DE + 1); }
	SET_BC(0);
	cpu.f &= ~(FH | FN | FP);
}

// call $951B: DE, BC = the place of save slot +$1A in the menu
static void slot_place(void) {
	cpu.cycles += 17 + 19 + 4 + 4 + 7 + 4 + 7 + 10 + 10;
	cpu.a = (uint8_t)(mem_read(SLOT + 0x1A) * 4);
	op_add8(5, 0);
	cpu.d = cpu.a;
	cpu.e = 0x12;
	SET_BC(0x040E);
}

static void slot_highlight(int on, uint16_t site) {   // call $950F (on) / $9515 at site
	cpu.cycles += 17;
	slot_place();
	cpu.cycles += 10;                  // jp $2DE3 / $2DC4
	task_call(on ? 0x2DE3 : 0x2DC4, (uint16_t)(site + 3));
}

// call $94BC at back - 3: the save in SRAM at HL, if any, shown at DE: the
// Avatar's name, level, sex, class, and the turn count
static void slot_summary(uint16_t back) {
	cpu.cycles += 17 + 11 + 10 + 10;
	push16(DE);
	SET_DE(0xC1C0); SET_BC(0x0040);
	tcall(0x0C49, 0x94C3);             // the save's head, out of SRAM
	cpu.cycles += 10 + 13 + 4;
	SET_DE(pop16());
	cpu.a = mem_read(0xC1C0);
	op_or(cpu.a);
	if (!cpu.a) { cpu.cycles += 11; return; }   // none
	cpu.cycles += 5 + 11 + 10;
	push16(DE);
	SET_HL(0xC1C1);
	tcall(0x2D6C, 0x94D0);             // the name
	cpu.cycles += 10 + 10 + 11 + 4 + 13;
	uint16_t at = pop16();             // ld hl,n / add hl,de / ex de,hl: on along the line
	SET_DE(op_add16(0x0040, at)); SET_HL(at);
	cpu.a = mem_read(0xC1D6);
	tcall(0x2D9A, 0x94DC);
	cpu.cycles += 10 + 11 + 4 + 13;
	at = DE;
	SET_DE(op_add16(0x0004, at)); SET_HL(at);
	cpu.a = mem_read(0xC1CE);
	tcall(0x2D9A, 0x94E7);
	cpu.cycles += 10;
	SET_HL(0x9529);
	tcall(0x2D6C, 0x94ED);
	cpu.cycles += 13 + 7;
	cpu.a = mem_read(0xC1CF);
	op_add8(0x30, 0);
	tcall(0x2D9A, 0x94F5);
	cpu.cycles += 10 + 11 + 4 + 10;
	at = DE;
	SET_DE(op_add16(0x0028, at));
	SET_HL(0xC1D1);
	tcall(0x36FF, 0x9500);             // the turns
	cpu.cycles += 10;
	SET_HL(0xC068);
	tcall(0x2D6C, 0x9506);
	cpu.cycles += 10 + 10;
	SET_HL(0x9531);
	task_call(0x2D6C, back);           // jp $2D6C
}

static void save_slots(uint16_t site) {   // call $9498 at site: the menu of the three saves
	cpu.cycles += 17 + 10;
	SET_HL(0x9537);
	tcall(0x2E79, 0x949B);
	static const uint16_t from[3] = { 0x8040, 0x8080, 0x80C0 }, at[3] = { 0xCBA6, 0xCCA6, 0xCDA6 };
	static const uint16_t back[3] = { 0x94A7, 0x94B0, 0x94B9 };
	for (int i = 0; i < 3; i++) {
		cpu.cycles += 10 + 10;
		SET_HL(from[i]); SET_DE(at[i]);
		slot_summary(back[i]);
	}
	cpu.cycles += 10;                  // jp $2E54
	task_call(0x2E54, (uint16_t)(site + 3));
}

// $9370: Save: not in towns ($46DF says so); else the slot picked (up,
// down; button 1 cancels) and the game written to SRAM: $C500-$C5FF,
// $C651-$C9BC and a head with the Avatar's name, level, sex, class and turns
static void camp_save(void) {
	newline();
	cpu.cycles += 13 + 4 + 7;
	cpu.a = mem_read(0xC99F);
	op_or(cpu.a);
	if (cpu.a) {
		tcall(0x46DF, 0x9379);
		done();
		return;
	}
	cpu.cycles += 5;
	say_inline(0x9382);                // "Save game. Please select a number:"
	cpu.cycles += 10;
	SET_HL(0x8100);
	tcall(0x0C3D, 0x93AB);             // the last one used
	cpu.cycles += 19;
	mem_write(SLOT + 0x1A, cpu.a);
	save_slots(0x93B1);
	slot_highlight(1, 0x93B4);
	for (;;) {
		task_frame_tick(0x93B7);
		cpu.cycles += 13 + 10 + 17;    // ld a / ld hl / call switch_on_a
		uint8_t joy = mem_read(0xC0B0);
		cpu.a = joy;
		static const uint8_t keys[4] = { 0x01, 0x02, 0x10, 0x20 };
		int k;
		for (k = 0; k < 4 && keys[k] != joy; k++) {}
		if (k == 4) {
			cpu.cycles += 58 * 4 + 22 + 12;
			cpu.b = 0;
			SET_HL(0x93D1);
			op_inc8(0xFF);
			continue;
		}
		cpu.cycles += 58 * k + 79;
		cpu.b = (uint8_t)(joy + 1);
		SET_HL(rd16(0x93C6 + 3 * k));
		cpu.a = cpu.l;
		cpu.f = 0xC3;                  // (the switch pops its return address into AF)
		if (k == 2) { native_continue_at(0x8081); return; }   // cancelled
		if (k == 3) break;
		call(0x40D9);                  // up or down: the click
		slot_highlight(0, k == 0 ? 0x93D5 : 0x93DF);
		cpu.cycles += 7 + (k == 0 ? 12 : 0) + 19 + 4 + 7;
		cpu.a = (uint8_t)(k == 0 ? 2 : 4);
		op_add8(mem_read(SLOT + 0x1A), 0);
		cpu.l = cpu.a; cpu.h = 3;
		call(0x2AFC);                  // H = it mod 3
		cpu.cycles += 19;
		mem_write(SLOT + 0x1A, cpu.h);
		slot_highlight(1, 0x93F0);
		cpu.cycles += 12;
	}
	call(0x40C9);                      // $93F5: saved
	cpu.cycles += 19 + 13 + 10;
	cpu.a = mem_read(SLOT + 0x1A);
	mem_write(0xC0CD, cpu.a);
	SET_HL(0x8100);
	tcall(0x0C43, 0x9401);             // the last one used, kept
	cpu.cycles += 7;
	op_add8(0x31, 0);
	call(0x548F);                      // its number
	newline();
	cpu.cycles += 7 + 13 + 10 + 10 + 10;
	mem_write(0xC1C0, 0xFF);
	SET_HL(0xC75E); SET_DE(0xC1C1); SET_BC(0x000D);
	ldir();
	cpu.cycles += 13 * 6 + 10 + 10 + 10;
	mem_write(0xC1CE, mem_read(0xC76B));
	mem_write(0xC1CF, mem_read(0xC75B));
	cpu.a = mem_read(0xC76C);
	mem_write(0xC1D0, cpu.a);
	SET_HL(0xC901); SET_DE(0xC1D1); SET_BC(0x0005);
	ldir();
	cpu.cycles += 13 + 13 + 19 + 4 + 4 + 4 + 7 + 14 + 15 + 10 + 19 + 19 + 10;
	cpu.a = mem_read(0xC0CE);
	mem_write(0xC1D6, cpu.a);
	cpu.a = (uint8_t)(mem_read(SLOT + 0x1A) * 4);
	op_add8(0, 0);
	SET_DE(cpu.a);
	cpu.iy = (uint16_t)(0x948C + cpu.a);
	SET_HL(0xC500);
	SET_DE(rd16(cpu.iy));
	SET_BC(0x0100);
	tcall(0x0C49, 0x9459);             // into SRAM
	cpu.cycles += 10 + 10;
	SET_HL(0xC651); SET_BC(0x036C);
	tcall(0x0C49, 0x9462);
	cpu.cycles += 10 + 19 + 19 + 10;
	SET_HL(0xC1C0);
	SET_DE(rd16(cpu.iy + 2));
	SET_BC(0x0040);
	tcall(0x0C49, 0x9471);
	save_slots(0x9474);
	slot_highlight(1, 0x9477);
	say_inline(0x947D);                // "Done."
	task_wait_button(0x9483);
	newline();
	done();
}

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
	case 2:                            // $9367: Status
		status(0x9367);
		newline();
		done();
		break;
	default: camp_save(); break;
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
	cpu.cycles += 5 + 19 + 10 + 10 + 10 + 52;   // ld a,(ix+$1A) / ld hl,$89DF / jp $0018 / jp $0083
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


// call $61FC at site (bank 1): the map of the land or the dungeon level,
// the party blinking on it, until a button
static void gem_map(uint16_t site) {
	cpu.cycles += 17 + 17 + 10 + 19 + 19 + 19 + 10;   // call $0AC5: its return kept in the slot
	wr16(SLOT + 0x28, (uint16_t)(site + 3));
	SET_HL((uint16_t)(site + 3));
	cpu.cycles += 7 + 10 + 10;
	cpu.a = 0x13;
	SET_HL(0xB16F); SET_DE(0x3E60);
	tcall(0x0788, 0x6207);             // the party's mark
	cpu.cycles += 10;
	SET_HL(0x031F);
	tcall(0x0843, 0x620D);
	tcall(0x4F8A, 0x6210);             // the map
	say_inline(0x6216);                // "PUSH BUTTON"
	for (;;) {
		yield_at(0x6222);
		cpu.cycles += 13 + 7 + 7;
		cpu.a = mem_read(0xC0B0);
		op_and(0x30);
		if (cpu.a) { cpu.cycles += 5; break; }
		tcall(0x335C, 0x622C);
		cpu.cycles += 13 + 7 + 10 + 7;
		cpu.a = mem_read(0xC99F);
		op_sub8(2, 0, 0);
		SET_DE(0x4949);                // in a dungeon: the middle
		if (cpu.a == 2) cpu.cycles += 5;
		else {                         // on the land: where the party is
			cpu.cycles += 13 + 4 + 4 + 7 + 4 + 13 + 4 + 4 + 7 + 4;
			cpu.a = (uint8_t)(mem_read(0xC985) * 4);
			op_add8(9, 0);
			cpu.e = cpu.a;
			cpu.a = (uint8_t)(mem_read(0xC986) * 4);
			op_add8(9, 0);
			cpu.d = cpu.a;
		}
		cpu.cycles += 7 + 23 + 20;
		cpu.c = 0xF3;
		uint8_t t = (uint8_t)(mem_read(SLOT + 0x1F) + 1);
		mem_write(SLOT + 0x1F, t);
		if (t & 2) tcall(0x09DA, 0x6252);   // the mark, every other two turns
		else cpu.cycles += 10;
		cpu.cycles += 12;
	}
	tcall(0x276D, 0x6257);             // the view back
	tcall(0x129D, 0x625A);
	newline();
	cpu.cycles += 10 + 19 + 19 + 4;    // jp $0ACE
	SET_HL(rd16(SLOT + 0x28));
}

// $8A14: a gem shows the map
static void use_gem(void) {
	cpu.cycles += 10 + 11;
	mem_write(0xC914, mem_read(0xC914) - 1);
	say_inline(0x8A1B);                // "Peer at a Gem!"
	gem_map(0x8A2B);
	done();
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

// dungeon_attack ($8603): "Direction:", then the creature there, if any
// (6C6A: carry when nobody); attacking the harmless costs karma. Then the
// fight ($3B52).
static void dungeon_attack(void) {
	call(0x4324);
	newline();
	call(0x4C28);                      // "Direction:"
	if (prompt(0x860C, 0x4C3C)) { native_continue_at(0x8081); return; }
	cpu.cycles += 19 + 17 + 100;       // $40B9: HL = the step that way
	cpu.a = mem_read(SLOT + 0x1A);
	SET_HL(rd16(0x40BF + 2 * cpu.a));
	cpu.cycles += 13 + 4 + 7 + 4 + 13 + 4 + 7 + 4;
	cpu.l = (mem_read(PARTY_XY) + cpu.l) & 0x0F;
	cpu.h = (mem_read(PARTY_XY + 1) + cpu.h) & 0x0F;
	cpu.a = cpu.h;
	call(0x6C6A);                      // carry: nobody there
	cpu.cycles += 7;
	if (cpu.f & FC) { cpu.cycles += 5; attack_nothing(); return; }
	call(0x48F7);                      // IY = them
	cpu.cycles += 19;
	cpu.a = mem_read(cpu.iy + 3);
	call(0x430A);                      // carry: hostile anyway
	if (cpu.f & FC) cpu.cycles += 10;
	else { cpu.cycles += 17; karma_cost(); }
	cpu.cycles += 10;
	native_continue_at(0x3B52);        // the fight
}
Native native_dungeon_attack = { "dungeon_attack", 0x8603, 2, dungeon_attack, 0, 0, 0, .task = 1 };

// ------------------------------------------------------- dungeon search --
#define UNDERFOOT   0xC990          // the dungeon cell the party stands on

static void found_nothing(void) {     // $8FDC
	call(0x40D1);
	say_inline(0x8FE2);                // "You find Nothing!"
	done();
}

// $922D: the member picked loses A * 100 hit points... at most; a flash
static void search_hurt(uint8_t a) {
	cpu.cycles += 11 + 11 + 11 + 4 + 7;
	cpu.l = a;
	cpu.h = 0x64;
	call(0x2AA1);                      // multiply_8x8
	cpu.cycles += 19;
	cpu.c = mem_read(SLOT + 0x08);
	call(0x4679);                      // member_hurt
	cpu.cycles += 7;
	cpu.a = 0x13;
	call(0x05F1);
	cpu.cycles += 19;
	cpu.c = mem_read(SLOT + 0x08);
	call(0x49A6);
	call(0x041C);
	call(0x4999);
	call(0x041C);
	cpu.cycles += 10 + 10 + 10 + 10;
}

// "Who ...?" and a member who can act; 0 when cancelled (or can't act)
static int search_who(uint16_t site) {
	call(0x4BCB);
	if (prompt(site, 0x4BD2)) { native_continue_at(0x8081); return 0; }
	cpu.cycles += 19;
	cpu.c = mem_read(SLOT + 0x08);
	call(0x4466);                      // member_able
	cpu.cycles += 10;
	if (cpu.f & FC) { native_continue_at(0x807C); return 0; }
	call(0x48B6);                      // IY = their record
	return 1;
}

// $9088: a stat goes up by the next amount at DE (to 50 at most); its name
static void stat_up(uint16_t name) {
	cpu.cycles += 7 + 6 + 4;
	uint8_t up = mem_read(DE);
	SET_DE(DE + 1);
	cpu.a = up;
	if (!up) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 4 + 7 + 7;
	unsigned v = up + cpu.b;
	if ((uint8_t)v >= 0x32) { cpu.cycles += 7; v = 0x32; }
	else cpu.cycles += 5;
	cpu.cycles += 4 + 17;
	cpu.b = (uint8_t)v;
	SET_HL(name);
	text_print(name);
	cpu.cycles += 10 + 10;
	SET_HL(0x90DD);
	text_print(0x90DD);                // " + 5"
}

// $9002: an orb: it hurts whoever touches it, and raises their stats
static void touch_orb(void) {
	say_inline(0x9005);                // "You find a Magical Ball... Who touches?"
	if (!search_who(0x9031)) return;
	cpu.cycles += 16 + 17;
	SET_HL(rd16(PARTY_XY));
	call_original(0x6DC0);             // HL = the cell
	cpu.cycles += 10 + 13 + 7 + 4 + 4 + 10 + 30 + 4 + 7 + 6;
	mem_write(HL, 0);                  // the orb is gone
	uint8_t row = (uint8_t)((mem_read(0xC9A0) - 0x12) * 4);
	SET_HL(DE);                        // (ex de,hl)
	SET_DE(0x909D + row + 1);
	uint8_t hurt = mem_read(0x909D + row);
	cpu.a = hurt;
	cpu.cycles += 17;
	search_hurt(hurt);
	static const uint16_t names[3] = { 0x90BD, 0x90C6, 0x90D0 };
	for (int i = 0; i < 3; i++) {      // strength, dexterity, intelligence
		cpu.cycles += 10 + 19;
		cpu.b = mem_read(cpu.iy + 6 + i);
		cpu.cycles += 17;
		stat_up(names[i]);
		cpu.cycles += 19;
		mem_write(cpu.iy + 6 + i, cpu.b);
	}
	done();
}

static void fountain_no_effect(void) { // $9138
	say_inline(0x913B);                // "Hmmm--No Effect!"
	cpu.cycles += 10;
}

// $90E3: a fountain: by its kind, it heals, hurts, cures or poisons
static void drink(void) {
	say_inline(0x90E6);                // "You find a Fountain. Who drinks?"
	if (!search_who(0x910B)) return;
	cpu.cycles += 13 + 7 + 10 + 73;    // ld a,(cell) / and $0F / ld hl / rst $18: the handler returns to $912B
	uint8_t kind = mem_read(UNDERFOOT) & 0x0F;
	cpu.a = kind;
	SET_HL(rd16(0x912E + 2 * kind));
	switch (kind) {
	case 0: fountain_no_effect(); break;
	case 1: {                          // $914E: healing
		cpu.cycles += 19 * 4 + 4 + 15 + 7;
		uint16_t hp = rd16(cpu.iy), max = rd16(cpu.iy + 2);
		if (hp == max) { cpu.cycles += 5; fountain_no_effect(); break; }
		cpu.cycles += 19 + 19;
		wr16(cpu.iy, max);
		say_inline(0x9168);            // "Ahh-Refreshing!"
		cpu.cycles += 10;
		break;
	}
	case 2:                            // $917A: bad water
		say_inline(0x917D);            // "Bleck--Nasty!"
		cpu.cycles += 7 + 10;
		search_hurt(1);
		break;
	case 3:                            // $9191: it cures poison
		cpu.cycles += 19 + 7 + 7;
		if (mem_read(cpu.iy + 0x1C) != 'P') { cpu.cycles += 5; fountain_no_effect(); break; }
		cpu.cycles += 19;
		mem_write(cpu.iy + 0x1C, 'G');
		say_inline(0x919F);            // "Hmm--Delicious!"
		cpu.cycles += 10;
		break;
	case 4:                            // $91B1: poison
		cpu.cycles += 19 + 7 + 7;
		if (mem_read(cpu.iy + 0x1C) == 'P') { cpu.cycles += 5; fountain_no_effect(); break; }
		cpu.cycles += 19;
		mem_write(cpu.iy + 0x1C, 'P');
		say_inline(0x91BF);            // "Argh-Choke-Gasp!"
		cpu.cycles += 7 + 10;
		search_hurt(1);
		break;
	default:                           // past the table: as the original does
		push16(0x912B);
		native_continue_at(HL);
		return;
	}
	done();                            // $912B: jp $8084
}

// $91D6: an altar holds the dungeon's stone, once
static void altar(void) {
	cpu.cycles += 13 + 7 + 7 + 10;
	uint8_t d = (uint8_t)(mem_read(0xC9A0) - 0x12);
	if (d >= 6) { found_nothing(); return; }
	cpu.cycles += 4 + 4 + 7;
	uint8_t bit = 0x80;
	for (int i = 0; i <= d; i++) {
		bit = (uint8_t)(bit << 1 | bit >> 7);
		cpu.cycles += 8 + (i < d ? 13 : 8);
	}
	cpu.c = bit;
	cpu.b = 0;
	cpu.cycles += 10 + 7 + 4 + 10;
	SET_HL(0xC955);
	if (mem_read(HL) & bit) { found_nothing(); return; }
	cpu.cycles += 7 + 4 + 7;
	mem_write(HL, mem_read(HL) | bit);
	call(0x40E1);
	say_inline(0x91F9);                // "You find the "
	cpu.cycles += 13 + 7 + 17 + 17;
	cpu.a = (uint8_t)(mem_read(0xC9A0) + 0x8E);
	call_original(0x340F);             // its name
	text_print(HL);
	say_inline(0x9212);                // " stone!"
	cpu.cycles += 7 + 7;
	cpu.a = 5; cpu.c = 5;
	call(0x4255);                      // karma_gain
	cpu.cycles += 10 + 7;
	SET_HL(0x00C8);
	cpu.c = 0;
	call(0x4205);                      // member_gain_xp
	done();
}

// dungeon_search ($8FCB): what lies underfoot: an orb, a fountain, an altar
static void dungeon_search(void) {
	call(0x4324);
	newline();
	cpu.cycles += 13 + 7 + 10 + 17;
	uint8_t what = mem_read(UNDERFOOT) & 0xF0;
	cpu.a = what;
	SET_HL(0x8FF8);
	static const uint8_t kinds[3] = { 0x70, 0x90, 0xB0 };
	int i;
	for (i = 0; i < 3 && kinds[i] != what; i++) {}
	if (i == 3) { cpu.cycles += 58 * 3 + 22; found_nothing(); return; }
	cpu.cycles += 58 * i + 79;
	if (i == 0) touch_orb();
	else if (i == 1) drink();
	else altar();
}
Native native_dungeon_search = { "dungeon_search", 0x8FCB, 2, dungeon_search, 0, 0, 0, .task = 1 };

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

static void run_spell(uint8_t spell);

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
		cpu.cycles += 19 + 10 + 10 + 10 + 52;   // ld a / ld hl / jp $0018 / jp $0083
		cpu.a = spell;
		SET_HL(rd16(0x5DFB + 2 * spell));
		run_spell(spell);
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

// -------------------------------------------------------------- spells --
// The spells ($5DFB table). Each ends at $5D49 (done), or $5D3A ("Failed!")
// - both reached by jp, here cast_end / spell_failed.
#define VIEW_XY    0xC985
#define EFFECT     0xC976               // a timed effect in combat; $4E = negated

static void spell_done(void) { cpu.cycles += 10; cast_end(1); }   // jp $5D49

static void spell_failed(void) {       // $5D3A (the caller counts its jump there)
	call(0x40D1);
	say_inline(0x5D40);                // "Failed!"
	op_or(cpu.a);
	cast_end(1);
}

// $6304: the caster pays the M.P.; a flash. 0 when magic is negated.
static int spend_mp(void) {
	cpu.cycles += 17 + 20;
	SET_BC(rd16(CASTER));
	call(0x48B6);                      // member_record
	cpu.cycles += 19 + 19 + 19;
	uint8_t mp = (uint8_t)(mem_read(cpu.iy + 9) - mem_read(SLOT + 0x1F));
	mem_write(cpu.iy + 9, mp);
	call(0x5DCE);                      // the caster's status
	cpu.cycles += 7;
	cpu.a = 0x15;
	call(0x05F1);                      // sound
	call(0x041C);                      // wait_vblank
	call(0x41A9);
	call(0x041C);
	call(0x41A9);
	call(0x041C);
	cpu.cycles += 13 + 7;
	cpu.a = mem_read(EFFECT);
	op_sub8(0x4E, 0, 0);
	if (cpu.a != 0x4E) { cpu.cycles += 11; return 1; }
	cpu.cycles += 5 + 10 + 10;         // pop af / jp $5D3A
	spell_failed();
	return 0;
}

// $6335 / $6350 / $6369: "Outdoors Only!" etc. unless the map mode fits
static int only_here(int ok, uint16_t text) {
	cpu.cycles += 17 + 13 + 7;
	if (ok) { cpu.cycles += 11; return 1; }
	cpu.cycles += 5;
	say_inline(text);
	cpu.cycles += 12 + 10 + 10;        // jr / pop af / jp $5D3A
	spell_failed();
	return 0;
}
static int only_outdoors(void) { return only_here(mem_read(MAP_KIND) == 0, 0x633E); }

// $641F: "Who: " and a member; the call's return kept (task_enter_3)
static int spell_who(uint16_t back) {
	cpu.cycles += 17 + 17 + 77;
	wr16(SLOT + 0x28, back);
	say_inline(0x6425);
	call(0x4BCB);
	if (prompt(0x642E, 0x4BD2)) { cast_cancel(); return 0; }
	cpu.cycles += 10 + 42;             // jp task_return_3
	SET_HL(back);
	return 1;
}

// $643C: "Direction:"; the step that way in (ix+$0E), (ix+$10)
static int spell_direction(uint16_t back) {
	cpu.cycles += 17 + 17 + 77;
	wr16(SLOT + 0x28, back);
	call(0x4C28);
	if (prompt(0x6442, 0x4C3C)) { cast_cancel(); return 0; }
	cpu.cycles += 19 + 17 + 100 + 19 + 19;
	cpu.a = mem_read(SLOT + 0x1A);
	SET_HL(rd16(0x40BF + 2 * cpu.a));
	mem_write(SLOT + 0x0E, cpu.l);
	mem_write(SLOT + 0x10, cpu.h);
	cpu.cycles += 10 + 42;
	SET_HL(back);
	return 1;
}

// Awaken, Cure, Resurrect: a member with status `from` (S, P, D) is good
// again (G). Awakened in combat, the figure stands up.
static void restore_status(uint16_t back, uint8_t from) {
	if (!spell_who(back) || !spend_mp()) return;
	cpu.cycles += 19;
	cpu.c = mem_read(SLOT + 0x08);
	call(0x48B6);
	cpu.cycles += 19 + 7 + 10;
	cpu.a = mem_read(cpu.iy + 0x1C);
	op_sub8(from, 0, 0);
	if (cpu.a != from) { spell_failed(); return; }
	cpu.cycles += 19;
	mem_write(cpu.iy + 0x1C, 'G');
	if (from != 'S') { spell_done(); return; }
	cpu.cycles += 13 + 7 + 10;
	cpu.a = mem_read(MAP_KIND);
	op_sub8(3, 0, 0);
	if (cpu.f & FC) { cast_end(1); return; }
	call(0x42F3);
	cpu.cycles += 4;
	cpu.b = cpu.a;
	call(0x48D5);                      // member_object
	cpu.cycles += 19 + 19;
	mem_write(cpu.iy + 2, cpu.b);
	mem_write(cpu.iy + 3, cpu.b);
	spell_done();
}

static int on_land(void) {             // not on a ship or in the balloon
	cpu.cycles += 13 + 7 + 10;
	uint8_t t = mem_read(0xC959);
	cpu.a = t;
	op_sub8(0x14, 0, 0);
	if (t < 0x14) { spell_failed(); return 0; }
	cpu.cycles += 7 + 10;
	op_sub8(0x18, 0, 0);
	if (t == 0x18) { spell_failed(); return 0; }
	return 1;
}

// Blink ($5E5F): jump as far as the view allows that way, back off to
// ground the party can stand on
static void blink(void) {
	if (!on_land() || !only_outdoors() || !spell_direction(0x5E72) || !spend_mp()) return;
	cpu.cycles += 13 + 10 + 7 + 7 + 10;
	cpu.a = mem_read(PARTY_XY) & mem_read(PARTY_XY + 1);
	op_sub8(0xC0, 0, 0);
	if (!(cpu.f & FC)) { spell_failed(); return; }
	cpu.cycles += 19 + 19 + 16 + 4 + 4;
	cpu.e = mem_read(SLOT + 0x0E);
	cpu.d = mem_read(SLOT + 0x10);
	SET_HL(rd16(VIEW_XY));
	cpu.c = cpu.l;
	cpu.b = cpu.h;
	for (;;) {                         // out to the edge of the view
		cpu.cycles += 4 + 4 + 4 + 4 + 4 + 4 + 4 + 7 + 7;
		cpu.l = (uint8_t)(cpu.l + cpu.e);
		cpu.h = (uint8_t)(cpu.h + cpu.d);
		if (cpu.l >= 0x20) { cpu.cycles += 5; break; }
		cpu.cycles += 4 + 7 + 7;
		if (cpu.h >= 0x20) break;
		cpu.cycles += 5;
	}
	for (;;) {                         // back until there is ground
		cpu.cycles += 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 7;
		cpu.l = (uint8_t)(cpu.l - cpu.e);
		cpu.h = (uint8_t)(cpu.h - cpu.d);
		cpu.a = cpu.l;
		op_sub8(cpu.c, 0, 0);
		if (cpu.l == cpu.c) {
			cpu.cycles += 4 + 4 + 10;
			cpu.a = cpu.h;
			op_sub8(cpu.b, 0, 0);
			if (cpu.h == cpu.b) { spell_failed(); return; }   // back where it started
		} else cpu.cycles += 5;
		cpu.cycles += 11;
		uint16_t at = HL;
		push16(at);
		call(0xB5A4);                  // the map cell
		cpu.cycles += 7 + 10;
		cpu.a = mem_read(HL);
		SET_HL(pop16());
		call(0x3EB7);                  // carry: can't stand there
		cpu.cycles += 7;
		if (!(cpu.f & FC)) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 13 + 4 + 4 + 13 + 13 + 4 + 4 + 13;
	mem_write(PARTY_XY, (uint8_t)(mem_read(PARTY_XY) + cpu.l - cpu.c));
	cpu.a = (uint8_t)(mem_read(PARTY_XY + 1) + cpu.h - cpu.b);
	mem_write(PARTY_XY + 1, cpu.a);
	call(0x0E22);                      // recenter_map_view
	spell_done();
}

// Gate ($5FF7): to one of the eight moongates
static void gate(void) {
	if (!on_land() || !only_outdoors()) return;
	cpu.cycles += 10;
	SET_HL(0x64A5);
	call(0x2E7F);                      // the menu of phases
	say_inline(0x6010);                // "To phase: "
	cpu.cycles += 19;
	mem_write(SLOT + 0x1A, 0);
	for (;;) {
		task_frame_tick(0x601F);
		cpu.cycles += 7 + 10;
		cpu.c = 8;
		SET_DE(0x0102);
		call(0x49F9);
		cpu.cycles += 7;
		if (!(cpu.f & FZ)) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 10;
	if (cpu.f & FC) { cast_cancel(); return; }
	cpu.cycles += 19 + 7 + 17;
	uint8_t phase = mem_read(SLOT + 0x1A);
	text_char((uint8_t)('1' + phase));
	newline();
	if (!spend_mp()) return;
	cpu.cycles += 19 + 10 + 91 + 16;
	cpu.a = mem_read(SLOT + 0x1A);
	SET_HL(rd16(0x7CA3 + 2 * cpu.a));
	wr16(PARTY_XY, HL);
	call(0x0E22);
	spell_done();
}

// Heal ($604D): 75-99 hit points
static void heal(void) {
	if (!spell_who(0x6050) || !spend_mp()) return;
	cpu.cycles += 19;
	cpu.c = mem_read(SLOT + 0x08);
	call(0x4445);                      // member_can_act
	cpu.cycles += 10;
	if (cpu.f & FC) { spell_failed(); return; }
	cpu.cycles += 7;
	cpu.a = 0x19;
	call(0x2BEB);                      // random_below
	cpu.cycles += 7 + 4 + 7;
	cpu.a = (uint8_t)(cpu.a + 0x4B);
	SET_HL(cpu.a);
	call(0x4221);                      // member_heal
	spell_done();
}

// Open ($609D): a chest, without its trap; not from the balloon
static void open_chest(void) {
	if (!spend_mp()) return;
	cpu.cycles += 13 + 7 + 7;
	uint8_t mode = mem_read(MAP_KIND);
	if (mode == 0) {
		cpu.cycles += 13 + 7 + 10;
		if (mem_read(0xC959) == 0x18) { spell_failed(); return; }
	} else cpu.cycles += 5;
	cpu.cycles += 13 + 7 + 7;
	cpu.a = mode;
	op_sub8(3, 0, 0);
	if (mode >= 3) { call(0x5978); cpu.cycles += 12; }
	else {
		cpu.cycles += 5 + 19;
		mem_write(SLOT + 0x08, 0xFF);
		call(0x5B13);
	}
	spell_done();
}

// View ($61EB): the map (its own task routine, which waits for frames)
static void view(void) {
	if (!spend_mp()) return;
	cpu.cycles += 13 + 7 + 10;
	if (mem_read(MAP_KIND) >= 3) { spell_failed(); return; }
	cpu.cycles += 17;
	push16(0x61F9);
	native_continue_at(0x61FC);
}

// Winds ($6263): the wind blows the way chosen
static void winds(void) {
	if (!only_outdoors() || !spell_direction(0x6269) || !spend_mp()) return;
	cpu.cycles += 19 + 13;
	cpu.a = mem_read(SLOT + 0x1A);
	mem_write(0xC98A, cpu.a);
	spell_done();
}

// ------------------------------------------------- combat and dungeon spells --
#define FOE_RECORDS 0xC839              // 16 foes, 8 bytes: +4 hp, +5 graphic, +7 asleep
#define LEVEL       0xC970              // the dungeon level

static int only_combat(void) { return only_here(mem_read(MAP_KIND) >= 3, 0x6359); }
static int only_dungeon(void) { return only_here(mem_read(MAP_KIND) == 2, 0x6372); }

// call an original routine that may wait for frames, from `site`
static void tcall(uint16_t addr, uint16_t site) {
	cpu.cycles += 17;
	task_call(addr, (uint16_t)(site + 3));
}

static void yield_at(uint16_t site) {  // "call yield" at site
	cpu.cycles += 17 + 58;
	SET_HL((uint16_t)(site + 3));
	task_wait_turn((uint16_t)(site + 3));
}

// $645C: "Direction:" in combat, diagonals too ($4C8F); the step in
// (ix+$0E), (ix+$10)
static int combat_direction(uint16_t back) {
	cpu.cycles += 17 + 17 + 77;
	wr16(SLOT + 0x28, back);
	call(0x4C28);
	for (;;) {
		task_frame_tick(0x6462);
		call(0x4C8F);
		cpu.cycles += 7;
		if (!(cpu.f & FZ)) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 12 + 10;             // jr $644A / jp c,$5D4D
	if (cpu.f & FC) { cast_cancel(); return 0; }
	cpu.cycles += 19 + 17 + 100 + 19 + 19;
	cpu.a = mem_read(SLOT + 0x1A);
	SET_HL(rd16(0x40BF + 2 * cpu.a));
	mem_write(SLOT + 0x0E, cpu.l);
	mem_write(SLOT + 0x10, cpu.h);
	cpu.cycles += 10 + 42;
	SET_HL(back);
	return 1;
}

// $640B: Jinx, Negate, Protection, Quickness: an effect for 10 rounds
static void timed_effect(uint8_t effect) {
	cpu.cycles += 7 + 10 + 19;         // ld a,n / jp $640B / ld (ix+$1B),a
	mem_write(SLOT + 0x1B, effect);
	if (!spend_mp()) return;
	cpu.cycles += 19 + 13 + 7 + 13;
	mem_write(EFFECT, effect);
	mem_write(EFFECT + 1, 10);
	cpu.a = 10;
	spell_done();
}

// $64F4: carry, after "Nothing to Attack!", when no foe is left
static int no_targets(void) {
	cpu.cycles += 17 + 10 + 10 + 7 + 4;
	uint8_t any = 0;
	for (int i = 0; i < 16; i++) {
		any |= mem_read(FOE_RECORDS + 5 + 8 * i);
		cpu.cycles += 7 + 11 + (i < 15 ? 13 : 8);
	}
	SET_HL(FOE_RECORDS + 5 + 0x80);
	SET_DE(0x0008);
	cpu.b = 0;
	cpu.a = any;
	op_or(any);
	if (any) { cpu.cycles += 11; return 0; }
	cpu.cycles += 5;
	call(0x40D1);
	say_inline(0x6508);
	cpu.cycles += 4 + 10;
	cpu.f |= FC;
	return 1;
}

// $6383: Fireball, Iceball, Kill, Magic Missile: a missile (graphic c) at
// a foe; its damage by kind b
static void missile_spell(uint8_t graphic, uint8_t kind) {
	cpu.cycles += 10 + 10 + 19 + 19;   // ld bc / jp $6383 / ld (ix+$22),c / ld (ix+$23),b
	SET_BC((uint16_t)(kind << 8 | graphic));
	mem_write(SLOT + 0x22, graphic);
	mem_write(SLOT + 0x23, kind);
	if (!only_combat()) return;
	if (no_targets()) { cpu.cycles += 10; cast_end(0); return; }
	cpu.cycles += 10;
	tcall(0x37DD, 0x6392);             // "Aim:"
	cpu.cycles += 10;
	if (cpu.f & FC) { cast_cancel(); return; }
	if (!spend_mp()) return;
	call(0x48E1);                      // IY = the caster's figure
	cpu.cycles += 19 * 5;
	SET_HL(rd16(cpu.iy));
	SET_DE(rd16(SLOT + 0x20));
	cpu.a = mem_read(SLOT + 0x22);
	tcall(0x38FC, 0x63AD);
	for (;;) {                         // $63B0: it flies
		tcall(0x392F, 0x63B0);
		tcall(0x38BD, 0x63B3);         // carry clear: a foe there
		cpu.cycles += 7;
		if (!(cpu.f & FC)) { cpu.cycles += 5; break; }
		tcall(0x38EB, 0x63B8);
		tcall(0x3974, 0x63BB);         // carry: it hit something else
		cpu.cycles += 7;
		if (cpu.f & FC) { cpu.cycles += 10; spell_failed(); return; }
		cpu.cycles += 5;
	}
	cpu.cycles += 19;
	mem_write(SLOT + 0x08, cpu.a);     // the foe
	tcall(0x3A91, 0x63C6);
	cpu.cycles += 19 + 10 + 17;        // ld a,(ix+$23) / ld hl / call switch_on_a
	static const uint8_t kinds[3] = { 0x4D, 0x4E, 0x4F };
	static const uint8_t range[3] = { 0x40, 0xE0, 0x80 }, base[3] = { 0x10, 0x20, 0x18 };
	int i;
	for (i = 0; i < 3 && kinds[i] != kind; i++) {}
	if (i == 3) { cpu.cycles += 58 * 3 + 22 + 7; cpu.a = 0xE8; }   // Kill
	else {
		cpu.cycles += 58 * i + 79 + 7;
		cpu.a = range[i];
		call(0x2BEB);                  // random_below
		cpu.cycles += 7 + 12;
		op_or(base[i]);
	}
	cpu.cycles += 11 + 13 + 4 + 19 + 10;
	cpu.b = mem_read(CASTER);
	cpu.c = mem_read(SLOT + 0x08);
	tcall(0x39B5, 0x63DD);             // damage_monster
	newline();
	spell_done();
}

// Sleep ($60F2): foes may fall asleep, by their hit points
static void sleep_spell(void) {
	if (!only_combat() || !spend_mp()) return;
	cpu.cycles += 14 + 10 + 7;
	SET_DE(0x0008);
	cpu.b = 0x10;
	for (uint16_t f = FOE_RECORDS; ; f += 8) {
		cpu.iy = f;
		cpu.cycles += 19 + 4 + 7;
		cpu.a = mem_read(f + 5);
		op_or(cpu.a);
		if (!cpu.a) { cpu.cycles += 5; goto next; }
		call(0x62F8);                  // Z: can't sleep (undead and such)
		cpu.cycles += 7;
		if (cpu.f & FZ) { cpu.cycles += 5; goto next; }
		cpu.cycles += 7 + 7;
		if (cpu.a == 0xFC) { cpu.cycles += 5; goto next; }
		call(0x2BBF);                  // random
		cpu.cycles += 19 + 7;
		op_sub8(mem_read(f + 4), 0, 0);
		if (cpu.f & FC) { cpu.cycles += 5; goto next; }
		cpu.cycles += 7;
		if (cpu.f & FZ) { cpu.cycles += 5; goto next; }
		cpu.cycles += 19;
		mem_write(f + 7, 1);
	next:
		cpu.cycles += 15;
		if (--cpu.b == 0) { cpu.cycles += 8; break; }
		cpu.cycles += 13;
	}
	spell_done();
}

// Undead ($61BB): the undead may be weakened to 23 hit points
static void undead_spell(void) {
	if (!only_combat() || !spend_mp()) return;
	cpu.cycles += 14 + 10 + 7;
	SET_DE(0x0008);
	cpu.b = 0x10;
	for (uint16_t f = FOE_RECORDS; ; f += 8) {
		cpu.iy = f;
		cpu.cycles += 19;
		cpu.a = mem_read(f + 5);
		call(0x62F8);                  // Z: undead
		cpu.cycles += 7;
		if (!(cpu.f & FZ)) { cpu.cycles += 5; goto next; }
		call(0x2BBF);
		cpu.cycles += 7 + 7;
		op_and(1);
		if (!cpu.a) { cpu.cycles += 5; goto next; }
		cpu.cycles += 19 + 7 + 7;
		cpu.a = mem_read(f + 4);
		op_sub8(0x17, 0, 0);
		if (cpu.f & FC) { cpu.cycles += 5; goto next; }
		cpu.cycles += 19;
		mem_write(f + 4, 0x17);
	next:
		cpu.cycles += 15;
		if (--cpu.b == 0) { cpu.cycles += 8; break; }
		cpu.cycles += 13;
	}
	spell_done();
}

// $61AF: a quake mark at the foe, shown for a frame
static void quake_mark(uint8_t a, uint16_t site) {
	cpu.cycles += 7;
	cpu.a = a;
	cpu.cycles += 17 + 19 + 19;
	cpu.e = mem_read(SLOT + 0x0B);
	cpu.d = mem_read(SLOT + 0x0D);
	call(0x3082);
	cpu.cycles += 10;                  // jp yield
	cpu.cycles += 58;
	SET_HL((uint16_t)(site + 3));
	task_wait_turn((uint16_t)(site + 3));
}

// Tremor ($6125): the ground shakes; each foe may be hurt badly or swallowed
static void tremor(void) {
	if (!only_combat() || !spend_mp()) return;
	cpu.cycles += 7;
	cpu.a = 0x16;
	call(0x05F1);
	tcall(0x4117, 0x6130);             // the screen shakes
	cpu.cycles += 19 + 19;
	mem_write(SLOT + 0x08, 0);
	mem_write(SLOT + 0x16, 0x10);
	do {
		cpu.cycles += 19;              // $613B
		cpu.c = mem_read(SLOT + 0x08);
		call(0x48EB);                  // IY = the foe
		cpu.cycles += 19 + 4 + 7;
		cpu.a = mem_read(cpu.iy + 5);
		op_or(cpu.a);
		if (!cpu.a) { cpu.cycles += 5; goto next; }
		cpu.cycles += 19 + 7 + 7;
		cpu.a = mem_read(cpu.iy + 4);
		op_sub8(0xC0, 0, 0);
		if (!(cpu.f & FC)) { cpu.cycles += 5; goto next; }
		call(0x2BBF);
		cpu.cycles += 7 + 7;
		op_and(1);
		if (cpu.a) {                   // weakened, half the time
			call(0x2BBF);
			cpu.cycles += 7 + 7;
			op_and(1);
			if (!cpu.a) { cpu.cycles += 5; goto next; }
			cpu.cycles += 19 + 7 + 7;
			cpu.a = mem_read(cpu.iy + 4);
			op_sub8(0x17, 0, 0);
			if (cpu.f & FC) { cpu.cycles += 5; goto next; }
			cpu.cycles += 19 + 12;
			mem_write(cpu.iy + 4, 0x17);
			goto next;
		}
		cpu.cycles += 5 + 19;          // $6169: swallowed
		cpu.a = mem_read(cpu.iy + 0);
		call(0x36F7);
		cpu.cycles += 19 + 19;
		mem_write(SLOT + 0x0B, cpu.a);
		cpu.a = mem_read(cpu.iy + 1);
		call(0x36F7);
		cpu.cycles += 19 + 7;
		mem_write(SLOT + 0x0D, cpu.a);
		cpu.a = 0x14;
		call(0x05F1);
		quake_mark(4, 0x6182);
		quake_mark(5, 0x6187);
		quake_mark(6, 0x618C);
		cpu.cycles += 13 + 4 + 19 + 7;
		cpu.b = mem_read(CASTER);
		cpu.c = mem_read(SLOT + 0x08);
		cpu.a = 0xFF;
		tcall(0x39B5, 0x6198);         // damage_monster
		newline();
		call(0x129D);
		yield_at(0x61A1);
	next:
		cpu.cycles += 23 + 23 + 7;     // $61A4
		mem_write(SLOT + 0x08, mem_read(SLOT + 0x08) + 1);
		mem_write(SLOT + 0x16, mem_read(SLOT + 0x16) - 1);
		if (!mem_read(SLOT + 0x16)) break;
		cpu.cycles += 5;
	} while (1);
	spell_done();
}

// $5F3B: a field (tiles $44-$47) at HL goes, the ground DE back
static void clear_field(void) {
	cpu.cycles += 7 + 7 + 10;
	cpu.a = mem_read(HL);
	if (cpu.a < 0x44) { spell_failed(); return; }
	cpu.cycles += 7 + 10;
	if (cpu.a >= 0x48) { spell_failed(); return; }
	cpu.cycles += 7 + 4 + 7;
	mem_write(HL, cpu.e);
	cpu.l++;
	mem_write(HL, cpu.d);
	spell_done();
}

// Dispel ($5EE7): a field goes; in the world nothing is there to dispel
static void dispel(void) {
	cpu.cycles += 13 + 10 + 17;
	uint8_t mode = mem_read(MAP_KIND);
	cpu.a = mode;
	if (mode < 3) {
		cpu.cycles += 58 * mode + 79;
		cpu.a = 0x5E; cpu.f = 0xF0;    // (the switch pops its return address)
		if (mode == 0) {               // $5F1F
			if (!spell_direction(0x5F22) || !spend_mp()) return;
			cpu.cycles += 10;
			spell_failed();
			return;
		}
		if (mode == 1) {               // $5F28: in towns
			if (!spell_direction(0x5F2B) || !spend_mp()) return;
			call(0x646C);              // HL = the cell that way; carry if on the map
			cpu.cycles += 10;
			if (!(cpu.f & FC)) { spell_failed(); return; }
			call(0xB5A4);
			cpu.cycles += 20;
			SET_DE(rd16(0xC9BB));      // the ground
			clear_field();
			return;
		}
		if (!spell_direction(0x5F4F) || !spend_mp()) return;   // $5F4C: dungeons
		call(0x6480);
		call(0x6DC0);
		cpu.cycles += 7 + 7 + 7 + 10;
		cpu.a = mem_read(HL) & 0xF0;
		if (cpu.a != 0xA0) { spell_failed(); return; }
		cpu.cycles += 10;
		mem_write(HL, 0);
		spell_done();
		return;
	}
	cpu.cycles += 58 * 3 + 22;         // combat
	if (!combat_direction(0x5EF3) || !spend_mp()) return;
	call(0x48E1);
	cpu.cycles += 19 + 19;
	SET_HL(rd16(cpu.iy));
	call(0x38F2);                      // HL = the member's cell
	cpu.cycles += 7 + 4 + 7 + 4 + 7 + 7;
	cpu.e = mem_read(HL);
	cpu.l++;
	cpu.d = mem_read(HL);
	cpu.a = cpu.e;
	if (cpu.e == 0x3C) { cpu.cycles += 10; SET_DE(0x0016); }
	else cpu.cycles += 5;
	call(0x6493);                      // HL = the cell that way
	call(0x38F2);
	cpu.cycles += 12;
	clear_field();
}

// Energy Field ($5F65): a field of fire, lightning, poison or sleep that
// way, in dungeons and in combat
static void energy_field(void) {
	cpu.cycles += 10;
	SET_HL(0x64CE);
	call(0x2E7F);
	say_inline(0x5F6E);                // "Energy type? "
	cpu.cycles += 19;
	mem_write(SLOT + 0x1A, 0);
	for (;;) {
		task_frame_tick(0x5F80);
		cpu.cycles += 7 + 10;
		cpu.c = 4;
		SET_DE(0x0102);
		call(0x49F9);
		cpu.cycles += 7;
		if (!(cpu.f & FZ)) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 10;
	if (cpu.f & FC) { cast_cancel(); return; }
	cpu.cycles += 19 + 19 + 10 + 91 + 17;
	uint8_t type = mem_read(SLOT + 0x1A);
	mem_write(SLOT + 0x07, type);
	SET_HL(rd16(0x64C6 + 2 * type));
	text_print(HL);
	newline();
	cpu.cycles += 13 + 7 + 7;
	uint8_t mode = mem_read(MAP_KIND);
	if (mode < 3) {
		cpu.cycles += 7 + 10;
		if (mode != 2) { spell_failed(); return; }
		if (!spell_direction(0x5FAF) || !spend_mp()) return;
		call(0x6480);
		call(0x6DC0);
		cpu.cycles += 7 + 7 + 10;
		if (mem_read(HL)) { spell_failed(); return; }
		cpu.cycles += 19 + 7 + 7;
		cpu.a = (uint8_t)(type | 0xA0);
		mem_write(HL, cpu.a);
		spell_done();
		return;
	}
	cpu.cycles += 5;
	if (!combat_direction(0x5FCA) || !spend_mp()) return;
	call(0x6493);
	call(0x38F2);
	cpu.cycles += 7;
	cpu.a = mem_read(HL);
	call(0x3EB7);                      // carry: nothing can stand there
	cpu.cycles += 10;
	if (cpu.f & FC) { spell_failed(); return; }
	cpu.cycles += 4 + 19 + 10 + 91 + 4 + 7 + 4 + 7;
	uint16_t tile = rd16(0x5FE9 + 2 * type);
	SET_DE(tile);
	mem_write(HL, (uint8_t)tile);
	cpu.l++;
	mem_write(HL, tile >> 8);
	spell_done();
}

// Light ($607D): light in the dungeon for a while
static void light(void) {
	if (!only_dungeon() || !spend_mp()) return;
	cpu.cycles += 13 + 7 + 7;
	unsigned l = mem_read(0xC95B) + 0xC8;
	if (l > 0xFF) { cpu.cycles += 7; l = 0xFF; }
	else cpu.cycles += 5;
	cpu.cycles += 13;
	cpu.a = (uint8_t)l;
	mem_write(0xC95B, cpu.a);
	spell_done();
}

// X-it ($6275): out of the dungeon
static void x_it(void) {
	if (!only_dungeon() || !spend_mp()) return;
	cpu.cycles += 7 + 13;
	cpu.a = 0xFF;
	mem_write(LEVEL, 0xFF);
	spell_done();
}

// $62E4: a random spot of the level; Z when it's empty floor
static void random_spot(void) {
	cpu.cycles += 17;
	call(0x2BBF);
	cpu.cycles += 7 + 4;
	cpu.l = cpu.a & 0x0F;
	call(0x2BBF);
	cpu.cycles += 7 + 4 + 11;
	cpu.h = cpu.a & 0x0F;
	uint16_t at = HL;
	call(0x6DC0);
	cpu.cycles += 7 + 10 + 4 + 10;
	cpu.a = mem_read(HL);
	SET_HL(at);
	op_or(cpu.a);
}

// Y-up / Z-down ($6283 / $62B1): a level up or down, to a random empty spot
static void change_level(int up) {
	if (!only_dungeon() || !spend_mp()) return;
	cpu.cycles += 13 + 7 + 10;
	if (mem_read(0xC9A0) == 0x19) { spell_failed(); return; }   // not in the Abyss
	if (up) {
		cpu.cycles += 10 + 11 + 10;
		uint8_t lv = (uint8_t)(mem_read(LEVEL) - 1);
		mem_write(LEVEL, lv);
		if (lv & 0x80) { cast_end(1); return; }    // up from the top: out (jp m,$5D49)
	} else {
		cpu.cycles += 13 + 7 + 10;
		uint8_t lv = mem_read(LEVEL);
		if (lv == 7) { spell_failed(); return; }
		cpu.cycles += 4 + 13;
		mem_write(LEVEL, lv + 1);
	}
	cpu.cycles += 7;
	cpu.b = 0x20;
	for (;;) {
		random_spot();
		cpu.cycles += 7;
		if (cpu.f & FZ) { cpu.cycles += 5; break; }
		if (--cpu.b == 0) {
			cpu.cycles += 8 + 10 + 11 + 10;   // no room there: back
			mem_write(LEVEL, (uint8_t)(mem_read(LEVEL) + (up ? 1 : -1)));
			spell_failed();
			return;
		}
		cpu.cycles += 13;
	}
	cpu.cycles += 16;
	wr16(0xC953, HL);
	call(0x129D);
	spell_done();
}

// the spell's routine
static void run_spell(uint8_t spell) {
	switch (spell) {
	case 0: restore_status(0x5E32, 'S'); break;   // Awaken
	case 1: blink(); break;
	case 2: restore_status(0x5ECF, 'P'); break;   // Cure
	case 6: gate(); break;
	case 7: heal(); break;
	case 14: open_chest(); break;
	case 17:                                      // Resurrect: not in combat
		cpu.cycles += 13 + 7 + 10;
		if (mem_read(MAP_KIND) >= 3) { spell_failed(); break; }
		restore_status(0x60DA, 'D');
		break;
	case 3: dispel(); break;
	case 4: energy_field(); break;
	case 5: missile_spell(0x08, 0x4F); break;     // Fireball
	case 8: missile_spell(0x09, 0x4E); break;     // Iceball
	case 9: timed_effect(0x4A); break;            // Jinx
	case 10: missile_spell(0x0A, 0x8C); break;    // Kill
	case 11: light(); break;
	case 12: missile_spell(0x07, 0x4D); break;    // Magic Missile
	case 13: timed_effect(0x4E); break;           // Negate
	case 15: timed_effect(0x50); break;           // Protection
	case 16: timed_effect(0x51); break;           // Quickness
	case 18: sleep_spell(); break;
	case 19: tremor(); break;
	case 20: undead_spell(); break;
	case 21: view(); break;
	case 22: winds(); break;
	case 23: x_it(); break;
	case 24: change_level(1); break;              // Y-up
	default: change_level(0); break;              // Z-down
	}
}
