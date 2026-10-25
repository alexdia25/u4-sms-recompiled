// The map window's frame and blank patterns, drawn when the map mode
// changes (bank 0, from refresh $129D).
// - window_strip ($1449): 16 bytes of patterns from bank $14 at HL to VRAM
//   at DE (far_vram_write, $07F4); then DE one tile column on ($220), HL
//   16 bytes on.
// - window_clear ($1431): the screen forgotten ($276D; $C602 = 0, $C601 =
//   1), and the window's 17 tile columns from VRAM $0230 given the blank
//   strip at $AFE8 (the last one with the 16 bytes after it).
// - window_border ($1461): window_clear, then the eight strips from $B008
//   at VRAM $0AB0 on, and $C0 bytes from $B088 at VRAM $2620.
// - window_reset ($141B): the wind shown again ($C98B = $FF), the text at
//   $1579 into the text area ($2D25), $C95D and $C95F = $FF, then
//   window_border.
// Cycle counts are the original's, instruction by instruction (I).
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static void push(uint16_t v) { push16(v); I(11); }
static void call(uint16_t addr) { I(17); call_original(addr); }

static void window_strip(void) {       // $1449
	push(BC); push(HL); push(DE);
	cpu.a = 0x14; I(7);
	SET_BC(0x0010); I(10);
	call(0x07F4);                      // far_vram_write
	SET_HL(pop16()); I(10);
	SET_BC(0x0220); I(10);
	SET_HL(op_add16(HL, BC)); I(11);
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
	SET_HL(pop16()); I(10);
	SET_BC(0x0010); I(10);
	SET_HL(op_add16(HL, BC)); I(11);
	SET_BC(pop16()); I(10);
	I(10);
}

static void window_clear(void) {       // $1431
	op_xor(cpu.a); I(4);
	mem_write(0xC602, cpu.a); I(13);
	cpu.a = op_inc8(cpu.a); I(4);
	mem_write(0xC601, cpu.a); I(13);
	call(0x276D);                      // forget_screen
	SET_DE(0x0230); I(10);
	cpu.b = 0x10; I(7);
	do {
		SET_HL(0xAFE8); I(10);
		I(17); window_strip();
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	window_strip();                    // falls into it
}

static void window_border(void) {      // $1461
	I(17); window_clear();
	SET_HL(0xB008); I(10);
	SET_DE(0x0AB0); I(10);
	cpu.b = 0x08; I(7);
	do {
		I(17); window_strip();
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	cpu.a = 0x14; I(7);
	SET_HL(0xB088); I(10);
	SET_DE(0x2620); I(10);
	SET_BC(0x00C0); I(10);
	I(10);
	native_continue_at(0x07F4);        // jp far_vram_write
}

static void window_reset(void) {       // $141B
	cpu.a = 0xFF; I(7);
	mem_write(0xC98B, cpu.a); I(13);
	SET_HL(0x1579); I(10);
	call(0x2D25);
	cpu.a = 0xFF; I(7);
	mem_write(0xC95D, cpu.a); I(13);
	mem_write(0xC95F, cpu.a); I(13);
	I(10);
	window_border();                   // jp $1461
}

#define WINDOW(name, addr, out) Native native_##name = { #name, addr, 0, name, out, .long_calls = 1 };
WINDOW(window_strip, 0x1449, OUT_A | OUT_F | OUT_DE | OUT_HL)
WINDOW(window_clear, 0x1431, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
WINDOW(window_border, 0x1461, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
WINDOW(window_reset, 0x141B, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
