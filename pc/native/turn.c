// The player's turn, as C tasks: the world and town turn loops (bank 2
// $ACB1 and $AF85) and the command menu ($8000).
//
// A turn: refresh the screen, wait a frame, check where the party stands
// (moongates, town gates: original code, which may take over), show the
// ">" prompt, then read the pad every frame (input_step) until something
// happens. The end of the turn - the creatures' moves, food, poison
// ($3F14, $B4CA) - and the commands behind button 2 are original code
// still; the C task hands the turn to them ("handoff") and the original
// loop comes back to the C one at the start of the next turn.
//
// Cycle counts are the original's.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define SLOT          cpu.ix
#define PARTY_XY      0xC953
#define LAST_XY       0xC971
#define TURN_DONE     0xC9A1
#define JOY_HELD      0xC0AF
#define JOY_PRESSED   0xC0B0
#define MAP_KIND      0xC99F
#define PARTY         0xC751
#define PARTY_SIZE    0xC957

static void call(uint16_t addr) { cpu.cycles += 17; call_original(addr); }

// continue in the original at `addr` as if called from `back`: the C task
// ends here and the original code carries the turn on
static void handoff(uint16_t addr, uint16_t back) {
	push16(back);
	native_continue_at(addr);
}

// button 2 alone: the command menu, which is original code
static int command_pressed(void) {
	uint8_t held = mem_read(JOY_HELD);
	return held != 0x01 && held != 0x02 && held != 0x04 && held != 0x08 &&
		mem_read(JOY_PRESSED) == 0x20;
}

static void tcall(uint16_t addr, uint16_t site) {   // a routine that may wait for frames
	cpu.cycles += 17;
	task_call(addr, (uint16_t)(site + 3));
}

// $B4EE: the sleeping may wake (1 in 8), the poisoned lose 2 hit points;
// starving, everyone who can act loses 2
static void statuses(void) {
	cpu.cycles += 17 + 14 + 10 + 13 + 4 + 7 + 19;
	cpu.iy = PARTY;
	SET_DE(0x001D);
	cpu.b = mem_read(PARTY_SIZE);
	cpu.c = 0;
	mem_write(SLOT + 0x17, 1);
	do {
		cpu.cycles += 19 + 7 + 7;
		uint8_t st = mem_read(cpu.iy + 0x1C);
		cpu.a = st;
		if (st == 'S') {
			call(0x2BBF);              // random
			cpu.cycles += 7 + 7;
			op_and(7);
			if (!cpu.a) { cpu.cycles += 19 + 12; mem_write(cpu.iy + 0x1C, 'G'); }
			else cpu.cycles += 5;
		} else {
			cpu.cycles += 5 + 7 + 7;
			if (st == 'P') {
				cpu.cycles += 10;
				SET_HL(0x0002);
				call(0x467F);          // member_take_damage
				call(0x49A6);
				cpu.cycles += 23;
				mem_write(SLOT + 0x17, mem_read(SLOT + 0x17) + 1);
			} else cpu.cycles += 5;
		}
		cpu.cycles += 15 + 4;
		cpu.iy += 0x1D;
		cpu.c++;
		cpu.cycles += --cpu.b ? 13 : 8;
	} while (cpu.b);
	cpu.cycles += 23 + 7;
	uint8_t hurt = (uint8_t)(mem_read(SLOT + 0x17) - 1);
	mem_write(SLOT + 0x17, hurt);
	if (hurt) {                        // someone was poisoned: a sound
		cpu.cycles += 7;
		cpu.a = 0x11;
		call(0x05F1);
		call(0x041C);
	} else cpu.cycles += 5;
	cpu.cycles += 13;
	cpu.a = mem_read(PARTY_SIZE);
	call(0x4753);                      // carry: no food left
	if (!(cpu.f & FC)) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 10 + 17;
	SET_HL(0xBD22);
	text_print(0xBD22);                // "Starving!!!"
	cpu.cycles += 13 + 4 + 7;
	cpu.b = mem_read(PARTY_SIZE);
	cpu.c = 0;
	do {
		call(0x4445);                  // member_can_act
		cpu.cycles += 7;
		if (!(cpu.f & FC)) {
			cpu.cycles += 10;
			SET_HL(0x0002);
			call(0x467F);
		} else cpu.cycles += 5;
		cpu.cycles += 4;
		cpu.c++;
		cpu.cycles += --cpu.b ? 13 : 8;
	} while (cpu.b);
	cpu.cycles += 7;
	cpu.a = 0x11;
	call(0x05F1);
	cpu.cycles += 10;                  // jp $4896: it returns for statuses' caller
	task_call(0x4896, 0xB4D3);
}

