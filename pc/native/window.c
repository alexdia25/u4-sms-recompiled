// The text window's building blocks, rewritten from bank 0 ($0499, $07EC,
// $0807, $2D0C-$2E7F, $340F): the RAM copy of the screen's text area at
// $CA00 (row * 64 + column * 2, two bytes a cell: tile = character + $40,
// attribute), menu boxes drawn into it, and the queue that copies changed
// parts to the VDP during the next vertical blank ($C200, 8 bytes a request,
// $C063 requests; the interrupt handler empties it).
//
// Each routine leaves the registers and flags as the original does and adds
// its cycle count, so the shadow test can check them call by call
// (U4_SHADOW_CYCLES=1 also checks the time).
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define TEXT_SHADOW   0xCA00
#define VRAM_QUEUE    0xC200
#define VRAM_QUEUED   0xC063
#define MENU_WINDOW   0xCA64          // the menu window's top-left cell
#define BANK_SCRATCH  0xC0C2

static const uint16_t bank_switch_scratch[] = { BANK_SCRATCH, BANK_SCRATCH + 1 };

// ------------------------------------------------------------- queue --
// queue_text_copy ($0499): copy B rows of C cells from the text shadow at DE
// to the same place in the VDP's name table, next vertical blank. When the
// queue is full the original waits a frame; the native leaves that case to it.
static void queue_text_copy(void) {
	uint8_t n = mem_read(VRAM_QUEUED);
	uint16_t at = (uint16_t)(VRAM_QUEUE + n * 8);
	mem_write(VRAM_QUEUED, n + 1);
	mem_write(at + 0, 0x80);
	mem_write(at + 1, cpu.e);
	mem_write(at + 2, (uint8_t)(cpu.d + 0xAE));   // VRAM $3800 + offset, as a write address
	mem_write(at + 3, cpu.e);
	mem_write(at + 4, cpu.d);
	uint8_t c2 = (uint8_t)(cpu.c << 1);            // sla c: 2 bytes a cell
	cpu.f = z80_szp[c2] | ((cpu.c & 0x80) ? FC : 0);
	cpu.c = c2;
	mem_write(at + 5, cpu.c);
	mem_write(at + 6, cpu.b);
	cpu.a = (uint8_t)(cpu.d + 0xAE);
	op_inc8((uint8_t)(at + 5));                    // the flags of the last inc l
	SET_HL(at + 6);
	cpu.cycles += 179;
}

static void nat_queue_text_copy(void) {
	if (mem_read(VRAM_QUEUED) >= 0x20) { native_decline(); return; }
	queue_text_copy();
}
Native native_queue_text_copy = { "queue_text_copy", 0x0499, 0, nat_queue_text_copy,
	OUT_A | OUT_F | OUT_BC | OUT_HL, 0, 0 };

// ------------------------------------------------------------- cells --
// cell_address ($2D0C): DE = the shadow address of column E, row D (A = D)
static void cell_address(void) {
	uint8_t a = cpu.e, c = 0;
	for (int i = 0; i < 3; i++) { c = a >> 7; a = (uint8_t)(a << 1); }   // add a,a x3
	uint8_t e = a;
	a = cpu.d;
	uint8_t c1 = a & 1; a = (uint8_t)((a >> 1) | (c << 7));               // rra
	uint8_t c2 = e & 1; e = (uint8_t)((e >> 1) | (c1 << 7));              // rr e
	uint8_t c3 = a & 1; a = (uint8_t)((a >> 1) | (c2 << 7));              // rra
	e = (uint8_t)((e >> 1) | (c3 << 7));                                  // rr e
	cpu.a = a;
	op_add8(0xCA, 0);
	cpu.d = cpu.a;
	cpu.e = e;
	cpu.cycles += 17 + 69;
}

// menu_text ($2D69): write the zero-terminated text at HL into the shadow at
// column E, row D. HL ends after the terminator, DE after the last cell.
static void menu_text(void) {
	cell_address();
	uint16_t hl = HL;
	for (;;) {
		uint8_t ch = mem_read(hl++);
		if (!ch) break;
		mem_write(DE, (uint8_t)(ch + 0x40));
		cpu.e++;
		mem_write(DE, 0x01);
		cpu.e++;
		cpu.cycles += 68;
	}
	SET_HL(hl);
	cpu.a = 0;
	cpu.f = (cpu.f & FC) | z80_szp[0];   // or a
	cpu.cycles += 28;
}
static void nat_menu_text(void) { menu_text(); }
Native native_menu_text = { "menu_text", 0x2D69, 0, nat_menu_text, OUT_A | OUT_DE | OUT_HL, 0, 0 };

