// The view drawn into video memory ($1A7C): the 11 x 11 cells of the view
// buffer ($D900: tile, attribute per cell; under it at $D800 the ground)
// become tile patterns. A cell is 12 pixels wide and 12 high, so two cells
// make three 8-pixel tiles, the middle one shared: half from each cell. A
// cell whose tile is a figure (people, monsters: the flag table at $8000 in
// bank $10) is drawn over the ground under it through a mask. What is on
// the screen is remembered at $DA00 (4 bytes a cell) and only cells that
// change are drawn; a row is redrawn in full when its turn comes ($C601).
//
// The video memory addresses to write come from a list at $22FC (two bytes
// each, sent to the VDP's control port by outi), read in order through the
// main registers; the alternate registers (exx) point into the graphics.
// This is a close translation: the same reads and writes, VDP accesses and
// bank switches, at the original's cycle counts.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define VDP_DATA    0xBE
#define BUFFER      0xC1C0          // the shared middle tile, built here
#define PHASE       0xC603          // the animation frame for this row

// each instruction's cycles; then the machine catches up (native_boundary),
// or after ei and the instruction after it, takes an interrupt pending
static int after_ei;
static void cy(int n) {
	cpu.cycles += n;
	if (after_ei == 2) { after_ei = 1; native_boundary(); }
	else if (after_ei == 1) { after_ei = 0; native_irq_point(); }
	else native_boundary();
}

static void exx(void) {
	uint8_t t;
	t = cpu.b; cpu.b = cpu.b_; cpu.b_ = t;
	t = cpu.c; cpu.c = cpu.c_; cpu.c_ = t;
	t = cpu.d; cpu.d = cpu.d_; cpu.d_ = t;
	t = cpu.e; cpu.e = cpu.e_; cpu.e_ = t;
	t = cpu.h; cpu.h = cpu.h_; cpu.h_ = t;
	t = cpu.l; cpu.l = cpu.l_; cpu.l_ = t;
	cy(4);
}

static uint8_t ix_at(int o) { return mem_read((uint16_t)(cpu.ix + o)); }
static uint8_t iy_at(int o) { return mem_read((uint16_t)(cpu.iy + o)); }
static void iy_set(int o, uint8_t v) { mem_write((uint16_t)(cpu.iy + o), v); }

// outi: (C) <- (HL), HL++, B--
static void outi(void) {
	uint8_t v = mem_read(HL);
	cpu.b--;
	io_out(cpu.c, v);
	SET_HL(HL + 1);
	cy(16);
}

// di / outi / outi / ei: the next video address from the list
static void next_address(void) {
	cpu.iff1 = cpu.iff2 = 0;
	cy(4);
	outi();
	outi();
	cpu.iff1 = cpu.iff2 = 1;
	after_ei = 2;
	cy(4);
}

static void out_data(uint8_t a) {      // out ($BE),a
	io_out(VDP_DATA, a);
	cy(11);
}

static uint8_t in_data(void) {         // in a,($BE)
	uint8_t v = io_in(VDP_DATA);
	cy(11);
	return v;
}

// ------------------------------------------------------- graphic addresses --
// A cell's tile number (L) and attribute (H) -> the graphic's address and
// bank: terrain ($00-$7F attribute): 128 bytes each in banks 6/7 (or 8/9);
// figures by kind and animation frame in banks $0A-$0D; $FF: bank $0E.
// Four variants: HL through slot 2 ($FFFF) or DE through slot 1 ($FFFE),
// with their own banks and offsets.
typedef struct Variant {
	int slot1;                         // 0: HL and $FFFF; 1: DE and $FFFE
	uint8_t base, orv, figures, small, add_small, add_big, add_ff;
} Variant;

static const Variant V2098 = { 0, 0x06, 0x80, 0x0A, 0x0B, 0x88, 0x96, 0x80 };
static const Variant V2125 = { 1, 0x06, 0x40, 0x0A, 0x0B, 0x48, 0x56, 0x40 };
static const Variant V21B2 = { 0, 0x08, 0x80, 0x0C, 0x0D, 0x88, 0x96, 0xA0 };
static const Variant V223F = { 0, 0x08, 0x40, 0x0C, 0x0D, 0x48, 0x56, 0x60 };

