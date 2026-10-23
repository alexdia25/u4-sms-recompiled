// Sprites put in the sprite tables ($C180: Y, $C100: X and tile, $C064 the
// next free X slot, 2 a sprite; the frame interrupt sends them). A figure
// is a list at HL: a count, then for each sprite its X and Y from the
// figure's place and its tile. Sprites off the screen (or whose place
// overflows) are left out; when the table is full the rest are.
// - put_figure ($096B): the place from the task's slot (IX+$0B X, +$0D Y);
//   put_figure_at ($09C7): at E, D. Their far forms ($0962, $09BE) page
//   bank A in for the list.
// - path_step ($0BEE): a figure's next step along its path ($0C19).
// - put_picture ($3082): figure A of the list at $A1B6 in bank $13, at E, D;
//   put_picture_cell ($30B6): the same at cell L, H (12 pixels a cell).
// The main registers walk the list, the alternate ones the tables (exx).
// Cycle counts are the original's, instruction by instruction (I).
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static void exx(void) { op_exx(); I(4); }

static void ex_af(void) { op_ex_af(); I(4); }

static void rra(void) {
	uint8_t c = cpu.a & 1;
	cpu.a = (uint8_t)(cpu.a >> 1 | (cpu.f & FC) << 7);
	cpu.f = (uint8_t)((cpu.f & ~(FC | FH | FN)) | c);
	I(4);
}

// the start: the count, and the tables' places from $C064; 0 if full
static int figure_start(void) {
	cpu.b = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	exx();
	cpu.a = mem_read(0xC064); I(13);
	op_sub8(0x80, 0, 0); I(7);
	if (!(cpu.f & FC)) { I(11); return 0; }
	I(5);
	cpu.e = cpu.a; I(4);
	rra();                             // with the carry: $80 + X slot / 2
	cpu.c = cpu.a; I(4);
	cpu.b = 0xC1; I(7);
	cpu.d = cpu.b; I(4);
	exx();
	return 1;
}

// $0981: the sprites, from E, D
static void figure_sprites(void) {
	for (;;) {
		int skip = 0;
		cpu.a = cpu.e; I(4);
		op_add8(mem_read(HL), 0); I(7);
		cpu.c = cpu.a; I(4);
		rra();
		op_xor(mem_read(HL)); I(7);
		cpu.a = cpu.c; I(4);
		I(10);
		if (cpu.f & FS) {              // X overflows
			SET_HL(HL + 1); I(6);
			skip = 1;
		} else {
			ex_af();
			SET_HL(HL + 1); I(6);
			cpu.a = cpu.d; I(4);
			op_add8(mem_read(HL), 0); I(7);
			cpu.c = cpu.a; I(4);
			rra();
			op_xor(mem_read(HL)); I(7);
			cpu.a = cpu.c; I(4);
			I(10);
			if (cpu.f & FS) {          // above the top: only just ($F0 up)
				op_sub8(0xF0, 0, 0); I(7);
				if (cpu.f & FC) { I(7); I(10); skip = 1; }
				else I(12);
			} else {                   // below the bottom ($C0 on)
				op_sub8(0xC0, 0, 0); I(7);
				if (!(cpu.f & FC)) { I(12); skip = 1; }
				else I(7);
			}
		}
		if (skip) {                    // $09BA
			SET_HL(HL + 1); I(6);
			I(10);
		} else {                       // $09A0
			exx();
			mem_write(BC, cpu.a); I(7);
			cpu.c = op_inc8(cpu.c); I(4);
			ex_af();
			mem_write(DE, cpu.a); I(7);
			cpu.e = op_inc8(cpu.e); I(4);
			exx();
			SET_HL(HL + 1); I(6);
			cpu.a = mem_read(HL); I(7);
			exx();
			mem_write(DE, cpu.a); I(7);
			cpu.e = op_inc8(cpu.e); I(4);
			I(10);
			if (cpu.f & FS) break;     // the table is full
			exx();
		}
		SET_HL(HL + 1); I(6);          // $09B0
		if (--cpu.b) { I(13); continue; }
		I(8);
		exx();
		break;
	}
	cpu.a = cpu.e; I(4);               // $09B4
	mem_write(0xC064, cpu.a); I(13);
	I(10);
}

static void put_figure(void) {         // $096B
	if (!figure_start()) return;
	cpu.d = mem_read((uint16_t)(cpu.ix + 0x0D)); I(19);
	cpu.e = mem_read((uint16_t)(cpu.ix + 0x0B)); I(19);
	figure_sprites();
}

static void put_figure_at(void) {      // $09C7
	if (!figure_start()) return;
	I(10);
	figure_sprites();
}

// bank_push from a call at site; bank_pop as the routine's tail
static void bank_push(uint16_t site) {
	push16((uint16_t)(site + 3)); I(17);
	wr16(0xC0C2, HL); I(16);
	SET_HL(0xDFFF); I(10);
	cpu.h = mem_read(HL); I(7);
	{ uint16_t t = rd16(cpu.sp); wr16(cpu.sp, HL); SET_HL(t); } I(19);
	push16(HL); I(11);
	SET_HL(rd16(0xC0C2)); I(16);
	mem_write(0xFFFF, cpu.a); I(13);
	pop16(); I(10);
}

