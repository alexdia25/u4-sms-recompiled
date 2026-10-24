// The restart helpers in bank 0, for the original code's rst calls:
// - rst $08 ($006F): HL as the VDP's read address; rst $10 ($0078): DE as
//   its write address (the interrupt held between the two writes).
// - $000B: A made positive.
// - rst $18 ($0083): jump to entry A of the table of words at HL.
// - rst $20 ($008E): HL = entry A of the table of words at HL (A may be
//   128 or more); $0092: the same for entry A / 2, $0097: HL = (HL).
// - rst $28: A = (HL + A); rst $30: HL += A.
// - switch_on_a ($334A): a jump by a table of keys.
// Cycle counts are the original's, instruction by instruction (I); the
// entries at the rst addresses add their jp where there is one.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static void out_bf(uint8_t v) { io_out((uint16_t)(cpu.a << 8 | 0xBF), v); }

static void vdp_address(uint8_t lo, uint8_t hi, int write) {
	cpu.iff1 = cpu.iff2 = 0; I(4);
	cpu.a = lo; I(4);
	out_bf(cpu.a); I(11);
	cpu.a = hi; I(4);
	if (write) { op_or(0x40); I(7); }
	out_bf(cpu.a); I(11);
	cpu.iff1 = cpu.iff2 = 1;           // ei / ret: an interrupt pending comes after the ret
	cpu.cycles += 4 + 10;
	native_irq_point();
}
static void read_address(void) { vdp_address(cpu.l, cpu.h, 0); }         // $006F
static void write_address(void) { vdp_address(cpu.e, cpu.d, 1); }        // $0078
static void rst08(void) { I(10); read_address(); }
static void rst10(void) { I(10); write_address(); }

static void abs_a(void) {              // $000B
	op_or(cpu.a); I(4);
	if (!(cpu.f & FS)) { I(11); return; }
	I(5);
	op_neg(); I(8);
	I(10);
}

static void add_a_to_hl(void) {        // add a,l / ld l,a / adc a,h / sub l / ld h,a
	op_add8(cpu.l, 0); I(4);
	cpu.l = cpu.a; I(4);
	op_add8(cpu.h, cpu.f & FC); I(4);
	op_sub8(cpu.l, 0, 1); I(4);
	cpu.h = cpu.a; I(4);
}

static void hl_from_table(void) {      // $0097: ld a,(hl) / inc hl / ld h,(hl) / ld l,a
	cpu.a = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	cpu.h = mem_read(HL); I(7);
	cpu.l = cpu.a; I(4);
}

static void jump_table(void) {         // $0083
	op_add8(cpu.a, 0); I(4);
	add_a_to_hl();
	hl_from_table();
	I(4);
	native_continue_at(HL);            // jp (hl): the target returns to the caller
}
static void rst18(void) { I(10); jump_table(); }

static void word_entry(void) {         // $0092
	add_a_to_hl();
	hl_from_table();
	I(10);
}
static void word_table(void) {         // $008E
	op_add8(cpu.a, 0); I(4);
	if (!(cpu.f & FC)) I(12);
	else { I(7); cpu.h = op_inc8(cpu.h); I(4); }
	word_entry();
}
static void rst20(void) { I(10); word_table(); }
static void word_at(void) { hl_from_table(); I(10); }   // $0097

static void rst28(void) {
	add_a_to_hl();
	cpu.a = mem_read(HL); I(7);
	I(10);
}

static void rst30(void) {
	op_add8(cpu.l, 0); I(4);
	cpu.l = cpu.a; I(4);
	if (!(cpu.f & FC)) { I(11); return; }
	I(5);
	cpu.h = op_inc8(cpu.h); I(4);
	I(10);
}

// switch_on_a ($334A): the table at HL of (key, handler) entries, ended by
// $FF: jump to the handler of the key equal to A (its caller's return
// popped into AF), or return with Z set when there is none
static void switch_on_a(void) {
	for (;;) {
		cpu.b = mem_read(HL); I(7);
		cpu.b = op_inc8(cpu.b); I(4);
		if (cpu.f & FZ) { I(11); return; }
		I(5);
		op_sub8(mem_read(HL), 0, 0); I(7);
		SET_HL(HL + 1); I(6);
		if (cpu.f & FZ) { I(12); break; }
		I(7);
		SET_HL(HL + 1); I(6);
		SET_HL(HL + 1); I(6);
		I(10);
	}
	{ uint16_t af = pop16(); cpu.a = af >> 8; cpu.f = (uint8_t)af; } I(10);
	cpu.a = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	cpu.h = mem_read(HL); I(7);
	cpu.l = cpu.a; I(4);
	I(4);
	native_continue_at(HL);            // jp (hl)
}

#define RST(name, addr, out) Native native_##name = { #name, addr, 0, name, out, .long_calls = 1 };
RST(rst08, 0x0008, OUT_A)
RST(abs_a, 0x000B, OUT_A | OUT_F)
RST(rst10, 0x0010, OUT_A)
RST(rst18, 0x0018, OUT_A | OUT_F | OUT_HL)
RST(rst20, 0x0020, OUT_A | OUT_F | OUT_HL)
RST(rst28, 0x0028, OUT_A | OUT_F | OUT_HL)
RST(rst30, 0x0030, OUT_A | OUT_F | OUT_HL)
RST(read_address, 0x006F, OUT_A)
RST(write_address, 0x0078, OUT_A | OUT_F)
RST(jump_table, 0x0083, OUT_A | OUT_F | OUT_HL)
RST(word_table, 0x008E, OUT_A | OUT_F | OUT_HL)
RST(word_entry, 0x0092, OUT_A | OUT_F | OUT_HL)
RST(word_at, 0x0097, OUT_A | OUT_HL)
RST(switch_on_a, 0x334A, OUT_A | OUT_F | OUT_BC | OUT_HL)
