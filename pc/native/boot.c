// Power-on (bank 0).
// - power_on ($0042, from the reset vector): the mapper's pages set, RAM
//   cleared, the stack set; then init_hw and the main program ($0CA6).
// - init_hw ($009C): the sound chip silenced (the bytes at $0142); the
//   console's region from port $3F/$DD ($C005: $FF export, else a pause
//   of two long loops); vdp_init; the frame rate from the lines between
//   two frames ($C001 bit 0: 50 Hz); the system's type from $C000 (kept
//   or set to $A8); the FM unit looked for at port $F2 ($C002); then on
//   at $063D.
// - vdp_init ($014D): the VDP's registers from $019E (kept at $C013), the
//   palette and its copy ($C023) cleared, the sprites ended, the display
//   flags, the interrupt's routine ($C00D = a ret) and the VRAM queue.
// Cycle counts are the original's, instruction by instruction (I).
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static void out(uint8_t port) { io_out((uint16_t)(cpu.a << 8 | port), cpu.a); I(11); }
static uint8_t in(uint8_t port) { return io_in((uint16_t)(cpu.a << 8 | port)); }

static void ldir(void) {
	while (op_block(6, 0)) I(21);
	I(16);
}

static void queue_reset(void) {        // $0453
	op_xor(cpu.a); I(4);
	mem_write(0xC063, cpu.a); I(13);
	I(10);
}

