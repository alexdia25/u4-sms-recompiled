#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "native.h"
#include "sms.h"
#include "z80.h"

enum { MODE_ON, MODE_OFF, MODE_SHADOW, MODE_TIMED };
static int mode = -1;
static Native *seen[256];
static void run_blocking(Native *n);
static void run_task(Native *n);
static int jump_set;   // the native chose where to continue (native_continue_at)
static int declined;   // the native left it to the original (native_decline)
static Native *skip_once;   // a native on a fiber declined: let the original run this time

// From a native, before it changes anything: let the original run instead
// (for cases the rewrite doesn't cover)
void native_decline(void) { declined = 1; }
int native_jumped(void) { return jump_set; }

void native_continue_at(uint16_t addr) { cpu.pc = addr; jump_set = 1; }

// after the native routine: return to the caller unless it chose another place
static void finish(void) {
	if (jump_set) jump_set = 0;
	else cpu.pc = pop16();
}
static int nseen;

static int get_mode(void) {
	if (mode < 0) {
		const char *m = getenv("U4_NATIVE");
		mode = !m || !strcmp(m, "on") ? MODE_ON : !strcmp(m, "shadow") ? MODE_SHADOW
			: !strcmp(m, "timed") ? MODE_TIMED : MODE_OFF;
	}
	return mode;
}

// --------------------------------------------------------------- time sink --
// A rewritten routine adds the original's cycle count when it is done, which
// can be many lines or even frames of time. Rather than let that time pass
// with nothing happening, the CPU is parked at PARK_PC and the cycles are
// spent there slice by slice, so interrupts (frame counter, sound, VRAM
// queue) still come at their times, as they would during the original.
static int sinking;
static int64_t sink_left;
static uint16_t sink_pc;

static void spend_overshoot(void) {
	if (cpu.cycles <= cpu.budget) return;
	sink_left = cpu.cycles - cpu.budget;
	cpu.cycles = cpu.budget;
	sink_pc = cpu.pc;
	cpu.pc = PARK_PC;
	sinking = 1;
}

static void sink_step(void) {   // the dispatcher is at PARK_PC with time to spend
	int64_t room = cpu.budget - cpu.cycles;
	int64_t s = sink_left < room ? sink_left : room;
	if (s > 0) { cpu.cycles += s; sink_left -= s; }
	if (sink_left <= 0) { cpu.pc = sink_pc; sinking = 0; }
}

static void track(Native *n) {
	for (int i = 0; i < nseen; i++) if (seen[i] == n) return;
	if (nseen < 256) seen[nseen++] = n;
}

// compare one register: if the original left it alone the native must too;
// if the native declares it as an output it must match
static int reg_ok(unsigned before, unsigned orig, unsigned nat, int is_output) {
	if (orig == before) return nat == before;
	return !is_output || nat == orig;
}

static int task_in_call(void);

