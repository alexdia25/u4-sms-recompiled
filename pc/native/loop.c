// The main loop ($0CD2) and the task scheduler ($0AE5).
// Each pass: the pads read, the sprite table cleared, then either the
// mode's set-up (the table at $0D55, while bit 7 of $C600 is clear) or a
// turn for each task; then a wait for the pass's frames ($C00C) and the
// next vertical blank, with the Pause button's pause.
// The scheduler gives each busy slot its turn: when the slot's count runs
// out it jumps to the slot's resume address with IX on the slot, and the
// task comes back to $0AF8 when it yields; the C goes on from there.
// Cycle counts are the original's, instruction by instruction (I): the
// machine catches up after each one (native_boundary), so interrupts come
// between the same instructions as in the original. The frame wait spins
// on the frame count that the interrupt changes, so this matters.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static void call(uint16_t addr) { I(17); call_original(addr); }

static uint8_t in_port(uint8_t port) { return io_in((uint16_t)(cpu.a << 8 | port)); }

// $065B: the pads ($C0AE: both, for two players), the keys newly pressed
// ($C0B0), held ($C0AF) and as a direction ($C0B1, the table at $06BD);
// a key held repeats after $C0B4 passes, then every $C0B3 passes
static void update_joypad(void) {
	cpu.a = mem_read(0xC0AE); I(13);
	op_or(cpu.a); I(4);
	if (cpu.f & FZ) {                  // $0646
		I(7); I(17);
		cpu.a = in_port(0xDC); I(11);
		cpu.a = (uint8_t)~cpu.a; I(4);
		op_and(0x3F); I(7);
		I(10); I(10);
	} else {                           // $064C: the second pad's up and down too
		I(12); I(17);
		cpu.a = in_port(0xDC); I(11);
		cpu.b = cpu.a; I(4);
		cpu.a = in_port(0xDD); I(11);
		for (int i = 0; i < 2; i++) {
			cpu.b = op_rot(2, cpu.b); I(8);   // rl b
			uint8_t c = cpu.a >> 7;           // rla
			cpu.a = (uint8_t)(cpu.a << 1 | (cpu.f & FC));
			cpu.f = (uint8_t)((cpu.f & ~(FC | FH | FN)) | c); I(4);
		}
		cpu.a = (uint8_t)~cpu.a; I(4);
		op_and(0x3F); I(7);
		I(10);
	}
	cpu.c = cpu.a; I(4);
	op_and(0x0F); I(7);
	cpu.e = cpu.a; I(4);
	cpu.d = 0; I(7);
	SET_HL(0x06BD); I(10);
	SET_HL(op_add16(HL, DE)); I(11);
	cpu.a = mem_read(HL); I(7);
	mem_write(0xC0B1, cpu.a); I(13);
	cpu.a = mem_read(0xC0AF); I(13);
	cpu.b = cpu.a; I(4);
	cpu.a = (uint8_t)~cpu.a; I(4);
	op_and(cpu.c); I(4);
	mem_write(0xC0B0, cpu.a); I(13);
	cpu.a = cpu.c; I(4);
	mem_write(0xC0AF, cpu.a); I(13);
	SET_HL(0xC0B5); I(10);
	op_add8(mem_read(HL), 0); I(7);    // stirs the random seed
	mem_write(HL, cpu.a); I(7);
	SET_HL(rd16(0xC0B3)); I(16);
	cpu.l = op_dec8(cpu.l); I(4);
	if (cpu.f & FZ) { I(11); return; } // no repeat
	I(5);
	cpu.a = cpu.b; I(4);
	op_sub8(cpu.c, 0, 0); I(4);
	if (!(cpu.f & FZ)) {               // changed: the count starts over
		I(7);
		op_xor(cpu.a); I(4);
		mem_write(0xC0B2, 0); I(13);
		I(10);
		return;
	}
	I(12);
	cpu.a = op_inc8(mem_read(0xC0B2)); I(13); I(4);
	mem_write(0xC0B2, cpu.a); I(13);
	op_sub8(cpu.h, 0, 0); I(4);
	if (cpu.f & FZ) {                  // the first repeat
		I(12);
		cpu.a = cpu.l; I(4);
		mem_write(0xC0B2, cpu.a); I(13);
	} else {
		I(7);
		op_sub8(cpu.l, 0, 0); I(4);
		if (!(cpu.f & FZ)) {
			I(7);
			if (cpu.f & FC) {
				I(7);
				op_xor(cpu.a); I(4);
				mem_write(0xC0B0, 0); I(13);
			} else {
				I(12);
				cpu.a = 0x80; I(7);
				mem_write(0xC0B0, 0x80); I(13);
			}
			I(10);
			return;
		}
		I(12);
	}
	cpu.a = cpu.c; I(4);               // $06B6: a repeat
	op_or(0x80); I(7);
	mem_write(0xC0B0, cpu.a); I(13);
	I(10);
}

