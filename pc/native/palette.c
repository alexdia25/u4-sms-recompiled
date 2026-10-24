// The palette kept in RAM ($C023: 16 background colours, $C033: 16 for the
// sprites); the interrupt sends it to the VDP when $C009 is set.
// - set_palette_entry ($0843): colour H as entry L.
// - The fades ($0855 all 32, $085C the background's, $0863 the sprites'):
//   each colour (%00BBGGRR) takes one step towards its target at HL, unless
//   the two are less than C apart (the three differences added up). The
//   step is in the component that differs most (blue before green before
//   red when they tie), by one.
// $C009 is set before the colours change, so the interrupt can send the
// palette in the middle of a fade: the C goes instruction by instruction
// (I) and the machine catches up after each. Cycle counts are the
// original's.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static void set_palette_entry(void) {  // $0843
	push16(DE); I(11);
	push16(HL); I(11);
	cpu.a = cpu.h; I(4);
	cpu.h = 0; I(7);
	SET_DE(0xC023); I(10);
	SET_HL(op_add16(HL, DE)); I(11);
	mem_write(HL, cpu.a); I(7);
	cpu.a = 0xFF; I(7);
	mem_write(0xC009, cpu.a); I(13);
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	I(10);
}

static void exx(void) { op_exx(); I(4); }

// %00BBGGRR in A -> its components, two bits each, into r1, r2, r3
static void split(uint8_t *r1, uint8_t *r2, uint8_t *r3) {
	op_rlca(); I(4);
	op_rlca(); I(4);
	uint8_t *rs[3] = { r1, r2, r3 };
	for (int i = 0; i < 3; i++)
		for (int k = 0; k < 2; k++) {
			op_rla(); I(4);
			*rs[i] = op_rot(2, *rs[i]); I(8);   // rl r
		}
}

// e = e - b; b = |e - b| (sub / jr nc / neg)
static void diff(uint8_t *t, uint8_t *abs_out) {
	cpu.a = *t; I(4);
	op_sub8(*abs_out, 0, 1); I(4);
	*t = cpu.a; I(4);
	if (cpu.f & FC) { I(7); op_neg(); I(8); }
	else I(12);
	*abs_out = cpu.a; I(4);
}

// xor a / inc r / dec r / jr z / ld a,up / jp p / ld a,down (/ jp)
static void step(uint8_t r, uint8_t up, uint8_t down) {
	op_xor(cpu.a); I(4);
	r = op_inc8(r); I(4);
	r = op_dec8(r); I(4);
	if (cpu.f & FZ) { I(12); return; }
	I(7);
	cpu.a = up; I(7);
	if (!(cpu.f & FS)) { I(10); return; }
	I(10);
	cpu.a = down; I(7);
	if (down != 0xF0) I(10);           // blue falls through to $0903
}

static void fade(uint16_t pal, uint8_t n, int jr) {   // $0868: DE = pal, B = n
	SET_DE(pal); I(10);
	cpu.b = n; I(7);
	if (jr) I(12);                     // jr $0868
	cpu.a = 0xFF; I(7);
	mem_write(0xC009, cpu.a); I(13);
	do {
		push16(BC); I(11);
		cpu.a = mem_read(DE); I(7);
		exx();
		SET_BC(0); I(10);
		SET_DE(0); I(10);
		SET_HL(0); I(10);
		split(&cpu.b, &cpu.c, &cpu.d);
		exx();
		cpu.a = mem_read(HL); I(7);
		exx();
		split(&cpu.e, &cpu.h, &cpu.l);
		diff(&cpu.e, &cpu.b);          // blue
		diff(&cpu.h, &cpu.c);          // green
		diff(&cpu.l, &cpu.d);          // red
		cpu.a = cpu.b; I(4);
		op_add8(cpu.c, 0); I(4);
		op_add8(cpu.d, 0); I(4);
		exx();
		op_sub8(cpu.c, 0, 0); I(4);    // cp c: close enough?
		int change = 0;
		if (cpu.f & FC) I(10);
		else {
			I(10);
			exx();
			cpu.a = cpu.b; I(4);
			op_sub8(cpu.c, 0, 0); I(4);
			int blue = 0;
			if (cpu.f & FC) I(12);
			else {
				I(7);
				op_sub8(cpu.d, 0, 0); I(4);
				if (cpu.f & FC) I(12);
				else { I(7); blue = 1; }
			}
			if (blue) {
				op_or(cpu.a); I(4);
				I(10);
				if (cpu.f & FZ) { exx(); I(10); }     // no difference at all
				else { step(cpu.e, 0x10, 0xF0); change = 1; }
			} else {                   // $08D4
				cpu.a = cpu.c; I(4);
				op_sub8(cpu.d, 0, 0); I(4);
				I(10);
				if (cpu.f & FC) step(cpu.l, 0x01, 0xFF);
				else step(cpu.h, 0x04, 0xFC);
				change = 1;
			}
		}
		if (change) {                  // $0903
			exx();
			cpu.b = cpu.a; I(4);
			cpu.a = mem_read(DE); I(7);
			op_add8(cpu.b, 0); I(4);
			mem_write(DE, cpu.a); I(7);
		}
		SET_HL(HL + 1); I(6);          // $0908
		SET_DE(DE + 1); I(6);
		SET_BC(pop16()); I(10);
		cpu.b = op_dec8(cpu.b); I(4);
		I(10);
	} while (!(cpu.f & FZ));
	I(10);
}

static void fade_all(void) { fade(0xC023, 0x20, 1); }          // $0855
static void fade_background(void) { fade(0xC023, 0x10, 1); }   // $085C
static void fade_sprites(void) { fade(0xC033, 0x10, 0); }      // $0863

Native native_set_palette_entry = { "set_palette_entry", 0x0843, 0, set_palette_entry, OUT_A | OUT_F, .long_calls = 1 };
Native native_fade_all = { "fade_all", 0x0855, 0, fade_all, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, .long_calls = 1 };
Native native_fade_background = { "fade_background", 0x085C, 0, fade_background, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, .long_calls = 1 };
Native native_fade_sprites = { "fade_sprites", 0x0863, 0, fade_sprites, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, .long_calls = 1 };