int native_call(Native *n) {
	int m = get_mode();
	static const char *trace = (const char *)-1;   // U4_TRACE=<names>: log entry times
	if (trace == (const char *)-1) trace = getenv("U4_TRACE");
	if (trace && strstr(trace, n->name)) fprintf(stderr, "enter %s %lld\n", n->name, (long long)cpu.cycles);
	if (m == MODE_OFF) return 0;
	if (skip_once == n) { skip_once = NULL; return 0; }
	if (n->hook) { n->fn(); return 0; }   // looks, and the original runs
	const char *only = getenv("U4_NATIVE_ONLY");   // test one rewrite at a time
	if (only && !strstr(only, n->name)) return 0;
	n->calls++;
	track(n);
	if (n->task) {             // a C task runs as a whole, or not at all
		if (m == MODE_SHADOW) return 0;
		if (task_in_call()) return 0;   // inside a C task's task_call: the original runs
		run_task(n);
		return 1;
	}
	if (n->long_calls && m != MODE_SHADOW) {
		run_blocking(n);
		return 1;
	}
	if (n->blocking) {
		// waits for frames: can't be compared step by step, so in shadow mode
		// the original runs instead (check these with playthroughs)
		if (m == MODE_SHADOW) return 0;
		run_blocking(n);
		return 1;
	}
	if (m == MODE_ON) {
		n->fn();
		if (declined) { declined = 0; return 0; }
		finish();
		spend_overshoot();
		return 1;
	}
	static Snap *before, *native, *orig;
	if (!before) { before = snap_new(); native = snap_new(); orig = snap_new(); }
	if (m == MODE_TIMED) {
		// for testing: the native's effects with the original's exact timing,
		// measured by running the original on a copy of the machine first
		snap_save(before);
		int64_t start = cpu.cycles;
		n->fn();
		if (declined) { declined = 0; return 0; }
		finish();
		uint16_t end_pc = cpu.pc, end_sp = cpu.sp;
		snap_save(native);
		snap_load(before);
		long steps = 0;
		while (!(cpu.pc == end_pc && cpu.sp == end_sp)) {
			z80_exec();
			if (++steps > 50000000) { fprintf(stderr, "timed: %s never returned\n", n->name); exit(2); }
		}
		int64_t taken = cpu.cycles - start;
		snap_load(native);
		n->orig_cycles += (double)taken;
		n->native_cycles += (double)(cpu.cycles - start);
		cpu.cycles = start + taken;
		spend_overshoot();
		return 1;
	}
	snap_save(before);
	uint16_t sp0 = cpu.sp;
	n->fn();
	if (declined) { declined = 0; return 0; }
	finish();
	uint16_t end_pc = cpu.pc, end_sp = cpu.sp;
	snap_save(native);
	snap_load(before);
	// run the original on the interpreter until it gets where the native did
	// (usually: returned to the caller)
	long steps = 0;
	// U4_PROFILE=<name>: count the original's cycles per instruction address
	static unsigned long prof_n[0x10000]; static double prof_c[0x10000];
	int prof = getenv("U4_PROFILE") && !strcmp(getenv("U4_PROFILE"), n->name);
	while (!(cpu.pc == end_pc && cpu.sp == end_sp)) {
		uint16_t at = cpu.pc; int64_t c0 = cpu.cycles;
		z80_exec();
		if (prof) { prof_n[at]++; prof_c[at] += (double)(cpu.cycles - c0); }
		if (++steps > 50000000) { fprintf(stderr, "shadow: %s never returned\n", n->name); exit(2); }
	}
	if (prof) {
		FILE *pf = fopen("profile.txt", "w");
		for (int i = 0; i < 0x10000; i++)
			if (prof_n[i]) fprintf(pf, "%04X %8lu %10.0f\n", i, prof_n[i], prof_c[i]);
		fclose(pf);
	}
	snap_save(orig);
	const Z80 *b = snap_cpu(before), *o = snap_cpu(orig), *v = snap_cpu(native);
	n->orig_cycles += (double)(o->cycles - b->cycles);
	n->native_cycles += (double)(v->cycles - b->cycles);
	char msg[512];
	// memory below the stack pointer is dead once the routine is done
	uint16_t stack_hi = end_sp > sp0 ? end_sp : sp0;
	int diffs = snap_compare(orig, native, (uint16_t)(sp0 - 0x100), stack_hi, n->scratch, n->nscratch, msg, sizeof msg);
	size_t used = strlen(msg);
#define REG(name, field, flag) \
	if (!reg_ok(b->field, o->field, v->field, (n->outputs & (flag)) != 0)) { \
		diffs++; used += snprintf(msg + used, sizeof msg - used, " %s %X/%X", name, o->field, v->field); }
	REG("A", a, OUT_A) REG("B", b, OUT_BC) REG("C", c, OUT_BC)
	REG("D", d, OUT_DE) REG("E", e, OUT_DE) REG("H", h, OUT_HL) REG("L", l, OUT_HL)
	REG("IX", ix, OUT_IX) REG("IY", iy, OUT_IY)
	// flags are only checked when declared as an output: routines rarely keep
	// them on purpose, and an unchanged F is usually a coincidence
	if ((n->outputs & OUT_F) && o->f != v->f) { diffs++; used += snprintf(msg + used, sizeof msg - used, " F %X/%X", o->f, v->f); }
	if ((n->outputs & OUT_CARRY) && ((o->f ^ v->f) & FC)) { diffs++; used += snprintf(msg + used, sizeof msg - used, " carry %d/%d", o->f & FC, v->f & FC); }
	REG("A'", a_, 0) REG("B'", b_, 0) REG("C'", c_, 0) REG("D'", d_, 0)
	REG("E'", e_, 0) REG("H'", h_, 0) REG("L'", l_, 0)
#undef REG
	// U4_SHADOW_CYCLES=1: the native must also take exactly the original's time
	static int check_cycles = -1;
	if (check_cycles < 0) check_cycles = getenv("U4_SHADOW_CYCLES") != NULL;
	if (check_cycles && o->cycles != v->cycles) {
		diffs++;
		used += snprintf(msg + used, sizeof msg - used, " cycles %lld/%lld",
			(long long)(o->cycles - b->cycles), (long long)(v->cycles - b->cycles));
	}
	if (o->sp != v->sp || o->pc != v->pc) { diffs++; snprintf(msg + used, sizeof msg - used, " SP/PC"); }
	if (diffs) {
		if (n->mismatches++ < 3)
			fprintf(stderr, "shadow: %s ($%04X) call %lu differs:%s\n", n->name, n->addr, n->calls, msg);
	}
	return 1;   // keep the original's result (already in the machine)
}

