// Arithmetic and random numbers, rewritten from bank 0.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"

#define RNG_SEED      0xC0B6   // 16-bit
#define INPUT_NOISE   0xC0B5   // update_joypad adds every joypad reading here

// random ($2BBF): seed = seed * 3, with the old low byte added to the high
// byte; the result is the new high byte plus the joypad noise byte, so player
// input stirs the sequence. Returns A (flags as set by that final add).
static uint8_t random_byte(void) {
	uint16_t seed = rd16(RNG_SEED);
	uint16_t next = (uint16_t)(seed * 3);
	next = (uint16_t)(next + ((seed & 0xFF) << 8));
	wr16(RNG_SEED, next);
	cpu.a = next >> 8;
	op_add8(mem_read(INPUT_NOISE), 0);   // a += noise, setting the flags like the original
	return cpu.a;
}

static void nat_random(void) {
	random_byte();
	cpu.cycles += 143;
}
Native native_random = { "random", 0x2BBF, 0, nat_random, OUT_A | OUT_F, 0, 0 };

// multiply_8x8 ($2AA1): HL = H * L, by shift and add over H's 8 bits.
// Returns the original's cycle count, which depends on the bits it adds.
int multiply_cycles(uint8_t h, uint8_t l) {
	uint32_t hl = (uint32_t)h << 8;
	int cycles = 46;
	for (int i = 0; i < 8; i++) {
		hl <<= 1;
		if (hl & 0x10000) { hl = (hl & 0xFFFF) + l; cycles += 29; }
		else cycles += 23;
		hl &= 0xFFFF;
	}
	return cycles;
}

static void nat_multiply_8x8(void) {
	cpu.cycles += multiply_cycles(cpu.h, cpu.l);
	SET_HL(cpu.h * cpu.l);
}
Native native_multiply_8x8 = { "multiply_8x8", 0x2AA1, 0, nat_multiply_8x8, OUT_HL, 0, 0 };

// random_below ($2BEB): A = a random number from 0 to A-1 (0 when A is 0),
// as (random * A) / 256.
static void nat_random_below(void) {
	uint8_t n = cpu.a;
	uint8_t r = random_byte();
	cpu.cycles += 77 + 143 + multiply_cycles(r, n);
	cpu.a = (uint8_t)((r * n) >> 8);
}
Native native_random_below = { "random_below", 0x2BEB, 0, nat_random_below, OUT_A, 0, 0 };
