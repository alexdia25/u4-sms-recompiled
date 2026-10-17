// Rewritten ("native") routines and the shadow tester that checks them.
//
// A native routine replaces one Z80 routine of the original game. It is called
// in place of the routine's first instruction, works on the same machine
// state (cpu registers, memory through mem_read/mem_write, ports) and returns
// like the original would; the caller's return address is popped for it.
//
// U4_NATIVE selects how they are used:
//   on      (default) run the native routine
//   off     ignore it and run the translated original
//   shadow  run the native routine, then rewind the machine, run the original
//           on the interpreter, compare the two results, and keep the
//           original's. Every mismatch is reported once per routine.
#ifndef U4_NATIVE_H
#define U4_NATIVE_H

#include <stdint.h>

// registers a native routine sets for its caller (other registers it may
// change freely, unless the original leaves them untouched)
enum {
	OUT_A = 1 << 0, OUT_F = 1 << 1, OUT_BC = 1 << 2, OUT_DE = 1 << 3, OUT_HL = 1 << 4,
	OUT_IX = 1 << 5, OUT_IY = 1 << 6,
	OUT_CARRY = 1 << 7,   // only the carry flag of F matters to callers
};

typedef struct Native {
	const char *name;
	uint16_t addr;          // CPU address of the original routine
	int bank;               // its bank
	void (*fn)(void);
	unsigned outputs;       // OUT_* flags
	const uint16_t *scratch;  // RAM the original uses as scratch (not compared)
	int nscratch;
	// statistics (shadow mode)
	unsigned long calls, mismatches;
	double orig_cycles, native_cycles;   // totals, to tune the natives' timing
	int blocking;           // may wait for frames (wait_vblank): runs on a fiber
	int long_calls;         // calls long original routines: runs on a fiber when
	                        // on (so interrupts happen during them), directly in shadow mode
	int task;               // a task step that goes on over several turns (task_wait_turn)
	int hook;               // only looks: fn runs, then the original
} Native;

// called by the generated code at the routine's entry; 1 = handled
int native_call(Native *n);
void native_report(void);   // print shadow-test statistics

// instead of returning to the caller, continue at addr (stack as left)
void native_continue_at(uint16_t addr);

// before changing anything: leave this call to the original routine
void native_decline(void);

// whether native_continue_at was called (the native will continue there)
int native_jumped(void);

// For blocking natives: wait for the next vertical blank. While waiting the
// CPU is parked at PARK_PC.
#define PARK_PC 0xFFF8
void wait_vblank(void);
int native_parked(void);

// For C tasks: end this turn (like yield, the slot resuming at `resume`) and
// continue on the task's next one.
void task_wait_turn(uint16_t resume);
void task_call(uint16_t addr, uint16_t back);   // an original routine that may yield, called from back

// the dispatcher calls this at PARK_PC, or anywhere while a C task waits;
// 1 = handled (the CPU has moved on)
int native_special(void);
extern int native_tasks_waiting;

// call an original (not yet rewritten) routine from C. From a native on a
// fiber it runs in the machine as usual, interrupts included; otherwise on
// the interpreter with interrupts held until it returns.
void call_original(uint16_t addr);

#endif