// menu_text_at ($2D65): the same with the position in the two bytes at HL
static void nat_menu_text_at(void) {
	cpu.e = mem_read(HL);
	cpu.d = mem_read(HL + 1);
	SET_HL(HL + 2);
	cpu.cycles += 26;
	menu_text();
}
Native native_menu_text_at = { "menu_text_at", 0x2D65, 0, nat_menu_text_at, OUT_A | OUT_DE | OUT_HL, 0, 0 };

// highlight_off ($2DC4) / highlight_on ($2DE3): clear / set attribute bit 3
// (the other palette: the menu cursor bar) over B rows of C cells at column
// E, row D, and queue them for the VDP. Callers check there is room in the
// queue and that B and C aren't 0 (the original would loop 256 times).
static void highlight(int on) {
	uint16_t hl = HL;
	uint8_t b = cpu.b, c = cpu.c;
	cell_address();
	uint16_t cell = DE;
	cpu.cycles += 33 + 4 + 10;
	uint16_t row = cell;
	for (int r = 0; r < b; r++) {
		uint16_t p = row;
		for (int i = 0; i < c; i++) {
			p = (uint16_t)((p & 0xFF00) | (uint8_t)(p + 1));   // inc l
			uint8_t v = mem_read(p);
			mem_write(p, on ? (v | 0x08) : (v & ~0x08));
			p = (uint16_t)((p & 0xFF00) | (uint8_t)(p + 1));
		}
		row += 0x40;
		cpu.cycles += 65 + 36 * c;
	}
	cpu.cycles -= 5;
	cpu.cycles += 10 + 10 + 17;
	SET_DE(cell);
	cpu.b = b; cpu.c = c;
	queue_text_copy();
	cpu.cycles += 10 + 10;
	SET_HL(hl);
}
static int queue_room(int n) { return mem_read(VRAM_QUEUED) + n <= 0x20; }
static void nat_highlight_off(void) {
	if (!cpu.b || !cpu.c || !queue_room(1)) { native_decline(); return; }
	highlight(0);
}
static void nat_highlight_on(void) {
	if (!cpu.b || !cpu.c || !queue_room(1)) { native_decline(); return; }
	highlight(1);
}
Native native_highlight_off = { "highlight_off", 0x2DC4, 0, nat_highlight_off, OUT_A | OUT_F | OUT_BC | OUT_DE, 0, 0 };
Native native_highlight_on = { "highlight_on", 0x2DE3, 0, nat_highlight_on, OUT_A | OUT_F | OUT_BC | OUT_DE, 0, 0 };

// --------------------------------------------------------- menu cursor --
// cursor_on ($49D5) / cursor_off ($49E0): the bar over menu line A (screen
// row A + 1, columns $12-$1F: $49EB), keeping BC and DE
static void menu_cursor(uint8_t line, int on) {
	uint16_t bc = BC, de = DE;
	cpu.cycles += 11 + 11 + 17 + 38 + 17;
	cpu.d = (uint8_t)(line + 1);
	cpu.e = 0x12;
	SET_BC(0x010E);
	cpu.a = cpu.d;
	highlight(on);
	cpu.cycles += 10 + 10 + 10;
	SET_BC(bc); SET_DE(de);
}
static void nat_cursor_on(void) {
	if (!queue_room(1)) { native_decline(); return; }
	menu_cursor(cpu.a, 1);
}
static void nat_cursor_off(void) {
	if (!queue_room(1)) { native_decline(); return; }
	menu_cursor(cpu.a, 0);
}
Native native_cursor_on = { "cursor_on", 0x49D5, 1, nat_cursor_on, OUT_A | OUT_F, 0, 0 };
Native native_cursor_off = { "cursor_off", 0x49E0, 1, nat_cursor_off, OUT_A | OUT_F, 0, 0 };

// menu_step ($49F4, stopping at the ends / $49F9, wrapping around): one
// frame of a menu of C lines; the current one is (ix+$1A), on screen at line
// (ix+$1A) * E + D. Up and down move the bar (with a click); button 1 returns
// carry set (cancel), button 2 Z clear (chosen), otherwise Z set.
#define MENU_LINE (cpu.ix + 0x1A)

static uint8_t menu_line_on_screen(void) {    // $4A5C
	uint8_t h = mem_read(MENU_LINE), e = cpu.e;
	cpu.cycles += 17 + 19 + 4 + 17 + multiply_cycles(h, e) + 4 + 4 + 10;
	uint16_t hl = (uint16_t)(h * e);
	SET_HL(hl);
	cpu.a = (uint8_t)(cpu.l + cpu.d);
	return cpu.a;
}

