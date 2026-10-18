// The game modes' set-ups (the table at $0D55, run by the main loop when
// $C600 changes): each clears the screen and the tasks, loads its
// graphics, starts its task ($0B33) and marks the mode running ($0DDD /
// $0DE1 via $7E5A / $7E5D). Plus the dungeon's own start ($69E4). Cycle
// counts are the original's.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

static void call(uint16_t addr) { cpu.cycles += 17; call_original(addr); }
static void jp(uint16_t addr) { cpu.cycles += 10; call_original(addr); }   // a tail jump: its ret is ours

static void tcall(uint16_t addr, uint16_t site) {   // from a task: a routine that may wait for frames
	cpu.cycles += 17;
	task_call(addr, (uint16_t)(site + 3));
}

static void start_task(uint16_t entry) {   // ld de,entry / call $0B33
	cpu.cycles += 10;
	SET_DE(entry);
	call(0x0B33);
}

static void far(uint8_t bank, uint16_t from, uint16_t to, int with_bc, uint16_t n, uint16_t routine) {
	cpu.cycles += 7 + 10 + 10 + (with_bc ? 10 : 0);
	cpu.a = bank;
	SET_HL(from); SET_DE(to);
	if (with_bc) SET_BC(n);
	call(routine);
}

static void palette(uint16_t at) {     // ld hl,at / call $081F
	cpu.cycles += 10;
	SET_HL(at);
	call(0x081F);
}

static void screen_off(void) {         // the screen and the sound cleared ($0277, $0281, $0265)
	call(0x0277);
	call(0x0281);
	call(0x0265);
}

// $8042 (mode 1): the opening logo, only once ($C005); then mode 2
static void mode_logo(void) {
	if (!mem_read(0xC005)) { native_decline(); return; }   // (the logo itself: original code)
	call(0x7E87);
	cpu.cycles += 13 + 4 + 7 + 7 + 13 + 10;
	cpu.a = mem_read(0xC005);
	op_or(cpu.a);
	cpu.a = 2;
	mem_write(0xC600, 2);
}

static void mode_opening(void) {       // $818B (mode 2)
	call(0x7E87);
	screen_off();
	start_task(0x8220);
	jp(0x7E5A);
}

static void mode_title(void) {         // $81A0 (mode 3)
	call(0x7E87);
	screen_off();
	far(0x15, 0x80BE, 0x1D20, 1, 0x0140, 0x07F4);
	call(0x7EED);
	far(0x14, 0x8000, 0xCA00, 1, 0x0380, 0x07EC);
	call(0x0458);
	palette(0x9C18);
	start_task(0x8305);
	jp(0x7E5A);
}

static void mode_interlude(void) {     // $81DD (mode 4): the world map and its text
	call(0x014D);
	call(0x06CD);
	call(0x0B21);
	call(0x01CB);
	call(0x7EED);
	call(0x7EF3);
	cpu.cycles += 10 + 7 + 10;
	SET_HL(0x8000); cpu.a = 0x11; SET_DE(0x0000);
	call(0x0749);
	cpu.cycles += 7 + 10;
	cpu.a = 0x11; SET_DE(0xCB42);
	call(0x071A);
	cpu.cycles += 10 + 10;
	SET_DE(0xCA00); SET_BC(0x1801);
	call(0x7E0C);
	cpu.cycles += 10;
	SET_HL(0x8A6D);
	call(0x7F0B);
	call(0x0458);
	call(0x043D);
	start_task(0x894D);
	jp(0x7E5A);
}

// $8B70 (mode 5, Journey Onward): no game saved, a new one (mode 6);
// else the saved games' menu ($8BF8)
static void mode_journey(void) {
	call(0x7E84);
	uint8_t any = 0;
	static const uint16_t slot[3] = { 0x8040, 0x8080, 0x80C0 };
	for (int i = 0; i < 3; i++) {
		cpu.cycles += 10;
		SET_HL(slot[i]);
		call(0x0C3D);
		if (i < 2) cpu.cycles += 4;    // ld b,a / ld c,a
		if (i == 0) cpu.b = cpu.a; else if (i == 1) cpu.c = cpu.a;
		any |= cpu.a;
	}
	cpu.cycles += 4 + 4 + 7;
	op_or(cpu.b);
	op_or(cpu.c);
	if (!any) {
		cpu.cycles += 7 + 13 + 10;
		cpu.a = 6;
		mem_write(0xC600, 6);
		return;
	}
	cpu.cycles += 5;
	call(0x7EF0);
	far(0x13, 0xB4C9, 0x2100, 0, 0, 0x0788);
	palette(0x9C68);
	start_task(0x8BF8);
	jp(0x7E5A);
}

static void mode_new_game(void) {      // $8BAE (mode 6)
	call(0x7E84);
	call(0x7EF0);
	far(0x15, 0x823E, 0x2000, 1, 0x0100, 0x07F4);
	start_task(0x8EBD);
	jp(0x7E5A);
}

static void mode_story(void) {         // $8BCB (mode 7)
	call(0x7E87);
	call(0x7EF3);
	call(0x7EED);
	call(0x0458);
	start_task(0x92D4);
	jp(0x7E5A);
}