static void vdp_init(void) {           // $014D
	cpu.iff1 = cpu.iff2 = 0; I(4);
	cpu.a = in(0xBF); I(11);
	SET_HL(0x019E); I(10);
	SET_DE(0xC013); I(10);
	SET_BC(0x000B); I(10);
	ldir();
	SET_HL(0xC013); I(10);
	SET_BC(0x16BF); I(10);
	cpu.a = 0x80; I(7);
	do {                               // each register: its value, then its number
		op_block(4, 3); I(16);         // outi
		out(0xBF);
		cpu.a = op_inc8(cpu.a); I(4);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	cpu.a = 0x00; I(7);
	out(0xBF);
	cpu.a = 0xC0; I(7);
	out(0xBF);                         // the palette
	SET_HL(0xC023); I(10);
	op_xor(cpu.a); I(4);
	cpu.b = 0x20; I(7);
	do {
		out(0xBE);
		mem_write(HL, cpu.a); I(7);
		SET_HL(HL + 1); I(6);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	cpu.a = 0x00; I(7);
	out(0xBF);
	cpu.a = 0x7F; I(7);
	out(0xBF);                         // the sprite table: none
	cpu.a = 0xD0; I(7);
	out(0xBE);
	op_xor(cpu.a); I(4);
	mem_write(0xC007, cpu.a); I(13);
	mem_write(0xC00F, cpu.a); I(13);
	mem_write(0xC010, cpu.a); I(13);
	SET_HL(0x019D); I(10);
	wr16(0xC00D, HL); I(16);
	I(17); queue_reset();
	I(10);
}

static void wait_frame_flag(void) {    // in a,($BF) / or a / jp p: until a frame ends
	do {
		cpu.a = in(0xBF); I(11);
		op_or(cpu.a); I(4);
		I(10);
	} while (!(cpu.f & FS));
}

static void init_hw(void) {            // $009C
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: it waits on the VDP
	SET_HL(0x0142); I(10);
	SET_BC(0x0B7F); I(10);
	while (op_block(6, 3)) I(21);      // otir: the sound chip silenced
	I(16);
	SET_HL(0xC005); I(10);
	cpu.a = 0xF5; I(7);
	out(0x3F);
	cpu.a = in(0xDD); I(11);
	op_and(0xC0); I(7);
	op_sub8(0xC0, 0, 0); I(7);
	int export = 0;
	if (!(cpu.f & FZ)) I(12);
	else {
		I(7);
		cpu.a = 0x55; I(7);
		out(0x3F);
		cpu.a = in(0xDD); I(11);
		op_and(0xC0); I(7);
		if (!(cpu.f & FZ)) I(12);
		else {
			I(7);
			cpu.a = 0xFF; I(7);
			out(0x3F);
			mem_write(HL, cpu.a); I(7);
			I(12);
			export = 1;
		}
	}
	if (!export) {                     // $00C4: a Japanese console: a pause
		mem_write(HL, 0x00); I(10);
		cpu.d = 0x02; I(7);
		do {
			SET_BC(0xC000); I(10);
			do {
				I(19); I(19);          // ex (sp),hl twice
				SET_BC(BC - 1); I(6);
				cpu.a = cpu.b; I(4);
				op_or(cpu.c); I(4);
				if (cpu.f & FZ) { I(7); break; }
				I(12);
			} while (1);
			cpu.d = op_dec8(cpu.d); I(4);
			if (cpu.f & FZ) { I(7); break; }
			I(12);
		} while (1);
	}
	cpu.iff1 = cpu.iff2 = 0; I(4);     // $00D5
	I(17); vdp_init();
	SET_HL(0x0000); I(10);
	wait_frame_flag();
	wait_frame_flag();
	do {                               // count to the next frame
		SET_HL(HL + 1); I(6);
		cpu.a = in(0xBF); I(11);
		op_or(cpu.a); I(4);
		I(10);
	} while (!(cpu.f & FS));
	op_xor(cpu.a); I(4);
	SET_DE(0x0800); I(10);
	op_sbc16(DE); I(15);
	op_rla(); I(4);
	mem_write(0xC001, cpu.a); I(13);
	cpu.a = mem_read(0xC000); I(13);
	op_cpl(); I(4);
	op_and(0xE8); I(7);
	static const uint8_t known[3] = { 0x20, 0x40, 0x80 };
	int kept = 0;
	for (int i = 0; i < 3 && !kept; i++) {
		op_sub8(known[i], 0, 0); I(7);
		I(10);
		kept = (cpu.f & FZ) != 0;
	}
	if (!kept) {
		cpu.a = 0xA8; I(7);
		mem_write(0xC000, cpu.a); I(13);
	}
	cpu.a = mem_read(0xC000); I(13);   // $0113
	op_or(0x04); I(7);
	out(0x3E);
	SET_BC(0x0700); I(10);
	do {                               // the FM unit: does port $F2 keep what is written?
		cpu.a = cpu.b; I(4);
		op_and(0x01); I(7);
		out(0xF2);
		cpu.e = cpu.a; I(4);
		cpu.a = in(0xF2); I(11);
		op_and(0x07); I(7);
		op_sub8(cpu.e, 0, 0); I(4);
		if (!(cpu.f & FZ)) I(12);
		else { I(7); cpu.c = op_inc8(cpu.c); I(4); }
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	cpu.a = cpu.c; I(4);
	op_sub8(0x07, 0, 0); I(7);
	if (cpu.f & FZ) I(12);
	else { I(7); op_xor(cpu.a); I(4); }
	op_and(0x01); I(7);
	out(0xF2);
	mem_write(0xC002, cpu.a); I(13);
	cpu.a = mem_read(0xC000); I(13);
	out(0x3E);
	I(10);
	native_continue_at(0x063D);
}

static void power_on(void) {           // $0042
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: it waits on the VDP
	SET_HL(0xFFFC); I(10);
	mem_write(HL, 0x80); I(10);
	cpu.l = op_inc8(cpu.l); I(4);
	mem_write(HL, 0x00); I(10);
	cpu.l = op_inc8(cpu.l); I(4);
	mem_write(HL, 0x01); I(10);
	cpu.l = op_inc8(cpu.l); I(4);
	mem_write(HL, 0x02); I(10);
	SET_HL(0xC001); I(10);
	SET_DE(0xC002); I(10);
	SET_BC(0x1FEE); I(10);
	mem_write(HL, 0x00); I(10);
	ldir();
	cpu.sp = 0xDFF0; I(10);
	I(17);
	push16(0x0063);                    // call init_hw, which returns to the jp $0CA6
	init_hw();
}

#define BOOT(name, addr, out) Native native_##name = { #name, addr, 0, name, out, .long_calls = 1 };
BOOT(power_on, 0x0042, 0)
BOOT(init_hw, 0x009C, 0)
BOOT(vdp_init, 0x014D, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
BOOT(queue_reset, 0x0453, OUT_A | OUT_F)
