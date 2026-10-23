// The interrupt handler ($0038): the VDP's status says which.
// - A line interrupt ($040B) sets the horizontal scroll for the coming line
//   from the table at $D000 + the line.
// - The frame interrupt ($02C0): the frame count, the pad and the reset
//   button, the display on or off ($C007), and when the frame is ready
//   ($C008) the scroll, the sprite tables, the palette ($C009), the queue
//   of VRAM copies ($C200, $C063 of them), the frame's own routine ($C00D);
//   then the sound ($1F:8000).
// The queue lets line interrupts in after its VDP address writes (ei), so
// line interrupts can come in the middle of it. Cycle counts are the
// original's; the C lets the machine catch up after each instruction that
// touches the VDP (native_sync), so lines end and interrupts come where
// the original's would.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

static void sync(void) { native_boundary(); }

static void out_bf(uint8_t v) {        // out ($BF),a
	io_out((uint16_t)(cpu.a << 8 | 0xBF), v);
	cpu.cycles += 11;
	sync();
}

static void ld_a_out(uint8_t v) {      // ld a,v / out ($BF),a
	cpu.cycles += 7;
	cpu.a = v;
	out_bf(v);
}

static void ei_then(int cycles) {      // ei, and the instruction after it (its cycles)
	cpu.iff1 = cpu.iff2 = 1;
	cpu.cycles += 4 + cycles;
	native_irq_point();
}

static void di(void) { cpu.iff1 = cpu.iff2 = 0; cpu.cycles += 4; }

static void outi(void) {
	op_block(4, 3);
	cpu.cycles += 16;
	sync();
}

static void outi_loop(void) {          // outi / jr nz,$-2
	for (;;) {
		outi();
		if (cpu.f & FZ) { cpu.cycles += 7; return; }
		cpu.cycles += 12;
	}
}

static void outi_run(uint16_t from, int n) {   // ld hl,from / call into the run of outi
	cpu.cycles += 10 + 17;
	SET_HL(from);
	for (int i = 0; i < n; i++) outi();
	cpu.cycles += 10;
}

static void djnz_wait(void) {          // djnz $ (B turns)
	do cpu.b--; while (cpu.b && (cpu.cycles += 13));
	cpu.cycles += 8;
	sync();
}

static void ex_de_hl(void) { uint16_t t = HL; SET_HL(DE); SET_DE(t); cpu.cycles += 4; }

static void pop_af(void) { uint16_t v = pop16(); cpu.a = (uint8_t)(v >> 8); cpu.f = (uint8_t)v; }

// $040B (AF pushed): the coming line's scroll
static void line_interrupt(void) {
	cpu.cycles += 11 + 11 + 4 + 7 + 7;
	push16(HL);
	cpu.a = io_in((uint16_t)(cpu.a << 8 | 0x7E));
	cpu.l = cpu.a; cpu.h = 0xD0;
	cpu.a = mem_read(HL);
	out_bf(cpu.a);
	ld_a_out(0x88);
	cpu.cycles += 10 + 10 + 4 + 10;
	SET_HL(pop16());
	pop_af();
	cpu.iff1 = cpu.iff2 = 1;           // ei / ret
}

