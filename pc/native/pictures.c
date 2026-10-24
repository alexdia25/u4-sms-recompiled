// Pictures drawn into the map window's patterns (bank 1). A picture (in
// bank A at HL): the address of its tiles ($C610, 32 bytes a tile), then
// its width in pixels (B) and height in tiles (C), then a tile number for
// each 8 pixels across, row by row. Tile rows are $220 bytes apart in VRAM.
// - picture ($5247): drawn at VRAM DE; a last part-tile is cut to width.
// - picture_reveal ($5290): the same faded in over 32 frames: each frame
//   draws it through the next 8 bytes of masks at $533E ($C612), one mask
//   a pixel row; first the frame's figures (set_up_reveal, $5311).
// - screen_pixel ($8B16, bank 3): one pixel of whatever tile is on the
//   screen there (the title's lettering).
// The C goes instruction by instruction (I). Cycle counts are the
// original's.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static void push(uint16_t v) { push16(v); I(11); }
static void ex_de_hl(void) { uint16_t t = HL; SET_HL(DE); SET_DE(t); I(4); }
static void out_be(uint8_t v) { io_out((uint16_t)(cpu.a << 8 | 0xBE), v); I(11); }
static void call(uint16_t addr) { I(17); call_original(addr); }

static void header(void) {             // $5331: $C610 = the tiles, BC = the size
	cpu.c = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	cpu.b = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	wr16(0xC610, BC); I(20);
	cpu.c = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	cpu.b = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	I(10);
}

static void tile_address(void) {       // HL = $C610 + (HL) * 32 (DE as given)
	cpu.l = mem_read(HL); I(7);
	cpu.h = 0; I(7);
	for (int i = 0; i < 5; i++) { SET_HL(op_add16(HL, HL)); I(11); }
}

static void next_tile_row(void) {      // pop de / DE += $220 / pop bc / dec c
	SET_DE(pop16()); I(10);
	ex_de_hl();
	SET_BC(0x0220); I(10);
	SET_HL(op_add16(HL, BC)); I(11);
	ex_de_hl();
	SET_BC(pop16()); I(10);
	cpu.c = op_dec8(cpu.c); I(4);
}

static void picture(void) {            // $5247
	vram_bank_push(0x5247);
	push(BC); push(DE); push(HL);
	I(17); header();
	do {                               // $5250: a row of tiles
		push(BC); push(DE);
		vram_set_write_address();
		SET_DE(rd16(0xC610)); I(20);
		cpu.c = 0xBE; I(7);
		for (;;) {                     // $5259
			push(BC); push(HL);
			tile_address();
			SET_HL(op_add16(HL, DE)); I(11);
			cpu.a = cpu.b; I(4);
			op_sub8(0x08, 0, 0); I(7);
			if (!(cpu.f & FC)) { I(7); cpu.b = 0x20; I(7); I(12); }
			else {
				I(12);
				op_add8(cpu.a, 0); I(4);
				op_add8(cpu.a, 0); I(4);
				cpu.b = cpu.a; I(4);
			}
			for (;;) {                 // outi / jr nz
				op_block(4, 3); I(16);
				if (cpu.f & FZ) { I(7); break; }
				I(12);
			}
			SET_HL(pop16()); I(10);
			SET_BC(pop16()); I(10);
			SET_HL(HL + 1); I(6);
			cpu.a = cpu.b; I(4);
			op_sub8(0x08, 0, 1); I(7);
			cpu.b = cpu.a; I(4);
			if (cpu.f & FZ) { I(12); break; }
			I(7);
			if (cpu.f & FC) { I(7); break; }
			I(12);
		}
		next_tile_row();
		if (cpu.f & FZ) { I(7); break; }
		I(12);
	} while (1);
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	vram_bank_pop();
}

static void sprites_clear(void) {      // $0950: the sprite tables emptied
	SET_HL(0xC180); I(10);
	SET_DE(0xC181); I(10);
	SET_BC(0x003F); I(10);
	mem_write(HL, 0xC0); I(10);
	while (op_block(6, 0)) I(21);
	I(16);
	op_xor(cpu.a); I(4);
	mem_write(0xC064, cpu.a); I(13);
	I(10);
}

