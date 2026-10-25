// What each turn costs the party (bank 2, from the end of turn $B4CA).
// - in_balloon ($957C): Z when the party travels by balloon (its figure,
//   $C959, is $18).
// - hull_mend ($B55E): one time in four, the ship's hull ($C961) mends a
//   point, up to 50.
// - afflictions ($B4EE): a sleeping member ($53) wakes ($47) one time in
//   eight; a poisoned one ($50) takes 2 damage and its status is shown
//   ($49A6), and if any did, a sound and a frame's wait. Then the party
//   eats (one hundredth of food each, $4753); with none left it is
//   "Starving!", every member who can act takes 2 damage, a sound, and
//   the party's state is checked ($4896).
// Cycle counts are the original's, instruction by instruction (I).
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define PARTY       0xC751
#define PARTY_SIZE  0xC957

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static void call(uint16_t addr) { I(17); call_original(addr); }

static void in_balloon(void) {         // $957C
	cpu.a = mem_read(0xC959); I(13);
	op_sub8(0x18, 0, 0); I(7);
	I(10);
}

static void hull_mend(void) {          // $B55E
	SET_HL(0xC961); I(10);
	cpu.a = mem_read(HL); I(7);
	op_sub8(0x32, 0, 0); I(7);
	if (!(cpu.f & FC)) { I(11); return; }
	I(5);
	call(0x2BBF);                      // random
	op_and(0x03); I(7);
	if (!(cpu.f & FZ)) { I(11); return; }
	I(5);
	mem_write(HL, op_inc8(mem_read(HL))); I(11);
	I(10);
}

static int waits_this_turn(void) {     // a poisoned member (the frame's wait) or no food left (printing)
	uint8_t n = mem_read(PARTY_SIZE);
	for (int i = 0; i < n; i++)
		if (mem_read((uint16_t)(PARTY + 29 * i + 0x1C)) == 0x50) return 1;
	uint32_t food = (uint32_t)rd16(0xC907) * 100 + mem_read(0xC906);
	return food < n;
}

static void afflictions(void) {        // $B4EE
	if (!native_on_fiber() && waits_this_turn()) { native_decline(); return; }   // shadow mode
	uint16_t count = (uint16_t)(cpu.ix + 0x17);
	cpu.iy = PARTY; I(14);
	SET_DE(0x001D); I(10);
	cpu.a = mem_read(PARTY_SIZE); I(13);
	cpu.b = cpu.a; I(4);
	cpu.c = 0x00; I(7);
	mem_write(count, 0x01); I(19);
	do {
		cpu.a = mem_read((uint16_t)(cpu.iy + 0x1C)); I(19);
		op_sub8(0x53, 0, 0); I(7);
		if (cpu.f & FZ) {              // asleep
			I(7);
			call(0x2BBF);              // random
			op_and(0x07); I(7);
			if (!(cpu.f & FZ)) I(12);
			else { I(7); mem_write((uint16_t)(cpu.iy + 0x1C), 0x47); I(19); I(12); }
		} else {
			I(12);
			op_sub8(0x50, 0, 0); I(7);
			if (!(cpu.f & FZ)) I(12);
			else {                     // poisoned
				I(7);
				SET_HL(0x0002); I(10);
				call(0x467F);          // member_take_damage
				call(0x49A6);
				mem_write(count, op_inc8(mem_read(count))); I(23);
			}
		}
		cpu.iy = op_add16(cpu.iy, DE); I(15);
		cpu.c = op_inc8(cpu.c); I(4);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	mem_write(count, op_dec8(mem_read(count))); I(23);
	if (cpu.f & FZ) I(12);
	else {
		I(7);
		cpu.a = 0x11; I(7);
		call(0x05F1);                  // play_sound
		call(0x041C);                  // wait_vblank
	}
	cpu.a = mem_read(PARTY_SIZE); I(13);
	call(0x4753);                      // eat
	if (!(cpu.f & FC)) { I(11); return; }
	I(5);
	SET_HL(0xBD22); I(10);
	call(0x5519);                      // print_text_hl: starving
	cpu.a = mem_read(PARTY_SIZE); I(13);
	cpu.b = cpu.a; I(4);
	cpu.c = 0x00; I(7);
	do {
		call(0x4445);                  // member_can_act
		if (cpu.f & FC) I(12);
		else {
			I(7);
			SET_HL(0x0002); I(10);
			call(0x467F);
		}
		cpu.c = op_inc8(cpu.c); I(4);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	cpu.a = 0x11; I(7);
	call(0x05F1);
	I(10);
	native_continue_at(0x4896);
}

#define UPKEEP(name, addr, out) Native native_##name = { #name, addr, 2, name, out, .long_calls = 1 };
UPKEEP(in_balloon, 0x957C, OUT_A | OUT_F)
UPKEEP(hull_mend, 0xB55E, OUT_A | OUT_F | OUT_HL)
UPKEEP(afflictions, 0xB4EE, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL | OUT_IY)
