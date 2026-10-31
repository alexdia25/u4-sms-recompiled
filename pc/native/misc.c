// The last small pieces of the original, none big enough for a file of its
// own: the power-on's first instructions and cold start, the pad port's
// setup, the random seed, the last message and map helpers, and the jumps
// left between rewritten routines.
// Cycle counts are the original's, instruction by instruction (I).
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }
#define C(addr) do { I(17); call_original(addr); } while (0)
#define ON_FIBER() do { if (!native_on_fiber()) { native_decline(); return; } } while (0)   // shadow mode: it waits

static void first_jump(void) {         // $0000: di / im 1 / jp reset
	cpu.iff1 = cpu.iff2 = 0; I(4);
	cpu.im = 1; I(8);
	I(10); native_continue_at(0x0042);
}
static void after_reset(void) { I(10); native_continue_at(0x0CA6); }   // $0063

// $0CB4: a game's start: the stack reset, a mode's first start, mode 1
static void cold_end(void) {
	cpu.iff1 = cpu.iff2 = 0; I(4);
	cpu.im = 1; I(8);
	cpu.sp = 0xDFF0; I(10);
	C(0x0DA3);                         // mode_fresh
	cpu.a = 0x01; I(7);
	mem_write(0xC600, 0x01); I(13);    // game_mode
	I(12); native_continue_at(0x0CD2); // jr main_loop
}
// $0CA6: buttons 1 and 2 held at power-on: $C002 cleared; $0C50 checks the save
static void cold_start(void) {
	C(0x064C);                         // read_pads_both
	op_sub8(0x30, 0, 0); I(7);
	if (!(cpu.f & FZ)) I(12);
	else {
		I(7);
		op_xor(cpu.a); I(4);
		mem_write(0xC002, cpu.a); I(13);
	}
	C(0x0C50);
	cold_end();
}

static void pad_port_setup(void) {     // $063D
	cpu.a = 0x92; I(7);
	io_out((uint16_t)(cpu.a << 8 | 0xDF), cpu.a); I(11);
	cpu.a = 0x07; I(7);
	io_out((uint16_t)(cpu.a << 8 | 0xDE), cpu.a); I(11);
	I(10);
}

static void seed_random(void) {        // $2BF7
	SET_HL(0x55AA); I(10);
	wr16(0xC0B6, HL); I(16);
	I(10);
}

static void text_block_far(void) {     // $2D1C: $2D25 with bank A in
	vram_bank_push(0x2D1C);
	C(0x2D25);
	vram_bank_pop();
}

static void just_ret(void) { I(10); }  // $3FF3

static void say_inline(uint16_t text) {
	uint16_t hl = HL;
	I(17 + 19 + 17);
	text_print(text);
	I(19 + 10);
	SET_HL(hl);
}

static void msg_what(void) {           // $46C2: "What?"
	ON_FIBER();
	C(0x40D1);
	say_inline(0x46C8);
	I(10);
}

static void two_newlines(void) {       // $54CB
	ON_FIBER();
	C(0x54CE);
	native_continue_at(0x54CE);
}

static void step_on_level(void) {      // $6480: (L, H) = the party's cell + the step, on the 16 x 16 level
	cpu.a = mem_read(0xC953); I(13);
	op_add8(mem_read((uint16_t)(cpu.ix + 0x0E)), 0); I(19);
	op_and(0x0F); I(7);
	cpu.l = cpu.a; I(4);
	cpu.a = mem_read(0xC954); I(13);
	op_add8(mem_read((uint16_t)(cpu.ix + 0x10)), 0); I(19);
	op_and(0x0F); I(7);
	cpu.h = cpu.a; I(4);
	I(10);
}

static void step_on_field(void) {      // $6493: (L, H) = the member's figure's cell + the step
	C(0x48E1);                         // current_object
	cpu.a = mem_read(cpu.iy); I(19);
	op_add8(mem_read((uint16_t)(cpu.ix + 0x0E)), 0); I(19);
	cpu.l = cpu.a; I(4);
	cpu.a = mem_read((uint16_t)(cpu.iy + 1)); I(19);
	op_add8(mem_read((uint16_t)(cpu.ix + 0x10)), 0); I(19);
	cpu.h = cpu.a; I(4);
	I(10);
}

// $6D96: a dungeon's creature reaches the party: "Attacked by <it>", the fight
static void attacked_by(void) {
	ON_FIBER();
	C(0x129D);
	say_inline(0x6D9C);
	cpu.a = mem_read((uint16_t)(cpu.iy + 3)); I(19);
	C(0x47D3);                         // figure_name
	C(0x5519);
	C(0x54CE);
	I(10); native_continue_at(0x3B52); // fight_in_dungeon
}

// $8084, for original routes into it (a spell's end, $8671): as command_end_at
static void command_end(void) {
	ON_FIBER();
	C(0x4324);
	C(0x041C);
	C(0x06D4);
	op_or(cpu.a); I(4);
	native_continue_at(0x808E);
}
static void spell_ended(void) { I(10); native_continue_at(0x8084); }   // $8671

