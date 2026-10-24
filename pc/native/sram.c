// The cartridge's battery RAM (the saved games), paged into $8000-$BFFF
// while bit 3 of the mapper's control byte ($FFFC) is set.
// - sram_on ($0C2D) / sram_off ($0C35): page it in / out.
// - sram_read ($0C3D): A = (HL); sram_write ($0C43): (HL) = A;
//   sram_copy ($0C49): ldir, all with the RAM paged in for the moment.
// - sram_check ($0C50, at power-on): the 39-byte mark at $8010 must match
//   the one at $0C7F; if not, the RAM from $8010 is cleared and the mark
//   written.
// Cycle counts are the original's, instruction by instruction (I).
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static void paging(int on) {           // push hl / ld hl,$FFFC / set|res 3,(hl) / pop hl / ret
	push16(HL); I(11);
	SET_HL(0xFFFC); I(10);
	uint8_t v = mem_read(HL);
	mem_write(HL, on ? (uint8_t)(v | 0x08) : (uint8_t)(v & ~0x08)); I(15);
	SET_HL(pop16()); I(10);
	I(10);
}
static void sram_on(void) { paging(1); }
static void sram_off(void) { paging(0); }

static void ldir(void) {
	while (op_block(6, 0)) I(21);
	I(16);
}

static void sram_read(void) { I(17); sram_on(); cpu.a = mem_read(HL); I(7); I(12); sram_off(); }
static void sram_write(void) { I(17); sram_on(); mem_write(HL, cpu.a); I(7); I(12); sram_off(); }
static void sram_copy(void) { I(17); sram_on(); ldir(); I(12); sram_off(); }

static void sram_check(void) {
	I(17); sram_on();
	SET_DE(0x8010); I(10);
	SET_HL(0x0C7F); I(10);
	cpu.b = 0x27; I(7);
	int same = 1;
	do {
		cpu.a = mem_read(DE); I(7);
		op_sub8(mem_read(HL), 0, 0); I(7);
		if (!(cpu.f & FZ)) { I(12); same = 0; break; }
		I(7);
		SET_DE(DE + 1); I(6);
		SET_HL(HL + 1); I(6);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	if (!same) {                       // $0C65: cleared, and the mark written
		SET_HL(0x8010); I(10);
		SET_DE(0x8011); I(10);
		SET_BC(0x1FEF); I(10);
		mem_write(HL, 0); I(10);
		ldir();
		SET_HL(0x0C7F); I(10);
		SET_DE(0x8010); I(10);
		SET_BC(0x0027); I(10);
		ldir();
	}
	I(12);
	sram_off();
}

#define SRAM(name, addr, out) Native native_##name = { #name, addr, 0, name, out, .long_calls = 1 };
SRAM(sram_on, 0x0C2D, 0)
SRAM(sram_off, 0x0C35, 0)
SRAM(sram_read, 0x0C3D, OUT_A)
SRAM(sram_write, 0x0C43, 0)
SRAM(sram_copy, 0x0C49, OUT_F | OUT_BC | OUT_DE | OUT_HL)
SRAM(sram_check, 0x0C50, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
