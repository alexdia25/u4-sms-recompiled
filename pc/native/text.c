// The text window, rewritten from bank 1 ($5460-$5551).
//
// The text area is a grid of character cells in the VDP name table at $3800
// (2 bytes per cell: tile = character + $40, attribute 1) with a RAM copy at
// $CA00 (row * 64 + column * 2). The cursor is at column $C639 (1-31) and row
// $C63A (up to 23); the bottom rows scroll up when it runs past the end.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define CURSOR_COL   0xC639
#define CURSOR_ROW   0xC63A
#define TEXT_SHADOW  0xCA00
#define NAME_TABLE   0x3800

static void vdp_set_write_address(uint16_t addr) {
	io_out(0xBF, (uint8_t)addr);
	io_out(0xBF, (uint8_t)(0x40 | (addr >> 8)));
}

// put_glyph ($5460): draw character A at the cursor (without moving it).
static void put_glyph(uint8_t ch) {
	uint8_t col = mem_read(CURSOR_COL), row = mem_read(CURSOR_ROW);
	uint16_t cell = (uint16_t)(row * 64 + col * 2);
	vdp_set_write_address(NAME_TABLE + cell);
	mem_write(TEXT_SHADOW + cell, (uint8_t)(ch + 0x40));
	io_out(0xBE, (uint8_t)(ch + 0x40));
	mem_write(TEXT_SHADOW + cell + 1, 0x01);
	io_out(0xBE, 0x01);
	cpu.a = 0x01;
	cpu.cycles += 228;
}

static void nat_put_glyph(void) { put_glyph(cpu.a); }
Native native_put_glyph = { "put_glyph", 0x5460, 1, nat_put_glyph, OUT_A, 0, 0 };

// put back the registers saved in k (as the original's pushes and pops do),
// leaving the rest of the CPU state - interrupt lines, cycles - as it is now
static void restore_registers(const Z80 *k) {
	cpu.a = k->a; cpu.f = k->f; cpu.b = k->b; cpu.c = k->c;
	cpu.d = k->d; cpu.e = k->e; cpu.h = k->h; cpu.l = k->l;
	cpu.ix = k->ix; cpu.iy = k->iy;
}

// ----------------------------------------------------------------------------
// The routines below add the original's cycle counts (T-states, from entry to
// its ret) so the rest of the game sees the same timing.

// print_newline ($54CE): cursor to column 1 of the next row. Past the last
// row the bottom of the window scrolls: rows 20-23 move up a row, row 23 is
// blanked (tile $160) except its first and last cells, and rows 19-23 are
// copied to the VDP after the next vertical blank. Keeps all registers. Waits
// for a frame when it scrolls, so it is a blocking native.
static void newline(void) {
	Z80 keep = cpu;
	mem_write(CURSOR_COL, 1);
	uint8_t row = mem_read(CURSOR_ROW) + 1;
	mem_write(CURSOR_ROW, row);
	if (row < 0x18) {
		cpu.cycles += 157;
	} else {
		mem_write(CURSOR_ROW, 0x17);
		cpu.cycles += 102 + 10 + 17;              // up to the call (the pops and ret come last)
		wait_vblank();
		for (int i = 0; i < 0x100; i++) mem_write(0xCEC0 + i, mem_read(0xCF00 + i));
		for (int i = 0xCFC2; i < 0xCFFE; i += 2) { mem_write(i, 0x60); mem_write(i + 1, 0x01); }
		vdp_set_write_address(NAME_TABLE + 19 * 64);
		for (int i = 0; i < 0x140; i++) io_out(0xBE, mem_read(0xCEC0 + i));
		cpu.cycles += 30 + 5371 + 56 + 1213 + 30 + 17 + 76 + 59 + 8988 + 50;
	}
	restore_registers(&keep);
}

// print_char ($548F): $01 = new line, $02 = back one cell (blanking it),
// space = advance (unless at the right edge); anything else wraps at column
// 31 and is drawn, then the cursor moves right.
static void print_char(uint8_t ch) {
	uint8_t col = mem_read(CURSOR_COL);
	if (ch == 0x01) { cpu.cycles += 17; newline(); return; }
	if (ch == 0x02) {
		if (col == 0) { cpu.a = 0; cpu.cycles += 64; return; }
		mem_write(CURSOR_COL, col - 1);
		put_glyph(0x20);
		cpu.cycles += 92;
		return;
	}
	if (ch == 0x20) {
		if (col >= 0x1F) { cpu.a = col; cpu.cycles += 81; return; }
		put_glyph(0x20);
		cpu.cycles += 391 - 228;
	} else {
		if (col >= 0x1F) { cpu.cycles += 7; newline(); }
		put_glyph(ch);
		cpu.cycles += 393 - 228;
	}
	mem_write(CURSOR_COL, mem_read(CURSOR_COL) + 1);
}

// text_page_wait ($5552): every fifth line, stop until button 1 or 2 is
// pressed so the text doesn't scroll away unread, keeping the cursor and the
// screen updated meanwhile.
#define JOY_PRESSED 0xC0B0
#define BUTTONS     0x30