static void graphic_address(const Variant *v, int slot1_reg) {
	uint16_t bank_reg = slot1_reg ? 0xFFFE : 0xFFFF;
	uint8_t *hi = v->slot1 ? &cpu.d : &cpu.h;
	uint8_t *lo = v->slot1 ? &cpu.e : &cpu.l;
	cy(17 + 4 + 7);                    // call / inc h / jr z
	if (*hi == 0xFF) {                 // $FF: bank $0E
		cy(5 + 7 + 13 + 4 + 4 + 7);
		mem_write(bank_reg, 0x0E);
		cpu.a = (uint8_t)(2 * *lo + v->add_ff);
		goto phase;
	}
	cy(4 + 10);                        // dec h / jp m
	if (!(*hi & 0x80)) {               // terrain: 128 bytes a graphic
		cy(4 + 4 + 7 + 7 + 13 + 4 + 7 + 8 + 8 + 7 + 7 + 4 + 10);
		uint8_t l = *lo;
		mem_write(bank_reg, (uint8_t)(v->base + (l >> 7)));
		*hi = (uint8_t)(((l >> 1) & 0x3F) | v->orv);
		*lo = (uint8_t)((l & 1) << 7);
		cpu.a = *hi;
		return;
	}
	cy(4 + 4 + 4 + 4 + 7 + 7);         // ld a,h / rlca x3 / and 3 / jr z
	uint8_t kind = (uint8_t)((*hi >> 5) & 3);
	if (kind == 0) {                   // the small figures: bank $0B ($0D)
		cy(5 + 7 + 13 + 4 + 7 + 7 + 7);
		mem_write(bank_reg, v->small);
		uint8_t a = *hi & 0x1F;
		if (a < 0x10) {
			cy(4 + 7 + 10);
			cpu.a = (uint8_t)(2 * a + v->add_small);
			goto phase;
		}
		cy(5 + 7 + 8 + 8 + 7 + 4 + 10);
		*lo = (uint8_t)((a & 1) << 7);
		*hi = (uint8_t)((a >> 1) + v->add_big);
		cpu.a = *hi;
		return;
	}
	cy(7 + 4 + 7);                     // ld b,0 / dec a / jr z
	uint8_t b = 0;
	if (kind == 1) cy(5);
	else {
		cy(7 + 4 + 7);
		b = 0x30;
		if (kind == 2) cy(5);
		else { cy(7); b = 0x60; }
	}
	cy(13 + 7 + 7);                    // the animation frame (3 shows as 1)
	uint8_t ph = mem_read(PHASE);
	if (ph == 3) { cy(7); ph = 1; } else cy(5);
	cy(4 * 4 + 4 + 4 + 4 + 7 + 4 + 7 + 8 + 8 + 7 + 7 + 7);
	uint8_t a = (uint8_t)(((uint8_t)(ph * 16 + b)) + (*hi & 0x0F));
	*lo = (uint8_t)((a & 1) << 7);
	a >>= 1;
	uint8_t bank = v->figures;
	if (a >= 0x40) { cy(7 + 4); a -= 0x40; bank++; } else cy(5);
	cy(7 + 4 + 4 + 13 + 10);
	*hi = (uint8_t)(a | v->orv);
	cpu.b = bank;
	cpu.a = bank;
	mem_write(bank_reg, bank);
	return;
phase:                                 // $2118: the frame picks the half
	cy(4 + 13 + 7 + 8 + 8 + 4 + 4 + 10);
	{
		uint8_t h = cpu.a, ph2 = mem_read(PHASE);
		*lo = (uint8_t)((ph2 & 1) << 7);
		*hi = (uint8_t)((ph2 >> 1) | h);
		cpu.a = *hi;
	}
}
static void gfx_2098(void) { graphic_address(&V2098, 0); }   // HL, slot 2
static void gfx_2125(void) { graphic_address(&V2125, 1); }   // DE, slot 1
static void gfx_21B2(void) { graphic_address(&V21B2, 0); }   // HL, slot 2, the shifted ones
static void gfx_223F(void) { graphic_address(&V223F, 1); }   // HL, slot 1, the shifted ones

