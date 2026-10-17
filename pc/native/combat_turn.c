// A party member's turn in combat, as a C task (bank 3 $9D3B-$9DA0) with the
// combat menu ($A073). Bank 3 reaches the routines of banks 0 and 1 through
// the vectors at $7E03-$7F77 (a jp each), so a "call vector" here is the
// call plus the jp. The commands are original code still: the C task hands
// over to them; they end the turn at $9DA0, or come back to the prompt at
// $9D6E. Cycle counts are the original's.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define SLOT        cpu.ix
#define MEMBER      0xC97F          // whose turn it is
#define JOY         0xC0B0
#define TURN_OVER   0x9DA0
#define MAP_MODE    0xC99F
#define ENTRY_DIRECTION 0xC9B9

static void call(uint16_t addr) { cpu.cycles += 17; call_original(addr); }

// call $9DF0: IY = the member's record
static void member_record_iy(void) {
	cpu.cycles += 17 + 11 + 20;
	uint16_t bc = BC;
	SET_BC(rd16(MEMBER));
	call(0x7E96);
	SET_BC(bc);
	cpu.cycles += 10 + 10;
}

// call $9DE3: the status window, the bar on the member
static void show_member(void) {
	cpu.cycles += 17;
	call(0x7F29);
	cpu.cycles += 20;
	SET_BC(rd16(MEMBER));
	call(0x7E66);
	cpu.cycles += 10;                  // jp wait_vblank
	call_original(0x041C);
}

// "call frame_tick" through its vector at site
static void frame_tick_v(uint16_t site) {
	cpu.cycles += 10;
	task_frame_tick(site);
}

static void prompt(void);

// ------------------------------------------------------------- moving --
// The battlefield is 11 x 11; its tiles are at $D900 + 2 * (11 * y + x).

static uint8_t times_11(uint8_t a) {   // call $B856
	cpu.cycles += 17 + 11 + 4 * 7 + 10 + 10;
	return (uint8_t)(a * 11);
}

static void cell_index(void) {         // call $A46F: L = the cell's offset
	cpu.cycles += 17 + 4;
	uint8_t a = times_11(cpu.h);
	cpu.cycles += 4 + 4 + 4 + 10;
	cpu.a = (uint8_t)(2 * (uint8_t)(a + cpu.l));
	cpu.l = cpu.a;
}

static uint8_t cell_tile(void) {       // call $A457: A = the tile at (L, H)
	uint16_t hl = HL;
	cpu.cycles += 17 + 11;
	cell_index();
	cpu.cycles += 7 + 7 + 10 + 10;
	cpu.a = mem_read(0xD900 | cpu.l);
	SET_HL(hl);
	return cpu.a;
}

static void turn_over(void) { cpu.cycles += 10; native_continue_at(TURN_OVER); }   // jp $9DA0