static void wait_for_reader(void) {
	Z80 keep = cpu;
	cpu.cycles += 74;                       // pushes
	for (;;) {
		cpu.cycles += 17; wait_vblank();
		cpu.cycles += 17; wait_vblank();
		cpu.cycles += 17; call_original(0x0950);   // clear the sprite list
		cpu.cycles += 17; call_original(0x065B);   // update_joypad
		cpu.cycles += 13 + 7;
		if (mem_read(JOY_PRESSED) & BUTTONS) { cpu.cycles += 12; break; }
		cpu.cycles += 7 + 17; call_original(0x335C);   // text cursor
		cpu.cycles += 12;
	}
	cpu.cycles += 68 + 10;                  // pops, ret
	restore_registers(&keep);
}

// one more line printed; *lines is the count kept in C by the original
static void page_line(uint8_t *lines) {
	if (++*lines < 5) { cpu.cycles += 26; return; }
	*lines = 0;
	cpu.cycles += 27;
	wait_for_reader();
}

static void nat_text_page_wait(void) {     // register version: C counts the lines
	uint8_t lines = cpu.c;
	cpu.a = (uint8_t)(lines + 1);
	op_sub8(5, 0, 0);                      // cp 5 (the caller may look at the flags)
	page_line(&lines);
	cpu.c = lines;
}
Native native_text_page_wait = { "text_page_wait", 0x5552, 1, nat_text_page_wait, OUT_A | OUT_BC, 0, 0, .blocking = 1 };

// print_text ($5519): print the zero-terminated text at addr with word wrap:
// before each word, if it doesn't fit on the line (and the cursor isn't at
// the start of one), start a new line. $01 is a line break; every fifth line
// waits for a button. Returns the address after the terminator.
static uint16_t print_text(uint16_t addr) {
	uint8_t lines = 0;
	cpu.cycles += 21;                      // push bc, ld bc,$0100
	for (;;) {
		// length of the next word, counting the space/$01/$00 after it
		int len = 0;
		cpu.cycles += 4 + 7 + 11;
		for (uint16_t p = addr;; p++) {
			uint8_t c = mem_read(p);
			len++;
			if (c == 0x20) { cpu.cycles += 36; break; }
			if (c == 0x01) { cpu.cycles += 47; break; }
			if (c == 0x00) { cpu.cycles += 53; break; }
			cpu.cycles += 58;
		}
		uint8_t col = mem_read(CURSOR_COL);
		cpu.cycles += 10 + 13 + 7;
		if (col == 1) cpu.cycles += 12;
		else if ((uint8_t)(col + len) < 0x21) cpu.cycles += 7 + 4 + 7 + 12;
		else {
			cpu.cycles += 7 + 4 + 7 + 7 + 17 + 17;
			page_line(&lines);
			newline();
		}
		for (int first = 1; len > 0; len--, first = 0) {
			if (!first) cpu.cycles += 4 + 12;  // dec b, jr nz (more of the word)
			uint8_t c = mem_read(addr);
			cpu.cycles += 7 + 4;
			if (c == 0x01) { cpu.cycles += 17; page_line(&lines); }
			else cpu.cycles += 10;
			addr++;
			cpu.cycles += 7 + 6 + 4;
			if (c == 0x00) { cpu.cycles += 12 + 10 + 10; return addr; }
			cpu.cycles += 7 + 17;
			print_char(c);
			cpu.cycles += 12;
		}
	}
}

uint16_t text_print(uint16_t text) {      // $5519, with its registers
	uint8_t b = cpu.b, c = cpu.c;
	SET_HL(print_text(text));
	cpu.b = b; cpu.c = c;
	cpu.a = 0;
	cpu.f = (cpu.f & ~(FS | FC | FN | FH)) | FZ;   // or a: Z set
	return HL;
}

void text_newline(void) { newline(); }
void text_char(uint8_t ch) { print_char(ch); }

static void nat_print_text_hl(void) { text_print(HL); }   // $5519: HL = text
Native native_print_text_hl = { "print_text_hl", 0x5519, 1, nat_print_text_hl, OUT_A | OUT_HL, 0, 0, .blocking = 1 };

static void nat_print_inline(void) {       // $5510: the text follows the call
	uint16_t text = pop16();
	uint8_t b = cpu.b, c = cpu.c;
	cpu.cycles += 19 + 17;
	uint16_t after = print_text(text);
	cpu.cycles += 19 + 10;
	cpu.b = b; cpu.c = c;
	cpu.a = 0;
	native_continue_at(after);             // HL is kept
}
Native native_print_inline = { "print_inline", 0x5510, 1, nat_print_inline, OUT_A, 0, 0, .blocking = 1 };

static void nat_print_char(void) { print_char(cpu.a); }   // $548F
Native native_print_char = { "print_char", 0x548F, 1, nat_print_char, 0, 0, 0, .blocking = 1 };

static void nat_print_newline(void) { newline(); }   // $54CE
Native native_print_newline = { "print_newline", 0x54CE, 1, nat_print_newline, 0, 0, 0, .blocking = 1 };