// ------------------------------------------------------------- row loops --
// 12 rows: a mask byte at (HL) then 4 planes; out (DE & mask) | (HL)
static void masked_rows_out(int de_first) {
	cy(7);
	cpu.b = 0x0C;
	do {
		cy(7);
		cpu.c = mem_read(HL);
		if (de_first) { cy(4); cpu.e++; }
		cy(4); cpu.l++;
		for (int i = 0; i < 4; i++) {
			cy(7 + 4 + 7);
			cpu.a = (uint8_t)((mem_read(DE) & cpu.c) | mem_read(HL));
			out_data(cpu.a);
			cy(4 + 4);
			cpu.e++; cpu.l++;
		}
		cy(--cpu.b ? 13 : 8);
	} while (cpu.b);
}

// 12 rows of 4 bytes from (HL), skipping each row's mask byte, to the VDP
static void plain_rows_out(void) {
	cy(10);
	cpu.b = 0x30; cpu.c = VDP_DATA;
	do {
		cy(4); cpu.l++;
		for (int i = 0; i < 4; i++) {
			outi();
			if (i < 3) cy(12);         // nop x3
		}
		cy(10);                        // jp nz
	} while (cpu.b);
}

// $1BB3: two cells: the left tile, the shared middle one, the right tile
static void ret_pair(void) { cy(10 + 11 + 10); SET_DE(8); SET_HL(HL + 8); }   // $1C05

static void right_cell(void);
static void right_simple_redraw(void);
static void right_composite_from_left_unchanged(void);
static void right_simple_left_unchanged(void);

