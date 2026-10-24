// The gem's view of the map (bank 1, $4F8A): the 64 x 32 cells of the
// map buffer ($D000, two bytes a cell) drawn small into the map window's
// patterns, three pixels a cell (each tile's 16 rows of little pictures at
// $AFE4 in bank 5, 16 bytes a tile; one cell's pixels in bits 7-2, the
// next's split between this byte and the next). In a dungeon ($C99F = 2,
// $5041) the level around the party instead, only what it can see.
// - clear_window ($2740): the window's patterns cleared, its right edge
//   from $22CC.
// - fill ($333B): B bytes of C from HL.
// Cycle counts are the original's, instruction by instruction (I).
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static void push(uint16_t v) { push16(v); I(11); }
static void exx(void) { op_exx(); I(4); }
static void ex_de_hl(void) { uint16_t t = HL; SET_HL(DE); SET_DE(t); I(4); }
static void call(uint16_t addr) { I(17); call_original(addr); }

static void clear_window(void) {       // $2740
	SET_HL(0x0220); I(10);
	SET_DE(0x0020); I(10);
	cpu.b = 0x10; I(7);
	do {
		push(BC);
		SET_BC(0x0210); I(10);
		op_xor(cpu.a); I(4);
		call(0x0219);                  // vram_fill_bytes
		ex_de_hl();
		SET_HL(op_add16(HL, DE)); I(11);
		ex_de_hl();
		SET_BC(pop16()); I(10);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	cpu.b = 0x0B; I(7);
	do {
		push(BC); push(DE);
		SET_HL(0x22CC); I(10);
		SET_BC(0x0030); I(10);
		call(0x024C);                  // vram_write
		SET_HL(pop16()); I(10);
		SET_BC(0x0030); I(10);
		SET_HL(op_add16(HL, BC)); I(11);
		ex_de_hl();
		SET_BC(pop16()); I(10);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	I(10);
}

static void fill(void) {               // $333B
	do {
		mem_write(HL, cpu.c); I(7);
		SET_HL(HL + 1); I(6);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	I(10);
}

static int dungeon;                    // which pictures: the map's or the dungeon's

// the next cell's little picture: DE = $AFE4 + tile * 16 (exx: the map);
// in a dungeon $B7E4 + the low byte of tile * 16, one byte a cell
static void next_picture(void) {
	if (dungeon) {
		exx();
		cpu.a = mem_read(HL); I(7);
		SET_HL(HL + 1); I(6);
		exx();
		for (int i = 0; i < 4; i++) { op_add8(cpu.a, 0); I(4); }
		op_add8(0xE4, 0); I(7);
		cpu.e = cpu.a; I(4);
		op_add8(0xB7, cpu.f & FC); I(7);
		op_sub8(cpu.e, 0, 1); I(4);
		cpu.d = cpu.a; I(4);
		return;
	}
	exx();
	cpu.a = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	SET_HL(HL + 1); I(6);
	exx();
	op_add8(cpu.a, 0); I(4);
	cpu.l = cpu.a; I(4);
	cpu.h = 0; I(7);
	for (int i = 0; i < 3; i++) { SET_HL(op_add16(HL, HL)); I(11); }
	SET_DE(0xAFE4); I(10);
	SET_HL(op_add16(HL, DE)); I(11);
	ex_de_hl();
}

static void dungeon_cells(void);
static void dungeon_seen(void);

static void gem_map(void) {            // $4F8A
	cpu.a = mem_read(0xC99F); I(13);
	op_sub8(0x02, 0, 0); I(7);
	I(10);
	dungeon = (cpu.f & FZ) != 0;
	if (dungeon) {                     // $5041: the level's cells, and what the party has seen
		I(17); dungeon_cells();
		I(17); dungeon_seen();
	}
	cpu.a = 0x05; I(7);
	vram_bank_push(dungeon ? 0x5049 : 0x4F94);
	I(17); clear_window();
	SET_HL(dungeon ? 0xD800 : 0xD000); I(10);
	exx();
	SET_HL(0x0028); I(10);
	cpu.b = 0x20; I(7);
	do {                               // a column of tiles: two cells across
		push(BC); push(HL);
		ex_de_hl();
		SET_HL(0xC1C0); I(10);
		SET_BC(0x1000); I(10);
		I(17); fill();
		ex_de_hl();
		cpu.b = 0x10; I(7);
		do {                           // a row of tiles
			push(BC); push(HL);
			next_picture();
			SET_HL(0xC1C0); I(10);
			cpu.b = 0x10; I(7);
			do {
				cpu.a = mem_read(DE); I(7);
				op_rrca(); I(4);
				op_rrca(); I(4);
				op_or(mem_read(HL)); I(7);
				mem_write(HL, cpu.a); I(7);
				SET_DE(DE + 1); I(6);
				cpu.l = op_inc8(cpu.l); I(4);
				if (--cpu.b) I(13); else I(8);
			} while (cpu.b);
			next_picture();
			SET_HL(0xC1C0); I(10);
			SET_BC(0xC1D0); I(10);
			cpu.a = 0x10; I(7);
			do {
				op_ex_af(); I(4);
				cpu.a = mem_read(DE); I(7);
				op_rlca(); I(4);
				op_rlca(); I(4);
				op_and(0x03); I(7);
				op_or(mem_read(HL)); I(7);
				mem_write(HL, cpu.a); I(7);
				cpu.a = mem_read(DE); I(7);
				op_add8(cpu.a, 0); I(4);
				op_add8(cpu.a, 0); I(4);
				mem_write(BC, cpu.a); I(7);
				SET_DE(DE + 1); I(6);
				cpu.l = op_inc8(cpu.l); I(4);
				cpu.c = op_inc8(cpu.c); I(4);
				op_ex_af(); I(4);
				cpu.a = op_dec8(cpu.a); I(4);
				I(10);
			} while (!(cpu.f & FZ));
			SET_DE(pop16()); I(10);    // $5000: the tile out
			vram_set_write_address();
			SET_HL(0xC1C0); I(10);
			SET_BC(0x10BE); I(10);
			for (;;) {
				op_block(4, 3); I(16);
				if (cpu.f & FZ) { I(7); break; }
				I(12);
			}
			SET_HL(0x0220); I(10);
			SET_HL(op_add16(HL, DE)); I(11);
			push(HL);
			SET_HL(0xC1D0); I(10);
			SET_DE(0xC1C0); I(10);
			SET_BC(0x0010); I(10);
			while (op_block(6, 0)) I(21);
			I(16);
			SET_HL(pop16()); I(10);
			SET_BC(pop16()); I(10);
			cpu.b = op_dec8(cpu.b); I(4);
			I(10);
		} while (!(cpu.f & FZ));
		ex_de_hl();                    // the column's last tile, with the window's edge
		vram_set_write_address();
		SET_HL(0x22CC); I(10);
		SET_DE(0xC1C0); I(10);
		cpu.b = 0x10; I(7);
		do {
			cpu.a = mem_read(DE); I(7);
			op_or(mem_read(HL)); I(7);
			io_out((uint16_t)(cpu.a << 8 | 0xBE), cpu.a); I(11);
			SET_HL(HL + 1); I(6);
			cpu.e = op_inc8(cpu.e); I(4);
			if (--cpu.b) I(13); else I(8);
		} while (cpu.b);
		SET_HL(pop16()); I(10);
		SET_BC(pop16()); I(10);
		SET_DE(0x0010); I(10);
		SET_HL(op_add16(HL, DE)); I(11);
		cpu.b = op_dec8(cpu.b); I(4);
		I(10);
	} while (!(cpu.f & FZ));
	vram_bank_pop();
}

static void bit_mem(int n, uint16_t a) {   // bit n,(hl) / (iy+d): X and Y from the address
	op_bit(n, mem_read(a));
	cpu.f = (uint8_t)((cpu.f & ~(FX | FY)) | ((a >> 8) & (FX | FY)));
}
static void set_mem(int n, uint16_t a) { mem_write(a, (uint8_t)(mem_read(a) | 1 << n)); }

static void copy_level(void) {         // $516A: the 16 x 16 cells at $DC00 to DE, rows 32 apart
	SET_HL(0xDC00); I(10);
	cpu.a = 0x10; I(7);
	do {
		SET_BC(0x0010); I(10);
		while (op_block(6, 0)) I(21);
		I(16);
		ex_de_hl();
		SET_BC(0x0010); I(10);
		SET_HL(op_add16(HL, BC)); I(11);
		ex_de_hl();
		cpu.a = op_dec8(cpu.a); I(4);
		if (cpu.f & FZ) { I(7); break; }
		I(12);
	} while (1);
	I(10);
}

static void mark_row(void) {           // $517E: 32 cells from HL marked as walls (bit 6)
	cpu.b = 0x20; I(7);
	do {
		set_mem(6, HL); I(15);
		cpu.l = op_inc8(cpu.l); I(4);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	I(10);
}

// $50F2: the party's level ($D000 + 256 * $C970) through the table at
// $8200 (bank $10) into $DC00, wrapped so the party is in the middle;
// $0F (walls) get bit 6. Then four copies of it make a 32 x 32 view at
// $D800, its edges walls too.
static void dungeon_cells(void) {
	cpu.a = 0x10; I(7);
	vram_bank_push(0x50F4);
	SET_HL(0xDC00); I(10);
	cpu.d = 0x82; I(7);
	exx();
	cpu.a = mem_read(0xC970); I(13);
	op_add8(0xD0, 0); I(7);
	cpu.h = cpu.a; I(4);
	cpu.a = mem_read(0xC954); I(13);
	for (int i = 0; i < 4; i++) { op_add8(cpu.a, 0); I(4); }
	cpu.d = cpu.a; I(4);
	cpu.a = mem_read(0xC953); I(13);
	op_or(cpu.d); I(4);
	cpu.l = cpu.a; I(4);
	cpu.e = 0x0F; I(7);
	cpu.c = 0x10; I(7);
	do {
		cpu.b = 0x10; I(7);
		do {
			cpu.a = mem_read(HL); I(7);
			exx();
			cpu.e = cpu.a; I(4);
			cpu.a = mem_read(DE); I(7);
			op_sub8(0x0F, 0, 0); I(7);
			if (!(cpu.f & FZ)) I(12);
			else { I(7); op_or(0x40); I(7); }
			mem_write(HL, cpu.a); I(7);
			cpu.l = op_inc8(cpu.l); I(4);
			exx();
			cpu.a = cpu.l; I(4);
			cpu.a = op_inc8(cpu.a); I(4);
			op_and(cpu.e); I(4);
			op_or(cpu.d); I(4);
			cpu.l = cpu.a; I(4);
			if (--cpu.b) I(13); else I(8);
		} while (cpu.b);
		op_add8(0x10, 0); I(7);
		cpu.l = cpu.a; I(4);
		op_and(0xF0); I(7);
		cpu.d = cpu.a; I(4);
		cpu.c = op_dec8(cpu.c); I(4);
		if (cpu.f & FZ) { I(7); break; }
		I(12);
	} while (1);
	static const uint16_t to[4] = { 0xD800, 0xD810, 0xDA00, 0xDA10 };
	for (int i = 0; i < 4; i++) { SET_DE(to[i]); I(10); I(17); copy_level(); }
	SET_HL(0xD800); I(10); I(17); mark_row();
	SET_HL(0xDBE0); I(10); I(17); mark_row();
	SET_HL(0xD800); I(10);
	SET_DE(0x001F); I(10);
	cpu.b = 0x20; I(7);
	do {
		set_mem(6, HL); I(15);
		SET_HL(op_add16(HL, DE)); I(11);
		set_mem(6, HL); I(15);
		SET_HL(HL + 1); I(6);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	vram_bank_pop();
}

// $5186: what the party can see: a flood fill (a stack of places at
// $DC00) from where it stands ($DA10) through the open cells, marking
// them and the walls around them (bit 7); then the cells not seen are
// cleared and the edges are drawn as walls
static void dungeon_seen(void) {
	push16(cpu.iy); I(15);
	cpu.iy = 0xDA10; I(14);
	SET_DE(0xDC00); I(10);
	SET_HL(0x0020); I(10);
	for (;;) {
		do {                           // $5192: back to the run's start
			cpu.iy--; I(10);
			bit_mem(6, cpu.iy); I(20);
			if (cpu.f & FZ) I(12); else I(7);
		} while (cpu.f & FZ);
		set_mem(7, cpu.iy); I(23);
		set_mem(7, (uint16_t)(cpu.iy + 0x20)); I(23);
		set_mem(7, (uint16_t)(cpu.iy - 0x20)); I(23);
		cpu.iy++; I(10);
		cpu.c = cpu.h; I(4);
		do {                           // $51A9: along the run
			set_mem(6, cpu.iy); I(23);
			set_mem(7, cpu.iy); I(23);
			bit_mem(6, (uint16_t)(cpu.iy + 0x20)); I(20);
			if (!(cpu.f & FZ)) {
				I(7);
				cpu.c &= ~1; I(8);
				set_mem(7, (uint16_t)(cpu.iy + 0x20)); I(23);
				I(12);
			} else {
				I(12);
				op_bit(0, cpu.c); I(8);
				if (!(cpu.f & FZ)) I(12);
				else {                 // the row below: open, to visit later
					I(7);
					cpu.c |= 1; I(8);
					cpu.a = (uint8_t)cpu.iy; I(8);
					op_add8(cpu.l, 0); I(4);
					mem_write(DE, cpu.a); I(7);
					cpu.e = op_inc8(cpu.e); I(4);
					cpu.a = (uint8_t)(cpu.iy >> 8); I(8);
					op_add8(cpu.h, cpu.f & FC); I(4);
					mem_write(DE, cpu.a); I(7);
					cpu.e = op_inc8(cpu.e); I(4);
				}
			}
			bit_mem(6, (uint16_t)(cpu.iy - 0x20)); I(20);   // $51CF
			if (!(cpu.f & FZ)) {
				I(7);
				cpu.c &= ~2; I(8);
				set_mem(7, (uint16_t)(cpu.iy - 0x20)); I(23);
				I(12);
			} else {
				I(12);
				op_bit(1, cpu.c); I(8);
				if (!(cpu.f & FZ)) I(12);
				else {                 // the row above
					I(7);
					cpu.c |= 2; I(8);
					cpu.a = (uint8_t)cpu.iy; I(8);
					op_sub8(cpu.l, 0, 1); I(4);
					mem_write(DE, cpu.a); I(7);
					cpu.e = op_inc8(cpu.e); I(4);
					cpu.a = (uint8_t)(cpu.iy >> 8); I(8);
					op_sub8(cpu.h, cpu.f & FC, 1); I(4);
					mem_write(DE, cpu.a); I(7);
					cpu.e = op_inc8(cpu.e); I(4);
				}
			}
			cpu.iy++; I(10);           // $51ED
			bit_mem(6, cpu.iy); I(20);
			I(10);
		} while (cpu.f & FZ);
		set_mem(7, cpu.iy); I(23);
		set_mem(7, (uint16_t)(cpu.iy + 0x20)); I(23);
		set_mem(7, (uint16_t)(cpu.iy - 0x20)); I(23);
		cpu.e = op_inc8(cpu.e); I(4);
		cpu.e = op_dec8(cpu.e); I(4);
		if (cpu.f & FZ) { I(12); break; }   // nothing left on the stack
		I(7);
		cpu.e = op_dec8(cpu.e); I(4);
		cpu.a = mem_read(DE); I(7);
		cpu.iy = (uint16_t)(cpu.a << 8 | (cpu.iy & 0xFF)); I(8);
		cpu.e = op_dec8(cpu.e); I(4);
		cpu.a = mem_read(DE); I(7);
		cpu.iy = (uint16_t)((cpu.iy & 0xFF00) | cpu.a); I(8);
		I(10);
	}
	SET_HL(0xD800); I(10);             // $5211: the cells not seen, cleared
	SET_BC(0x0004); I(10);
	op_xor(cpu.a); I(4);
	do {
		do {
			bit_mem(7, HL); I(12);
			if (!(cpu.f & FZ)) I(12);
			else { I(7); mem_write(HL, cpu.a); I(7); }
			SET_HL(HL + 1); I(6);
			if (--cpu.b) I(13); else I(8);
		} while (cpu.b);
		cpu.c = op_dec8(cpu.c); I(4);
		if (cpu.f & FZ) { I(7); break; }
		I(12);
	} while (1);
	SET_HL(0xD800); I(10);
	SET_BC(0x2000); I(10);
	I(17); fill();
	SET_HL(0xDBE0); I(10);
	SET_BC(0x2000); I(10);
	I(17); fill();
	SET_HL(0xD800); I(10);
	SET_DE(0x001F); I(10);
	SET_BC(0x2000); I(10);
	do {
		mem_write(HL, cpu.c); I(7);
		SET_HL(op_add16(HL, DE)); I(11);
		mem_write(HL, cpu.c); I(7);
		SET_HL(HL + 1); I(6);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	cpu.iy = pop16(); I(14);
	I(10);
}

#define GEM(name, addr, bank, out) Native native_##name = { #name, addr, bank, name, out, .long_calls = 1 };
GEM(clear_window, 0x2740, 0, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
GEM(fill, 0x333B, 0, OUT_F | OUT_BC | OUT_HL)
GEM(gem_map, 0x4F8A, 1, OUT_A | OUT_F)
GEM(dungeon_cells, 0x50F2, 1, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
GEM(dungeon_seen, 0x5186, 1, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