// $037D: the VRAM copies queued at $C200, 8 bytes each. Bit 7 of the
// first byte clear: from that bank, the VDP address (+1), from (+3), A (+6)
// runs of B (+5) bytes. Set: B (+6) rows of A (+5) bytes, $40 apart in both,
// the VDP address (+3), from (+1).
static void vram_queue(void) {
	cpu.cycles += 10;
	SET_HL(0xC200);
	for (;;) {
		cpu.cycles += 4 + 7 + 4 + 10;
		op_ex_af();
		cpu.a = mem_read(HL);
		op_or(cpu.a);
		if (!(cpu.f & FS)) {
			cpu.cycles += 13 + 4 + 7 + 4;
			mem_write(0xFFFF, cpu.a);
			cpu.l++;
			cpu.a = mem_read(HL);
			cpu.l++;
			di();
			out_bf(cpu.a);
			cpu.cycles += 7;
			cpu.a = mem_read(HL);
			out_bf(cpu.a);
			cpu.l++;
			ei_then(4);
			cpu.cycles += 7 + 4 + 7 + 4 + 7 + 4 + 7 + 4 + 4;
			cpu.e = mem_read(HL); cpu.l++;
			cpu.d = mem_read(HL); cpu.l++;
			cpu.b = mem_read(HL); cpu.l++;
			cpu.a = mem_read(HL); cpu.l++;
			cpu.l++;
			ex_de_hl();
			do {
				outi_loop();
				cpu.cycles += 4 + 10;
				cpu.a = op_dec8(cpu.a);
			} while (!(cpu.f & FZ));
			ex_de_hl();
			cpu.cycles += 4 + 4 + 10;
			op_ex_af();
			cpu.a = op_dec8(cpu.a);
			if (!(cpu.f & FZ)) continue;
			cpu.cycles += 10;
			break;
		}
		cpu.cycles += 4 + 7 + 4 + 7 + 4 + 11 + 7 + 4 + 7 + 4 + 7 + 4 + 7 + 4 + 4 + 19;
		cpu.l++;
		cpu.e = mem_read(HL); cpu.l++;
		cpu.d = mem_read(HL); cpu.l++;
		push16(DE);
		cpu.e = mem_read(HL); cpu.l++;
		cpu.d = mem_read(HL); cpu.l++;
		cpu.a = mem_read(HL); cpu.l++;
		cpu.b = mem_read(HL); cpu.l++;
		cpu.l++;
		{ uint16_t t = rd16(cpu.sp); wr16(cpu.sp, HL); SET_HL(t); }   // ex (sp),hl
		ex_de_hl();
		do {                           // $03C0: a row
			cpu.cycles += 11 + 11 + 4 + 4;
			push16(BC); push16(HL);
			cpu.b = cpu.a;
			cpu.c++;
			di();
			io_out(BC, cpu.e); cpu.cycles += 12; sync();
			io_out(BC, cpu.d); cpu.cycles += 12; sync();
			cpu.c--;
			ei_then(4);
			outi_loop();
			cpu.cycles += 10 + 7 + 11 + 4 + 11 + 4 + 10;
			SET_HL(pop16());
			cpu.c = 0x40;
			SET_HL(op_add16(HL, BC));
			SET_DE(op_add16(DE, BC));
			SET_BC(pop16());
			if (--cpu.b) cpu.cycles += 13; else cpu.cycles += 8;
		} while (cpu.b);
		cpu.cycles += 10 + 4 + 4 + 10;
		SET_HL(pop16());
		op_ex_af();
		cpu.a = op_dec8(cpu.a);
		if (cpu.f & FZ) break;
	}
	cpu.cycles += 13;                  // $03DF
	mem_write(0xC063, cpu.a);
}