static void draw_pair(void) {
	cy(17 + 10 + 15 + 10 + 15 + 7 + 13 + 7 + 19 + 7 + 4 + 7);
	cpu.ix += 4; cpu.iy += 8;
	mem_write(0xFFFF, 0x10);
	cpu.d = 0x80;
	cpu.e = ix_at(-4);
	cpu.a = mem_read(DE);
	if (cpu.a) {                       // $1C0A: a figure on the ground
		cy(5);
		exx();
		cy(19 + 19 + 8 + 19 + 19 + 8);
		cpu.l = ix_at(-4); cpu.h = ix_at(-3);
		cpu.e = ix_at(-0x104); cpu.d = ix_at(-0x103);
		int same = 0;
		cy(4 + 19 + 7);
		if (cpu.e == iy_at(-8)) {
			cy(4 + 19 + 7);
			if (cpu.d == iy_at(-7)) {
				cy(4 + 19 + 7);
				if (cpu.l == iy_at(-6)) {
					cy(4 + 19 + 7);
					if (cpu.h == iy_at(-5)) same = 1;
				}
			}
		}
		if (same) {
			exx();
			cy(10);
			goto right_check;
		}
		cy(5);
		exx();                         // $1C37
		next_address();
		cy(10);
		SET_DE(BUFFER);
		exx();
		cy(19 * 4);
		iy_set(-8, cpu.e); iy_set(-7, cpu.d); iy_set(-6, cpu.l); iy_set(-5, cpu.h);
		gfx_2098();
		gfx_2125();
		masked_rows_out(1);            // the left tile
		cy(7);                         // the left half of the middle tile, into the buffer
		cpu.b = 0x0C;
		do {
			cy(7 + 4 + 4);
			cpu.c = mem_read(HL);
			cpu.e++; cpu.l++;
			for (int i = 0; i < 4; i++) {
				cy(7 + 4 + 7);
				cpu.a = (uint8_t)((mem_read(DE) & cpu.c) | mem_read(HL));
				exx();
				cy(7 + 4);
				mem_write(DE, cpu.a);
				cpu.e++;
				exx();
				cy(4 + 4);
				cpu.e++; cpu.l++;
			}
			cy(--cpu.b ? 13 : 8);
		} while (cpu.b);
		cy(10);
		right_cell();
		return;
	}
	cy(4 + 19 + 10);
	if (cpu.e != iy_at(-8)) goto left_redraw;
	cy(19 + 19 + 10);
	if (ix_at(-3) != iy_at(-7)) goto left_redraw;
	cy(19 + 19 + 10);
	if (iy_at(-6) | iy_at(-5)) goto left_redraw;
right_check:                           // $1BE4: the left cell is as it was
	cy(19 + 7 + 4 + 10);
	cpu.e = ix_at(-2);
	cpu.a = mem_read(DE);
	if (cpu.a) { right_composite_from_left_unchanged(); return; }
	cy(4 + 19 + 10);
	if (cpu.e != iy_at(-4)) { right_simple_left_unchanged(); return; }
	cy(19 + 19 + 10);
	if (ix_at(-1) != iy_at(-3)) { right_simple_left_unchanged(); return; }
	cy(19 + 19 + 10);
	if (iy_at(-2) | iy_at(-1)) { right_simple_left_unchanged(); return; }
	ret_pair();
	return;
left_redraw:                           // $1CA5: a plain cell
	next_address();
	exx();
	cy(19 + 19 + 19 + 19 + 19 + 19);
	cpu.l = ix_at(-4); cpu.h = ix_at(-3);
	iy_set(-8, cpu.l); iy_set(-7, cpu.h); iy_set(-6, 0); iy_set(-5, 0);
	gfx_2098();
	plain_rows_out();                  // the left tile
	cy(10 + 10);                       // the left half of the middle tile, into the buffer
	SET_DE(BUFFER);
	SET_BC(0x0030);
	do {
		cy(4); cpu.l++;
		for (int i = 0; i < 4; i++) {
			cy(16);
			mem_write(DE, mem_read(HL));
			SET_HL(HL + 1); SET_DE(DE + 1); SET_BC(BC - 1);
		}
		cy(10);
	} while (BC);
	right_cell();
}

// $1CED: the right cell, after the left one was drawn (alternate registers)
static void right_unchanged_middle(void) {   // $1D14: only the middle tile again
	exx();
	next_address();
	exx();
	cy(10 + 10);
	SET_HL(BUFFER);
	cpu.b = 0x0C; cpu.c = 0x0F;
	do {
		for (int i = 0; i < 4; i++) {
			cpu.a = (uint8_t)((in_data() & cpu.c) | mem_read(HL));
			cy(4 + 7 + 7 + 4);
			mem_write(HL, cpu.a);
			cpu.l++;
		}
		cy(--cpu.b ? 13 : 8);
	} while (cpu.b);
	exx();
	next_address();
	cy(6 + 6);
	SET_HL(HL + 2);
	exx();
	cy(10 + 10);
	SET_HL(BUFFER);
	cpu.b = 0x30; cpu.c = VDP_DATA;
	do {
		outi();
		cy(7);
		if (cpu.b) cy(5);
	} while (cpu.b);
	exx();
	cy(10);
}

static void right_fg(void);