static void clear_sprites(void) {      // $0950: every sprite off the screen ($C0)
	I(17);
	SET_HL(0xC180); I(10);
	SET_DE(0xC181); I(10);
	SET_BC(0x003F); I(10);
	mem_write(HL, 0xC0); I(10);
	while (op_block(6, 0)) I(21);
	I(16);
	op_xor(cpu.a); I(4);
	mem_write(0xC064, 0); I(13);
	I(10);
}

// $0CD2: a pass of the main loop, up to the mode's set-up or the tasks
static void main_loop(void) {
	cpu.sp = 0xDFF0; I(10);
	cpu.a = mem_read(0xC00A); I(13);
	mem_write(0xC00B, cpu.a); I(13);   // the frame the pass started in
	SET_HL(rd16(0xC0C8)); I(16);
	SET_HL(HL + 1); I(6);
	wr16(0xC0C8, HL); I(16);
	I(17);
	update_joypad();
	clear_sprites();
	SET_HL(0x0D06); I(10);
	push16(HL); I(11);                 // where the set-up or the tasks return
	cpu.a = mem_read(0xC600); I(13);
	cpu.b = cpu.a; I(4);
	op_add8(cpu.a, 0); I(4);
	if (cpu.f & FC) { I(10); native_continue_at(0x0AE5); return; }   // the mode is running: its tasks
	I(10);
	op_add8(cpu.b, 0); I(4);
	op_add8(0x55, 0); I(7);
	cpu.l = cpu.a; I(4);
	op_add8(0x0D, cpu.f & FC); I(7);
	op_sub8(cpu.l, 0, 1); I(4);
	cpu.h = cpu.a; I(4);               // HL = $0D55 + 3 * the mode
	cpu.a = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	cpu.e = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	cpu.d = mem_read(HL); I(7);
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
	mem_write(0xFFFF, cpu.a); I(13);
	I(4);
	native_continue_at(HL);            // the mode's set-up
}

// $0D06: the rest of the pass: its frames, the vertical blank, a pause
static void pass_end(void) {
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: it waits for frames
	SET_HL(0xC00A); I(10);
	SET_BC(rd16(0xC00B)); I(20);       // C: the frame it started in, B: its frames
	for (;;) {
		cpu.a = mem_read(HL); I(7);
		op_sub8(cpu.c, 0, 1); I(4);
		op_sub8(cpu.b, 0, 0); I(4);
		if (!(cpu.f & FC)) { I(7); break; }
		I(12);
	}
	call(0x041C);                      // wait_vblank
	cpu.a = mem_read(0xC0C6); I(13);
	SET_HL(0xC0C5); I(10);
	op_or(mem_read(HL)); I(7);
	if (cpu.f & FZ) {
		I(12);
		cpu.a = mem_read(0xC003); I(13);
		op_or(cpu.a); I(4);
		if (cpu.f & FZ) { I(10); native_continue_at(0x0CD2); return; }
		I(10);
		cpu.a = 0x1F; I(7);            // paused: the sound held, until Pause again
		mem_write(0xFFFF, 0x1F); I(13);
		call(0x8012);
		cpu.b = 3; I(7);
		call(0x042D);
		op_xor(cpu.a); I(4);
		mem_write(0xC003, 0); I(13);
		do {
			call(0x041C);
			cpu.a = mem_read(0xC003); I(13);
			op_or(cpu.a); I(4);
			I(cpu.f & FZ ? 12 : 7);
		} while (cpu.f & FZ);
		cpu.a = 0x1F; I(7);
		mem_write(0xFFFF, 0x1F); I(13);
		call(0x8015);
		cpu.b = 3; I(7);
		call(0x042D);
		I(12);
	} else I(7);
	op_xor(cpu.a); I(4);               // $0D1E (a pause is not kept while a mode starts)
	mem_write(0xC003, 0); I(13);
	I(10);
	native_continue_at(0x0CD2);
}

