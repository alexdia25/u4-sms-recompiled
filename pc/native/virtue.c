// Virtues and karma, rewritten from bank 1.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define KARMA 0xC90B   // 8 bytes: Honesty, Compassion, Valor, Justice, Sacrifice, Honor, Spirituality, Humility

// karma_gain ($4255): C = virtue (0-7), A = amount. Karma counts up to 99.
// A karma of 0 means the party already gained that eighth of Avatarhood;
// it stays 0. All registers are preserved.
static void nat_karma_gain(void) {
	uint16_t addr = KARMA + cpu.c;
	uint8_t karma = mem_read(addr);
	if (karma != 0) {
		uint8_t sum = karma + cpu.a;   // 8-bit, as in the original
		mem_write(addr, sum < 99 ? sum : 99);
		cpu.cycles += sum < 99 ? 153 : 155;   // the original's cycles
	} else {
		cpu.cycles += 128;
	}
}
Native native_karma_gain = { "karma_gain", 0x4255, 1, nat_karma_gain, 0, 0, 0 };

// karma_lose ($426F): virtue C loses A karma, down to 1 at least; a
// karma of 0 (that eighth gained) goes back to 99, with a sound and the
// news printed ($429B), after the corner glyph is redrawn. All registers
// are preserved. Cycle counts instruction by instruction (I).
static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }
static void push(uint16_t v) { push16(v); I(11); }

static void karma_lose(void) {
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: printing may wait for frames
	push((uint16_t)(cpu.a << 8 | cpu.f)); push(BC); push(HL);
	SET_HL(KARMA); I(10);
	cpu.b = 0x00; I(7);
	SET_HL(op_add16(HL, BC)); I(11);
	cpu.b = cpu.a; I(4);
	cpu.a = mem_read(HL); I(7);
	op_or(cpu.a); I(4);
	if (cpu.f & FZ) {
		I(7);
		mem_write(HL, 0x63); I(10);
		op_add8(0x1B, 0); I(7);
		I(17); call_original(0x05F1);  // the sound
		I(17); call_original(0x342B);  // status_glyph
		SET_HL(0x429B); I(10);
		I(17); call_original(0x5519);  // print_text_hl
		I(12);
	} else {
		I(12);
		op_sub8(cpu.b, 0, 1); I(4);
		if (cpu.f & FZ) { I(12); cpu.a = 0x01; I(7); }
		else {
			I(7);
			if (cpu.f & FC) { I(7); cpu.a = 0x01; I(7); }
			else I(12);
		}
		mem_write(HL, cpu.a); I(7);
	}
	SET_HL(pop16()); I(10);
	SET_BC(pop16()); I(10);
	uint16_t af = pop16(); cpu.a = af >> 8; cpu.f = (uint8_t)af; I(10);
	I(10);
}
Native native_karma_lose = { "karma_lose", 0x426F, 1, karma_lose, 0, .long_calls = 1 };