void native_report(void) {
	if (get_mode() != MODE_SHADOW && !getenv("U4_NATIVE_REPORT")) return;
	for (int i = 0; i < nseen; i++)
		fprintf(stderr, "shadow: %-28s calls %8lu  mismatches %lu  cycles/call original %.0f native %.0f\n",
			seen[i]->name, seen[i]->calls, seen[i]->mismatches,
			seen[i]->orig_cycles / seen[i]->calls, seen[i]->native_cycles / seen[i]->calls);
}

// ------------------------------------------------------------------ fibers --
// Rewritten code that waits runs on a fiber (its own C stack), so it can stop
// in the middle and let the machine go on:
// - a "blocking" native (or one with long_calls) runs on a free fiber of the
//   pool. While it waits - wait_vblank, or an original routine it called -
//   the CPU is parked at PARK_PC: the translated code's dispatcher sees that
//   address and resumes the fiber, and interrupts are taken as usual in
//   between (they return to PARK_PC).
// - a C task (Native.task) runs on the fiber of its task slot (IX). When it
//   waits for its next turn (task_wait_turn), its slot gets the resume
//   address the original would have left there and the CPU goes back to
//   run_tasks; when run_tasks jumps to that address with IX on that slot,
//   the dispatcher resumes the fiber instead. An original routine that
//   yields is called with task_call: the fiber waits the same way, for the
//   routine to return to the call's return address.
// Fibers can park inside each other (a C task calls an original routine that
// uses a blocking native), so parked fibers are kept on a stack.
#define POOL   8
#define SLOTS  10
#define TASK_SLOTS 0xC300
#define FIBER_STACK (256 * 1024)
typedef struct Fiber {
	Native *n;               // what runs on it; NULL when free
	uint16_t wait_pc;        // a C task waiting for its turn: where its slot resumes
	uint16_t wait_sp;        // ... or for an original routine to return there (task_call)
#ifdef _WIN32
	void *h;
#else
	ucontext_t ctx;
	char *stack;
#endif
} Fiber;
static Fiber pool[POOL], task_fiber[SLOTS];
static Fiber *current;       // the fiber running now; NULL: the machine
static Fiber *parked[POOL + SLOTS];   // fibers parked at PARK_PC, innermost last
static int nparked;
int native_tasks_waiting;    // C tasks waiting for their turn

#ifdef _WIN32
#include <windows.h>
static void *main_fiber;
#else
#include <ucontext.h>
static ucontext_t main_ctx;
#endif