// the scheduler from $0AEE: the slots from IX, B of them left
static void next_slots(void) {
	for (;;) {
		cpu.a = mem_read((uint16_t)(cpu.ix + 1)); I(19);
		op_or(cpu.a); I(4);
		if (!(cpu.f & FZ)) {           // busy: its count down ($0B01)
			I(7);
			push16(BC); I(11);
			push16(0x0AF8); I(17);
			mem_write((uint16_t)(cpu.ix + 1), op_dec8(mem_read((uint16_t)(cpu.ix + 1)))); I(23);
			if (cpu.f & FZ) {          // its turn: on at its resume address, its bank paged
				I(5);
				mem_write((uint16_t)(cpu.ix + 1), 1); I(19);
				cpu.l = mem_read((uint16_t)(cpu.ix + 2)); I(19);
				cpu.h = mem_read((uint16_t)(cpu.ix + 3)); I(19);
				cpu.a = mem_read((uint16_t)(cpu.ix + 4)); I(19);
				mem_write(0xFFFF, cpu.a); I(13);
				I(4);
				native_continue_at(HL);
				return;
			}
			pop16(); I(11);            // ret nz
			SET_BC(pop16()); I(10);
			SET_DE(0x0030); I(10);
		} else I(12);
		cpu.ix = op_add16(cpu.ix, DE); I(15);
		if (--cpu.b) { I(13); continue; }
		I(8);
		I(10);              // all done: back to the main loop ($0D06)
		return;
	}
}

static void run_tasks(void) {          // $0AE5
	cpu.ix = 0xC300; I(14);
	SET_DE(0x0030); I(10);
	cpu.b = 0x0A; I(7);
	next_slots();
}

static void task_back(void) {          // $0AF8: a task's turn is over
	SET_BC(pop16()); I(10);
	SET_DE(0x0030); I(10);
	cpu.ix = op_add16(cpu.ix, DE); I(15);
	if (--cpu.b) { I(13); next_slots(); return; }
	I(8);
	I(10);
}

Native native_main_loop = { "main_loop", 0x0CD2, 0, main_loop, 0, .long_calls = 1 };
Native native_pass_end = { "pass_end", 0x0D06, 0, pass_end, 0, .long_calls = 1 };
// spawn_task ($0B33, C = $FF) / spawn_task_c ($0B35): the first free slot
// (its first byte 0) of the ten at $C300 gets kind C, a count of 1, the
// entry DE and the bank in slot 2: IY on it; carry when all are busy
static void spawn_body(void) {
	push16(DE); I(11);
	SET_HL(0xC300); I(10);
	SET_DE(0x0030); I(10);
	cpu.b = 0x0A; I(7);
	op_xor(cpu.a); I(4);
	for (;;) {
		op_sub8(mem_read(HL), 0, 0); I(7);
		if (cpu.f & FZ) { I(12); break; }
		I(7);
		SET_HL(op_add16(HL, DE)); I(11);
		if (--cpu.b) { I(13); continue; }
		I(8);
		SET_DE(pop16()); I(10);
		op_scf(); I(4);
		I(10);
		return;
	}
	SET_DE(pop16()); I(10);
	push16(HL); I(11);
	cpu.iy = pop16(); I(14);
	mem_write(HL, cpu.c); I(7);
	cpu.l = op_inc8(cpu.l); I(4);
	mem_write(HL, 0x01); I(10);
	cpu.l = op_inc8(cpu.l); I(4);
	mem_write(HL, cpu.e); I(7);
	cpu.l = op_inc8(cpu.l); I(4);
	mem_write(HL, cpu.d); I(7);
	cpu.l = op_inc8(cpu.l); I(4);
	cpu.a = mem_read(0xDFFF); I(13);
	mem_write(HL, cpu.a); I(7);
	I(10);
}
static void spawn_task(void) { cpu.c = 0xFF; I(7); spawn_body(); }

static void clear_bytes(uint16_t from, uint16_t n) {   // ld (hl),0 / ldir over n more
	SET_HL(from);
	SET_DE((uint16_t)(from + 1));
	SET_BC(n);
	mem_write(HL, 0x00); I(10);
	while (op_block(6, 0)) I(21);
	I(16);
}

