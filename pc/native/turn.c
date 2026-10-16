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

typedef struct TurnLoop {
	int world;
	uint16_t after_yield, check, after_check, prompt_jr, tick_at, input_at, end_at, after_end;
} TurnLoop;

static const TurnLoop world = { 1, 0xACBD, 0xACDC, 0xACC0, 0, 0xACCC, 0xACCF, 0xACD4, 0xACD7 };
static const TurnLoop town = { 0, 0xAF8B, 0xAFAA, 0xAF8E, 0, 0xAF9A, 0xAF9D, 0xAFA2, 0xAFA5 };

static void turn_loop(const TurnLoop *t) {
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
		handoff(0x3F14, t->after_end); // the end of the turn
		cpu.cycles += 17;
		return;
	}
}

static void world_turn(void) { turn_loop(&world); }   // $ACB1
static void town_turn(void) { turn_loop(&town); }     // $AF85
Native native_world_turn = { "world_turn", 0xACB1, 2, world_turn, 0, 0, 0, .task = 1 };
Native native_town_turn = { "town_turn", 0xAF85, 2, town_turn, 0, 0, 0, .task = 1 };

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