static int popcount8(uint8_t v) { int n = 0; for (; v; v >>= 1) n += v & 1; return n; }

static void menu_move(int8_t step) {          // $4A25 / $4A45, then $4A2D
	menu_line_on_screen();
	cpu.cycles += 17;
	menu_cursor(cpu.a, 0);
	cpu.cycles += 7 + 19 + 4 + 4 + 4 + 17;
	uint8_t l = (uint8_t)((uint8_t)step + mem_read(MENU_LINE) + cpu.c);
	uint8_t q = l / cpu.c, r = l % cpu.c;
	cpu.cycles += 242 + 3 * popcount8(q);     // divide_8
	cpu.h = r; cpu.l = q;
	cpu.a = r;
	cpu.cycles += 19;
	mem_write(MENU_LINE, r);
	cpu.cycles += 17;
	call_original(0x40D9);                    // click
	cpu.cycles += 12;
}

static void menu_step(int wrap) {
	int rows_room = queue_room(2);
	if (!rows_room || cpu.c == 0) { native_decline(); return; }
	cpu.cycles += wrap ? 10 : 10 + 12;
	uint8_t key = mem_read(0xC0B0) & 0x7F;
	cpu.cycles += 13 + 7 + 17;
	// switch_on_a over the key table: which entry matches
	static const uint8_t keys4[6] = { 0x01, 0x02, 0x01, 0x02, 0x10, 0x20 };
	static const uint8_t keys9[4] = { 0x01, 0x02, 0x10, 0x20 };
	const uint8_t *keys = wrap ? keys9 : keys4;
	int n = wrap ? 4 : 6, i;
	for (i = 0; i < n && keys[i] != key; i++) {}
	cpu.cycles += i < n ? 58 * i + 79 : 58 * n + 22;
	cpu.b = i < n ? (uint8_t)(keys[i] + 1) : 0;   // as switch_on_a leaves it
	cpu.a = key;
	int what = i == n ? 0 : key;          // 0: nothing pressed
	switch (what) {
	case 0x01: {                          // up
		if (!wrap) {
			cpu.cycles += 19 + 4 + 7;
			if (mem_read(MENU_LINE) == 0) { cpu.cycles += 5; break; }
		}
		menu_move(-1);
		break;
	}
	case 0x02: {                          // down
		if (!wrap) {
			cpu.cycles += 19 + 4 + 4 + 7;
			if ((uint8_t)(mem_read(MENU_LINE) + 1) >= cpu.c) { cpu.cycles += 5; break; }
		}
		menu_line_on_screen();
		cpu.cycles += 17;
		menu_cursor(cpu.a, 0);
		cpu.cycles += 7 + 12;
		// then as up from $4A2D with step 1
		cpu.cycles += 19 + 4 + 4 + 4 + 17;
		uint8_t l = (uint8_t)(1 + mem_read(MENU_LINE) + cpu.c);
		uint8_t q = l / cpu.c, r = l % cpu.c;
		cpu.cycles += 242 + 3 * popcount8(q);
		cpu.h = r; cpu.l = q; cpu.a = r;
		cpu.cycles += 19;
		mem_write(MENU_LINE, r);
		cpu.cycles += 17;
		call_original(0x40D9);
		cpu.cycles += 12;
		break;
	}
	case 0x10:                            // cancel
		cpu.cycles += 17;
		call_original(0x40D1);
		cpu.cycles += 7 + 4 + 10;
		cpu.a = 0xFF;
		op_add8(0xFF, 0);
		return;
	case 0x20:                            // chosen
		cpu.cycles += 17;
		call_original(0x40C9);
		cpu.cycles += 4 + 4 + 10;
		op_xor(cpu.a);
		cpu.a = op_dec8(cpu.a);
		return;
	}
	menu_line_on_screen();                // $4A04: the bar where it is now
	cpu.cycles += 17;
	menu_cursor(cpu.a, 1);
	cpu.cycles += 4 + 10;
	op_xor(cpu.a);
}
static void nat_menu_step(void) { menu_step(0); }
static void nat_menu_step_wrap(void) { menu_step(1); }
Native native_menu_step = { "menu_step", 0x49F4, 1, nat_menu_step, OUT_A | OUT_F, 0, 0 };
Native native_menu_step_wrap = { "menu_step_wrap", 0x49F9, 1, nat_menu_step_wrap, OUT_A | OUT_F, 0, 0 };