// end_task ($0B5A): the current slot (IX) cleared, its 48 bytes
static void end_task(void) {
	native_drop_slot(cpu.ix);
	push16(cpu.ix); I(15);
	SET_HL(pop16()); I(10);
	I(4 + 4);                          // ld d,h / ld e,l
	op_inc8(cpu.l); I(4);              // inc e: its flags
	I(10);                             // ld bc,$2F
	clear_bytes(cpu.ix, 0x002F);
	I(10);
}

// clear_tasks ($0B21): all ten slots cleared, and $C00C (the main loop's
// wait for frames) 0
static void clear_tasks(void) {
	native_drop_all_slots();
	I(10 + 10 + 10);
	clear_bytes(0xC300, 0x01DF);
	op_xor(cpu.a); I(4);
	mem_write(0xC00C, 0); I(13);
	I(10);
}

// task_enter_N ($0AA5, $0AB5, $0AC5, $0AD5): called first thing by a
// routine that will yield: its own return is kept in the slot (+$24,
// +$26, +$28, +$2A) instead of on the shared stack, and HL = it;
// task_return_N ($0AAE ...) goes back there
static void task_enter(uint8_t at) {
	uint16_t back = pop16(); I(10);    // pop hl
	SET_HL(rd16(cpu.sp));              // ex (sp),hl
	wr16(cpu.sp, back); I(19);
	mem_write((uint16_t)(cpu.ix + at), cpu.l); I(19);
	mem_write((uint16_t)(cpu.ix + at + 1), cpu.h); I(19);
	I(10);                             // ret
}
static void task_return(uint8_t at) {
	cpu.l = mem_read((uint16_t)(cpu.ix + at)); I(19);
	cpu.h = mem_read((uint16_t)(cpu.ix + at + 1)); I(19);
	I(4);
	native_continue_at(HL);            // jp (hl)
}
static void task_enter_1(void) { task_enter(0x24); }
static void task_enter_2(void) { task_enter(0x26); }
static void task_enter_3(void) { task_enter(0x28); }
static void task_enter_4(void) { task_enter(0x2A); }
static void task_return_1(void) { task_return(0x24); }
static void task_return_2(void) { task_return(0x26); }
static void task_return_3(void) { task_return(0x28); }
static void task_return_4(void) { task_return(0x2A); }

Native native_run_tasks = { "run_tasks", 0x0AE5, 0, run_tasks, 0, .long_calls = 1 };
Native native_task_back = { "task_back", 0x0AF8, 0, task_back, 0, .long_calls = 1 };
Native native_update_joypad = { "update_joypad", 0x065B, 0, update_joypad, 0, .long_calls = 1 };
Native native_spawn_task = { "spawn_task", 0x0B33, 0, spawn_task, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL | OUT_IY, .long_calls = 1 };
Native native_spawn_task_c = { "spawn_task_c", 0x0B35, 0, spawn_body, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL | OUT_IY, .long_calls = 1 };
Native native_end_task = { "end_task", 0x0B5A, 0, end_task, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, .long_calls = 1 };
Native native_clear_tasks = { "clear_tasks", 0x0B21, 0, clear_tasks, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, .long_calls = 1 };
Native native_task_enter_1 = { "task_enter_1", 0x0AA5, 0, task_enter_1, OUT_HL, .long_calls = 1 };
Native native_task_return_1 = { "task_return_1", 0x0AAE, 0, task_return_1, OUT_HL, .long_calls = 1 };
Native native_task_enter_2 = { "task_enter_2", 0x0AB5, 0, task_enter_2, OUT_HL, .long_calls = 1 };
Native native_task_return_2 = { "task_return_2", 0x0ABE, 0, task_return_2, OUT_HL, .long_calls = 1 };
Native native_task_enter_3 = { "task_enter_3", 0x0AC5, 0, task_enter_3, OUT_HL, .long_calls = 1 };
Native native_task_return_3 = { "task_return_3", 0x0ACE, 0, task_return_3, OUT_HL, .long_calls = 1 };
Native native_task_enter_4 = { "task_enter_4", 0x0AD5, 0, task_enter_4, OUT_HL, .long_calls = 1 };
Native native_task_return_4 = { "task_return_4", 0x0ADE, 0, task_return_4, OUT_HL, .long_calls = 1 };