static void bank_pop(void) {
	I(10);
	wr16(0xC0C2, HL); I(16);
	SET_HL(pop16()); I(10);
	cpu.l = cpu.a; I(4);
	cpu.a = cpu.h; I(4);
	mem_write(0xFFFF, cpu.a); I(13);
	cpu.a = cpu.l; I(4);
	SET_HL(rd16(0xC0C2)); I(16);
	I(10);
}

static void far_figure(uint16_t site, void (*body)(void)) {
	bank_push(site);
	push16((uint16_t)(site + 6)); I(17);
	body();
	pop16();                           // its ret, counted in the body
	bank_pop();
}
static void far_put_figure(void) { far_figure(0x0962, put_figure); }          // $0962
static void far_put_figure_at(void) { far_figure(0x09BE, put_figure_at); }    // $09BE

static void put_picture(void) {        // $3082
	ex_af();
	cpu.a = 0x13; I(7);
	bank_push(0x3085);
	ex_af();
	push16(HL); I(11);
	SET_HL(0xA1B6); I(10);
	I(11);
	call_original(0x0020);             // rst $20: HL = the A-th word there
	push16(0x3091); I(17);
	put_figure_at();
	pop16();
	SET_HL(pop16()); I(10);
	bank_pop();
}

static void times12(void) {            // $30C5: A * 12
	I(17);
	op_add8(cpu.a, 0); I(4);
	op_add8(cpu.a, 0); I(4);
	cpu.c = cpu.a; I(4);
	op_add8(cpu.a, 0); I(4);
	op_add8(cpu.c, 0); I(4);
	I(10);
}

static void put_picture_cell(void) {   // $30B6
	cpu.b = cpu.a; I(4);
	cpu.a = cpu.l; I(4);
	times12();
	cpu.e = cpu.a; I(4);
	cpu.a = cpu.h; I(4);
	times12();
	cpu.d = cpu.a; I(4);
	cpu.a = cpu.b; I(4);
	I(10);
	put_picture();
}

// A figure's path: a list of steps in bank IX+8 from IX+$12/$13, each a
// byte: X step in the high four bits and Y step in the low (signed), $88
// the end (carry).
static void far_read(uint16_t site) {  // $0815: A = (HL) in bank A
	bank_push(site);
	cpu.a = mem_read(HL); I(7);
	bank_pop();
}

static void path_byte(void) {          // $0C19: the next byte, the pointer on
	cpu.a = mem_read((uint16_t)(cpu.ix + 0x08)); I(19);
	cpu.l = mem_read((uint16_t)(cpu.ix + 0x12)); I(19);
	cpu.h = mem_read((uint16_t)(cpu.ix + 0x13)); I(19);
	push16(0x0C25); I(17);
	far_read(0x0815);
	pop16();
	SET_HL(HL + 1); I(6);
	mem_write((uint16_t)(cpu.ix + 0x12), cpu.l); I(19);
	mem_write((uint16_t)(cpu.ix + 0x13), cpu.h); I(19);
	I(10);
}

static void sra4(void) { for (int i = 0; i < 4; i++) { cpu.a = op_rot(5, cpu.a); I(8); } }

static void path_step(void) {          // $0BEE
	push16(0x0BF1); I(17);
	path_byte();
	pop16();
	op_sub8(0x88, 0, 0); I(7);
	cpu.f |= FC; cpu.f &= ~(FH | FN); I(4);   // scf
	if (cpu.f & FZ) { I(11); return; }
	I(5);
	cpu.b = cpu.a; I(4);
	for (int i = 0; i < 4; i++) {      // rrca
		uint8_t c = cpu.a & 1;
		cpu.a = (uint8_t)(cpu.a >> 1 | c << 7);
		cpu.f = (uint8_t)((cpu.f & ~(FC | FH | FN)) | c);
		I(4);
	}
	sra4();
	op_add8(mem_read((uint16_t)(cpu.ix + 0x0B)), 0); I(19);
	mem_write((uint16_t)(cpu.ix + 0x0B), cpu.a); I(19);
	cpu.a = cpu.b; I(4);
	sra4();
	op_add8(mem_read((uint16_t)(cpu.ix + 0x0D)), 0); I(19);
	mem_write((uint16_t)(cpu.ix + 0x0D), cpu.a); I(19);
	op_or(cpu.a); I(4);
	I(10);
}

#define SPRITES(name, addr) Native native_##name = { #name, addr, 0, name, 0, .long_calls = 1 };
SPRITES(put_figure, 0x096B)
SPRITES(put_figure_at, 0x09C7)
SPRITES(far_put_figure, 0x0962)
SPRITES(far_put_figure_at, 0x09BE)
SPRITES(put_picture, 0x3082)
SPRITES(put_picture_cell, 0x30B6)
SPRITES(path_step, 0x0BEE)
SPRITES(path_byte, 0x0C19)
