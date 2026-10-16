// Virtues and karma, rewritten from bank 1.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"

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
