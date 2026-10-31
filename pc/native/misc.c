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

// $46D0: "Can't!"
static void msg_cant(void) {
	ON_FIBER();
	C(0x40D1);
	say_inline(0x46D6);
	I(10);
}

// $646C: (L, H) = the party's cell + the step; carry when it is on the
// 32 x 32 map
static void step_in_town(void) {
	cpu.a = mem_read(0xC953); I(13);
	op_add8(mem_read((uint16_t)(cpu.ix + 0x0E)), 0); I(19);
	cpu.l = cpu.a; I(4);
	op_sub8(0x20, 0, 0); I(7);
	if (!(cpu.f & FC)) { I(11); return; }
	I(5);
	cpu.a = mem_read(0xC954); I(13);
	op_add8(mem_read((uint16_t)(cpu.ix + 0x10)), 0); I(19);
	cpu.h = cpu.a; I(4);
	op_sub8(0x20, 0, 0); I(7);
	I(10);
}

// $4933: a place in the table of people for something new (IY): a free one,
// else a chest ($3C, from the end), else a ship or horse not under the
// party, else one at random of the last ones
static void person_for_thing(void) {
	push16(BC); I(11); push16(DE); I(11); push16(HL); I(11);
	SET_HL(rd16(0xC953)); I(16);
	cpu.a = mem_read(0xC9A0); I(13);
	op_or(cpu.a); I(4);
	if (cpu.f & FZ) { I(12); cpu.iy = 0xC691; I(14); cpu.c = 0x18; I(7); }
	else { I(7); cpu.iy = 0xC651; I(14); cpu.c = 0x20; I(7); I(12); }
	SET_DE(0x0008); I(10);
	cpu.b = cpu.c; I(4);
	op_xor(cpu.a); I(4);
	int found = 0;
	do {
		op_sub8(mem_read((uint16_t)(cpu.iy + 3)), 0, 0); I(19);
		if (cpu.f & FZ) { I(12); found = 1; break; }
		I(7);
		cpu.iy = op_add16(cpu.iy, DE); I(15);
		I(--cpu.b ? 13 : 8);
	} while (cpu.b);
	if (!found) {
		SET_DE(0xFFF8); I(10);
		cpu.b = cpu.c; I(4);
		cpu.a = 0x3C; I(7);
		do {
			cpu.iy = op_add16(cpu.iy, DE); I(15);
			op_sub8(mem_read((uint16_t)(cpu.iy + 3)), 0, 0); I(19);
			if (cpu.f & FZ) { I(12); found = 1; break; }
			I(7);
			I(--cpu.b ? 13 : 8);
		} while (cpu.b);
	}
	if (!found) {
		SET_DE(0x0008); I(10);
		cpu.b = cpu.c; I(4);
		do {
			cpu.a = mem_read((uint16_t)(cpu.iy + 3)); I(19);
			op_sub8(0x10, 0, 0); I(7);
			if (cpu.f & FC) I(12);
			else {
				I(7);
				op_sub8(0x16, 0, 0); I(7);
				if (!(cpu.f & FC)) I(12);
				else {
					I(7);
					cpu.a = mem_read((uint16_t)(cpu.iy + 1)); I(19);
					op_sub8(cpu.l, 0, 0); I(4);
					if (!(cpu.f & FZ)) { I(12); found = 1; break; }
					I(7);
					cpu.a = mem_read((uint16_t)(cpu.iy + 2)); I(19);
					op_sub8(cpu.h, 0, 0); I(4);
					if (!(cpu.f & FZ)) { I(12); found = 1; break; }
					I(7);
				}
			}
			cpu.iy = op_add16(cpu.iy, DE); I(15);
			I(--cpu.b ? 13 : 8);
		} while (cpu.b);
	}
	if (!found) {
		cpu.a = 0x20; I(7);
		op_sub8(cpu.c, 0, 1); I(4);
		cpu.b = cpu.a; I(4);
		cpu.a = cpu.c; I(4);
		C(0x2BEB);
		op_add8(cpu.b, 0); I(4);
		C(0x48F7);
	}
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

// $AB38 (bank 3): a foe's touch takes 25 food; the status window
static void foe_eats(void) {
	ON_FIBER();
	push16(DE); I(11); push16(HL); I(11);
	C(0x7F11);
	SET_HL(rd16(0xC907)); I(16);
	SET_DE(0x0019); I(10);
	op_or(cpu.a); I(4);
	op_sbc16(DE); I(15);
	if (!(cpu.f & FC)) I(12);
	else { I(7); SET_HL(0x0000); I(10); }
	wr16(0xC907, HL); I(16);
	C(0x7F29);
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	I(10);
}

// $AB54 (bank 3): one time in four a foe takes up to 63 gold ($7F35)
static void foe_steals(void) {
	ON_FIBER();
	C(0x7ED8);
	op_and(0x03); I(7);
	if (!(cpu.f & FZ)) { I(11); return; }
	I(5);
	push16(HL); I(11);
	C(0x7F11);
	C(0x7ED8);
	op_and(0x3F); I(7);
	cpu.l = cpu.a; I(4);
	cpu.h = 0x00; I(7);
	C(0x7F35);
	SET_HL(pop16()); I(10);
	I(10);
}

// $A173: the pub's six rumours ($C9B1, from the flags in $C5xx): NZ when any
static void pub_rumours(void) {
	static const uint16_t flags[6] = { 0xC535, 0xC51F, 0xC52B, 0xC526, 0xC51A, 0xC529 };
	SET_HL(0xC9B1); I(10);
	for (int i = 0; i < 6; i++) {
		cpu.a = mem_read(flags[i]); I(13);
		mem_write(HL, cpu.a); I(7);
		SET_HL(HL + 1); I(6);
	}
	SET_HL(0xC9B1); I(10);
	cpu.b = 0x06; I(7);
	op_xor(cpu.a); I(4);
	do {
		op_or(mem_read(HL)); I(7);
		SET_HL(HL + 1); I(6);
		I(--cpu.b ? 13 : 8);
	} while (cpu.b);
	I(10);
}

// $AB92: a shop's offer: half of (price of IX+$1B at the table HL) x IX+$21,
// 9999 at most, kept at IX+$1F and printed
static void shop_offer(void) {
	ON_FIBER();
	cpu.a = mem_read((uint16_t)(cpu.ix + 0x1B)); I(19);
	I(11); call_original(0x0020);      // rst $20
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
	cpu.b = mem_read((uint16_t)(cpu.ix + 0x21)); I(19);
	SET_HL(0x0000); I(10);
	int over = 0;
	do {
		SET_HL(op_add16(HL, DE)); I(11);
		if (cpu.f & FC) { I(12); over = 1; break; }
		I(7);
		I(--cpu.b ? 13 : 8);
	} while (cpu.b);
	if (over) { SET_HL(0xFFFF); I(10); }
	else I(12);
	cpu.h = op_rot(7, cpu.h); I(8);
	cpu.l = op_rot(3, cpu.l); I(8);
	C(0x36DA);                         // cap_9999
	mem_write((uint16_t)(cpu.ix + 0x1F), cpu.l); I(19);
	mem_write((uint16_t)(cpu.ix + 0x20), cpu.h); I(19);
	I(10); native_continue_at(0x5627);
}

// $ABE7: the seer's menu of the eight virtues
static void virtues_menu(void) {
	mem_write((uint16_t)(cpu.ix + 0x1A), 0x00); I(19);
	C(0x2E1B);
	SET_DE(0x0213); I(10);
	SET_BC(0x0897); I(10);
	do {
		push16(BC); I(11);
		push16(DE); I(11);
		cpu.a = cpu.c; I(4);
		C(0x340F);                     // item_name
		C(0x2D69);                     // menu_text
		SET_DE(pop16()); I(10);
		SET_BC(pop16()); I(10);
		cpu.d = op_inc8(cpu.d); I(4);
		cpu.d = op_inc8(cpu.d); I(4);
		cpu.c = op_inc8(cpu.c); I(4);
		I(--cpu.b ? 13 : 8);
	} while (cpu.b);
	I(10); native_continue_at(0x2E54);
}

static void tile_over_party(void) {    // $BC14 (its call counted): the whirl drawn at the party's cell
	push16(HL); I(11);
	C(0x129D);
	SET_HL(pop16()); I(10);
	wr16(0xD978, HL); I(16);
	I(10); call_original(0x1A7C);
}

// $BC1F: a whirlpool takes the party: harm, everyone at that place gone,
// and on to mode $0F
static void whirlpool_takes(void) {
	ON_FIBER();
	SET_HL(0x008C); I(10);
	I(17); tile_over_party();
	C(0x4812);
	cpu.iy = 0xC651; I(14);
	SET_HL(rd16(0xC953)); I(16);
	SET_DE(0x0008); I(10);
	cpu.b = 0x20; I(7);
	do {
		cpu.a = mem_read((uint16_t)(cpu.iy + 1)); I(19);
		op_sub8(cpu.l, 0, 0); I(4);
		if (!(cpu.f & FZ)) I(12);
		else {
			I(7);
			cpu.a = mem_read((uint16_t)(cpu.iy + 2)); I(19);
			op_sub8(cpu.h, 0, 0); I(4);
			if (!(cpu.f & FZ)) I(12);
			else {
				I(7);
				mem_write((uint16_t)(cpu.iy + 3), 0); I(19);
				mem_write((uint16_t)(cpu.iy + 0), 0); I(19);
			}
		}
		cpu.iy = op_add16(cpu.iy, DE); I(15);
		I(--cpu.b ? 13 : 8);
	} while (cpu.b);
	cpu.a = 0x0F; I(7);
	mem_write(0xC600, 0x0F); I(13);
	I(10); native_continue_at(0x0CD2);
}

// $BC54: a twister: four times harm, and all but ships at that place gone
static void twister_takes(void) {
	ON_FIBER();
	SET_HL(0x008E); I(10);
	I(17); tile_over_party();
	for (int i = 0; i < 4; i++) C(0x4812);
	cpu.iy = 0xC651; I(14);
	SET_HL(rd16(0xC953)); I(16);
	SET_DE(0x0008); I(10);
	cpu.b = 0x20; I(7);
	do {
		cpu.a = mem_read((uint16_t)(cpu.iy + 1)); I(19);
		op_sub8(cpu.l, 0, 0); I(4);
		if (!(cpu.f & FZ)) I(12);
		else {
			I(7);
			cpu.a = mem_read((uint16_t)(cpu.iy + 2)); I(19);
			op_sub8(cpu.h, 0, 0); I(4);
			if (!(cpu.f & FZ)) I(12);
			else {
				I(7);
				cpu.a = mem_read((uint16_t)(cpu.iy + 3)); I(19);
				op_sub8(0x10, 0, 0); I(7);
				int ship = 0;
				if (cpu.f & FC) I(12);
				else {
					I(7);
					op_sub8(0x14, 0, 0); I(7);
					ship = (cpu.f & FC) != 0;
					I(ship ? 12 : 7);
				}
				if (!ship) {
					mem_write((uint16_t)(cpu.iy + 3), 0); I(19);
					mem_write((uint16_t)(cpu.iy + 0), 0); I(19);
				}
			}
		}
		cpu.iy = op_add16(cpu.iy, DE); I(15);
		I(--cpu.b ? 13 : 8);
	} while (cpu.b);
	I(10);
}
M(msg_cant, 0x46D0, 1, ALL)
M(step_in_town, 0x646C, 1, OUT_A | OUT_F | OUT_HL)
M(person_for_thing, 0x4933, 1, OUT_A | OUT_F | OUT_IY)
M(foe_eats, 0xAB38, 3, ALL)
M(foe_steals, 0xAB54, 3, ALL)
M(pub_rumours, 0xA173, 2, OUT_A | OUT_F | OUT_BC | OUT_HL)
M(shop_offer, 0xAB92, 2, ALL)
M(virtues_menu, 0xABE7, 2, ALL)
M(whirlpool_takes, 0xBC1F, 2, 0)
M(twister_takes, 0xBC54, 2, ALL)

// $36A3: Lord British's topics in the menu window, eight at most from
// IX+$1B, their names by the table at $8162 of bank $1A; the cursor on
// IX+$1C
static void british_topics(void) {
	cpu.a = 0x1A; I(7);
	vram_bank_push(0x36A5);
	C(0x2E1B);                         // menu_window_clear
	SET_DE(0xCAA6); I(10);
	cpu.c = mem_read((uint16_t)(cpu.ix + 0x1B)); I(19);
	cpu.b = 0x08; I(7);
	do {
		cpu.a = cpu.c; I(4);
		SET_HL(0xC5E0); I(10);
		I(11); call_original(0x0028);  // rst $28
		op_or(cpu.a); I(4);
		if (cpu.f & FZ) { I(12); break; }
		I(7);
		SET_HL(0x8162); I(10);
		I(11); call_original(0x0020);  // rst $20
		push16(DE); I(11);
		C(0x2D6C);                     // text_chars
		SET_DE(pop16()); I(10);
		SET_HL(0x0080); I(10);
		SET_HL(op_add16(HL, DE)); I(11);
		{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
		cpu.c = op_inc8(cpu.c); I(4);
		I(--cpu.b ? 13 : 8);
	} while (cpu.b);
	cpu.a = mem_read((uint16_t)(cpu.ix + 0x1C)); I(19);
	op_add8(cpu.a, 0); I(4);
	cpu.a = op_inc8(cpu.a); I(4);
	C(0x49D5);                         // cursor_on
	C(0x2E54);                         // menu_window_show
	vram_bank_pop();
}
M(british_topics, 0x36A3, 0, ALL)

// $3F3D: on a bridge, "Bridge Trolls!": a fight (the people's slot kept)
static void bridge_trolls(void) {
	ON_FIBER();
	say_inline(0x3F40);
	cpu.a = 0xA4; I(7);
	mem_write(0xC97D, 0xA4); I(13);
	SET_HL(rd16(0xC971)); I(16);
	wr16(0xC953, HL); I(16);
	wr16(0xC97A, HL); I(16);
	cpu.a = 0x17; I(7);
	mem_write(0xC97C, 0x17); I(13);
	I(10); native_continue_at(0x3B9F);
}

// the dungeon's traps: $3FA7 "Winds!" (the torch out), $3FBB "Falling
// Rocks!" and $3FD0 "Pit!" (harm)
static void trap_winds(void) {
	ON_FIBER();
	say_inline(0x3FAA);
	op_xor(cpu.a); I(4);
	mem_write(0xC95B, cpu.a); I(13);
	I(10);
}
static void trap_rocks(void) {
	ON_FIBER();
	say_inline(0x3FBE);
	I(12); I(10); native_continue_at(0x4812);
}
static void trap_pit(void) {
	ON_FIBER();
	say_inline(0x3FD3);
	I(10); native_continue_at(0x4812);
}
M(bridge_trolls, 0x3F3D, 0, 0)
M(trap_winds, 0x3FA7, 0, ALL)
M(trap_rocks, 0x3FBB, 0, 0)
M(trap_pit, 0x3FD0, 0, 0)

// The picture text of the help pages controller 2 opens in a dungeon: letters
// 6 pixels wide on the 4-plane tiles of the view, read back from VRAM,
// merged and written again, with the 8 pixel columns 2 tiles tall.
// $5670: character A at ($C60E, $C60F), in 6-pixel cells
static void page_letter(void) {
	op_sub8(0x20, 0, 1); I(7);
	op_add8(cpu.a, 0); I(4);
	cpu.l = cpu.a; I(4);
	cpu.h = 0x00; I(7);
	cpu.e = cpu.l; I(4);
	cpu.d = cpu.h; I(4);
	SET_HL(op_add16(HL, HL)); I(11);
	SET_HL(op_add16(HL, DE)); I(11);
	SET_DE(0xBD92); I(10);
	SET_HL(op_add16(HL, DE)); I(11);
	cpu.a = mem_read(0xDFFF); I(13);
	push16((uint16_t)(cpu.a << 8 | cpu.f)); I(11);
	cpu.a = 0x16; I(7);
	mem_write(0xFFFF, cpu.a); I(13);
	cpu.a = mem_read(0xC60E); I(13);
	op_add8(cpu.a, 0); I(4);
	cpu.b = cpu.a; I(4);
	op_add8(cpu.a, 0); I(4);
	op_add8(cpu.b, 0); I(4);
	cpu.b = cpu.a; I(4);
	op_and(0x07); I(7);
	cpu.c = cpu.a; I(4);
	cpu.c = op_inc8(cpu.c); I(4);
	cpu.a = cpu.b; I(4);
	op_exx(); I(4);
	op_and(0xF8); I(7);
	op_rrca(); I(4);
	op_rrca(); I(4);
	cpu.e = cpu.a; I(4);
	cpu.d = 0x00; I(7);
	SET_HL(0x543E); I(10);
	SET_HL(op_add16(HL, DE)); I(11);
	cpu.e = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	cpu.d = mem_read(HL); I(7);
	cpu.a = mem_read(0xC60F); I(13);
	op_add8(cpu.a, 0); I(4);
	cpu.b = cpu.a; I(4);
	op_add8(cpu.a, 0); I(4);
	op_add8(cpu.b, 0); I(4);
	cpu.l = cpu.a; I(4);
	cpu.h = 0x00; I(7);
	SET_HL(op_add16(HL, HL)); I(11);
	SET_HL(op_add16(HL, HL)); I(11);
	SET_HL(op_add16(HL, DE)); I(11);
	push16(HL); I(11);
	I(11); call_original(0x0008);      // rst $08: read from HL
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
	SET_HL(0xC1C0); I(10);
	SET_BC(0x18BE); I(10);
	do {
		op_block(4, 2); I(16);
		if (!(cpu.f & FZ)) I(12); else { I(7); break; }
	} while (1);
	SET_HL(0x0220); I(10);
	SET_HL(op_add16(HL, DE)); I(11);
	I(11); call_original(0x0008);
	SET_HL(0xC1D8); I(10);
	cpu.b = 0x18; I(7);
	do {
		op_block(4, 2); I(16);
		if (!(cpu.f & FZ)) I(12); else { I(7); break; }
	} while (1);
	SET_DE(pop16()); I(10);
	I(11); call_original(0x0010);      // rst $10: write at DE
	push16(DE); I(11);
	op_exx(); I(4);
	push16(cpu.iy); I(15);
	cpu.iy = 0xC1C0; I(14);
	cpu.d = 0x00; I(7);
	cpu.a = 0x06; I(7);
	do {
		op_ex_af(); I(4);
		cpu.a = mem_read(cpu.iy); I(19);
		io_out((uint16_t)(cpu.a << 8 | 0xBE), cpu.a); I(11);
		cpu.a = mem_read(HL); I(7);
		SET_HL(HL + 1); I(6);
		cpu.e = cpu.d; I(4);
		cpu.b = cpu.c; I(4);
		cpu.b = op_dec8(cpu.b); I(4);
		if (cpu.f & FZ) I(12);
		else {
			I(7);
			op_or(cpu.a); I(4);
			do {
				op_rra(); I(4);
				cpu.e = op_rot(3, cpu.e); I(8);
				I(--cpu.b ? 13 : 8);
			} while (cpu.b);
		}
		op_or(mem_read((uint16_t)(cpu.iy + 1))); I(19);
		io_out((uint16_t)(cpu.a << 8 | 0xBE), cpu.a); I(11);
		cpu.a = cpu.e; I(4);
		op_or(mem_read((uint16_t)(cpu.iy + 0x19))); I(19);
		mem_write((uint16_t)(cpu.iy + 0x19), cpu.a); I(19);
		cpu.a = mem_read((uint16_t)(cpu.iy + 2)); I(19);
		io_out((uint16_t)(cpu.a << 8 | 0xBE), cpu.a); I(11);
		cpu.a = mem_read((uint16_t)(cpu.iy + 3)); I(19);
		io_out((uint16_t)(cpu.a << 8 | 0xBE), cpu.a); I(11);
		cpu.e = 0x04; I(7);
		cpu.iy = op_add16(cpu.iy, DE); I(15);
		op_ex_af(); I(4);
		cpu.a = op_dec8(cpu.a); I(4);
		if (!(cpu.f & FZ)) I(12); else { I(7); break; }
	} while (1);
	cpu.iy = pop16(); I(14);
	SET_DE(pop16()); I(10);
	SET_HL(0x0220); I(10);
	SET_HL(op_add16(HL, DE)); I(11);
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
	I(11); call_original(0x0010);
	SET_HL(0xC1D8); I(10);
	SET_BC(0x18BE); I(10);
	do {
		op_block(4, 3); I(16);
		if (!(cpu.f & FZ)) I(12); else { I(7); break; }
	} while (1);
	SET_HL(0xC60E); I(10);
	mem_write(HL, op_inc8(mem_read(HL))); I(11);
	{ uint16_t af = pop16(); cpu.a = af >> 8; cpu.f = (uint8_t)af; } I(10);
	mem_write(0xFFFF, cpu.a); I(13);
	I(10);
}

// $5659: at HL, lines of (x, y, letters, 0), ended by $FF
static void page_text(void) {
	for (;;) {
		cpu.e = mem_read(HL); I(7);
		SET_HL(HL + 1); I(6);
		cpu.e = op_inc8(cpu.e); I(4);
		if (cpu.f & FZ) { I(11); return; }
		I(5);
		cpu.e = op_dec8(cpu.e); I(4);
		cpu.d = mem_read(HL); I(7);
		SET_HL(HL + 1); I(6);
		wr16(0xC60E, DE); I(20);
		for (;;) {
			cpu.a = mem_read(HL); I(7);
			SET_HL(HL + 1); I(6);
			op_or(cpu.a); I(4);
			if (cpu.f & FZ) { I(12); break; }
			I(7);
			push16(HL); I(11);
			I(17); page_letter();
			SET_HL(pop16()); I(10);
			I(12);
		}
	}
}

// $57C5: help page IX+$1A (of bank $16, the table at $B1CE)
static void help_page(void) {
	cpu.a = 0x16; I(7);
	vram_bank_push(0x57C7);
	cpu.a = mem_read((uint16_t)(cpu.ix + 0x1A)); I(19);
	SET_HL(0xB1CE); I(10);
	I(11); call_original(0x0020);      // rst $20
	I(17); page_text();
	vram_bank_pop();
}
M(page_text, 0x5659, 1, ALL)
M(help_page, 0x57C5, 1, ALL)

static void far_text(void) {           // $5597: the text at HL of bank A
	ON_FIBER();
	vram_bank_push(0x5597);
	C(0x5519);
	vram_bank_pop();
}

static void step_from_de(void) {       // $6C59: (L, H) = (E, D) one step in direction A, on the level
	push16(DE); I(11);
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
	C(0x40B9);                         // step_of
	cpu.a = cpu.e; I(4);
	op_add8(cpu.l, 0); I(4);
	op_and(0x0F); I(7);
	cpu.l = cpu.a; I(4);
	cpu.a = cpu.d; I(4);
	op_add8(cpu.h, 0); I(4);
	op_and(0x0F); I(7);
	cpu.h = cpu.a; I(4);
	SET_DE(pop16()); I(10);
	I(10);
}

// $6C07: into a room through the wall tile IX+$1A: the party steps past
// it, the room's number from the tile (in the Abyss by level too), mode $16
static void room_from_wall(void) {
	cpu.a = 0x00; I(7);
	C(0x6C59);
	C(0x6DB9);
	op_sub8(mem_read((uint16_t)(cpu.ix + 0x1A)), 0, 0); I(19);
	if (cpu.f & FZ) I(12);
	else { I(7); cpu.a = 0x02; I(7); C(0x6C59); }
	cpu.a = 0x01; I(7);
	C(0x6C59);
	C(0x6DB9);
	op_sub8(mem_read((uint16_t)(cpu.ix + 0x1A)), 0, 0); I(19);
	if (cpu.f & FZ) I(12);
	else { I(7); cpu.a = 0x03; I(7); C(0x6C59); }
	wr16(0xC953, HL); I(16);
	cpu.a = mem_read(0xC9A0); I(13);
	op_sub8(0x19, 0, 0); I(7);
	cpu.b = 0x00; I(7);
	if (!(cpu.f & FZ)) I(12);
	else {
		I(7);
		cpu.a = mem_read(0xC970); I(13);
		op_add8(cpu.a, 0); I(4);
		op_add8(cpu.a, 0); I(4);
		op_add8(cpu.a, 0); I(4);
		op_and(0xF0); I(7);
		cpu.b = cpu.a; I(4);
	}
	cpu.a = mem_read((uint16_t)(cpu.ix + 0x1A)); I(19);
	op_and(0x0F); I(7);
	op_or(cpu.b); I(4);
	mem_write(0xC9B8, cpu.a); I(13);   // dungeon_room
	C(0x3D82);                         // load_dungeon_room
	cpu.a = 0x02; I(7);
	mem_write(0xC975, 0x02); I(13);
	cpu.a = 0x16; I(7);
	mem_write(0xC600, 0x16); I(13);
	I(10); native_continue_at(0x0CD2);
}

// $028B: the logo's name table: at HL, width and height (bytes), the
// bytes for the cells from DE; byte 2 of a cell from $C0A9 or A
static void logo_names(void) {
	mem_write(0xC0A8, cpu.a); I(13);
	cpu.c = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	cpu.b = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	do {
		push16(BC); I(11);
		I(11); call_original(0x0010);  // rst $10
		cpu.b = cpu.c; I(4);
		do {
			cpu.a = mem_read(0xC0A9); I(13);
			op_or(cpu.a); I(4);
			cpu.a = mem_read(HL); I(7);
			SET_HL(HL + 1); I(6);
			int skip = 0;
			if (cpu.f & FZ) I(12);
			else {
				I(7);
				op_or(cpu.a); I(4);
				if (!(cpu.f & FZ)) I(12);
				else { I(7); skip = 1; }
			}
			if (skip) {
				cpu.a = io_in((uint16_t)(cpu.a << 8 | 0xBE)); I(11);
				C(0x0262);
				cpu.a = io_in((uint16_t)(cpu.a << 8 | 0xBE)); I(11);
				I(10);
			} else {
				io_out((uint16_t)(cpu.a << 8 | 0xBE), cpu.a); I(11);
				C(0x0262);
				cpu.a = mem_read(0xC0A8); I(13);
				io_out((uint16_t)(cpu.a << 8 | 0xBE), cpu.a); I(11);
			}
			I(--cpu.b ? 13 : 8);
		} while (cpu.b);
		{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
		SET_BC(0x0040); I(10);
		SET_HL(op_add16(HL, BC)); I(11);
		{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
		SET_BC(pop16()); I(10);
		I(--cpu.b ? 13 : 8);
	} while (cpu.b);
	I(10);
}

// the turns' waits for input by the machine's route: a frame, then input_step
static void world_input_frame(void) { push16(0xACCF); I(17); native_continue_at(0x2FFB); }
static void world_input(void) { push16(0xACD2); I(17); native_continue_at(0xB08B); }
static void town_input_frame(void) { push16(0xAF9D); I(17); native_continue_at(0x2FFB); }
static void town_input(void) { push16(0xAFA0); I(17); native_continue_at(0xB08B); }

M(far_text, 0x5597, 1, ALL)
M(step_from_de, 0x6C59, 1, OUT_A | OUT_F | OUT_HL)
M(room_from_wall, 0x6C07, 1, 0)
M(logo_names, 0x028B, 0, ALL)
M(world_input_frame, 0xACCC, 2, 0)
M(world_input, 0xACCF, 2, 0)
M(town_input_frame, 0xAF9A, 2, 0)
M(town_input, 0xAF9D, 2, 0)