static void frame_interrupt(void) {
	cpu.cycles += 13 + 11 * 3 + 4 + 4 + 11 * 4 + 15 + 15;
	mem_write(0xC012, cpu.a);
	push16(BC); push16(DE); push16(HL);
	op_exx(); op_ex_af();
	push16((uint16_t)(cpu.a << 8 | cpu.f)); push16(BC); push16(DE); push16(HL);
	push16(cpu.ix); push16(cpu.iy);
	cpu.cycles += 7 + 10 + 7 + 13 + 4 + 11 + 10;
	cpu.c = 0xBE;
	SET_HL(0xFFFC);
	cpu.e = mem_read(0xFFFC);
	cpu.a = mem_read(0xDFFF);
	cpu.d = cpu.a;
	push16(DE);                        // the banks, kept
	mem_write(0xFFFC, 0x80);
	cpu.cycles += 10 + 11 + 13 + 4 + 11 + 13 + 4 + 4 + 8 + 10;
	SET_HL(0xC00A);
	mem_write(0xC00A, op_inc8(mem_read(0xC00A)));   // the frame count
	cpu.b = mem_read(0xC006);
	cpu.a = io_in((uint16_t)(cpu.a << 8 | 0xDD));
	mem_write(0xC006, cpu.a);
	cpu.a = (uint8_t)~cpu.a;
	op_and(cpu.b);
	op_bit(4, cpu.a);
	if (!(cpu.f & FZ)) { native_continue_at(0x0CB4); return; }   // reset, newly pressed
	cpu.cycles += 13 + 4 + 10;
	cpu.a = mem_read(0xC007);
	op_or(cpu.a);
	if (cpu.f & FS) {                  // the display on (bits 0-6 not 0) or off
		cpu.cycles += 7 + 13 + 10;
		op_and(0x7F);
		mem_write(0xC007, cpu.a);
		SET_HL(0xC014);
		if (cpu.f & FZ) { cpu.cycles += 12 + 15; mem_write(HL, mem_read(HL) & ~0x40); }
		else { cpu.cycles += 7 + 15 + 10; mem_write(HL, mem_read(HL) | 0x40); }
		cpu.cycles += 7;
		uint8_t f = cpu.f;
		cpu.a = mem_read(HL);
		out_bf(cpu.a);
		ld_a_out(0x81);
		cpu.f = f;
	}
	cpu.cycles += 10;                  // $0310
	if (!(cpu.f & FZ)) {
		cpu.cycles += 13 + 4 + 10;
		cpu.a = mem_read(0xC008);
		op_or(cpu.a);
	}
	if (!(cpu.f & FZ)) {               // the frame is ready
		cpu.cycles += 4 + 13 + 16 + 16 + 4;
		op_xor(cpu.a);
		mem_write(0xC008, 0);
		SET_HL(rd16(0xC00F));
		wr16(0xC01B, HL);              // the scroll
		cpu.a = cpu.l;
		out_bf(cpu.a);
		ld_a_out(0x88);
		cpu.cycles += 4;
		cpu.a = cpu.h;
		out_bf(cpu.a);
		ld_a_out(0x89);
		ld_a_out(0x00);
		ld_a_out(0x7F);
		outi_run(0xC180, 64);          // the sprites' Y
		ld_a_out(0x80);
		ld_a_out(0x7F);
		outi_run(0xC100, 128);         // their X and tiles
		cpu.cycles += 13 + 4;
		cpu.a = mem_read(0xC009);
		op_or(cpu.a);
		if (cpu.f & FZ) cpu.cycles += 12;
		else {                         // the palette, after a wait
			cpu.cycles += 7 + 4 + 13 + 7;
			op_xor(cpu.a);
			mem_write(0xC009, 0);
			cpu.b = 0x70;
			djnz_wait();
			cpu.cycles += 13 + 4;
			cpu.a = mem_read(0xC001);
			op_or(cpu.a);
			if (cpu.f & FZ) {
				cpu.cycles += 7 + 7;
				cpu.b = 0x80;
				djnz_wait();
				djnz_wait();
			} else cpu.cycles += 12;
			ld_a_out(0x00);
			ld_a_out(0xC0);
			outi_run(0xC023, 32);
		}
		cpu.cycles += 13 + 4 + 10;     // $0376
		cpu.a = mem_read(0xC063);
		op_or(cpu.a);
		if (!(cpu.f & FZ)) vram_queue();
		cpu.cycles += 10 + 11 + 16 + 4;   // $03E2: the frame's own routine, back at $03EA
		SET_HL(rd16(0xC00D));
		call_original(HL);
	}
	cpu.cycles += 7 + 13 + 17;         // $03EA: the sound
	cpu.a = 0x1F;
	mem_write(0xFFFF, 0x1F);
	call_original(0x8000);
	cpu.cycles += 10 + 4 + 13 + 4 + 13 + 14 + 14 + 40 + 4 + 4 + 40 + 4 + 10;
	SET_DE(pop16());
	cpu.a = cpu.e;
	mem_write(0xFFFC, cpu.e);
	cpu.a = cpu.d;
	mem_write(0xFFFF, cpu.d);
	cpu.iy = pop16(); cpu.ix = pop16();
	SET_HL(pop16()); SET_DE(pop16()); SET_BC(pop16());
	pop_af();
	op_ex_af(); op_exx();
	SET_HL(pop16()); SET_DE(pop16()); SET_BC(pop16());
	pop_af();
	cpu.iff1 = cpu.iff2 = 1;           // ei / ret
}

static void interrupt(void) {          // $0038
	cpu.cycles += 11 + 11 + 4 + 10;
	push16((uint16_t)(cpu.a << 8 | cpu.f));
	cpu.a = io_in((uint16_t)(cpu.a << 8 | 0xBF));
	op_or(cpu.a);
	if (cpu.f & FS) { cpu.cycles += 10; frame_interrupt(); }
	else line_interrupt();
	if (native_jumped()) return;
	// after ei and ret: an interrupt pending is taken at once
	if (cpu.irq_line && cpu.cycles < cpu.budget) cpu.budget = cpu.cycles;
}
Native native_interrupt = { "interrupt", 0x0038, 0, interrupt, 0, .long_calls = 1 };