// $A3B7: off the edge of the battlefield: the member flees. Fleeing at full
// health from foes that don't attack first costs karma.
static void flee(void) {
	cpu.cycles += 13 + 7 + 7;
	if (mem_read(MAP_MODE) == 5) {     // a dungeon room: only by the way in
		cpu.cycles += 10 + 7 + 4 + 13 + 10;
		SET_HL(0xC9BA);
		uint8_t way_in = mem_read(HL);
		cpu.a = mem_read(ENTRY_DIRECTION);
		if (!(way_in & 0x80)) {
			cpu.cycles += 7 + 7;
			if (cpu.a != way_in) {
				cpu.cycles += 7;
				cpu.a = 0x10;
				call(0x05F1);
				cpu.cycles += 10;
				SET_HL(0xAC83);
				call(0x7F62);          // "Not that way!"
				turn_over();
				return;
			}
			cpu.cycles += 5;
		}
		cpu.cycles += 7;
		mem_write(HL, cpu.a);
	} else cpu.cycles += 5;
	call(0x7E8A);                      // $A3DB: the figure leaves
	cpu.cycles += 19 + 19 + 19 + 7;
	mem_write(cpu.iy + 0, 0x80);
	mem_write(cpu.iy + 1, 0x80);
	mem_write(cpu.iy + 2, 0x00);
	cpu.a = 0x1B;
	call(0x05F1);
	cpu.cycles += 13 + 7 + 7;
	if (mem_read(MAP_MODE) == 3) {
		cpu.cycles += 13;
		cpu.a = mem_read(0xC97D);      // the foe
		call(0x7E75);                  // carry: harmless
		cpu.cycles += 7;
		if (cpu.f & FC) {
			cpu.cycles += 15;
			push16(cpu.iy);
			member_record_iy();
			cpu.cycles += 19 + 19 + 19 + 19 + 4 + 15 + 14 + 7;
			uint16_t hp = rd16(cpu.iy + 0), max = rd16(cpu.iy + 2);
			SET_HL((uint16_t)(max - hp));
			SET_DE(hp);
			cpu.iy = pop16();
			if (max == hp) {
				cpu.cycles += 7 + 7;
				cpu.a = 2; cpu.c = 2;
				call(0x7F2F);          // karma_lose: compassion
				cpu.cycles += 7 + 7;
				cpu.a = 2; cpu.c = 4;
				call(0x7F2F);          // valour
			} else cpu.cycles += 5;
		} else cpu.cycles += 5;
	} else cpu.cycles += 5;
	call(0x7F6E);
	turn_over();
}

// $A316: one step in direction dir (0 west, 1 north, 2 east, 3 south)
static void combat_move(uint8_t dir) {
	cpu.cycles += 13;
	mem_write(ENTRY_DIRECTION, dir);
	cpu.a = dir;
	call(0x7F68);                      // "North" etc.
	call(0x7E8A);                      // IY = the member's figure
	cpu.cycles += 13 + 17 + 10 + 100;  // $40B9 by its vector: HL = the step
	cpu.a = dir;
	SET_HL(rd16(0x40BF + 2 * dir));
	cpu.cycles += 19 + 4 + 4 + 7 + 10;
	cpu.a = cpu.l = (uint8_t)(mem_read(cpu.iy + 0) + cpu.l);
	if (cpu.l >= 0x0B) { flee(); return; }
	cpu.cycles += 19 + 4 + 4 + 7 + 10;
	cpu.a = cpu.h = (uint8_t)(mem_read(cpu.iy + 1) + cpu.h);
	if (cpu.h >= 0x0B) { flee(); return; }
	cell_tile();
	call(0x7E99);                      // carry: can't walk there
	cpu.cycles += 7;
	if (cpu.f & FC) {
		cpu.cycles += 7;
		cpu.a = ' ';
		call(0x7E33);
		call(0x7E7E);                  // "Blocked!"
		turn_over();
		return;
	}
	cpu.cycles += 5;
	call(0x7F47);                      // carry: someone stands there
	cpu.cycles += 7;
	if (cpu.f & FC) {
		cpu.cycles += 7;
		cpu.a = ' ';
		call(0x7E33);
		call(0x7F1D);
		turn_over();
		return;
	}
	cpu.cycles += 5 + 19 + 19 + 7;     // $A35C: step
	mem_write(cpu.iy + 0, cpu.l);
	mem_write(cpu.iy + 1, cpu.h);
	cpu.a = 0x0A;
	call(0x05F1);
	call(0x7F6E);
	cpu.cycles += 13 + 7 + 10;
	if (mem_read(MAP_MODE) != 5) { native_continue_at(TURN_OVER); return; }
	// a dungeon room: stepping on a trigger changes tiles ($C8E1, 4 x 7 bytes)
	cpu.cycles += 14 + 10 + 7;
	SET_DE(0x0007);
	cpu.b = 4;
	for (uint16_t t = 0xC8E1; ; t += 7) {
		cpu.iy = t;
		cpu.cycles += 19 + 4 + 4 + 7;
		cpu.a = cpu.c = mem_read(t);
		op_or(cpu.a);
		int hit = 0;
		if (cpu.a) {
			cpu.cycles += 4 + 19 + 7;
			cpu.a = cpu.l;
			op_sub8(mem_read(t + 2), 0, 0);
			if (cpu.l == mem_read(t + 2)) {
				cpu.cycles += 4 + 19 + 7;
				cpu.a = cpu.h;
				op_sub8(mem_read(t + 1), 0, 0);
				if (cpu.h == mem_read(t + 1)) hit = 1;
				else cpu.cycles += 5;
			} else cpu.cycles += 5;
		} else cpu.cycles += 5;
		if (hit) {
			cpu.cycles += 11;
			uint16_t at = HL;
			push16(at);
			for (int k = 4; k <= 6; k += 2) {   // two cells it changes
				cpu.cycles += 19 + 19 + 4 + 4 + 7;
				cpu.l = mem_read(t + k);
				cpu.h = mem_read(t + k - 1);
				cpu.a = cpu.l | cpu.h;
				op_or(cpu.h);
				cpu.a = cpu.l | cpu.h;
				if (!cpu.a) { cpu.cycles += 5; continue; }
				cpu.cycles += 17;      // call $A469
				cell_index();
				cpu.cycles += 7 + 10;
				cpu.h = 0xD8;
				call(0x7EE4);
			}
			cpu.cycles += 10;
			SET_HL(pop16());
		}
		cpu.cycles += 15;
		if (--cpu.b == 0) { cpu.cycles += 8; break; }
		cpu.cycles += 13;
	}
	turn_over();
}