static void right_cell(void) {
	cy(7 + 13 + 7 + 19 + 7 + 4 + 7);
	mem_write(0xFFFF, 0x10);
	cpu.d = 0x80;
	cpu.e = ix_at(-2);
	cpu.a = mem_read(DE);
	if (cpu.a) {                       // $1D52: a figure
		cy(5 + 8 + 19 + 19 + 8);
		cpu.l = ix_at(-0x102); cpu.h = ix_at(-0x101);
		int same = 0;
		cy(4 + 19 + 7);
		if (cpu.l == iy_at(-4)) {
			cy(4 + 19 + 7);
			if (cpu.h == iy_at(-3)) {
				cy(4 + 19 + 7);
				if (cpu.e == iy_at(-2)) {
					cy(19 + 19 + 10);
					if (ix_at(-1) == iy_at(-1)) same = 1;
				} else cy(5);
			} else cy(5);
		} else cy(5);
		if (same) { right_unchanged_middle(); return; }
		cy(19 + 19);                   // $1D77
		iy_set(-4, cpu.l); iy_set(-3, cpu.h);
		gfx_21B2();
		cy(10 + 7);
		SET_DE(BUFFER);
		cpu.b = 0x0C;
		do {
			cy(4); cpu.l++;
			for (int i = 0; i < 4; i++) {
				cy(7 + 7 + 7 + 4 + 4);
				mem_write(DE, (uint8_t)(mem_read(DE) | mem_read(HL)));
				cpu.e++; cpu.l++;
			}
			cy(--cpu.b ? 13 : 8);
		} while (cpu.b);
		cy(11);
		push16(HL);
		exx();
		cy(6 + 6);
		SET_HL(HL + 2);
		next_address();
		exx();
		cy(10);
		right_fg();
		return;
	}
	cy(4 + 19 + 10);
	if (cpu.e != iy_at(-4)) { right_simple_redraw(); return; }
	cy(19 + 19 + 10);
	if (ix_at(-1) != iy_at(-3)) { right_simple_redraw(); return; }
	cy(19 + 19 + 10);
	if (iy_at(-2) | iy_at(-1)) { right_simple_redraw(); return; }
	right_unchanged_middle();
}

static void right_simple_redraw(void) {   // $1DAA
	cy(19 * 6);
	cpu.l = ix_at(-2); cpu.h = ix_at(-1);
	iy_set(-4, cpu.l); iy_set(-3, cpu.h); iy_set(-2, 0); iy_set(-1, 0);
	gfx_21B2();
	exx();
	cy(6 + 6);
	SET_HL(HL + 2);
	next_address();
	exx();
	cy(10 + 7);                        // the middle tile: the buffer | the right cell's left part
	SET_DE(BUFFER);
	cpu.b = 0x0C;
	do {
		cy(4); cpu.l++;
		for (int i = 0; i < 4; i++) {
			cy(7 + 7);
			cpu.a = (uint8_t)(mem_read(DE) | mem_read(HL));
			out_data(cpu.a);
			cy(4 + 4);
			cpu.e++; cpu.l++;
		}
		cy(--cpu.b ? 13 : 8);
	} while (cpu.b);
	exx();
	next_address();
	exx();
	plain_rows_out();                  // the right tile
	exx();
	cy(10);
}

// $1E0D: left as it was, the right cell a figure (main registers on entry)
static void right_composite_from_left_unchanged(void) {
	exx();
	cy(8 + 19 + 19 + 8);
	cpu.l = ix_at(-0x102); cpu.h = ix_at(-0x101);
	int same = 0;
	cy(4 + 19 + 7);
	if (cpu.l == iy_at(-4)) {
		cy(4 + 19 + 7);
		if (cpu.h == iy_at(-3)) {
			cy(19 + 19 + 7);
			if (ix_at(-2) == iy_at(-2)) {
				cy(19 + 19 + 7);
				if (ix_at(-1) == iy_at(-1)) same = 1;
				else cy(5);
			} else cy(5);
		} else cy(5);
	} else cy(5);
	if (same) { exx(); cy(10); ret_pair(); return; }
	exx();                             // $1E38
	cy(6 + 6);
	SET_HL(HL + 2);
	next_address();
	exx();
	cy(19 + 19);
	iy_set(-4, cpu.l); iy_set(-3, cpu.h);
	gfx_21B2();
	cy(10 + 10);                       // the buffer: the screen's middle tile's left half | the ground's
	SET_DE(BUFFER);
	cpu.b = 0x0C; cpu.c = 0xF0;
	do {
		cy(4); cpu.l++;
		for (int i = 0; i < 4; i++) {
			cpu.a = (uint8_t)((in_data() & cpu.c) | mem_read(HL));
			cy(4 + 7 + 7 + 4 + 4);
			mem_write(DE, cpu.a);
			cpu.e++; cpu.l++;
		}
		cy(--cpu.b ? 13 : 8);
	} while (cpu.b);
	cy(11);
	push16(HL);
	exx();
	next_address();
	exx();
	right_fg();
}