int native_parked(void) { return nparked > 0 || sinking; }

static void to_main(void) {
	Fiber *f = current;
	current = NULL;
#ifdef _WIN32
	(void)f;
	SwitchToFiber(main_fiber);
#else
	swapcontext(&f->ctx, &main_ctx);
#endif
}

static void fiber_body(Fiber *f) {
	for (;;) {
		f->n->fn();
		if (declined) {                  // run the original instead, from its start
			declined = 0;
			skip_once = f->n;
			cpu.pc = f->n->addr;
			f->n = NULL;
			to_main();
			continue;
		}
		finish();
		f->n = NULL;
		spend_overshoot();
		to_main();
	}
}

#ifdef _WIN32
static VOID CALLBACK fiber_entry(LPVOID p) { fiber_body((Fiber *)p); }
#else
static Fiber *starting;
static void fiber_entry(void) { fiber_body(starting); }
#endif

// (re)create f's fiber, so it starts at fiber_body
static void make_fiber(Fiber *f) {
#ifdef _WIN32
	if (!main_fiber) main_fiber = ConvertThreadToFiber(NULL);
	if (f->h) DeleteFiber(f->h);
	f->h = CreateFiber(FIBER_STACK, fiber_entry, f);
#else
	if (!f->stack) f->stack = malloc(FIBER_STACK);
	getcontext(&f->ctx);
	f->ctx.uc_stack.ss_sp = f->stack;
	f->ctx.uc_stack.ss_size = FIBER_STACK;
	f->ctx.uc_link = NULL;
	starting = f;
	makecontext(&f->ctx, fiber_entry, 0);
#endif
}

static void switch_to(Fiber *f) {
	current = f;
#ifdef _WIN32
	SwitchToFiber(f->h);
#else
	swapcontext(&main_ctx, &f->ctx);
#endif
}

// park the running fiber at PARK_PC until the machine gets back there
static void park(void) {
	parked[nparked++] = current;
	to_main();
}

// start a blocking native (called instead of n->fn in native_call). A free
// pool fiber is waiting at the end of fiber_body's loop, so it just goes on.
static void run_blocking(Native *n) {
	for (int i = 0; i < POOL; i++)
		if (!pool[i].n) {
#ifdef _WIN32
			if (!pool[i].h) make_fiber(&pool[i]);
#else
			make_fiber(&pool[i]);
#endif
			pool[i].n = n;
			switch_to(&pool[i]);
			return;
		}
	fprintf(stderr, "native: no free fiber for %s\n", n->name);
	exit(2);
}

static int slot_of_ix(void) {
	int off = cpu.ix - TASK_SLOTS;
	if (off < 0 || off % 0x30 || off / 0x30 >= SLOTS) {
		fprintf(stderr, "native: IX %04X is not a task slot\n", cpu.ix);
		exit(2);
	}
	return off / 0x30;
}

// a C task is gone: the game ended or cleared its slot
static void drop_task(Fiber *f) {
	if (f->wait_pc) native_tasks_waiting--;
	f->wait_pc = 0;
	f->n = NULL;
}

// the C task of the current slot waits in task_call for an original
// routine: a C task started there now would take its fiber
static int task_in_call(void) {
	int off = cpu.ix - TASK_SLOTS;
	if (off < 0 || off % 0x30 || off / 0x30 >= SLOTS) return 0;
	Fiber *f = &task_fiber[off / 0x30];
	return f->n && f->wait_pc && f->wait_sp;
}

// start a C task in the current slot (afresh, if one was still there)
static void run_task(Native *n) {
	Fiber *f = &task_fiber[slot_of_ix()];
	drop_task(f);
	make_fiber(f);
	f->n = n;
	switch_to(f);
}

// From a C task: end this turn with yield ($0B19), the slot resuming at
// `resume`, and come back here when run_tasks jumps there with IX on this
// slot (the task's next turn).
void task_wait_turn(uint16_t resume) {
	// the caller has counted yield's own time; yield itself runs on the
	// machine, so that an interrupt due meanwhile comes at the same place
	cpu.cycles -= 58;
	push16(resume);
	cpu.pc = 0x0B19;
	current->wait_pc = resume;
	current->wait_sp = 0;
	native_tasks_waiting++;
	to_main();
}