// $A073: the combat menu. Its caller's return address (back) is kept in the
// slot (task_enter_1). Cancelled: "Aborted!" and carry; else the command.
static void combat_menu(uint16_t back) {
	cpu.cycles += 17 + 17 + 77;
	wr16(SLOT + 0x24, back);
	SET_HL(back);
	call(0x7F17);                      // click
	cpu.cycles += 10;
	SET_HL(0xAC70);
	call(0x7F62);                      // "Command-"
	cpu.cycles += 10;
	SET_HL(0x0B0A);
	call(0x06DA);
	cpu.cycles += 10;
	SET_HL(0xACBD);
	call(0x7E60);                      // the menu
	cpu.cycles += 19;
	mem_write(SLOT + 0x1A, 0);
	for (;;) {
		frame_tick_v(0xA08F);
		cpu.cycles += 7 + 10;
		cpu.c = 6;
		SET_DE(0x0102);
		call(0x7EB1);                  // menu_step_wrap
		cpu.cycles += 7;
		if (!(cpu.f & FZ)) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 7;
	if (cpu.f & FC) {                  // $A0C0: cancelled
		cpu.cycles += 5 + 10;
		SET_HL(0xAC79);
		call(0x7F62);                  // "Aborted!"
		cpu.cycles += 4 + 12 + 11;     // scf / jr $A0CD / push af
		cpu.f |= FC;
		uint8_t a = cpu.a, f = cpu.f;
		push16((uint16_t)(a << 8 | f));
		call(0x06D4);
		show_member();
		pop16();
		cpu.a = a; cpu.f = f;
		cpu.cycles += 10 + 10 + 42;    // pop af / jp task_return_1
		SET_HL(back);
		cpu.cycles += 7 + 5;           // $9D9E: jr c,$9D6E
		prompt();
		return;
	}
	cpu.cycles += 19 + 10 + 91;        // the command's name
	uint8_t cmd = mem_read(SLOT + 0x1A);
	SET_HL(rd16(0xACB1 + 2 * cmd));
	call(0x7F62);
	call(0x7F6E);
	cpu.cycles += 19 + 10 + 10 + 10 + 52;   // ld a / ld hl / jp $0018 / jp $0083: the command (original)
	cpu.a = cmd;
	SET_HL(rd16(0xA0B4 + 2 * cmd));
	native_continue_at(HL);
}

// $9D6E: ">" and wait: a direction moves, button 1 passes (original code),
// button 2 opens the menu
static void prompt(void) {
	call(0x7E8A);                      // IY = the member's figure
	cpu.cycles += 19 + 19 + 19 + 19;
	mem_write(SLOT + 0x1E, mem_read(cpu.iy + 0));   // where it stands
	mem_write(SLOT + 0x1F, mem_read(cpu.iy + 1));
	SET_HL(rd16(cpu.iy));
	member_record_iy();
	cpu.cycles += 19 + 19 + 7;
	mem_write(SLOT + 0x07, mem_read(cpu.iy + 0x0B));   // the weapon
	cpu.a = 0x7F;
	call(0x7E33);                      // ">"
	static const uint8_t keys[5] = { 0x01, 0x02, 0x04, 0x08, 0x10 };
	for (;;) {
		frame_tick_v(0x9D8B);
		cpu.cycles += 13 + 10 + 17 + 10;   // ld a,(joy) / ld hl / call switch_on_a by its vector
		uint8_t joy = mem_read(JOY);
		SET_HL(0xA2F5);
		cpu.a = joy;
		for (int i = 0; i < 5; i++)
			if (keys[i] == joy) {      // move or pass
				cpu.cycles += 58 * i + 79;
				cpu.a = 0x9D; cpu.f = 0x97;   // (the switch pops its return address into AF)
				cpu.b = (uint8_t)(joy + 1);
				SET_HL(rd16(0xA2F6 + 3 * i));
				if (i == 4) {          // $A305: pass
					call(0x7E27);
					turn_over();
					return;
				}
				// $A30B north / $A30E south / $A311 west / $A314 east: each
				// "ld a,n" and skips the next ones as "ld bc,..."
				static const uint8_t dirs[4] = { 1, 3, 0, 2 };
				cpu.cycles += 7 + 10 * (3 - i);
				if (i < 3) SET_BC(0x023E);
				combat_move(dirs[i]);
				return;
			}
		cpu.cycles += 58 * 5 + 22 + 7 + 7;
		cpu.b = 0;
		SET_HL(0xA304);
		op_and(0x20);
		if (cpu.a) break;
		cpu.cycles += 5;
	}
	combat_menu(0x9D9E);
}

// $9D3B: the member's turn, unless they are gone from the field or can't act
static void member_turn(void) {
	call(0x7E8A);
	cpu.cycles += 19 + 4 + 7;
	cpu.a = mem_read(cpu.iy + 2);
	op_or(cpu.a);
	if (!cpu.a) { cpu.cycles += 5; native_continue_at(TURN_OVER); return; }
	cpu.cycles += 20;
	SET_BC(rd16(MEMBER));
	call(0x7ECF);                      // member_able
	cpu.cycles += 7;
	if (cpu.f & FC) { cpu.cycles += 5; native_continue_at(TURN_OVER); return; }
	show_member();
	call(0x7F6E);
	cpu.cycles += 20;
	SET_BC(rd16(MEMBER));
	call(0x7ECC);                      // the name
	cpu.cycles += 10;
	SET_HL(0xAC4C);
	call(0x7F62);                      // ", armed with"
	member_record_iy();
	cpu.cycles += 19 + 7;
	cpu.a = (uint8_t)(mem_read(cpu.iy + 0x0B) + 0x25);
	call(0x7F6B);                      // the weapon's name
	call(0x7F6E);
	prompt();
}

Native native_combat_turn = { "combat_turn", 0x9D3B, 3, member_turn, 0, 0, 0, .task = 1 };
Native native_combat_prompt = { "combat_prompt", 0x9D6E, 3, prompt, 0, 0, 0, .task = 1 };