static void ask_price(void) {          // $A5F8: the healer asks (HL kept at IX+$20)
	ON_FIBER();
	mem_write((uint16_t)(cpu.ix + 0x20), cpu.l); I(19);
	mem_write((uint16_t)(cpu.ix + 0x21), cpu.h); I(19);
	cpu.a = 0x17; I(7);
	C(0x5582);                         // print_message
	I(10); native_continue_at(0x4B5E); // yes_no_start
}

// $A627: the healer's touch: a sound, the member's line flashed ($41A9
// twice), frame by frame
static void healed_flash(void) {
	ON_FIBER();
	cpu.a = 0x15; I(7);
	C(0x05F1);
	cpu.c = mem_read((uint16_t)(cpu.ix + 0x08)); I(19);
	C(0x49A6);
	C(0x041C);
	C(0x41A9);
	C(0x041C);
	C(0x41A9);
	C(0x041C);
	cpu.c = mem_read((uint16_t)(cpu.ix + 0x08)); I(19);
	I(10); native_continue_at(0x48B6);
}

static void inn_light(void) {          // $A836: one step of the inn's light back
	SET_HL(0x7C83); I(10);
	cpu.c = 0x01; I(7);
	I(10); native_continue_at(0x0855);
}

// the jumps at the end of the turns' waits for input ("jr c" back)
static void world_input_back(void) {   // $ACD2
	if (cpu.f & FC) { I(12); native_continue_at(0xACCC); }
	else { I(7); native_continue_at(0xACD4); }
}
static void town_input_back(void) {    // $AFA0
	if (cpu.f & FC) { I(12); native_continue_at(0xAF9A); }
	else { I(7); native_continue_at(0xAFA2); }
}
static void combat_input_back(void) {  // $9D9E
	if (cpu.f & FC) { I(12); native_continue_at(0x9D6E); }
	else { I(7); native_continue_at(0x9DA0); }
}
static void town_turns_again(void) {   // $AF7B: nothing done yet, no time spent
	op_xor(cpu.a); I(4);
	mem_write(0xC9A1, cpu.a); I(13);
	SET_HL(0x0000); I(10);
	wr16(0xC99C, HL); I(16);
	native_continue_at(0xAF85);
}

// $A43E: (L, H) = the battlefield cell of the missile's place (IX+$0D, $0B),
// by $7E3C, 12 pixels a cell from a cell's middle
static void missile_cell(void) {
	cpu.a = mem_read((uint16_t)(cpu.ix + 0x0D)); I(19);
	op_add8(0x06, 0); I(7);
	cpu.l = cpu.a; I(4);
	cpu.h = 0x0C; I(7);
	C(0x7E3C);
	cpu.b = cpu.l; I(4);
	cpu.a = mem_read((uint16_t)(cpu.ix + 0x0B)); I(19);
	op_add8(0x06, 0); I(7);
	cpu.l = cpu.a; I(4);
	cpu.h = 0x0C; I(7);
	C(0x7E3C);
	cpu.h = cpu.b; I(4);
	I(10);
}

// $B085: the vision's picture A at the shrine ($8923 and the table at $B09E,
// by the vectors $7E78 and $7E7B)
static void vision_picture(void) {
	ON_FIBER();
	uint8_t a = cpu.a, f = cpu.f;
	push16((uint16_t)(a << 8 | f)); I(11);
	cpu.a = 0x10; I(7);
	SET_HL(0x8923); I(10);
	SET_DE(0x0020); I(10);
	C(0x7E78);
	{ uint16_t af = pop16(); cpu.a = af >> 8; cpu.f = (uint8_t)af; } I(10);
	SET_HL(0xB09E); I(10);
	I(11); call_original(0x0020);      // rst $20
	cpu.a = 0x10; I(7);
	SET_DE(0x0DA0); I(10);
	I(10); native_continue_at(0x7E7B);
}

#define ALL (OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
#define M(name, addr, bank, out) Native native_##name = { #name, addr, bank, name, out, .long_calls = 1 };
M(first_jump, 0x0000, 0, 0)
M(after_reset, 0x0063, 0, 0)
M(cold_start, 0x0CA6, 0, 0)
M(cold_end, 0x0CB4, 0, 0)
M(pad_port_setup, 0x063D, 0, OUT_A)
M(seed_random, 0x2BF7, 0, OUT_HL)
M(text_block_far, 0x2D1C, 0, ALL)
M(just_ret, 0x3FF3, 0, 0)
M(msg_what, 0x46C2, 1, ALL)
M(two_newlines, 0x54CB, 1, ALL)
M(step_on_level, 0x6480, 1, OUT_A | OUT_F | OUT_HL)
M(step_on_field, 0x6493, 1, OUT_A | OUT_F | OUT_HL)
M(attacked_by, 0x6D96, 1, 0)
M(command_end, 0x8084, 2, 0)
M(spell_ended, 0x8671, 2, 0)
M(ask_price, 0xA5F8, 2, ALL)
M(healed_flash, 0xA627, 2, ALL)
M(inn_light, 0xA836, 2, ALL)
M(world_input_back, 0xACD2, 2, 0)
M(town_input_back, 0xAFA0, 2, 0)
M(town_turns_again, 0xAF7B, 2, 0)
M(combat_input_back, 0x9D9E, 3, 0)
M(missile_cell, 0xA43E, 3, ALL)
M(vision_picture, 0xB085, 3, ALL)