// From a C task: call an original routine that may wait for frames (yield)
// as "call addr" would from `back`, and come back here when it returns there
// with IX on this slot. Meanwhile the machine runs it as usual.
void task_call(uint16_t addr, uint16_t back) {
	push16(back);
	cpu.pc = addr;
	current->wait_pc = back;
	current->wait_sp = cpu.sp + 2;
	native_tasks_waiting++;
	to_main();
}

// end_task ($0B5A) and clear_tasks ($0B21) clear slots: forget C tasks there
static void hook_end_task(void) {
	int off = cpu.ix - TASK_SLOTS;
	if (off >= 0 && off % 0x30 == 0 && off / 0x30 < SLOTS) drop_task(&task_fiber[off / 0x30]);
}
static void hook_clear_tasks(void) {
	for (int i = 0; i < SLOTS; i++) drop_task(&task_fiber[i]);
}
Native native_hook_end_task = { "hook_end_task", 0x0B5A, 0, hook_end_task, .hook = 1 };
Native native_hook_clear_tasks = { "hook_clear_tasks", 0x0B21, 0, hook_clear_tasks, .hook = 1 };

// the dispatcher reached PARK_PC, or a C task may be waiting for this turn
int native_special(void) {
	if (cpu.pc == PARK_PC) {
		if (sinking) { sink_step(); return 1; }
		if (!nparked) return 0;
		switch_to(parked[--nparked]);
		return 1;
	}
	if (native_tasks_waiting) {
		int off = cpu.ix - TASK_SLOTS;
		if (off >= 0 && off % 0x30 == 0 && off / 0x30 < SLOTS) {
			Fiber *f = &task_fiber[off / 0x30];
			if (f->n && f->wait_pc && f->wait_pc == cpu.pc && (!f->wait_sp || f->wait_sp == cpu.sp)) {
				f->wait_pc = 0;
				native_tasks_waiting--;
				switch_to(f);
				return 1;
			}
		}
	}
	return 0;
}

// Run an original routine from C and come back when it returns. Used by
// rewritten code that still relies on parts not rewritten yet. From a fiber
// it runs in the machine as usual, interrupts included; otherwise on the
// interpreter with interrupts held off until it returns.
void call_original(uint16_t addr) {
	if (current) {
		push16(PARK_PC);
		cpu.pc = addr;
		park();
		return;
	}
	uint16_t sp0 = cpu.sp;
	push16(0xFFFF);            // a return address no code ever runs at
	cpu.pc = addr;
	long steps = 0;
	while (!(cpu.pc == 0xFFFF && cpu.sp == sp0)) {
		z80_exec();
		if (++steps > 50000000) { fprintf(stderr, "call_original($%04X) never returned\n", addr); exit(2); }
	}
}

// wait for the next vertical blank, like wait_vblank ($041C), with its
// timing: the original spins on "cp (hl) / jr z" (19 cycles a turn) until
// the interrupt clears $C008, and the machine only looks at the time (and
// takes the interrupt) after a whole turn.
void wait_vblank(void) {
	cpu.cycles += 11 + 11 + 17;             // push af, push hl, call
	uint8_t a = cpu.a; uint16_t hl = HL;    // the original keeps AF and HL
	call_original(0x01CB);                  // its first step (VDP setup)
	mem_write(0xC008, 0xFF);                // the interrupt clears this
	cpu.cycles += 7 + 10 + 7;
	while (mem_read(0xC008) == 0xFF) {
		int64_t turns = (cpu.budget - cpu.cycles + 18) / 19;
		cpu.cycles += 19 * (turns < 1 ? 1 : turns);
		cpu.pc = PARK_PC;
		park();
	}
	cpu.a = a; SET_HL(hl);
	cpu.cycles += 7 + 7 + 10 + 10 + 10;     // the last check, pops and ret
}
