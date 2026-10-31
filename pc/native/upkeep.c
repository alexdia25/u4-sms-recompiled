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
// - below_c8fd ($BBC6): carry when the 4-byte number at $C8FD is below
//   the one at HL (both low byte first); HL 4 on.
// - ship_wind ($B36C): a ship's move takes longer by the wind ($C98A, 4
//   calm) against its heading ($C9B9): calm x4, the wind's direction the
//   same as the heading x5, the opposite one x2 (the move's time at
//   $C99A), each told; across it, as it is.
// - palette_dim ($A81A, bank 2): every colour of the palette's copy whose
//   bits A are not 0 one step darker in them; sent next frame.
// - whirl_check ($BBE7): on the world, whether one of the first four
//   figures ($C651) is a whirlpool ($8C) or a twister ($8E) on the party;
//   what it does then is left to the original ($BC1F, $BC54).
// - wind_holds_back ($B42E): whether a sail in direction A is held back
//   this turn (carry), by the wind ($C98A) and the move count ($C8FD):
//   calm, 7 turns in 8; the wind's own direction, 3 in 4; the opposite
//   one, 1 in 4; across it, never.
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

static void below_c8fd(void) {         // $BBC6
	push16(BC); I(11);
	push16(DE); I(11);
	SET_DE(0xC8FD); I(10);
	op_xor(cpu.a); I(4);
	cpu.b = 0x04; I(7);
	do {
		cpu.a = mem_read(DE); I(7);
		op_sub8(mem_read(HL), cpu.f & FC, 1); I(7);
		SET_DE(DE + 1); I(6);
		SET_HL(HL + 1); I(6);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

static void say_inline(uint16_t text) {   // call print_inline / the text follows
	uint16_t hl = HL;
	uint8_t b = cpu.b, c = cpu.c;
	cpu.cycles += 17 + 19 + 17;
	text_print(text);
	cpu.cycles += 19 + 10;
	cpu.b = b; cpu.c = c;
	cpu.a = 0;
	SET_HL(hl);
}

static void ship_wind(void) {          // $B36C
	uint8_t heading = mem_read(0xC9B9), wind = mem_read(0xC98A);
	if (!native_on_fiber() && (wind == 4 || wind == heading || ((wind + 2) & 3) == heading)) {
		native_decline();              // shadow mode: it prints, which may wait for frames
		return;
	}
	cpu.a = heading; I(13);
	cpu.b = cpu.a; I(4);
	cpu.a = wind; I(13);
	op_sub8(0x04, 0, 0); I(7);
	int times;
	if (cpu.f & FZ) {                  // calm
		I(12);
		say_inline(0xB3B8);
		times = 4;
	} else {
		I(7);
		op_sub8(cpu.b, 0, 0); I(4);
		if (cpu.f & FZ) {              // the same direction
			I(12);
			say_inline(0xB3A0);
			times = 3;
		} else {
			I(7);
			op_add8(0x02, 0); I(7);
			op_and(0x03); I(7);
			op_sub8(cpu.b, 0, 0); I(4);
			if (!(cpu.f & FZ)) { I(7); I(10); return; }
			I(12);
			say_inline(0xB385);
			times = 2;
		}
	}
	SET_HL(rd16(0xC99A)); I(16);
	if (times == 3) {
		cpu.e = cpu.l; I(4);
		cpu.d = cpu.h; I(4);
		SET_HL(op_add16(HL, HL)); I(11);
		SET_HL(op_add16(HL, DE)); I(11);
		I(12);
	} else if (times == 4) {
		SET_HL(op_add16(HL, HL)); I(11);
		SET_HL(op_add16(HL, HL)); I(11);
		I(12);
	} else {
		SET_HL(op_add16(HL, HL)); I(11);
	}
	wr16(0xC99A, HL); I(16);
	I(10);
}

static void palette_dim(void) {        // $A81A
	cpu.d = cpu.a; I(4);
	op_cpl(); I(4);
	cpu.e = cpu.a; I(4);
	SET_HL(0xC023); I(10);
	cpu.b = 0x20; I(7);
	do {
		cpu.a = mem_read(HL); I(7);
		op_and(cpu.d); I(4);
		if (cpu.f & FZ) I(12);
		else {
			I(7);
			cpu.a = op_dec8(cpu.a); I(4);
			op_and(cpu.d); I(4);
			cpu.c = cpu.a; I(4);
			cpu.a = mem_read(HL); I(7);
			op_and(cpu.e); I(4);
			op_or(cpu.c); I(4);
			mem_write(HL, cpu.a); I(7);
		}
		SET_HL(HL + 1); I(6);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	cpu.a = 0xFF; I(7);
	mem_write(0xC009, cpu.a); I(13);
	I(10);
}

static int whirl_on_party(void) {
	for (int i = 0; i < 4; i++) {
		uint16_t r = (uint16_t)(0xC651 + 8 * i);
		uint8_t f = mem_read(r + 3);
		if (mem_read(r + 1) == mem_read(0xC953) && mem_read(r + 2) == mem_read(0xC954) && (f == 0x8C || f == 0x8E))
			return 1;
	}
	return 0;
}

static void whirl_check(void) {        // $BBE7
	// shadow mode: a whirlpool or a twister on the party waits for frames
	if (!native_on_fiber() && !mem_read(0xC99F) && whirl_on_party()) { native_decline(); return; }
	cpu.a = mem_read(0xC99F); I(13);
	op_or(cpu.a); I(4);
	if (!(cpu.f & FZ)) { I(11); return; }
	I(5);
	cpu.iy = 0xC651; I(14);
	SET_HL(rd16(0xC953)); I(16);
	SET_DE(0x0008); I(10);
	cpu.b = 0x04; I(7);
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
				op_sub8(0x8C, 0, 0); I(7);
				if (cpu.f & FZ) { I(12); native_continue_at(0xBC1F); return; }   // a whirlpool
				I(7);
				op_sub8(0x8E, 0, 0); I(7);
				if (cpu.f & FZ) { I(12); native_continue_at(0xBC54); return; }   // a twister
				I(7);
			}
		}
		cpu.iy = op_add16(cpu.iy, DE); I(15);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	I(10);
}

static void wind_holds_back(void) {    // $B42E
	push16(BC); I(11);
	cpu.b = cpu.a; I(4);
	cpu.a = mem_read(0xC98A); I(13);
	int held;
	op_sub8(0x04, 0, 0); I(7);
	if (cpu.f & FZ) {                  // calm
		I(12);
		cpu.a = mem_read(0xC8FD); I(13);
		op_and(0x07); I(7);
		if (cpu.f & FZ) { I(12); held = 0; } else { I(7); I(12); held = 1; }
	} else {
		I(7);
		op_sub8(cpu.b, 0, 0); I(4);
		if (cpu.f & FZ) {              // the wind's direction
			I(12);
			cpu.a = mem_read(0xC8FD); I(13);
			op_and(0x03); I(7);
			if (cpu.f & FZ) { I(12); held = 0; } else { I(7); I(12); held = 1; }
		} else {
			I(7);
			op_add8(0x02, 0); I(7);
			op_and(0x03); I(7);
			op_sub8(cpu.b, 0, 0); I(4);
			if (cpu.f & FZ) {          // the opposite one
				I(12);
				cpu.a = mem_read(0xC8FD); I(13);
				op_and(0x03); I(7);
				if (cpu.f & FZ) { I(12); held = 1; } else { I(7); I(12); held = 0; }
			} else { I(7); I(12); held = 0; }
		}
	}
	if (held) { op_scf(); I(4); }
	else { op_or(cpu.a); I(4); I(12); }
	SET_BC(pop16()); I(10);
	I(10);
}

#define UPKEEP(name, addr, out) Native native_##name = { #name, addr, 2, name, out, .long_calls = 1 };
UPKEEP(in_balloon, 0x957C, OUT_A | OUT_F)
UPKEEP(hull_mend, 0xB55E, OUT_A | OUT_F | OUT_HL)
UPKEEP(afflictions, 0xB4EE, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL | OUT_IY)
UPKEEP(below_c8fd, 0xBBC6, OUT_A | OUT_F | OUT_HL)
UPKEEP(ship_wind, 0xB36C, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
UPKEEP(palette_dim, 0xA81A, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
UPKEEP(whirl_check, 0xBBE7, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL | OUT_IY)
UPKEEP(wind_holds_back, 0xB42E, OUT_A | OUT_F)