// $1E79: the right cell's figure, over the buffer (middle) and the ground (right)
static void right_fg(void) {
	cy(19 * 4);
	cpu.l = ix_at(-2); cpu.h = ix_at(-1);
	iy_set(-2, cpu.l); iy_set(-1, cpu.h);
	gfx_223F();
	cy(10);
	SET_DE(BUFFER);
	masked_rows_out(0);                // the middle tile
	cy(10);
	SET_DE(pop16());                   // the ground's graphic
	exx();
	next_address();
	exx();
	masked_rows_out(1);                // the right tile
	exx();
	cy(10);
}

// $1EDB: left as it was, the right cell plain (main registers on entry)
static void right_simple_left_unchanged(void) {
	cy(6 + 6);
	SET_HL(HL + 2);
	next_address();
	exx();
	cy(19 * 6);
	cpu.l = ix_at(-2); cpu.h = ix_at(-1);
	iy_set(-4, cpu.l); iy_set(-3, cpu.h); iy_set(-2, 0); iy_set(-1, 0);
	gfx_21B2();
	cy(10 + 10);
	SET_DE(BUFFER);
	cpu.b = 0x0C; cpu.c = 0xF0;
	do {
		cy(4); cpu.l++;
		for (int i = 0; i < 4; i++) {
			cpu.a = (uint8_t)((in_data() & cpu.c) | mem_read(HL));
			cy(4 + 7 + 7 + 4 + 4);
			mem_write(DE, cpu.a);
			cpu.l++; cpu.e++;
		}
		cy(--cpu.b ? 13 : 8);
	} while (cpu.b);
	exx();
	next_address();
	exx();
	cy(4);                             // ex de,hl
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); }
	cy(10 + 10);
	SET_HL(BUFFER);
	cpu.b = 0x30; cpu.c = VDP_DATA;
	do {
		outi();
		cy(7);
		if (cpu.b) cy(5);
	} while (cpu.b);
	cy(4);
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); }
	exx();
	next_address();
	exx();
	plain_rows_out();                  // the right tile
	exx();
	cy(10);
}

