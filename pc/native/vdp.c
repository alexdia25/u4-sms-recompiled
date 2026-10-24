// The VDP's registers and the waits for the vertical blank.
// - set_vdp_reg ($01A9): register C = B, kept in RAM at $C013 + C (the
//   interrupt is held between the two writes to the control port).
// - The bits of register 1 and 0 kept at $C014 and $C013: frame interrupt
//   on ($01CB, register 1 bit 5), line interrupt off ($01E3) and on ($01F1)
//   (register 0 bit 4), and the left column blanked ($0201, register 0 bit 5).
// - wait_vblank ($041C): the frame interrupt on, then a spin until the
//   interrupt clears $C008; wait_frames ($0437, B of them) and
//   wait_frames_6b ($042D, 6 * B).
// - display_on ($043D): $C007 = $81, the interrupt turns the display on;
//   display_off ($0443): $C007 = $80 and a wait until the interrupt has
//   turned it off (bit 7 clear).
// The waits spin on RAM the interrupt changes, so the C goes instruction by
// instruction (I) and the machine catches up after each: the interrupt comes
// between the same instructions as in the original. Cycle counts are the
// original's. In shadow mode (no fiber) the waits are left to the original.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static void set_vdp_reg(void) {        // $01A9
	push16(HL); I(11);
	cpu.a = cpu.c; I(4);
	op_add8(0x13, 0); I(7);
	cpu.l = cpu.a; I(4);
	cpu.h = 0xC0; I(7);
	cpu.a = cpu.b; I(4);
	cpu.iff1 = cpu.iff2 = 0; I(4);
	mem_write(HL, cpu.a); I(7);
	io_out((uint16_t)(cpu.a << 8 | 0xBF), cpu.a); I(11);
	cpu.a = cpu.c; I(4);
	op_or(0x80); I(7);
	io_out((uint16_t)(cpu.a << 8 | 0xBF), cpu.a); I(11);
	cpu.iff1 = cpu.iff2 = 1; cpu.cycles += 4;   // ei / pop hl: an interrupt pending comes after the pop
	SET_HL(pop16()); cpu.cycles += 10;
	native_irq_point();
	I(10);
}

static void call_set_vdp_reg(void) { I(17); set_vdp_reg(); }

// $01C3 / $01E9: B = A, C = the register, then set_vdp_reg; pop bc / ret
static void set_reg_from_a(uint8_t reg) {
	cpu.b = cpu.a; I(4);
	cpu.c = reg; I(7);
	call_set_vdp_reg();
	SET_BC(pop16()); I(10);
	I(10);
}

static void reg1_or(uint8_t bit) {     // push bc / ld a,($C014) / or bit / jr $01C3
	push16(BC); I(11);
	cpu.a = mem_read(0xC014); I(13);
	op_or(bit); I(7);
	I(12);
	set_reg_from_a(1);
}

static void reg0(uint8_t and_mask, uint8_t or_bits) {
	push16(BC); I(11);
	cpu.a = mem_read(0xC013); I(13);
	if (and_mask != 0xFF) op_and(and_mask); else op_or(or_bits);
	I(7);
	if (and_mask == 0xFF) I(12);       // the "or" forms jr to $01E9
	set_reg_from_a(0);
}

static void frame_irq_on(void) { reg1_or(0x20); }        // $01CB
static void line_irq_off(void) { reg0(0xEF, 0); }        // $01E3
static void line_irq_on(void) { reg0(0xFF, 0x10); }      // $01F1
static void blank_left_column(void) { reg0(0xFF, 0x20); } // $0201

static void vblank_wait(void) {        // $041C
	push16((uint16_t)(cpu.a << 8 | cpu.f)); I(11);
	push16(HL); I(11);
	I(17); frame_irq_on();
	cpu.a = 0xFF; I(7);
	SET_HL(0xC008); I(10);
	mem_write(HL, cpu.a); I(7);
	for (;;) {                         // cp (hl) / jr z
		op_sub8(mem_read(HL), 0, 0); I(7);
		if (!(cpu.f & FZ)) { I(7); break; }
		I(12);
	}
	SET_HL(pop16()); I(10);
	uint16_t af = pop16(); cpu.a = af >> 8; cpu.f = (uint8_t)af; I(10);
	I(10);
}

static int waits_ok(void) {
	if (native_on_fiber()) return 1;
	native_decline();                  // shadow mode: it waits for frames
	return 0;
}

static void nat_wait_vblank(void) { if (waits_ok()) vblank_wait(); }

static void wait_frames(void) {        // $0437: B frames
	do {
		I(17); vblank_wait();
		cpu.b--;
		if (!cpu.b) { I(8); break; }
		I(13);
	} while (1);
	I(10);
}

static void nat_wait_frames(void) { if (waits_ok()) wait_frames(); }

static void nat_wait_frames_6b(void) { // $042D: 6 frames, B times
	if (!waits_ok()) return;
	do {
		push16(BC); I(11);
		cpu.b = 6; I(7);
		I(17); wait_frames();
		SET_BC(pop16()); I(10);
		cpu.b--;
		if (!cpu.b) { I(8); break; }
		I(13);
	} while (1);
	I(10);
}

static void display_on(void) {         // $043D
	cpu.a = 0x81; I(7);
	mem_write(0xC007, cpu.a); I(13);
	I(10);
}

static void display_off(void) {        // $0443
	if (!waits_ok()) return;
	I(17); frame_irq_on();
	cpu.a = 0x80; I(7);
	mem_write(0xC007, cpu.a); I(13);
	do {
		cpu.a = mem_read(0xC007); I(13);
		op_or(cpu.a); I(4);
		I(10);                         // jp m
	} while (cpu.f & FS);
	I(10);
}

Native native_set_vdp_reg = { "set_vdp_reg", 0x01A9, 0, set_vdp_reg, 0, .long_calls = 1 };
Native native_frame_irq_on = { "frame_irq_on", 0x01CB, 0, frame_irq_on, OUT_A | OUT_F, .long_calls = 1 };
Native native_line_irq_off = { "line_irq_off", 0x01E3, 0, line_irq_off, OUT_A | OUT_F, .long_calls = 1 };
Native native_line_irq_on = { "line_irq_on", 0x01F1, 0, line_irq_on, OUT_A | OUT_F, .long_calls = 1 };
Native native_blank_left_column = { "blank_left_column", 0x0201, 0, blank_left_column, OUT_A | OUT_F, .long_calls = 1 };
Native native_wait_vblank = { "wait_vblank", 0x041C, 0, nat_wait_vblank, OUT_A | OUT_F | OUT_HL, .long_calls = 1 };
Native native_wait_frames = { "wait_frames", 0x0437, 0, nat_wait_frames, OUT_A | OUT_F | OUT_BC, .long_calls = 1 };
Native native_wait_frames_6b = { "wait_frames_6b", 0x042D, 0, nat_wait_frames_6b, OUT_A | OUT_F | OUT_BC, .long_calls = 1 };
Native native_display_on = { "display_on", 0x043D, 0, display_on, OUT_A, .long_calls = 1 };
Native native_display_off = { "display_off", 0x0443, 0, display_off, OUT_A | OUT_F, .long_calls = 1 };
