// Harm from the ground and from foes, and four sounds (bank 1, $4004-$40E8).
// The member is the one whose turn it is ($C97F) unless said.
// - poison_member ($4004): one standing ($47) is poisoned ($50), then the
//   status window and its line ($4247) again (the tail at $4011).
// - sleep_member ($401B): one able to act falls asleep ($53), its figure
//   too ($38).
// - burn_member ($4037): 16-47 damage (member_hurt).
// - sleep_field ($404A) / poison_field ($4071): each member able to act, one
//   time in four, asleep ($53) or poisoned ($50).
// - blocked ($40A6): a sound, and "Blocked!".
// - sounds $0C, $0D, $1E and $20, A kept ($40C9, $40D1, $40D9, $40E1).
// All but the sounds hand shadow mode back to the original: the member's
// line ($4247) may wait for frames.
// Cycle counts are the original's, instruction by instruction (I).
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define TURN  0xC97F

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static void push(uint16_t v) { push16(v); I(11); }
static void call(uint16_t addr) { I(17); call_original(addr); }
static void iy_set(uint8_t d, uint8_t v) { mem_write((uint16_t)(cpu.iy + d), v); I(19); }

static void member_shown(void) {       // $4011: the status window, then the member's line
	call(0x4324);
	SET_BC(rd16(TURN)); I(20);
	I(10);
	native_continue_at(0x4247);
}

static void poison_member(void) {      // $4004
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: the member's line ($4247) may wait for frames
	call(0x48CB);                      // current_record
	cpu.a = mem_read((uint16_t)(cpu.iy + 0x1C)); I(19);
	op_sub8(0x47, 0, 0); I(7);
	if (!(cpu.f & FZ)) { I(11); return; }
	I(5);
	iy_set(0x1C, 0x50);
	member_shown();
}

static void sleep_member(void) {       // $401B
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: the member's line ($4247) may wait for frames
	SET_BC(rd16(TURN)); I(20);
	call(0x4466);                      // member_able
	if (cpu.f & FC) { I(11); return; }
	I(5);
	call(0x48CB);
	iy_set(0x1C, 0x53);
	call(0x48E1);                      // current_object
	iy_set(0x02, 0x38);
	iy_set(0x03, 0x38);
	I(12);
	member_shown();
}

static void burn_member(void) {        // $4037
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: the member's line ($4247) may wait for frames
	cpu.a = 0x20; I(7);
	call(0x2BEB);                      // random_below
	op_add8(0x10, 0); I(7);
	cpu.l = cpu.a; I(4);
	cpu.h = 0x00; I(7);
	SET_BC(rd16(TURN)); I(20);
	call(0x4679);                      // member_hurt
	I(12);
	member_shown();
}

static void field(uint8_t status) {    // $404A / $4071
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: the member's line ($4247) may wait for frames
	push(BC); push(DE);
	push16(cpu.iy); I(15);
	call(0x4098);                      // party_iter_setup
	do {
		call(0x4466);
		if (cpu.f & FC) I(12);
		else {
			I(7);
			call(0x2BBF);              // random
			op_and(0x03); I(7);
			if (!(cpu.f & FZ)) I(12);
			else {
				I(7);
				iy_set(0x1C, status);
				call(0x4247);
				call(0x4324);
			}
		}
		cpu.iy = op_add16(cpu.iy, DE); I(15);
		cpu.c = op_inc8(cpu.c); I(4);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	cpu.iy = pop16(); I(14);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}
static void sleep_field(void) { field(0x53); }
static void poison_field(void) { field(0x50); }

static void blocked(void) {            // $40A6
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: printing may wait for frames
	cpu.a = 0x10; I(7);
	call(0x05F1);                      // play_sound
	uint16_t hl = HL;                  // call print_inline / the text follows
	cpu.cycles += 17 + 19 + 17;
	text_print(0x40AE);
	cpu.cycles += 19 + 10;
	cpu.a = 0;
	SET_HL(hl);
	I(10);
}

static void sound_kept_a(uint8_t n) {  // push af / ld a,n / call play_sound / pop af / ret
	push((uint16_t)(cpu.a << 8 | cpu.f));
	cpu.a = n; I(7);
	call(0x05F1);
	uint16_t af = pop16(); cpu.a = af >> 8; cpu.f = (uint8_t)af; I(10);
	I(10);
}
static void sound_0c(void) { sound_kept_a(0x0C); }
static void sound_0d(void) { sound_kept_a(0x0D); }
static void sound_1e(void) { sound_kept_a(0x1E); }
static void sound_20(void) { sound_kept_a(0x20); }

#define FIELD(name, addr, out) Native native_##name = { #name, addr, 1, name, out, .long_calls = 1 };
FIELD(poison_member, 0x4004, OUT_A | OUT_F | OUT_BC | OUT_IY)
FIELD(sleep_member, 0x401B, OUT_A | OUT_F | OUT_BC | OUT_IY)
FIELD(burn_member, 0x4037, OUT_A | OUT_F | OUT_BC | OUT_HL)
FIELD(sleep_field, 0x404A, OUT_A | OUT_F)
FIELD(poison_field, 0x4071, OUT_A | OUT_F)
FIELD(blocked, 0x40A6, OUT_A | OUT_F)
FIELD(sound_0c, 0x40C9, OUT_A | OUT_F)
FIELD(sound_0d, 0x40D1, OUT_A | OUT_F)
FIELD(sound_1e, 0x40D9, OUT_A | OUT_F)
FIELD(sound_20, 0x40E1, OUT_A | OUT_F)