// ------------------------------------------------------- menu window --
// menu_window_clear ($2E1B): the empty menu window (16 rows of the 28-byte
// pattern at $2E38) into the shadow
static void menu_window_clear(void) {
	for (int r = 0; r < 16; r++)
		for (int i = 0; i < 0x1C; i++)
			mem_write(MENU_WINDOW + r * 0x40 + i, mem_read(0x2E38 + i));
	cpu.a = 0;
	cpu.f = (cpu.f & FC) | FZ | FN;        // the last dec a
	cpu.cycles += 10453;
}
static void nat_menu_window_clear(void) { menu_window_clear(); }
Native native_menu_window_clear = { "menu_window_clear", 0x2E1B, 0, nat_menu_window_clear, OUT_A, 0, 0 };

// menu_window_show ($2E54): copy the menu window from the shadow to the VDP
// now (16 rows of 28 bytes at name table $3864)
static void nat_menu_window_show(void) {
	uint16_t de = 0x3864, hl = MENU_WINDOW;
	for (int r = 0; r < 16; r++) {
		io_out(0xBF, (uint8_t)de);
		io_out(0xBF, (uint8_t)((de >> 8) | 0x40));
		for (int i = 0; i < 0x1C; i++) io_out(0xBE, mem_read(hl + i));
		hl += 0x40;
		de += 0x40;
	}
	cpu.a = (uint8_t)((de - 0x40) >> 8 | 0x40);
	cpu.cycles += 15234;
}
Native native_menu_window_show = { "menu_window_show", 0x2E54, 0, nat_menu_window_show, 0, 0, 0 };

// ----------------------------------------------------------- names --
// item_name ($340F): copy the 25-byte name of item A (bank 16, table at $8300)
// to $C065; HL = $C065
static void nat_item_name(void) {
	uint8_t saved = mem_read(0xDFFF);
	mem_write(0xFFFF, 0x10);
	uint16_t name = rd16(0x8300 + 2 * cpu.a);
	for (int i = 0; i < 0x19; i++) mem_write(0xC065 + i, mem_read(name + i));
	mem_write(0xFFFF, saved);
	wr16(BANK_SCRATCH, 0xC065 + 0x19);
	SET_HL(0xC065);
	cpu.a = 0x10;
	cpu.cycles += 1129;
}
Native native_item_name = { "item_name", 0x340F, 0, nat_item_name, OUT_A | OUT_HL,
	bank_switch_scratch, 2 };

// menu_draw ($2E7F): a fresh menu window with the labels at HL (each:
// column, row, text; the list ends with $FF), shown at once
static void menu_draw(void) {
	cpu.cycles += 17;
	menu_window_clear();
	cpu.cycles += 17;                      // call $2D5D
	for (;;) {
		uint8_t a = mem_read(HL);
		cpu.cycles += 7 + 4;
		if (a == 0xFF) { cpu.a = 0; cpu.f = (cpu.f & FC) | FZ | FH; cpu.cycles += 11; break; }
		cpu.cycles += 5 + 17;
		cpu.e = mem_read(HL);
		cpu.d = mem_read(HL + 1);
		SET_HL(HL + 2);
		cpu.cycles += 26;
		menu_text();
		cpu.cycles += 12;
	}
	cpu.cycles += 10;
	nat_menu_window_show();
}
static void nat_menu_draw(void) { menu_draw(); }
Native native_menu_draw = { "menu_draw", 0x2E7F, 0, nat_menu_draw, OUT_A | OUT_DE | OUT_HL, 0, 0 };

// yes_no_start ($4B5E): the YES / NO menu, answer (ix+7) = 0 (yes)
static void nat_yes_no_start(void) {
	cpu.cycles += 10 + 17;
	SET_HL(0x4BBF);
	menu_draw();
	cpu.cycles += 19 + 10;
	mem_write(cpu.ix + 0x07, 0);
}
Native native_yes_no_start = { "yes_no_start", 0x4B5E, 1, nat_yes_no_start, OUT_A | OUT_DE | OUT_HL, 0, 0 };

// two_choices ($AB4B: BUY / SELL; $AB4E: the labels at HL): first line chosen
static void two_choices(void) {
	cpu.cycles += 19 + 10;
	mem_write(cpu.ix + 0x1A, 0);
	menu_draw();
}
static void nat_buy_or_sell(void) { cpu.cycles += 10; SET_HL(0xAB77); two_choices(); }
static void nat_two_choices(void) { two_choices(); }
Native native_buy_or_sell = { "buy_or_sell", 0xAB4B, 2, nat_buy_or_sell, OUT_A | OUT_DE | OUT_HL, 0, 0 };
Native native_two_choices = { "two_choices", 0xAB4E, 2, nat_two_choices, OUT_A | OUT_DE | OUT_HL, 0, 0 };