// $1F56: the 11th cell of a row: its tile and the half tile after it (the
// rest filled from $22CC)
static void draw_last(void) {
	cy(17 + 10 + 15 + 10 + 15 + 7 + 13 + 7 + 19 + 7 + 4 + 7);
	cpu.ix += 2; cpu.iy += 4;
	mem_write(0xFFFF, 0x10);
	cpu.d = 0x80;
	cpu.e = ix_at(-2);
	cpu.a = mem_read(DE);
	if (cpu.a) {                       // $1F8C: a figure
		cy(5);
		exx();
		cy(19 + 19 + 8 + 19 + 19 + 8);
		cpu.l = ix_at(-2); cpu.h = ix_at(-1);
		cpu.e = ix_at(-0x102); cpu.d = ix_at(-0x101);
		int same = 0;
		cy(4 + 19 + 7);
		if (cpu.e == iy_at(-4)) {
			cy(4 + 19 + 7);
			if (cpu.d == iy_at(-3)) {
				cy(4 + 19 + 7);
				if (cpu.l == iy_at(-2)) {
					cy(4 + 19 + 7);
					if (cpu.h == iy_at(-1)) same = 1;
				}
			}
		}
		if (same) { exx(); cy(10 + 10 + 11 + 10); SET_DE(4); SET_HL(HL + 4); return; }
		cy(5);
		exx();                         // $1FB9
		next_address();
		exx();
		cy(19 * 4);
		iy_set(-4, cpu.e); iy_set(-3, cpu.d); iy_set(-2, cpu.l); iy_set(-1, cpu.h);
		gfx_2098();
		gfx_2125();
		masked_rows_out(1);
		exx();
		next_address();
		cy(4 + 10);
		{ uint16_t t = HL; SET_HL(DE); SET_DE(t); }
		SET_HL(0x22CC);
		exx();
		cy(7);
		cpu.b = 0x0C;
		do {
			cy(7 + 4 + 4);
			cpu.c = mem_read(HL);
			cpu.e++; cpu.l++;
			for (int i = 0; i < 4; i++) {
				cy(7 + 4 + 7);
				cpu.a = (uint8_t)((mem_read(DE) & cpu.c) | mem_read(HL));
				exx();
				cy(7 + 6);
				cpu.a |= mem_read(HL);
				SET_HL(HL + 1);
				exx();
				out_data(cpu.a);
				cy(4 + 4);
				cpu.e++; cpu.l++;
			}
			cy(--cpu.b ? 13 : 8);
		} while (cpu.b);
		exx();
		cy(4 + 10);
		{ uint16_t t = HL; SET_HL(DE); SET_DE(t); }
		return;
	}
	cy(4 + 19 + 10);
	int redraw = cpu.e != iy_at(-4);
	if (!redraw) {
		cy(19 + 19 + 10);
		redraw = ix_at(-1) != iy_at(-3);
		if (!redraw) {
			cy(19 + 19 + 10);
			redraw = (iy_at(-2) | iy_at(-1)) != 0;
		}
	}
	if (!redraw) { cy(10 + 11 + 10); SET_DE(4); SET_HL(HL + 4); return; }
	next_address();                    // $2038
	exx();
	cy(19 * 6);
	cpu.l = ix_at(-2); cpu.h = ix_at(-1);
	iy_set(-4, cpu.l); iy_set(-3, cpu.h); iy_set(-2, 0); iy_set(-1, 0);
	gfx_2098();
	plain_rows_out();
	exx();
	next_address();
	exx();
	cy(10 + 7);
	SET_DE(0x22CC);
	cpu.b = 0x0C;
	do {
		cy(4); cpu.l++;
		for (int i = 0; i < 4; i++) {
			cy(7 + 7);
			cpu.a = (uint8_t)(mem_read(DE) | mem_read(HL));
			out_data(cpu.a);
			cy(4 + 6);
			cpu.l++;
			SET_DE(DE + 1);
		}
		cy(--cpu.b ? 13 : 8);
	} while (cpu.b);
	exx();
	cy(10);
}

// $1AAB: the 11 rows
static void draw_rows(void) {
	cy(17 + 16 + 11 + 14 + 14 + 10 + 7 + 4);
	uint16_t banks = rd16(0xDFFE);
	cpu.ix = 0xD900; cpu.iy = 0xDA00;
	SET_HL(0x22FC);
	cpu.c = 0xBF;
	for (uint8_t row = 0; row < 11; row++) {
		cy(11 + 20 + 4 + 4 + 7 + 7 + 13);
		uint8_t first = mem_read(0xC601), turn = mem_read(0xC602);
		mem_write(PHASE, (uint8_t)((turn + (row < first ? 1 : 0)) & 3));
		for (int i = 0; i < 5; i++) draw_pair();
		draw_last();
		cy(10 + 4 + 7 + 10);
	}
	cy(10 + 16 + 10);
	mem_write(0xFFFE, (uint8_t)banks);
	mem_write(0xFFFF, (uint8_t)(banks >> 8));
	cpu.a = 0x0B;
}

// $1AE9: the next row's turn to be redrawn in full
static void next_full_row(void) {
	cy(17 + 10 + 11 + 7 + 7);
	uint8_t n = op_inc8(mem_read(0xC601));
	mem_write(0xC601, n);
	SET_HL(0xC601);
	cpu.a = n;
	op_sub8(0x0C, 0, 0);
	if (n < 0x0C) cy(11);
	else {
		cy(5 + 10 + 10 + 11 + 10);
		mem_write(0xC601, 1);
		SET_HL(0xC602);
		mem_write(0xC602, op_inc8(mem_read(0xC602)));
	}
}