// $B56D: in towns, a door that was opened locks again after a while
static void relock_door(void) {
	cpu.cycles += 17 + 13 + 7;
	if (mem_read(MAP_KIND) != 1) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 11 + 10 + 7 + 4 + 7;
	uint16_t hl = HL;
	SET_HL(0xC9A4);
	uint8_t t = mem_read(0xC9A4);
	if (t) {
		cpu.cycles += 11 + 7;
		t--;
		mem_write(0xC9A4, t);
		if (!t) {
			cpu.cycles += 13 + 10 + 7 + 7;
			if (mem_read(0xC9A2) == mem_read(PARTY_XY)) {
				cpu.cycles += 13 + 10 + 7 + 7;
				if (mem_read(0xC9A3) == mem_read(PARTY_XY + 1)) {
					cpu.cycles += 7 + 13 + 12;   // the party stands in it: later
					mem_write(0xC9A4, 1);
					goto out;
				}
			}
			cpu.cycles += 5 + 16;
			SET_HL(rd16(0xC9A2));
			call(0xB5A4);
			cpu.cycles += 10 + 4 + 10;
			mem_write(HL, 0x3B);       // locked again
			cpu.l++;
			mem_write(HL, 0x00);
		} else cpu.cycles += 5;
	} else cpu.cycles += 5;
out:
	cpu.cycles += 10 + 10;
	SET_HL(hl);
}

// $372E: one more move (32-bit), and the 5-digit BCD move counter
static void count_move(void) {
	cpu.cycles += 17 + 11 + 11 + 16 + 10 + 11 + 16 + 7;
	uint16_t hl = HL, bc = BC;
	uint16_t lo = (uint16_t)(rd16(0xC8FD) + 1);
	wr16(0xC8FD, lo);
	if (!lo) { cpu.cycles += 16 + 6 + 16; wr16(0xC8FF, (uint16_t)(rd16(0xC8FF) + 1)); }
	else cpu.cycles += 5;
	cpu.cycles += 10 + 10 + 4;
	int carry = 1;
	for (int i = 0; i < 5; i++) {      // scf / adc a,0 / daa, five bytes
		cpu.a = mem_read(0xC901 + i);
		op_add8(0, carry);
		op_daa();
		carry = cpu.f & FC;
		mem_write(0xC901 + i, cpu.a);
		cpu.cycles += 7 + 4 + 4 + 7 + 6 + (i < 4 ? 13 : 8);
	}
	SET_HL(hl);                        // (pushed and popped)
	SET_BC(bc);
	cpu.cycles += 10 + 10 + 10;
}

// $3754: a timed effect in combat wears off
static void effect_wears(void) {
	cpu.cycles += 17 + 13 + 4;
	uint8_t n = mem_read(0xC977);
	cpu.a = n;
	op_or(n);
	if (!n) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 4 + 13;
	n--;
	cpu.a = n;
	mem_write(0xC977, n);
	if (n) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 13 + 10;
	mem_write(0xC976, 0);
}

// $B4CA: the end of the turn: food, statuses, levels, the creatures
static int end_of_turn(uint16_t back) {
	tcall(0x465D, 0xB4CA);             // everyone dead: game over
	cpu.cycles += 17 + 10 + 7 + 7;     // $B55E: the ship's hull mends, slowly
	SET_HL(0xC961);
	if (mem_read(0xC961) < 0x32) {
		cpu.cycles += 5;
		call(0x2BBF);
		cpu.cycles += 7;
		op_and(3);
		if (!cpu.a) { cpu.cycles += 5 + 11 + 10; mem_write(0xC961, mem_read(0xC961) + 1); }
		else cpu.cycles += 11;
	} else cpu.cycles += 11 - 5 + 5;
	statuses();
	tcall(0x4776, 0xB4D3);             // magic points, food
	call(0x957C);                      // Z: in a dungeon
	cpu.cycles += 7;
	if (!(cpu.f & FZ)) {
		cpu.cycles += 17;
		if (creatures_move(back)) return 1;   // a monster attacks: the fight takes over
		cpu.cycles += 17;
		creatures_appear();
	} else cpu.cycles += 5;
	count_move();
	effect_wears();
	call(0x4324);                      // the status window
	relock_door();
	cpu.cycles += 10;
	return 0;
}