static void set_up_reveal(void) {      // $5311
	push(BC); push(DE); push(HL);
	cpu.a = 0x05; I(7);
	SET_HL(0xB8E4); I(10);
	SET_DE(0x3E80); I(10);
	call(0x0788);                      // far_vram_rle_slow: the figures' patterns
	I(17); sprites_clear();
	cpu.a = 0x05; I(7);
	SET_HL(0xB8F0); I(10);
	SET_DE(0x0888); I(10);
	call(0x09BE);                      // far_put_figure_at
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

static void picture_reveal(void) {     // $5290
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: it waits for frames
	vram_bank_push(0x5290);
	push(BC); push(DE); push(HL);
	I(17); set_up_reveal();
	SET_BC(0x533E); I(10);
	wr16(0xC612, BC); I(20);
	I(17); header();
	cpu.a = 0x20; I(7);
	do {                               // $52A5: a frame
		push((uint16_t)(cpu.a << 8 | cpu.f)); push(BC); push(DE); push(HL);
		call(0x041C);                  // wait_vblank
		do {                           // $52AC: a row of tiles
			push(BC); push(DE);
			vram_set_write_address();
			for (;;) {                 // $52AF
				push(BC); push(HL);
				tile_address();
				SET_DE(rd16(0xC610)); I(20);
				SET_HL(op_add16(HL, DE)); I(11);
				cpu.a = cpu.b; I(4);
				op_sub8(0x08, 0, 0); I(7);
				if (cpu.f & FC) I(12);
				else { I(7); cpu.b = 0x08; I(7); }
				SET_DE(rd16(0xC612)); I(20);
				do {                   // $52C9: a pixel row, through its mask
					cpu.a = mem_read(DE); I(7);
					cpu.c = cpu.a; I(4);
					for (int p = 0; p < 4; p++) {
						if (p) { cpu.a = cpu.c; I(4); }
						op_and(mem_read(HL)); I(7);
						out_be(cpu.a);
						SET_HL(HL + 1); I(6);
					}
					SET_DE(DE + 1); I(6);
					if (--cpu.b) I(13); else I(8);
				} while (cpu.b);
				SET_HL(pop16()); I(10);
				SET_BC(pop16()); I(10);
				SET_HL(HL + 1); I(6);
				cpu.a = cpu.b; I(4);
				op_sub8(0x08, 0, 1); I(7);
				cpu.b = cpu.a; I(4);
				if (cpu.f & FZ) { I(12); break; }
				I(7);
				I(10);                 // jp nc
				if (cpu.f & FC) break;
			}
			next_tile_row();
			I(10);                     // jp nz
		} while (!(cpu.f & FZ));
		SET_HL(rd16(0xC612)); I(16);
		SET_DE(0x0008); I(10);
		SET_HL(op_add16(HL, DE)); I(11);
		wr16(0xC612, HL); I(16);
		SET_HL(pop16()); I(10);
		SET_DE(pop16()); I(10);
		SET_BC(pop16()); I(10);
		{ uint16_t af = pop16(); cpu.a = af >> 8; cpu.f = (uint8_t)af; } I(10);
		cpu.a = op_dec8(cpu.a); I(4);
		I(10);
	} while (!(cpu.f & FZ));
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	vram_bank_pop();
}

static void read_address(void) {       // rst $08: HL as the VDP's read address
	I(11); I(10);
	cpu.iff1 = cpu.iff2 = 0; I(4);
	cpu.a = cpu.l; I(4);
	io_out((uint16_t)(cpu.a << 8 | 0xBF), cpu.a); I(11);
	cpu.a = cpu.h; I(4);
	io_out((uint16_t)(cpu.a << 8 | 0xBF), cpu.a); I(11);
	cpu.iff1 = cpu.iff2 = 1;
	cpu.cycles += 4 + 10;
	native_irq_point();
}

// screen_pixel ($8B16, bank 3): the pixel at screen (L, H) set to colour A,
// in whichever tile the name table's copy ($CA00) shows there
static void screen_pixel(void) {
	push(BC); push(DE); push(HL);
	cpu.e = cpu.a; I(4);
	cpu.a = cpu.l; I(4);
	op_and(0x07); I(7);
	cpu.a = op_inc8(cpu.a); I(4);
	cpu.b = cpu.a; I(4);
	cpu.a = 0x01; I(7);
	do {
		op_rrca(); I(4);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	cpu.d = cpu.a; I(4);               // the pixel's bit
	cpu.a = cpu.l; I(4);
	op_and(0xF8); I(7);
	op_rrca(); I(4);
	op_rrca(); I(4);
	cpu.a = op_inc8(cpu.a); I(4);
	cpu.c = cpu.a; I(4);
	cpu.b = 0xCA; I(7);
	cpu.a = cpu.h; I(4);
	op_ex_af(); I(4);
	cpu.a = cpu.h; I(4);
	op_and(0xF8); I(7);
	cpu.l = cpu.a; I(4);
	cpu.h = 0; I(7);
	for (int i = 0; i < 3; i++) { SET_HL(op_add16(HL, HL)); I(11); }
	SET_HL(op_add16(HL, BC)); I(11);
	cpu.a = mem_read(HL); I(7);        // the cell's attribute: bit 0, the tile's 9th bit
	cpu.l = op_dec8(cpu.l); I(4);
	cpu.l = mem_read(HL); I(7);
	op_and(0x01); I(7);
	cpu.h = cpu.a; I(4);
	for (int i = 0; i < 3; i++) { SET_HL(op_add16(HL, HL)); I(11); }
	op_ex_af(); I(4);
	op_and(0x07); I(7);
	cpu.c = cpu.a; I(4);
	cpu.b = 0; I(7);
	SET_HL(op_add16(HL, BC)); I(11);
	SET_HL(op_add16(HL, HL)); I(11);
	SET_HL(op_add16(HL, HL)); I(11);
	push(HL);
	read_address();
	SET_HL(0xC1C0); I(10);
	SET_BC(0x04BE); I(10);
	for (;;) {
		op_block(4, 2); I(16);
		if (cpu.f & FZ) { I(7); break; }
		I(12);
	}
	SET_HL(pop16()); I(10);
	ex_de_hl();
	vram_set_write_address();
	SET_DE(0xC1C0); I(10);
	cpu.b = 0x04; I(7);
	do {
		cpu.a = mem_read(DE); I(7);
		cpu.e = op_inc8(cpu.e); I(4);
		op_or(cpu.h); I(4);
		cpu.l = op_rot(3, cpu.l); I(8);
		if (cpu.f & FC) I(12);
		else { I(7); op_xor(cpu.h); I(4); }
		out_be(cpu.a);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

#define PICTURE(name, addr, bank, out) Native native_##name = { #name, addr, bank, name, out, .long_calls = 1 };
PICTURE(picture, 0x5247, 1, OUT_A | OUT_F)
PICTURE(picture_reveal, 0x5290, 1, OUT_A | OUT_F)
PICTURE(set_up_reveal, 0x5311, 1, OUT_A | OUT_F)
PICTURE(header, 0x5331, 1, OUT_BC | OUT_HL)
PICTURE(sprites_clear, 0x0950, 0, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
PICTURE(screen_pixel, 0x8B16, 3, OUT_A | OUT_F)