// $1AF8 (by jp from the vector $7E45): the 11 rows, keeping the registers
static void redraw_view(void) {
	uint16_t bc = BC, de = DE, hl = HL, ix = cpu.ix, iy = cpu.iy;
	cy(11 + 11 + 11 + 15 + 15);
	draw_rows();
	cy(14 + 14 + 10 + 10 + 10 + 10);
	SET_BC(bc); SET_DE(de); SET_HL(hl); cpu.ix = ix; cpu.iy = iy;
}

// $1B3B (by jp from the vector $7E3F): the same for a view 19 cells wide
// and 5 high, with its own list of VRAM addresses ($24E0)
static void compose_wide(void) {
	cy(16 + 11 + 15 + 15 + 13 + 4 + 7);
	uint16_t banks = rd16(0xDFFE), ix = cpu.ix, iy = cpu.iy;
	uint8_t r = (uint8_t)(mem_read(0xC601) - 1);
	if (r < 0x05) {                    // this row is redrawn in full: forget it
		cy(7 + 4 * 9 + 7 + 4 + 4 + 7 + 4 + 4 + 4 + 4 + 4 + 10 + 10);
		uint8_t a = (uint8_t)(r * 38 + 6);   // 76 bytes a row, from the 4th cell
		uint16_t at = (uint16_t)(0xDA00 + a * 2);
		for (int i = 0; i < 56; i++) mem_write((uint16_t)(at + i), 0xFF);
		cy(21 * 54 + 16);
		SET_HL(at + 55); SET_DE(at + 56); SET_BC(0);
	} else cy(12);
	cy(14 + 14 + 10 + 7 + 4);
	cpu.ix = 0xD900; cpu.iy = 0xDA00;
	SET_HL(0x24E0);
	cpu.c = 0xBF;
	for (uint8_t row = 0; row < 5; row++) {
		cy(11 + 20 + 4 + 4 + 7 + 7 + 13);
		uint8_t first = mem_read(0xC601), turn = mem_read(0xC602);
		mem_write(PHASE, (uint8_t)((turn + (row < first ? 1 : 0)) & 3));
		for (int i = 0; i < 9; i++) draw_pair();
		draw_last();
		cy(10 + 4 + 7 + 10);
	}
	next_full_row();
	cy(14 + 14 + 10 + 16 + 10);
	cpu.ix = ix; cpu.iy = iy;
	SET_HL(banks);
	mem_write(0xFFFE, (uint8_t)banks);
	mem_write(0xFFFF, (uint8_t)(banks >> 8));
}

// $1A7C
static void compose_screen(void) {
	cy(15 + 15 + 13 + 4 + 7 + 7);
	uint16_t ix = cpu.ix, iy = cpu.iy;
	uint8_t r = (uint8_t)(mem_read(0xC601) - 1);
	if (r < 0x0B) {                    // this row is redrawn in full: forget it
		cy(4 * 9 + 4 + 7 + 4 + 4 + 4 + 4 + 4 + 10 + 10);
		uint16_t at = (uint16_t)(0xDA00 + r * 44);
		for (int i = 0; i < 44; i++) mem_write(at + i, 0xFF);
		cy(21 * 42 + 16);
		SET_HL(at + 44); SET_DE(at + 44); SET_BC(0);
	} else cy(5);
	draw_rows();
	next_full_row();
	cy(14 + 14 + 10);
	cpu.ix = ix; cpu.iy = iy;
}
Native native_compose_screen = { "compose_screen", 0x1A7C, 0, compose_screen, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, 0, 0, .long_calls = 1 };
Native native_redraw_view = { "redraw_view", 0x1AF8, 0, redraw_view, OUT_A | OUT_F, 0, 0, .long_calls = 1 };
Native native_compose_wide = { "compose_wide", 0x1B3B, 0, compose_wide, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, 0, 0, .long_calls = 1 };
