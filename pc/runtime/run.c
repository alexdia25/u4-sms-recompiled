// The machine's loop: the C rewrites of pc/native take over at their addresses
// (the table pc/gen/natives.c, from tools/hints.txt). Anything else would be
// original code the rewrites don't reach: the interpreter runs it and counts
// it (machine_interpreted, shown by --headless, and tests/run.sh wants it 0).
#include <stdlib.h>
#include "z80.h"
#include "sms.h"
#include "native.h"

uint64_t machine_interpreted;
void (*machine_on_interpreted)(uint16_t pc);

static Native **rewrite[32];           // by bank, then address in the 16K bank

static void build_table(void) {
	for (int i = 0; i < native_table_size; i++) {
		const NativeEntry *e = &native_table[i];
		if (!rewrite[e->bank]) rewrite[e->bank] = calloc(0x4000, sizeof(Native *));
		rewrite[e->bank][e->addr & 0x3FFF] = e->native;
	}
}

// Interrupts are taken here, between time slices. Anything that can raise an
// interrupt mid-slice (EI, VDP port access) sets cpu.budget = cpu.cycles so that
// the loop comes back here after the current instruction.
void machine_run(void) {
	static int built;
	if (!built) { build_table(); built = 1; }
	const int64_t end = cpu.budget;
	for (;;) {
		cpu.budget = end;
		if (cpu.halted) {
			if (!z80_interrupt()) { if (cpu.cycles < end) cpu.cycles = end; return; }
		} else if (cpu.ei_delay) {
			// the instruction after EI runs before an interrupt is accepted
			cpu.ei_delay = 0;
			if (cpu.irq_line || cpu.nmi_pending) cpu.budget = cpu.cycles + 1;
		} else z80_interrupt();
		while (cpu.cycles < cpu.budget) {
			if ((cpu.pc >= 0xFFF0 || native_tasks_waiting) && native_special()) continue;   // rewritten code waiting
			int bank = rom_bank_at(cpu.pc);
			if (bank >= 0 && bank < 32 && rewrite[bank] && (cpu.pc >> 14) == (bank == 0 ? 0 : bank == 1 ? 1 : 2)) {
				Native *n = rewrite[bank][cpu.pc & 0x3FFF];
				if (n && native_call(n)) continue;
			}
			if (machine_on_interpreted) machine_on_interpreted(cpu.pc);
			z80_exec();
			machine_interpreted++;
			if (cpu.halted) break;
		}
		if (cpu.cycles >= end) break;
	}
	cpu.budget = end;
}