typedef struct TurnLoop {
	int world;
	uint16_t after_yield, check, after_check, prompt_jr, tick_at, input_at, end_at, after_end;
} TurnLoop;

static const TurnLoop world = { 1, 0xACBD, 0xACDC, 0xACC0, 0, 0xACCC, 0xACCF, 0xACD4, 0xACD7 };
static const TurnLoop town = { 0, 0xAF8B, 0xAFAA, 0xAF8E, 0, 0xAF9A, 0xAF9D, 0xAFA2, 0xAFA5 };

// stage: where the loop is entered. 0 its top; 1 after the check of where
// the party stands (the original's, which the C hands off to, returns
// there); 2 after the input (a command, which is the original's too, ends
// there): the ground's hazards and the end of the turn
static void turn_loop(const TurnLoop *t, int stage) {
	if (stage == 1) goto after_check;
	if (stage == 2) goto end;
	for (;;) {
		if (t->world) {                // where the party was at the start of the turn
			cpu.cycles += 16 + 16;
			wr16(LAST_XY, rd16(PARTY_XY));
		}
		call(0x129D);                  // the screen's moving parts
		cpu.cycles += 17 + 58;         // yield
		SET_HL(t->after_yield);
		task_wait_turn(t->after_yield);
		// where the party stands: only when it has just moved
		cpu.cycles += 17 + 13 + 4;
		if (mem_read(TURN_DONE)) {
			cpu.cycles -= 13 + 4;      // (the original does those itself)
			handoff(t->check, t->after_check);
			return;
		}
		cpu.cycles += 11;
	after_check:
		call(0xAF57);                  // everyone asleep? then no input this turn
		cpu.cycles += 7;
		if (cpu.f & FC) cpu.cycles += 5;
		else {
			cpu.cycles += 7 + 17;
			text_char(0x7F);           // the ">" prompt
			cpu.cycles += 12;
			for (int first = 1;; first = 0) {
				if (!first) task_frame_tick(t->tick_at);
				if (command_pressed()) {   // the command menu takes over the turn
					cpu.cycles += 17;
					push16(t->input_at + 3);
					native_continue_at(0xB08B);
					return;
				}
				call(0xB08B);          // input_step
				cpu.cycles += 7;
				if (!(cpu.f & FC)) break;
				cpu.cycles += 5;
			}
		}
	end:
		tcall(0x3F14, t->end_at);      // the ground's hazards
		cpu.cycles += 17;              // call $B4CA
		if (end_of_turn((uint16_t)(t->after_end + 3))) return;
		cpu.cycles += 12;              // jr back to the top
	}
}

static void world_turn(void) { turn_loop(&world, 0); }   // $ACB1
static void town_turn(void) { turn_loop(&town, 0); }     // $AF85
static void world_after_check(void) { turn_loop(&world, 1); }   // $ACC0
static void town_after_check(void) { turn_loop(&town, 1); }     // $AF8E
static void world_turn_end(void) { turn_loop(&world, 2); }      // $ACD4
static void town_turn_end(void) { turn_loop(&town, 2); }        // $AFA2
Native native_world_turn = { "world_turn", 0xACB1, 2, world_turn, 0, 0, 0, .task = 1 };
Native native_town_turn = { "town_turn", 0xAF85, 2, town_turn, 0, 0, 0, .task = 1 };
Native native_world_after_check = { "world_after_check", 0xACC0, 2, world_after_check, 0, 0, 0, .task = 1 };
Native native_town_after_check = { "town_after_check", 0xAF8E, 2, town_after_check, 0, 0, 0, .task = 1 };
Native native_world_turn_end = { "world_turn_end", 0xACD4, 2, world_turn_end, 0, 0, 0, .task = 1 };
Native native_town_turn_end = { "town_turn_end", 0xAFA2, 2, town_turn_end, 0, 0, 0, .task = 1 };