static void mode_gypsy(void) {         // $8BE0 (mode 8)
	call(0x7E84);
	cpu.cycles += 10 + 10 + 10;
	SET_HL(0x9A12); SET_DE(0x2BA0); SET_BC(0x0060);
	call(0x024C);
	start_task(0x945C);
	jp(0x7E5A);
}

static void mode_sound_test(void) {    // $9A72 (mode 9)
	call(0x7E87);
	call(0x7EF0);
	call(0x7EED);
	cpu.cycles += 10;
	SET_HL(0x9B83);
	call(0x7F0B);
	call(0x0458);
	start_task(0x9A8D);
	jp(0x7E5D);
}

// modes $14-$19 (bank 3): a task on the screen as it is
static void one_task(uint16_t entry) { call(0x7E84); start_task(entry); jp(0x7E5D); }
static void mode_fight(void) { one_task(0x9D1E); }           // $9CFE
static void mode_camp(void) { one_task(0xAD0F); }            // $AD03
static void mode_room_fight(void) { one_task(0x9D16); }      // $9D0A
static void mode_shrine(void) { one_task(0xAE63); }          // $AE57
static void mode_ending(void) { one_task(0xB127); }          // $B109
static void mode_credits(void) {       // $B115: the credits and the ankh
	call(0x7E84);
	start_task(0xB7CF);
	start_task(0xB63F);
	jp(0x7E5A);
}

// $AC07 (mode $10): the game's screen: the status window's frame, the
// font, the tiles; then the world (mode $11, by the task $AC62)
static void mode_game(void) {
	call(0x0DC5);
	call(0x0277);
	call(0x0281);
	call(0x2FA7);
	far(0x13, 0xA25A, 0x26E0, 0, 0, 0x0749);
	far(0x13, 0xB14F, 0x37E0, 1, 0x0020, 0x07F4);
	far(0x14, 0xAE88, 0x2440, 1, 0x0160, 0x07F4);
	far(0x14, 0xB148, 0xCA00, 1, 0x0600, 0x07EC);
	call(0x0458);
	cpu.cycles += 7 + 13 + 13 + 10 + 16;
	cpu.a = 0xFF;
	mem_write(0xC99F, 0xFF);
	mem_write(0xC98F, 0xFF);
	SET_HL(0x1701);
	wr16(0xC639, 0x1701);
	start_task(0xAC62);
	jp(0x0DE1);
}

static void map_task(uint16_t entry) { call(0x0DD7); start_task(entry); jp(0x0DE1); }
static void mode_world(void) { map_task(0xAC8C); }           // $AC74 (mode $11)
static void mode_town(void) { map_task(0xAF69); }            // $AC80 (mode $12)
static void mode_dungeon(void) { map_task(0x69E4); }         // $69D8 (mode $13, bank 1)

// $69E4: in a dungeon: standing in a room, its fight ($6C07); else the
// level drawn, the music, the turns ($6A0F)
static void dungeon_start(void) {
	cpu.cycles += 16;
	SET_HL(rd16(0xC953));
	call(0x6DB9);
	cpu.cycles += 19 + 7 + 7 + 10;
	mem_write(cpu.ix + 0x1A, cpu.a);
	op_and(0xF0);
	op_sub8(0xD0, 0, 0);
	if (cpu.a == 0xD0) { native_continue_at(0x6C07); return; }
	tcall(0x323D, 0x69F4);
	tcall(0x129D, 0x69F7);
	tcall(0x4324, 0x69FA);
	tcall(0x315F, 0x69FD);             // (it waits for frames)
	cpu.cycles += 7;
	cpu.a = 8;
	tcall(0x0621, 0x6A02);
	cpu.cycles += 4 + 13 + 10 + 16;
	cpu.a = 0;
	op_xor(0);
	mem_write(0xC9A1, 0);
	SET_HL(0x0000);
	wr16(0xC99C, 0);
	native_continue_at(0x6A0F);
}
Native native_dungeon_start = { "dungeon_start", 0x69E4, 1, dungeon_start, 0, 0, 0, .task = 1 };

#define MODE(name, addr, bank) Native native_##name = { #name, addr, bank, name, 0, .long_calls = 1 };
MODE(mode_logo, 0x8042, 3)
MODE(mode_opening, 0x818B, 3)
MODE(mode_title, 0x81A0, 3)
MODE(mode_interlude, 0x81DD, 3)
MODE(mode_journey, 0x8B70, 3)
MODE(mode_new_game, 0x8BAE, 3)
MODE(mode_story, 0x8BCB, 3)
MODE(mode_gypsy, 0x8BE0, 3)
MODE(mode_sound_test, 0x9A72, 3)
MODE(mode_fight, 0x9CFE, 3)
MODE(mode_camp, 0xAD03, 3)
MODE(mode_room_fight, 0x9D0A, 3)
MODE(mode_shrine, 0xAE57, 3)
MODE(mode_ending, 0xB109, 3)
MODE(mode_credits, 0xB115, 3)
MODE(mode_game, 0xAC07, 2)
MODE(mode_world, 0xAC74, 2)
MODE(mode_town, 0xAC80, 2)
MODE(mode_dungeon, 0x69D8, 1)