// $AC8C (game mode $11, the world) and $AF69 (mode $12, a town): the map
// set up, drawn, the music started, the turn count cleared; then the turns
static void world_start(void) {
	cpu.cycles += 4 + 13;
	cpu.a = 0;
	op_xor(0);
	mem_write(0xC9A0, 0);              // no location: the world
	tcall(0x323D, 0xAC90);
	tcall(0x0E22, 0xAC93);
	tcall(0x0E17, 0xAC96);
	tcall(0x129D, 0xAC99);
	tcall(0x4324, 0xAC9C);
	tcall(0x30F6, 0xAC9F);
	cpu.cycles += 7;
	cpu.a = 5;
	tcall(0x0621, 0xACA4);             // the world's music
	cpu.cycles += 4 + 13 + 10 + 16;
	cpu.a = 0;
	op_xor(0);
	mem_write(TURN_DONE, 0);
	SET_HL(0x0000);
	wr16(0xC99C, 0);
	world_turn();
}
Native native_world_start = { "world_start", 0xAC8C, 2, world_start, 0, 0, 0, .task = 1 };

static void town_start(void) {
	tcall(0x323D, 0xAF69);
	tcall(0x129D, 0xAF6C);
	tcall(0x4324, 0xAF6F);
	tcall(0x30F6, 0xAF72);
	tcall(0x33E5, 0xAF75);             // A = the place's music
	tcall(0x0621, 0xAF78);
	cpu.cycles += 4 + 13 + 10 + 16;
	cpu.a = 0;
	op_xor(0);
	mem_write(TURN_DONE, 0);
	SET_HL(0x0000);
	wr16(0xC99C, 0);
	town_turn();
}
Native native_town_start = { "town_start", 0xAF69, 2, town_start, 0, 0, 0, .task = 1 };

// $AC62 (game mode $10): the screen faded in; then the world (mode $11)
static void game_fade_in(void) {
	cpu.cycles += 10 + 10;
	SET_HL(0x7C83); SET_BC(0x0301);
	tcall(0x0A22, 0xAC68);
	tcall(0x0A82, 0xAC6B);
	cpu.cycles += 7 + 13 + 10;
	cpu.a = 0x11;
	mem_write(0xC600, 0x11);
}
Native native_game_fade_in = { "game_fade_in", 0xAC62, 2, game_fade_in, 0, 0, 0, .task = 1 };

// ------------------------------------------------------------ commands --
// command_menu ($8000): "Command-" and the menu of eight; button 1 leaves
// ($8081), button 2 goes to the command (original code, from the table at
// $804B, or $805B in dungeons). Entered with the turn loop's return address
// on the stack, kept in the slot (task_enter_1) for the command's end.
static void command_menu(void) {
	uint16_t back = pop16();
	cpu.cycles += 17 + 77;             // task_enter_1
	wr16(SLOT + 0x24, back);
	SET_HL(back);
	call(0x40C9);                      // click
	cpu.cycles += 17 + 19 + 17;
	text_print(0x8009);                // "Command-"
	cpu.cycles += 19 + 10;
	cpu.cycles += 10;
	SET_HL(0x9592);
	call(0x2E7F);                      // menu_draw
	cpu.cycles += 10;
	SET_HL(0x0B0A);
	call(0x06DA);
	cpu.cycles += 19;
	mem_write(SLOT + 0x1A, 0);
	for (;;) {
		task_frame_tick(0x8022);
		cpu.cycles += 7 + 10;
		cpu.c = 8;
		SET_DE(0x0102);
		call(0x49F9);                  // menu_step_wrap
		cpu.cycles += 7;
		if (!(cpu.f & FZ)) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 10;
	if (cpu.f & FC) { native_continue_at(0x8081); return; }   // cancelled
	cpu.cycles += 10 + 17 + 19 + 91 + 10;   // ld hl / call $806B: the command's name
	uint8_t cmd = mem_read(SLOT + 0x1A);
	uint16_t name = rd16(0x9582 + 2 * cmd);
	SET_HL(name);
	text_print(name);
	cpu.cycles += 13 + 7 + 10 + 7;
	uint16_t table = 0x804B;
	if (mem_read(MAP_KIND) == 2) { table = 0x805B; cpu.cycles += 10; }
	else cpu.cycles += 5;
	cpu.cycles += 19 + 10 + 10 + 52;   // ld a,(ix+$1A) / jp $0018
	cpu.a = cmd;
	SET_HL(rd16(table + 2 * cmd));
	native_continue_at(HL);
}
Native native_command_menu = { "command_menu", 0x8000, 2, command_menu, 0, 0, 0, .task = 1 };
