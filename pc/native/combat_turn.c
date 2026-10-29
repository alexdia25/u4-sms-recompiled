// A party member's turn in combat, as a C task (bank 3 $9D3B-$9DA0) with the
// combat menu ($A073). Bank 3 reaches the routines of banks 0 and 1 through
// the vectors at $7E03-$7F77 (a jp each), so a "call vector" here is the
// call plus the jp. Original routines that wait for frames are called with
// task_call (tcall). Casting is a C task of its own, so the menu hands over
// to it; it comes back to the prompt at $9D6E, or ends the turn at $9DA0. Cycle counts are the original's.
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
static void combat_attack(void);

// call an original routine that may wait for frames, from `site`
static void tcall(uint16_t addr, uint16_t site) {
	if (window_anim_call(addr, site)) return;
	cpu.cycles += 17;
	task_call(addr, (uint16_t)(site + 3));
}

// "call vec_print_inline" with the text after it
static void say_inline_v(uint16_t text) {
	uint16_t hl = HL;
	cpu.cycles += 17 + 10 + 19 + 17;
	text_print(text);
	cpu.cycles += 19 + 10;
	SET_HL(hl);
}

// a command's end: $A0C0 "Aborted!", $A0C6 cancelled (carry), $A0C9 a new
// line, $A0CC done; then back in the turn at $9D9E: cancelled, the prompt
// again, else the turn is over
enum { END_ABORTED, END_CANCELLED, END_NEWLINE, END_DONE, END_AS_IS };
static void command_end(int how) {
	if (how == END_ABORTED) {
		cpu.cycles += 10;
		SET_HL(0xAC79);
		call(0x7F62);                  // "Aborted!"
	}
	if (how <= END_CANCELLED) {
		cpu.cycles += 4 + 12;          // scf / jr $A0CD
		cpu.f |= FC;
	} else if (how != END_AS_IS) {     // ($A0CD keeps the command's carry)
		if (how == END_NEWLINE) call(0x7F6E);
		cpu.cycles += 4;               // or a
		op_or(cpu.a);
	}
	cpu.cycles += 11;                  // push af
	uint8_t a = cpu.a, f = cpu.f;
	push16((uint16_t)(a << 8 | f));
	call(0x06D4);
	show_member();
	pop16();
	cpu.a = a; cpu.f = f;
	cpu.cycles += 10 + 10 + 42;        // pop af / jp task_return_1
	SET_HL(rd16(SLOT + 0x24));
	cpu.cycles += 7;                   // $9D9E: jr c,$9D6E
	if (f & FC) { cpu.cycles += 5; prompt(); }
	else native_continue_at(TURN_OVER);
}

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
		cpu.cycles += 5;
		command_end(END_ABORTED);
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
	switch (cmd) {
	case 0: combat_attack(); break;
	case 1: native_continue_at(HL); break;   // Cast: cast_spell is a C task of its own
	case 2:                            // $A2DA: Get Chest
		tcall(0x7F29, 0xA2DA);
		tcall(0x7F20, 0xA2DD);
		cpu.cycles += 10;
		command_end(END_DONE);
		break;
	case 3:                            // $A2E3: Ready Weapon
		cpu.cycles += 10;              // call $7EDE: jp $57D7
		ready_thing(0, 0xA2E3);
		cpu.cycles += 10;
		command_end(END_AS_IS);
		break;
	case 4:                            // $A2E9: Use Item
		cpu.cycles += 10;              // call $7E2D: jp $651E
		if (use_things(0xA2E9)) return;
		cpu.cycles += 10;
		command_end(END_DONE);
		break;
	default:                           // $A2EF: Status
		cpu.cycles += 10;              // call $7E2A: jp $7588
		status_pages(0xA2EF);
		cpu.cycles += 10;
		command_end(END_DONE);
		break;
	}
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

// ------------------------------------------------------------- attack --
#define FOES        0xC83E          // 16 foes, 8 bytes each: +0 the graphic (0 = none)
#define WEAPON      (SLOT + 0x07)
#define RANGE       (SLOT + 0x1B)
#define TARGET      (SLOT + 0x08)

// $A24F: carry, after "Nothing to Attack!", when no foe is left
static int no_foes(void) {
	cpu.cycles += 17 + 10 + 10 + 7 + 4;
	uint8_t any = 0;
	for (int i = 0; i < 16; i++) {
		any |= mem_read(FOES + 8 * i);
		cpu.cycles += 7 + 11 + (i < 15 ? 13 : 8);
	}
	SET_HL(FOES + 0x80);
	SET_DE(0x0008);
	cpu.b = 0;
	cpu.a = any;
	op_or(any);
	if (any) { cpu.cycles += 11; return 0; }
	cpu.cycles += 5;
	call(0x7F11);
	say_inline_v(0xA263);              // "Nothing to Attack!"
	cpu.cycles += 4 + 10;
	cpu.f |= FC;
	return 1;
}

static void miss(uint16_t then, uint16_t site) {   // "Miss", then $A219 or $A21C
	cpu.cycles += 10;
	SET_HL(0xACA8);
	call(0x7F62);
	tcall(then, site);
	cpu.cycles += 10;                  // jp $A0C9
	command_end(END_NEWLINE);
}

// $A221 (called at site): after a blow, by the weapon: flaming oil
// ($09) sets the foe's cell on fire ($46 $82) unless it is one of the
// first three tiles; the magic axe ($0B) flies back to the member, a
// frame a step (IX+$16 steps; its return kept by task_enter_2)
static void weapon_after(uint16_t site) {
	cpu.cycles += 17 + 19 + 7;
	cpu.a = mem_read(WEAPON);
	op_sub8(0x0B, 0, 0);
	if (cpu.f & FZ) {                  // $A23B: the axe comes back
		cpu.cycles += 12 + 17 + 10 + 19 + 19 + 19 + 10;
		wr16((uint16_t)(cpu.sp - 2), 0xA23E);   // what ex (sp),hl leaves on the stack
		wr16((uint16_t)(cpu.sp - 4), 0xA23E);
		wr16(SLOT + 0x26, (uint16_t)(site + 3));
		SET_HL((uint16_t)(site + 3));
		do {
			call(0x0BC9);              // slot_step_sub
			call(0xA482);              // figure_flip
			task_next_pass(0xA244);     // a frame
			cpu.cycles += 23;
			mem_write(SLOT + 0x16, op_dec8(mem_read(SLOT + 0x16)));
			cpu.cycles += mem_read(SLOT + 0x16) ? 12 : 7;
		} while (mem_read(SLOT + 0x16));
		cpu.cycles += 10 + 19 + 19 + 4;   // jp task_return_2
		SET_HL(rd16(SLOT + 0x26));
		return;
	}
	cpu.cycles += 7 + 7;
	op_sub8(0x09, 0, 0);
	if (!(cpu.f & FZ)) { cpu.cycles += 11; return; }
	cpu.cycles += 5;
	call(0xA43E);
	call(0xA469);                      // HL = the foe's cell
	cpu.cycles += 7 + 7;
	cpu.a = mem_read(HL);
	op_sub8(0x03, 0, 0);
	if (cpu.f & FC) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 10 + 4 + 10 + 10;
	mem_write(HL, 0x46);
	cpu.l = op_inc8(cpu.l);
	mem_write(HL, 0x82);
}

// combat_attack ($A0D8): aim at a foe; hand weapons strike next to the
// member (the pattern at $A2A2, or $A2B3 for the halberd), the others fly
// up to their range. A hit: dexterity decides, then damage by weapon and
// strength.
static void combat_attack(void) {
	if (no_foes()) { cpu.cycles += 10; command_end(END_CANCELLED); return; }
	cpu.cycles += 10;
	cpu.cycles += 10;                  // call $7E06: jp $37DD, "Aim:" at a foe
	aim_at(0xA0DE);
	cpu.cycles += 10;
	if (cpu.f & FC) { command_end(END_ABORTED); return; }
	cpu.cycles += 19 + 10 + 48 + 19 + 4 + 7;
	uint8_t weapon = mem_read(WEAPON);
	SET_HL(0xA292 + weapon);
	uint8_t range = mem_read(HL);
	mem_write(RANGE, range);
	cpu.a = range;
	op_or(range);
	int thrown = 0;
	if (range) {
		cpu.cycles += 19 + 7 + 7;
		cpu.a = weapon;
		op_sub8(2, 0, 0);
		if (weapon != 2) { cpu.cycles += 5; thrown = 1; }
		else {                         // a dagger: thrown unless the foe is next to the member
			cpu.cycles += 10;
			SET_HL(0xA2A2);
			call(0xA279);              // target_in_pattern
			cpu.cycles += 7;
			if (cpu.f & FC) { cpu.cycles += 5; thrown = 1; }
		}
	} else cpu.cycles += 5;
	if (!thrown) {                     // $A100: next to the member?
		cpu.cycles += 19 + 7 + 10 + 7;
		cpu.a = weapon;
		op_sub8(0x0A, 0, 0);
		SET_HL(0xA2A2);
		if (weapon == 0x0A) { cpu.cycles += 10; SET_HL(0xA2B3); }
		else cpu.cycles += 5;
		call(0xA279);
		cpu.cycles += 7;
		if (cpu.f & FC) {              // too far
			cpu.cycles += 7;
			cpu.a = 0x19;
			call(0x05F1);
			cpu.cycles += 10;
			SET_HL(0xACA8);
			call(0x7F62);              // "Miss"
			cpu.cycles += 10;
			command_end(END_NEWLINE);
			return;
		}
		cpu.cycles += 5 + 7;           // $A120: the blow
		cpu.a = 6;
		tcall(0x7EF6, 0xA122);
		cpu.cycles += 19 + 19 + 19;
		mem_write(SLOT + 0x09, 8);
		cpu.l = mem_read(SLOT + 0x20);
		cpu.h = mem_read(SLOT + 0x21);
		call(0xA477);
		cpu.cycles += 19 + 19 + 7;
		mem_write(SLOT + 0x0B, cpu.l);
		mem_write(SLOT + 0x0D, cpu.h);
		cpu.a = 0x18;
		call(0x05F1);
		cpu.cycles += 19;
		mem_write(SLOT + 0x16, 5);
		for (;;) {
			tcall(0xA482, 0xA141);
			task_next_pass(0xA144);
			cpu.cycles += 23 + 7;
			uint8_t left = (uint8_t)(mem_read(SLOT + 0x16) - 1);
			mem_write(SLOT + 0x16, left);
			if (!left) break;
			cpu.cycles += 5;
		}
		cpu.cycles += 10;              // jp $A1BA
	} else {
		cpu.cycles += 19 + 7 + 7;      // $A14F: daggers and oil are used up
		cpu.a = weapon;
		if (weapon == 2) cpu.cycles += 5;
		else cpu.cycles += 7 + 7 + (weapon == 9 ? 0 : 5);
		if (weapon == 2 || weapon == 9) {
			cpu.cycles += 10 + 30 + 7 + 4 + 7;
			SET_HL(0xC91F + weapon);
			uint8_t spare = mem_read(HL);
			cpu.a = spare;
			op_or(spare);
			if (spare) { cpu.cycles += 11 + 12; mem_write(HL, spare - 1); }
			else {                     // the last one: the hands are empty
				cpu.cycles += 5;
				member_record_iy();
				cpu.cycles += 19 + 10;
				mem_write(cpu.iy + 0x0B, 0);
				SET_HL(0xAC9D);
				call(0x7F62);          // "Last one! "
			}
		}
		cpu.cycles += 19 + 7 + 7 + 7;  // $A172: the missile's graphic
		cpu.a = 6;
		if (mem_read(WEAPON) == 0x0E) { cpu.cycles += 7; cpu.a = 9; }
		else cpu.cycles += 5;
		cpu.cycles += 19 + 19 + 19 + 19;
		SET_HL(rd16(SLOT + 0x1E));
		SET_DE(rd16(SLOT + 0x20));
		tcall(0x7EBD, 0xA189);
		cpu.cycles += 19;
		mem_write(SLOT + 0x16, 0);
		for (;;) {                     // $A190: it flies
			cpu.cycles += 23;
			mem_write(SLOT + 0x16, mem_read(SLOT + 0x16) + 1);
			cpu.cycles += 10;          // call $7EBA: jp $392F
			missile_step_at(0xA193);
			tcall(0x7E24, 0xA196);     // carry clear: a foe there
			cpu.cycles += 7;
			if (!(cpu.f & FC)) { cpu.cycles += 5; break; }
			tcall(0xA460, 0xA19B);
			tcall(0x7EB7, 0xA19E);     // carry: it hits something
			cpu.cycles += 7;
			if (!(cpu.f & FC)) {
				cpu.cycles += 19 + 19 + 7;
				cpu.a = mem_read(SLOT + 0x16);
				op_sub8(mem_read(RANGE), 0, 0);
				if (cpu.f & FC) { cpu.cycles += 5; continue; }
			} else cpu.cycles += 5;
			miss(0xA219, 0xA1B1);      // $A1AB
			return;
		}
		cpu.cycles += 19;              // $A1B7: the foe it reached
		mem_write(TARGET, cpu.a);
	}
	cpu.cycles += 13 + 7 + 7;          // $A1BA: in the Abyss only magic weapons
	if (mem_read(0xC9A0) == 0x19) {
		cpu.cycles += 19 + 7 + 7;
		cpu.a = mem_read(WEAPON);
		op_sub8(0x0B, 0, 0);
		if (cpu.f & FC) { cpu.cycles += 5; miss(0xA21C, 0xA213); return; }
	} else cpu.cycles += 5;
	member_record_iy();                // $A1C8: dexterity
	cpu.cycles += 19 + 7 + 7;
	uint8_t dex = mem_read(cpu.iy + 7);
	cpu.a = dex;
	op_sub8(0x28, 0, 0);
	if (dex < 0x28) {
		cpu.cycles += 7 + 4;
		cpu.b = (uint8_t)(dex + 0x80);
		call(0x7ED8);                  // random
		cpu.cycles += 4 + 7;
		op_sub8(cpu.b, 0, 0);
		if (!(cpu.f & FC)) { cpu.cycles += 5; miss(0xA21C, 0xA213); return; }
	} else cpu.cycles += 5;
	cpu.cycles += 10;                  // $A1DB: a hit
	SET_HL(0xACAD);
	call(0x7F62);                      // "Hit"
	cpu.cycles += 10;                  // call $7F1A: jp $3A91
	burst_at(0xA1E1);
	member_record_iy();
	cpu.cycles += 19 + 10 + 48 + 19 + 7;
	SET_HL(0xAC34 + mem_read(cpu.iy + 0x0B));
	unsigned dmg = mem_read(HL) + mem_read(cpu.iy + 6);   // the weapon's damage + strength
	if (dmg > 0xFF) { cpu.cycles += 7; dmg = 0xFF; }
	else cpu.cycles += 5;
	cpu.cycles += 11 + 13 + 4 + 19 + 10;
	cpu.b = mem_read(MEMBER);
	cpu.c = mem_read(TARGET);
	cpu.a = (uint8_t)dmg;
	call(0x7EDB);                      // random_below
	tcall(0x7E36, 0xA201);             // damage_monster
	tcall(0x7F4D, 0xA204);
	weapon_after(0xA207);
	cpu.cycles += 10;
	command_end(END_NEWLINE);
}

// ---------------------------------------------------------- foes' turn --
// foes_turn ($A493): after the party, each of the 16 foes in turn, one per
// frame: asleep ones may wake, the energy-draining kind ($EC) negates magic,
// the phantom kind ($DC) may blink elsewhere; each picks the nearest member
// who can act and goes for them (foe_acts); then the field it
// stands on may hurt it (fire, poison) or put it to sleep. Called with its
// return kept in the slot (task_enter_2).

static void yield_at(uint16_t site) {  // "call yield" at site: the next turn goes on after it
	cpu.cycles += 17 + 58;
	SET_HL((uint16_t)(site + 3));
	task_wait_turn((uint16_t)(site + 3));
}

// $ABB7 (called at site): member C struck: its figure ($7E90) blinks
// three frames at its place, pictures 4, 5, 6 ($A48A) with the screen's
// moving parts ($7F4D) each; the return kept at IX+$14 meanwhile
static void member_struck(uint16_t site) {
	cpu.cycles += 17 + 10 + 19 + 19;   // call / pop hl / ld (ix+$14),l / ld (ix+$15),h
	wr16(SLOT + 0x14, (uint16_t)(site + 3));
	SET_HL((uint16_t)(site + 3));
	tcall(0x7E90, 0xABBE);
	cpu.cycles += 19 + 19;
	SET_HL(rd16(cpu.iy));
	tcall(0xA477, 0xABC7);
	cpu.cycles += 19 + 19;
	mem_write(SLOT + 0x0B, cpu.l);
	mem_write(SLOT + 0x0D, cpu.h);
	for (int i = 0; i < 3; i++) {
		uint16_t at = (uint16_t)(0xABD0 + 11 * i);
		cpu.cycles += 7;
		cpu.a = (uint8_t)(4 + i);
		tcall(0xA48A, (uint16_t)(at + 2));
		tcall(0x7F4D, (uint16_t)(at + 5));
		yield_at((uint16_t)(at + 8));
	}
	cpu.cycles += 19 + 19 + 4;         // jp (hl): back
	SET_HL(rd16(SLOT + 0x14));
}

static uint8_t random_v(void) {        // call random by its vector
	call(0x7ED8);
	return cpu.a;
}

// $A58D: the foe takes up to 127 damage (fire, poison)
static void foe_burns(void) {
	cpu.cycles += 7;
	cpu.a = 0x14;
	call(0x05F1);
	cpu.cycles += 7 + 19;
	cpu.b = 0xFF;
	cpu.c = mem_read(SLOT + 0x1B);
	random_v();
	cpu.cycles += 7;
	op_and(0x7F);
	tcall(0x7E36, 0xA59C);             // damage_monster
	cpu.cycles += 12;
}

// ------------------------------------------------------- a foe's action --
#define TARGET_MEMBER (SLOT + 0x1C)     // the member it goes for
#define DISTANCE      (SLOT + 0x1D)
#define FOE_INDEX     (SLOT + 0x1B)
#define STEP_X        (SLOT + 0x20)
#define STEP_Y        (SLOT + 0x21)
#define EFFECT        0xC976            // $4A jinx, $4E negate, $50 protection

static uint8_t sign_of(uint8_t a) {    // call $ABF8: -1, 0 or 1
	cpu.cycles += 17 + 4;
	if (!a) { cpu.cycles += 11; cpu.a = 0; return 0; }
	cpu.cycles += 5 + 7;
	if (!(a & 0x80)) { cpu.cycles += 11; cpu.a = 1; return 1; }
	cpu.cycles += 5 + 7 + 10;
	cpu.a = 0xFF;
	return 0xFF;
}

// ------------------------------------------------------------ the shots --
// A blow shown on the field ($A7B0), a foe's missile flying at a member
// ($A888) and a member hit ($A802). The blow and the missile wait for
// frames with their return kept in the slot ($0AD5, back by $0ADE). The
// shot is a sprite: its position is 16-bit fixed point in the slot (+$0A/+$0B
// x, +$0C/+$0D y), moved by +$0E..+$11 each frame.

#define SHOT_KIND   (SLOT + 0x22)
#define HIT_MEMBER  (SLOT + 0x08)

static void keep_return(uint16_t back) {   // call $0AD5
	cpu.cycles += 17 + 10 + 19 + 19 + 19 + 10;
	mem_write(SLOT + 0x2A, (uint8_t)back);
	mem_write(SLOT + 0x2B, (uint8_t)(back >> 8));
	SET_HL(back);
}

static void kept_return(void) {        // jp $0ADE
	cpu.cycles += 10 + 19 + 19 + 4;
	SET_HL(rd16(SLOT + 0x2A));
}

static uint8_t times_12(uint8_t a) {   // call $B860
	cpu.cycles += 17 + 11 + 4 * 5 + 10 + 10;
	return (uint8_t)(a * 12);
}

static void shot_flips(void) {         // call $A482: the sprite's other frame, drawn
	cpu.cycles += 17 + 19 + 7 + 19 + 19 + 19 + 10;
	cpu.a = mem_read(SLOT + 0x09);
	op_xor(1);
	mem_write(SLOT + 0x09, cpu.a);
	cpu.e = mem_read(SLOT + 0x0B);
	cpu.d = mem_read(SLOT + 0x0D);
	call_original(0x7F26);
}

static void shot_cell(void) {          // call $A43E: L, H = the cell under the shot
	cpu.cycles += 17 + 19 + 7 + 4 + 7;
	cpu.a = mem_read(SLOT + 0x0D);
	op_add8(6, 0);
	cpu.l = cpu.a;
	cpu.h = 0x0C;
	call(0x7E3C);
	cpu.cycles += 4 + 19 + 7 + 4 + 7;
	cpu.b = cpu.l;
	cpu.a = mem_read(SLOT + 0x0B);
	op_add8(6, 0);
	cpu.l = cpu.a;
	cpu.h = 0x0C;
	call(0x7E3C);
	cpu.cycles += 4 + 10;
	cpu.h = cpu.b;
}

static void shot_moves(int back) {     // call $0BA4 (or $0BC9: back)
	cpu.cycles += 17 + 12 * 19 + 10;
	for (int i = 0; i < 4; i += 2) {
		uint16_t at = rd16(SLOT + 0x0A + i), by = rd16(SLOT + 0x0E + i);
		uint32_t r = back ? (uint32_t)at - by : (uint32_t)at + by;
		mem_write(SLOT + 0x0A + i, (uint8_t)r);
		mem_write(SLOT + 0x0B + i, (uint8_t)(r >> 8));
		uint8_t hi = mem_read(SLOT + 0x0F + i), carry;
		uint8_t ahi = (uint8_t)(at >> 8);
		cpu.a = ahi;
		if (back) { carry = (uint8_t)at < (uint8_t)by; op_sub8(hi, carry, 1); }
		else { carry = (uint8_t)((uint8_t)at + (uint8_t)by) < (uint8_t)at; op_add8(hi, carry); }
	}
}

static uint8_t field_tile(void) {      // call $A460: A = the tile at (L, H) on the field's map
	uint16_t hl = HL;
	cpu.cycles += 17 + 11;
	cell_index();
	cpu.cycles += 7 + 7 + 10 + 10;
	cpu.a = mem_read(0xD800 | cpu.l);
	SET_HL(hl);
	return cpu.a;
}

// call $A7DA: carry clear and A = the member standing at (L, H), if any
static void member_at(void) {
	cpu.cycles += 17 + 11 + 15 + 13 + 4 + 7;
	uint16_t bc = BC, iy = cpu.iy;
	uint8_t n = mem_read(0xC957);
	cpu.a = n; cpu.b = n; cpu.c = 0;
	for (;;) {
		call(0x7ED2);                  // can't act: not there
		cpu.cycles += 7;
		if (!(cpu.f & FC)) {
			call(0x7E90);
			cpu.cycles += 4 + 19 + 7;
			cpu.a = cpu.l;
			op_sub8(mem_read(cpu.iy + 0), 0, 0);
			if (cpu.f & FZ) {
				cpu.cycles += 4 + 19 + 7;
				cpu.a = cpu.h;
				op_sub8(mem_read(cpu.iy + 1), 0, 0);
				if (cpu.f & FZ) { cpu.cycles += 4 + 12; cpu.a = cpu.c; break; }
			}
		}
		cpu.cycles += 5 + 4;
		cpu.c++;
		if (--cpu.b) cpu.cycles += 13;
		else { cpu.cycles += 8 + 4; cpu.f |= FC; break; }
	}
	cpu.cycles += 14 + 10 + 10;
	cpu.iy = iy;
	SET_BC(bc);
}

static void pushw(uint16_t v, int c) { cpu.cycles += c; push16(v); }

// $A802: member C hit by foe B: damage by the foe's strength; killed?
static void member_hit(void) {
	cpu.cycles += 17;
	pushw(DE, 11); pushw(HL, 11); pushw(cpu.iy, 15);
	cpu.cycles += 7;
	cpu.a = 0x12;
	call(0x05F1);                      // the sound
	call(0x7E66);
	call(0x041C);
	tcall(0x7F4D, 0xA811);
	pushw(BC, 11);
	cpu.cycles += 4;
	cpu.c = cpu.b;
	call(0x7E8D);                      // IY = the foe
	cpu.cycles += 19;
	cpu.a = mem_read(cpu.iy + 5);
	call(0xA42A);                      // its kind
	cpu.cycles += 10 + 48 + 8;         // ld hl,$AC00 / rst $28 / srl a
	SET_HL(0xAC00 + cpu.a);
	cpu.a = op_rot(7, mem_read(HL));
	call(0x7EDB);                      // a random number below it
	cpu.cycles += 4 + 4 * 4 + 7 + 4 + 7;
	cpu.d = cpu.a;
	for (int i = 0; i < 4; i++) op_rrca();
	op_and(0x0F);
	cpu.l = cpu.a; cpu.h = 0x0A;
	call(0x7EC3);
	cpu.cycles += 4 + 4 + 7;
	cpu.e = cpu.l; cpu.l = cpu.d; cpu.h = 0x0A;
	call(0x7E3C);                      // in decimal: tens * 10 + ones
	cpu.cycles += 4 + 4 + 4 + 7;
	cpu.a = cpu.h;
	op_add8(cpu.e, 0);
	cpu.l = cpu.a; cpu.h = 0;
	cpu.cycles += 10;
	SET_BC(pop16());
	tcall(0x7E63, 0xA842);             // the damage
	tcall(0x7F38, 0xA845);             // carry: killed
	cpu.cycles += 7;
	if (cpu.f & FC) {
		call(0x7E96);
		cpu.cycles += 15 + 10 + 10 + 11;
		SET_DE(0x000D);
		SET_HL(op_add16(cpu.iy, 0x000D));
		tcall(0x7F62, 0xA854);         // the name
		say_inline_v(0xA85A);          // " is Killed!"
		cpu.cycles += 7 + 7;
		cpu.a = 1; cpu.c = 4;
		call(0x7E00);                  // karma
		tcall(0x7EC6, 0xA86E);
		cpu.cycles += 4;
		op_scf();
	} else cpu.cycles += 5;
	cpu.cycles += 14 + 10 + 10 + 10;
	cpu.iy = pop16();
	SET_HL(pop16());
	SET_DE(pop16());
}

// $A7B0: a blow at (L, H) with sound B and graphic C
static void blow(uint16_t site) {
	cpu.cycles += 17 + 4;
	cpu.a = cpu.b;
	call(0x05F1);
	cpu.cycles += 4;
	cpu.a = cpu.c;
	tcall(0x7EF6, 0xA7B5);
	cpu.cycles += 17 + 4 + 4 + 4 + 4 + 10;   // $A477: the cell to pixels
	uint8_t l = times_12(cpu.l);
	cpu.l = l;
	cpu.a = cpu.h;
	cpu.h = times_12(cpu.h);
	cpu.a = cpu.h;
	cpu.cycles += 19 + 19 + 19;
	mem_write(SLOT + 0x0B, cpu.l);
	mem_write(SLOT + 0x0D, cpu.h);
	mem_write(SLOT + 0x09, 8);
	keep_return((uint16_t)(site + 3));
	cpu.cycles += 19;
	mem_write(SLOT + 0x19, 5);
	for (;;) {
		shot_flips();
		task_next_pass(0xA7CF);
		cpu.cycles += 23 + 7;
		uint8_t n = (uint8_t)(mem_read(SLOT + 0x19) - 1);
		mem_write(SLOT + 0x19, n);
		op_dec8((uint8_t)(n + 1));
		if (!n) break;
		cpu.cycles += 5;
	}
	kept_return();
}

// $A888: foe B's missile (kind D, graphic E) at member C
static void missile(uint16_t site) {
	cpu.cycles += 17;
	keep_return((uint16_t)(site + 3));
	cpu.cycles += 19 + 19;
	mem_write(SHOT_KIND, cpu.d);
	mem_write(SLOT + 0x23, cpu.e);
	call(0x7E90);                      // IY = the member
	cpu.cycles += 19 + 19 + 4;
	cpu.e = mem_read(cpu.iy + 0);
	cpu.d = mem_read(cpu.iy + 1);
	cpu.c = cpu.b;
	call(0x7E8D);                      // IY = the foe
	cpu.cycles += 19 + 19 + 19;
	cpu.l = mem_read(cpu.iy + 0);
	cpu.h = mem_read(cpu.iy + 1);
	cpu.a = mem_read(SLOT + 0x23);
	tcall(0x7EBD, 0xA8A7);             // the shot, from the foe to the member
	for (;;) {
		shot_moves(0);
		shot_flips();
		task_next_pass(0xA8B0);
		shot_cell();
		member_at();
		cpu.cycles += 7;
		if (!(cpu.f & FC)) break;      // it hits a member
		field_tile();
		tcall(0x7EB7, 0xA8BE);         // carry: something stops it
		cpu.cycles += 7;
		if (!(cpu.f & FC)) { cpu.cycles += 5; continue; }
		cpu.cycles += 19 + 7 + 7;
		cpu.a = mem_read(SHOT_KIND);
		op_sub8(0x4C, 0, 0);
		if (cpu.a != 0x4C) { cpu.cycles += 5; goto end; }
		shot_moves(1);                 // lava: a pool of it where it stops
		shot_cell();
		cell_index();
		cpu.cycles += 17 + 7 + 10 + 10 + 4 + 10 + 12;
		cpu.h = 0xD8;
		mem_write(HL, 0x4C);
		cpu.l++;
		mem_write(HL, 0x89);
		goto end;
	}
	cpu.cycles += 5 + 19 + 4;          // $A8DA
	mem_write(HIT_MEMBER, cpu.a);
	cpu.c = cpu.a;
	tcall(0x7F6E, 0xA8DE);
	tcall(0x7ECC, 0xA8E1);
	cpu.cycles += 7;
	cpu.a = 0x20;
	tcall(0x7E33, 0xA8E6);
	cpu.cycles += 19 + 10 + 17 + 10;   // ld a / ld hl / call switch_on_a by its vector
	{
		static const uint8_t kinds[6] = { 0x44, 0x45, 0x46, 0x47, 0x4C, 0x4E };
		uint8_t kind = mem_read(SHOT_KIND);
		int i;
		for (i = 0; i < 6 && kinds[i] != kind; i++) {}
		cpu.a = kind;
		if (i == 6) {
			cpu.cycles += 58 * 6 + 22;
			cpu.b = 0;
			SET_HL(0xA922);
			say_inline_v(0xA8F5);      // "Hit!"
		} else {
			cpu.cycles += 58 * i + 79;
			cpu.b = (uint8_t)(kind + 1);
			SET_HL(rd16(0xA911 + 3 * i));
			cpu.a = 0xA8; cpu.f = 0xF2;   // (the switch pops its return address into AF)
			static const uint16_t text[6] = { 0xA926, 0xA954, 0xA968, 0xA97A, 0xA9B1, 0xA9C2 };
			say_inline_v(text[i]);
			if (kind == 0x44 || kind == 0x47) {   // poison or sleep: half the time, if well
				cpu.cycles += 19;
				cpu.c = mem_read(HIT_MEMBER);
				cpu.cycles += 17 + 7;      // $A877: the sound, the bar, the damage
				cpu.a = 0x12;
				call(0x05F1);
				call(0x7E66);
				call(0x041C);
				tcall(0x7F4D, 0xA882);
				cpu.cycles += 10;
				task_call(0x7E63, kind == 0x44 ? 0xA937 : 0xA988);
				random_v();
				cpu.cycles += 7 + (kind == 0x44 ? 10 : 7);   // and 1 / jp z or jr z
				op_and(1);
				int failed = 1;
				if (cpu.a) {
					call(0x7E96);
					cpu.cycles += 19 + 7 + (kind == 0x44 ? 10 : 7);
					cpu.a = mem_read(cpu.iy + 0x1C);
					op_sub8('G', 0, 0);
					failed = cpu.a != 'G';
				}
				if (failed) {
					if (kind == 0x47) cpu.cycles += 5;   // the jr taken
					say_inline_v(0xA9D6);      // $A9D3: "Failed."
					cpu.cycles += 10;
				} else if (kind == 0x44) {
					cpu.cycles += 19 + 10;
					mem_write(cpu.iy + 0x1C, 'P');
				} else {
					cpu.cycles += 19 + 19;
					mem_write(cpu.iy + 0x1C, 'S');
					cpu.c = mem_read(HIT_MEMBER);
					call(0x7E90);
					cpu.cycles += 19 + 19 + 10;
					mem_write(cpu.iy + 2, 0x38);
					mem_write(cpu.iy + 3, 0x38);
				}
				goto end;
			}
			cpu.cycles += 10;          // jp $A8FB
		}
	}
	cpu.cycles += 19;                  // $A8FB: the damage
	cpu.c = mem_read(HIT_MEMBER);
	member_struck(0xA8FE);
	cpu.cycles += 19 + 19;
	cpu.b = mem_read(FOE_INDEX);
	cpu.c = mem_read(HIT_MEMBER);
	member_hit();
end:
	tcall(0x7F29, 0xA90A);             // $A90A
	kept_return();
}

// $A74E: it strikes at the member: armour may turn the blow
static void foe_strikes(void) {
	cpu.cycles += 19;
	cpu.c = mem_read(TARGET_MEMBER);
	call(0x7E90);
	cpu.cycles += 19 + 19 + 10;
	cpu.l = mem_read(cpu.iy + 0);
	cpu.h = mem_read(cpu.iy + 1);
	SET_BC(0x1806);
	blow(0xA75D);                      // the blow, shown
	cpu.cycles += 13 + 7 + 7;
	if (mem_read(EFFECT) == 0x50) {    // protection: half the blows miss
		random_v();
		cpu.cycles += 7 + 7;
		op_and(1);
		if (!cpu.a) { cpu.cycles += 5; goto done; }
	} else cpu.cycles += 5;
	random_v();
	cpu.cycles += 4 + 19;
	cpu.b = cpu.a;
	cpu.c = mem_read(TARGET_MEMBER);
	call(0x7E96);                      // IY = the member's record
	cpu.cycles += 19 + 10 + 48 + 4 + 7;
	SET_HL(0xAC44 + mem_read(cpu.iy + 0x0C));   // the armour's defence
	cpu.a = mem_read(HL);
	op_sub8(cpu.b, 0, 0);
	if (!(cpu.f & FC)) { cpu.cycles += 5; goto done; }
	cpu.cycles += 19;
	cpu.c = mem_read(TARGET_MEMBER);
	member_struck(0xA785);
	cpu.cycles += 19 + 19;
	cpu.b = mem_read(FOE_INDEX);
	cpu.c = mem_read(TARGET_MEMBER);
	member_hit();
done:
	cpu.cycles += 10;                  // jp task_return_3
}

// $AB6B: foe IX+$1B leaves the battlefield: "<name> Flees!"; one that is
// not a creature to fight ($430A) gains the party a point each of
// Compassion and Justice; a sound, and the foe is gone
static void foe_flees(void) {
	cpu.cycles += 17 + 11 + 11 + 11 + 15;
	push16(BC); push16(DE); push16(HL); push16(cpu.iy);
	tcall(0x7F6E, 0xAB70);             // print_newline
	cpu.cycles += 19;
	cpu.c = mem_read(cpu.ix + 0x1B);
	call(0x7E8D);                      // foe_record
	cpu.cycles += 19;
	cpu.a = mem_read(cpu.iy + 5);
	call(0x7EC0);                      // its name
	tcall(0x7F62, 0xAB7F);             // print_text_hl
	say_inline_v(0xAB85);              // " Flees!"
	cpu.cycles += 19;
	cpu.a = mem_read(cpu.iy + 5);
	call(0x7E75);                      // is_foe
	if (cpu.f & FC) cpu.cycles += 12;
	else {
		cpu.cycles += 7 + 7 + 7;
		cpu.a = 1; cpu.c = 1;
		call(0x7E00);                  // karma_gain
		cpu.cycles += 7 + 7;
		cpu.a = 1; cpu.c = 3;
		call(0x7E00);
	}
	cpu.cycles += 7;
	cpu.a = 0x19;
	call(0x05F1);                      // play_sound
	cpu.cycles += 19 + 19 + 14 + 10 + 10 + 10 + 10;
	mem_write(cpu.iy + 5, 0);
	mem_write(cpu.iy + 4, 0);
	cpu.iy = pop16();
	SET_HL(pop16());
	SET_DE(pop16());
	SET_BC(pop16());
}

// foe_acts ($A5BC): the foe goes for the member it picked: casts or breathes
// at them from afar, puts the party to sleep, flees when hurt, steps
// towards them (around what's in the way), or strikes when next to them.
static void foe_acts(void) {
	cpu.cycles += 17 + 77;             // task_enter_3
	wr16(SLOT + 0x28, 0xA548);
	cpu.cycles += 19;
	cpu.c = mem_read(TARGET_MEMBER);
	call(0x7E90);
	cpu.cycles += 19 + 19 + 19;
	cpu.l = mem_read(cpu.iy + 0);
	cpu.h = mem_read(cpu.iy + 1);
	cpu.c = mem_read(FOE_INDEX);
	call(0x7E8D);                      // IY = the foe
	cpu.cycles += 4 + 19;
	cpu.a = cpu.l;
	op_sub8(mem_read(cpu.iy + 0), 0, 1);
	mem_write(STEP_X, sign_of(cpu.a));
	cpu.cycles += 19 + 4 + 19;
	cpu.a = cpu.h;
	op_sub8(mem_read(cpu.iy + 1), 0, 1);
	mem_write(STEP_Y, sign_of(cpu.a));
	cpu.cycles += 19 + 19 + 7 + 7;
	uint8_t kind = mem_read(cpu.iy + 5);
	int ranged_try = 1;
	if (kind == 0xAC) {                // the mimic: a chest until the party comes close
		cpu.cycles += 19 + 19 + 7 + 7;
		mem_write(cpu.iy + 6, 0x3C);
		if (mem_read(DISTANCE) >= 5) { cpu.cycles += 5; ranged_try = 0; }
		else { cpu.cycles += 19; mem_write(cpu.iy + 6, 0xAC); }
	} else cpu.cycles += 5;
	if (ranged_try) {                  // $A5FB: one time in four, magic or breath
		random_v();
		cpu.cycles += 7 + 7;
		op_and(3);
		if (!cpu.a) {
			cpu.cycles += 19;
			cpu.a = mem_read(cpu.iy + 5);
			call(0xAAC7);              // carry: it has none; D = what
			cpu.cycles += 7;
			int none = cpu.f & FC;
			if (!none) {
				cpu.cycles += 4 + 7 + 7;
				cpu.a = cpu.d;
				if (cpu.d == 0x4E) {   // magic, but magic is negated
					cpu.cycles += 13 + 7 + 7;
					if (mem_read(EFFECT) == 0x4E) { cpu.cycles += 5; none = 1; }
				} else cpu.cycles += 5;
			} else cpu.cycles += 5;
			if (!none) {
				cpu.cycles += 19 + 19;
				cpu.b = mem_read(FOE_INDEX);
				cpu.c = mem_read(TARGET_MEMBER);
				missile(0xA61C);
				call(0x7F4D);
				cpu.cycles += 10 + 10;    // jp $A74B / jp task_return_3
				return;
			}
		} else cpu.cycles += 5;
	}
	// $A625: the reapers and the gazers put members to sleep
	cpu.cycles += 19 + 7 + 7;
	kind = mem_read(cpu.iy + 5);
	cpu.a = kind;
	int sleeper = kind == 0xFC;
	if (sleeper) cpu.cycles += 5;
	else { cpu.cycles += 7 + 7; sleeper = kind == 0xB0; if (!sleeper) cpu.cycles += 5; }
	if (sleeper) {
		cpu.cycles += 13 + 7 + 7;
		if (mem_read(EFFECT) == 0x4E) { cpu.cycles += 5; sleeper = 0; }
		else {
			random_v();
			cpu.cycles += 7 + 7;
			op_and(3);
			if (cpu.a) { cpu.cycles += 5; sleeper = 0; }
		}
	}
	if (sleeper) {
		call(0x7F4D);
		cpu.cycles += 7;
		cpu.a = 0x15;
		call(0x05F1);
		call(0x7E69);
		call(0x7E69);
		cpu.cycles += 13 + 4 + 7;
		cpu.b = mem_read(0xC957);
		cpu.c = 0;
		do {
			call(0x7E90);
			cpu.cycles += 19 + 4 + 7;
			if (!mem_read(cpu.iy + 2)) { cpu.cycles += 5; goto next_member; }
			call(0x7E96);
			cpu.cycles += 19 + 7 + 7;
			if (mem_read(cpu.iy + 0x1C) != 'G') { cpu.cycles += 5; goto next_member; }
			random_v();
			cpu.cycles += 7 + 7;
			op_and(1);
			if (!cpu.a) { cpu.cycles += 5; goto next_member; }
			cpu.cycles += 19;
			mem_write(cpu.iy + 0x1C, 'S');
			call(0x7E90);
			cpu.cycles += 19 + 19;
			mem_write(cpu.iy + 2, 0x38);   // lying down
			mem_write(cpu.iy + 3, 0x38);
		next_member:
			cpu.cycles += 4;
			cpu.c++;
			cpu.cycles += --cpu.b ? 13 : 8;
		} while (cpu.b);
		call(0x7F29);
		cpu.cycles += 10 + 10;
		return;
	}
	// $A684: hurt, it runs away; next to the member, it strikes
	cpu.cycles += 19 + 7 + 7;
	if (mem_read(cpu.iy + 4) < 0x18) {
		cpu.cycles += 19 + 8 + 19 + 19 + 8 + 19 + 12;
		mem_write(STEP_X, (uint8_t)-mem_read(STEP_X));
		mem_write(STEP_Y, (uint8_t)-mem_read(STEP_Y));
	} else {
		cpu.cycles += 5 + 19 + 7 + 7;
		if (mem_read(DISTANCE) < 2) {
			cpu.cycles += 19 + 7;
			if (mem_read(cpu.iy + 5) == 0xA8) tcall(0xAB38, 0xA6A9);
			else cpu.cycles += 10;
			cpu.cycles += 19 + 7;
			if (mem_read(cpu.iy + 5) == 0xC8) tcall(0xAB54, 0xA6B1);
			else cpu.cycles += 10;
			cpu.cycles += 10;
			foe_strikes();
			return;
		}
		cpu.cycles += 5;
	}
	// $A6B7: the mimic and the gazers don't move
	cpu.cycles += 19 + 7 + 10;
	kind = mem_read(cpu.iy + 5);
	if (kind == 0xAC) { cpu.cycles += 10; return; }
	cpu.cycles += 7 + 10;
	if (kind == 0xB0) { cpu.cycles += 10; return; }
	cpu.cycles += 19;
	mem_write(SLOT + 0x17, 2);         // two tries
	for (;;) {
		random_v();                    // $A6C8: across or along, at random
		cpu.cycles += 7 + 7;
		op_and(1);
		int along_x = cpu.a != 0;
		if (!along_x) {
			cpu.cycles += 5 + 19 + 4 + 7;
			if (!mem_read(STEP_Y)) { cpu.cycles += 5; along_x = 1; }
		} else {
			cpu.cycles += 19 + 4 + 7;
			if (!mem_read(STEP_X)) { cpu.cycles += 5; along_x = 0; }
		}
		if (along_x) {
			cpu.cycles += 19 + 19 + 4 + 19 + 12;
			cpu.l = (uint8_t)(mem_read(cpu.iy + 0) + mem_read(STEP_X));
			cpu.a = cpu.l;
			cpu.h = mem_read(cpu.iy + 1);
		} else {
			cpu.cycles += 19 + 19 + 4 + 19;
			cpu.h = (uint8_t)(mem_read(cpu.iy + 1) + mem_read(STEP_Y));
			cpu.a = cpu.h;
			cpu.l = mem_read(cpu.iy + 0);
		}
		cpu.cycles += 13 + 7 + 7;      // $A6F1: jinxed, it may strike another foe
		if (mem_read(EFFECT) == 0x4A) {
			call(0x7E24);              // carry clear: a foe there
			cpu.cycles += 10;
			if (!(cpu.f & FC)) {       // $A794
				cpu.cycles += 19 + 10;
				mem_write(SLOT + 0x22, cpu.a);
				SET_BC(0x1208);
				blow(0xA79A);
				random_v();
				cpu.cycles += 7 + 7 + 19;
				op_and(0x3F);
				cpu.b = 0xFF;
				cpu.c = mem_read(SLOT + 0x22);
				tcall(0x7E36, 0xA7A7);   // damage_monster
				call(0x7F4D);
				cpu.cycles += 10;
				return;
			}
		} else cpu.cycles += 5;
		cpu.cycles += 19;              // $A6FE: is the way free?
		cpu.c = mem_read(FOE_INDEX);
		call(0xA9E2);
		cpu.cycles += 7;
		if (!(cpu.f & FC)) break;
		random_v();                    // blocked: another way at random
		mem_write(STEP_X, sign_of(cpu.a));
		cpu.cycles += 19;
		random_v();
		mem_write(STEP_Y, sign_of(cpu.a));
		cpu.cycles += 19 + 23 + 7;
		uint8_t tries = (uint8_t)(mem_read(SLOT + 0x17) - 1);
		mem_write(SLOT + 0x17, tries);
		if (tries) { cpu.cycles += 5; continue; }
		cpu.cycles += 19 + 7 + 10;
		if (mem_read(DISTANCE) < 2) { foe_strikes(); return; }
		cpu.cycles += 12 + 10;
		return;
	}
	cpu.cycles += 5 + 19 * 6;          // $A727: it steps
	mem_write(cpu.iy + 2, mem_read(cpu.iy + 0));
	mem_write(cpu.iy + 3, mem_read(cpu.iy + 1));
	cpu.e = mem_read(cpu.iy + 2);
	cpu.d = mem_read(cpu.iy + 3);
	mem_write(cpu.iy + 0, cpu.l);
	mem_write(cpu.iy + 1, cpu.h);
	cpu.cycles += 4 + 7 + 7;
	if (cpu.l < 0x0B) {
		cpu.cycles += 4 + 7 + 7;
		if (cpu.h < 0x0B) {
			call(0x7F4D);
			cpu.cycles += 12 + 10;
			return;
		}
	}
	cpu.cycles += 5;
	foe_flees();                       // off the field: gone ($AB6B)
	cpu.cycles += 10;
}

static void foes_turn(void) {
	uint16_t back = pop16();
	cpu.cycles += 17 + 77;             // task_enter_2
	wr16(SLOT + 0x26, back);
	SET_HL(back);
	call(0x7F29);
	call(0x7F4D);
	yield_at(0xA49C);
	cpu.cycles += 19 + 19;
	mem_write(SLOT + 0x16, 0x10);
	mem_write(SLOT + 0x1B, 0);
	do {
		cpu.cycles += 19;              // $A4A7
		cpu.c = mem_read(SLOT + 0x1B);
		call(0x7E8D);                  // IY = the foe
		cpu.cycles += 19 + 4 + 10;
		cpu.a = mem_read(cpu.iy + 5);
		op_or(cpu.a);
		if (!cpu.a) goto next;         // none there
		cpu.cycles += 19 + 4 + 7;
		cpu.a = mem_read(cpu.iy + 7);
		op_or(cpu.a);
		if (cpu.a) {                   // asleep: wakes one time in eight
			random_v();
			cpu.cycles += 7 + 10;
			op_and(7);
			if (cpu.a) goto next;
		} else cpu.cycles += 5;
		cpu.cycles += 19 + 19 + 7 + 7; // $A4C2
		mem_write(cpu.iy + 7, 0);
		cpu.a = mem_read(cpu.iy + 5);
		op_sub8(0xEC, 0, 0);
		if (cpu.a == 0xEC) {           // it negates magic for a while
			cpu.cycles += 7 + 13 + 7 + 13;
			mem_write(0xC976, 0x4E);
			mem_write(0xC977, 2);
			cpu.a = 2;
			call(0x7F29);
			cpu.cycles += 12;
		} else {
			cpu.cycles += 5 + 7 + 7;
			op_sub8(0xDC, 0, 0);
			if (cpu.a == 0xDC) {
				random_v();
				cpu.cycles += 7 + 7;
				op_and(7);
				if (!cpu.a) {          // it blinks to a free spot
					do {
						cpu.cycles += 7;
						cpu.a = 0x0B;
						call(0x7EDB);  // random_below
						cpu.cycles += 4 + 7;
						cpu.l = cpu.a;
						cpu.a = 0x0B;
						call(0x7EDB);
						cpu.cycles += 4;
						cpu.h = cpu.a;
						call(0xA9E2);  // carry: not free
						cpu.cycles += 7;
						if (!(cpu.f & FC)) break;
						cpu.cycles += 5;
					} while (1);
					cpu.cycles += 19 + 19;
					mem_write(cpu.iy + 0, cpu.l);
					mem_write(cpu.iy + 1, cpu.h);
				} else cpu.cycles += 5;
			} else cpu.cycles += 5;
		}
		// $A4FE: the nearest member who can act
		cpu.cycles += 19 + 19 + 19 + 19 + 13 + 4 + 7;
		cpu.l = mem_read(cpu.iy + 0);
		cpu.h = mem_read(cpu.iy + 1);
		mem_write(SLOT + 0x1C, 0xFF);
		mem_write(SLOT + 0x1D, 0xFF);
		cpu.b = mem_read(0xC957);
		cpu.c = 0;
		do {
			call(0x7ED2);              // member_can_act: carry if not
			cpu.cycles += 7;
			if (cpu.f & FC) { cpu.cycles += 5; goto counted; }
			call(0x7E90);              // IY = the member's figure
			cpu.cycles += 19 + 4 + 7;
			cpu.a = mem_read(cpu.iy + 2);
			op_or(cpu.a);
			if (!cpu.a) { cpu.cycles += 5; goto counted; }
			cpu.cycles += 4 + 19 + 7;
			cpu.a = cpu.l;
			op_sub8(mem_read(cpu.iy + 0), 0, 1);
			if (cpu.f & FC) { cpu.cycles += 8; cpu.a = (uint8_t)-cpu.a; }
			else cpu.cycles += 5;
			cpu.cycles += 4 + 4 + 19 + 7;
			cpu.d = cpu.a;
			cpu.a = cpu.h;
			op_sub8(mem_read(cpu.iy + 1), 0, 1);
			if (cpu.f & FC) { cpu.cycles += 8; cpu.a = (uint8_t)-cpu.a; }
			else cpu.cycles += 5;
			cpu.cycles += 4 + 19 + 7;
			op_add8(cpu.d, 0);
			op_sub8(mem_read(SLOT + 0x1D), 0, 0);
			if (!(cpu.f & FC)) { cpu.cycles += 5; goto counted; }
			cpu.cycles += 19 + 19;
			mem_write(SLOT + 0x1D, cpu.a);
			mem_write(SLOT + 0x1C, cpu.c);
		counted:
			cpu.cycles += 4;
			cpu.c++;
			if (--cpu.b) cpu.cycles += 13;
			else cpu.cycles += 8;
		} while (cpu.b);
		cpu.cycles += 19 + 7;          // $A540
		cpu.a = mem_read(SLOT + 0x1C);
		op_sub8(0xFF, 0, 0);
		if (cpu.a != 0xFF) {           // it goes for them
			cpu.cycles += 17;
			foe_acts();
			cpu.cycles += 42;          // task_return_3
			SET_HL(0xA548);
		} else cpu.cycles += 10;
		cpu.cycles += 19;              // $A548: the field it stands on
		cpu.c = mem_read(SLOT + 0x1B);
		call(0x7E8D);
		cpu.cycles += 19 + 19;
		cpu.l = mem_read(cpu.iy + 0);
		cpu.h = mem_read(cpu.iy + 1);
		call(0xA460);                  // A = the tile
		cpu.cycles += 10 + 17 + 10;    // ld hl,$A575 / call switch_on_a by its vector
		{
			static const uint8_t tiles[4] = { 0x46, 0x4C, 0x44, 0x47 };
			uint8_t tile = cpu.a;
			int i;
			for (i = 0; i < 4 && tiles[i] != tile; i++) {}
			if (i == 4) {
				cpu.cycles += 58 * 4 + 22;
				cpu.b = 0;
				SET_HL(0xA581);
			} else {
				cpu.cycles += 58 * i + 79;
				cpu.b = (uint8_t)(tile + 1);
				SET_HL(rd16(0xA576 + 3 * i));
				cpu.a = 0xA5; cpu.f = 0x5D;   // (the switch pops its return address)
				if (i < 2) {           // $A582: fire, unless it lives in it
					cpu.cycles += 19 + 7 + 7;
					uint8_t g = mem_read(cpu.iy + 5);
					cpu.a = g;
					op_sub8(0xE8, 0, 0);
					if (g == 0xE8) cpu.cycles += 5;
					else {
						cpu.cycles += 7 + 7;
						op_sub8(0xF0, 0, 0);
						if (g >= 0xF0) cpu.cycles += 5;
						else foe_burns();
					}
				} else if (i == 2) foe_burns();   // poison
				else {                 // $A5A1: a sleep field
					cpu.cycles += 19;
					cpu.a = mem_read(cpu.iy + 5);
					call(0x7F4A);      // Z: it doesn't sleep
					cpu.cycles += 7;
					if (cpu.f & FZ) cpu.cycles += 5;
					else {
						random_v();
						cpu.cycles += 19 + 7;
						op_sub8(mem_read(cpu.iy + 4), 0, 0);
						if (cpu.f & FC) cpu.cycles += 5;
						else {
							cpu.cycles += 7;
							cpu.a = 0x14;
							call(0x05F1);
							cpu.cycles += 19 + 12;
							mem_write(cpu.iy + 7, 0xFF);
						}
					}
				}
			}
		}
		yield_at(0xA55D);
	next:
		cpu.cycles += 23 + 23 + 10;    // $A560
		mem_write(SLOT + 0x1B, mem_read(SLOT + 0x1B) + 1);
		mem_write(SLOT + 0x16, mem_read(SLOT + 0x16) - 1);
	} while (mem_read(SLOT + 0x16));
	tcall(0x7EE1, 0xA569);
	tcall(0x7E81, 0xA56C);
	tcall(0x7F23, 0xA56F);
	cpu.cycles += 10 + 42;             // jp task_return_2
	SET_HL(rd16(SLOT + 0x26));
	native_continue_at(HL);
}
Native native_foes_turn = { "foes_turn", 0xA493, 3, foes_turn, 0, 0, 0, .task = 1 };

// ------------------------------------------------------------ the round --
// After each member's turn ($9DA0): their statuses, whether the fight goes
// on, the next member - or the foes' turn and a new round ($9D37). The end
// ($9DBF): the outcome, and back to the game mode the fight came from.

// $9FD8: carry while the fight goes on (members on the field, and foes -
// in a dungeon room until the party leaves it)
static int fight_goes_on(void) {
	cpu.cycles += 17 + 10 + 10 + 7 + 4;
	uint8_t any = 0;
	for (int i = 0; i < 8; i++) { any |= mem_read(0xC8BB + 5 * i); cpu.cycles += 7 + 11 + (i < 7 ? 13 : 8); }
	SET_HL(0xC8BB + 40); SET_DE(0x0005); cpu.b = 0;
	cpu.a = any;
	op_or(any);
	if (!any) { cpu.cycles += 11; return 0; }
	cpu.cycles += 5 + 13 + 7 + 4;
	cpu.a = mem_read(MAP_MODE);
	op_sub8(5, 0, 0);
	cpu.f |= FC;
	if (cpu.a == 5) { cpu.cycles += 11; return 1; }
	cpu.cycles += 5 + 10 + 10 + 7 + 4;
	any = 0;
	for (int i = 0; i < 16; i++) { any |= mem_read(FOES + 8 * i); cpu.cycles += 7 + 11 + (i < 15 ? 13 : 8); }
	SET_HL(FOES + 0x80); SET_DE(0x0008);
	cpu.a = any;
	op_or(any);
	if (!any) { cpu.cycles += 11; return 0; }
	cpu.cycles += 5 + 4 + 10;
	cpu.f |= FC;
	return 1;
}

// $9DFA: a member's statuses after their turn: what is underfoot, falling
// down, quickness (another turn), waking up
static void member_after(void) {
	cpu.cycles += 17;
	call(0x7E8A);                      // IY = the figure
	cpu.cycles += 19 + 4;
	if (!mem_read(cpu.iy + 2)) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 20;
	SET_BC(rd16(MEMBER));
	call(0x7ECF);                      // member_able: carry if not
	if (!(cpu.f & FC)) tcall(0x7E54, 0x9E09);   // the ground's hazards
	else cpu.cycles += 10;
	cpu.cycles += 20;
	SET_BC(rd16(MEMBER));
	call(0x7ED2);                      // member_can_act
	cpu.cycles += 7;
	if (cpu.f & FC) {                  // down: off the field
		call(0x7E8A);
		cpu.cycles += 19 + 10;
		mem_write(cpu.iy + 2, 0);
		return;
	}
	cpu.cycles += 5 + 20;              // $9E1D
	SET_BC(rd16(MEMBER));
	call(0x7ECF);
	cpu.cycles += 7;
	if (!(cpu.f & FC)) {
		cpu.cycles += 13 + 7 + 7;
		if (mem_read(0xC976) == 0x51) {   // quickness: half the time, the same member again
			random_v();
			cpu.cycles += 7 + 7;
			if (cpu.a & 1) {
				cpu.cycles += 10 + 11 + 10;
				mem_write(MEMBER, mem_read(MEMBER) - 1);
				SET_HL(MEMBER);
				return;
			}
			cpu.cycles += 5;
		} else cpu.cycles += 5;
	} else cpu.cycles += 5;
	member_record_iy();                // $9E39: asleep: wakes 1 time in 8
	cpu.cycles += 19 + 7 + 7;
	if (mem_read(cpu.iy + 0x1C) == 'S') {
		random_v();
		cpu.cycles += 7 + 7;
		if (!(cpu.a & 7)) {
			cpu.cycles += 19;
			mem_write(cpu.iy + 0x1C, 'G');
			call(0x7E8A);
			cpu.cycles += 20;
			SET_BC(rd16(MEMBER));
			call(0x7ED5);              // their figure, standing
			cpu.cycles += 19 + 19;
			mem_write(cpu.iy + 2, cpu.a);
			mem_write(cpu.iy + 3, cpu.a);
			call(0x7F29);
		} else cpu.cycles += 5;
	} else cpu.cycles += 5;
	cpu.cycles += 7 + 10;              // ld a,1 / jp $7F32
	cpu.a = 1;
	task_call(0x7F32, 0x9DA3);
}

static void combat_over(void) { cpu.cycles += 5; native_continue_at(0x9DBF); }   // jr nc,$9DBF

// $9DA0
static void member_done(void) {
	member_after();
	int on = fight_goes_on();
	cpu.cycles += 7;
	if (!on) { combat_over(); return; }
	cpu.cycles += 10 + 11 + 7 + 10 + 7 + 10;
	uint8_t next = (uint8_t)(mem_read(MEMBER) + 1);
	mem_write(MEMBER, next);
	if (next < mem_read(0xC957)) { native_continue_at(0x9D3B); return; }   // the next member
	cpu.cycles += 17;                  // the foes
	push16(0x9DB7);
	native_continue_at(0xA493);
}
Native native_member_done = { "member_done", 0x9DA0, 3, member_done, 0, 0, 0, .task = 1 };

// $9DB7: after the foes: a new round
static void round_over(void) {
	int on = fight_goes_on();
	cpu.cycles += 7;
	if (!on) { combat_over(); return; }
	cpu.cycles += 10 + 4 + 13;         // jp $9D37 / xor a / ld (member),a
	mem_write(MEMBER, 0);
	cpu.a = 0;
	native_continue_at(0x9D3B);
}
Native native_round_over = { "round_over", 0x9DB7, 3, round_over, 0, 0, 0, .task = 1 };

// ------------------------------------------------------------ the end --
// $9DBF: the fight is over: the party's fate ($9FC8), the outcome ($9E66),
// the map back, and the game mode the fight came from.

#define op_cp(v) op_sub8(v, 0, 0)

static uint16_t rst30(uint16_t hl, uint8_t a) {      // rst $30: HL += A
	uint16_t r = (uint16_t)(hl + a);
	cpu.cycles += 11 + 4 + 4 + ((uint8_t)(hl + a) < a ? 5 + 4 + 10 : 11);
	SET_HL(r);
	cpu.a = (uint8_t)r;
	return r;
}

// $A029: HL = the dungeon cell at (L, H) on this level
static void room_cell(void) {
	cpu.cycles += 17 + 4 * 7 + 13 + 7 + 4 + 10;
	cpu.a = (uint8_t)(cpu.h << 4 | cpu.l);
	SET_HL((uint16_t)((mem_read(0xC970) + 0xD0) << 8 | cpu.a));
	cpu.a = cpu.h;
}

// $9FFD/$A000/$A003/$A006: one step out of the room in direction `dir`;
// zero if the cell there is $C0
static int room_step(int dir) {
	static const uint8_t cost[4] = { 7 + 10 + 10 + 10, 7 + 10 + 10, 7 + 10, 7 };
	cpu.cycles += 17 + cost[dir];
	cpu.a = (uint8_t)dir;
	call(0x7E39);                      // L, H = the step
	cpu.cycles += 13 + 4 + 7 + 4 + 13 + 4 + 7 + 4 + 16;
	cpu.l = (mem_read(0xC953) + cpu.l) & 0x0F;
	cpu.h = (mem_read(0xC954) + cpu.h) & 0x0F;
	wr16(0xC953, HL);
	cpu.cycles += 17 + 11;             // call $A022 / push hl
	uint16_t hl = HL;
	room_cell();
	cpu.a = mem_read(HL);
	SET_HL(hl);
	cpu.cycles += 7 + 10 + 10 + 7 + 10;
	op_cp(0xC0);
	return !(cpu.f & FZ);
}

// $9E66 in a dungeon room: out by the side the party left, or by the
// room's exit to another place
static void leave_room(void) {
	say_inline_v(0x9E71);              // "Leave Room!"
	cpu.cycles += 13 + 4 + 10;
	uint8_t way = mem_read(0xC984);
	if (!(way & 0x80)) {               // $9EB8: to the place in the room's table
		cpu.cycles += 4 + 4 + 10 + 7 + 4 + 4 + 4 + 4 + 10;
		uint8_t i = (uint8_t)(way * 4 + mem_read(0xC9BA));
		cpu.b = i;
		rst30(0xA037, (uint8_t)(i * 5));
		cpu.cycles += 7 + 6 + 7 + 6 + 20 + 7 + 6 + 7 + 6 + 20 + 7 + 10 + 7 + 7;
		wr16(0xC953, rd16(HL));
		wr16(0xC96D, rd16(HL + 2));
		SET_DE(rd16(HL + 2));
		SET_HL(HL + 4);
		cpu.a = mem_read(HL);
		SET_HL(0xC9A0);
		op_cp(mem_read(HL));
		mem_write(HL, cpu.a);
		if (cpu.f & FZ) { cpu.cycles += 11; return; }
		cpu.cycles += 5;
		say_inline_v(0x9EE0);          // "into Dungeon " and its name
		cpu.cycles += 13 + 7;
		cpu.a = (uint8_t)(mem_read(0xC9A0) + 0x7D);
		tcall(0x7F6B, 0x9EF3);
		tcall(0x7F6E, 0x9EF6);
		tcall(0x7E48, 0x9EF9);
		cpu.cycles += 7 + 13 + 10;
		cpu.a = 7;
		mem_write(0xC970, 7);
		return;
	}
	cpu.cycles += 13 + 10 + 10 + 10 + 52;   // by the side: jp $0018 / jp $0083
	uint8_t side = mem_read(0xC9BA);
	SET_HL(rd16(0x9E8E + 2 * side));
	cpu.a = cpu.l;
	static const int then[4] = { 3, 2, 3, 2 };
	if (side >= 2) room_step(side);
	if (!room_step(side)) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 10 - 17;         // jp, not call: its ret ends $9E66
	room_step(then[side]);
}

// $9E66 elsewhere ($9F02): victory if a member is still on the field, else
// the party fled - or fell
static void outcome(void) {
	cpu.cycles += 14 + 10 + 7;
	SET_DE(0x0005);
	cpu.b = 8;
	uint16_t iy = 0xC8B9;
	int on = 0;
	for (;;) {
		cpu.cycles += 19 + 4 + 7;
		cpu.a = mem_read(iy + 2);
		op_or(cpu.a);
		if (cpu.a) { cpu.cycles += 5; on = 1; break; }
		cpu.cycles += 15;
		iy += 5;
		if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
	}
	cpu.iy = iy;
	if (!on) {
		cpu.cycles += 13;
		cpu.a = mem_read(0xC97D);
		call(0x7E75);
		cpu.cycles += 7;
		if (cpu.f & FC) {              // $9F2B
			cpu.cycles += 5;
			say_inline_v(0x9F2E);      // "Battle is lost!"
			cpu.cycles += 7 + 7 + 10;
			cpu.a = 2; cpu.c = 2;
			task_call(0x7F2F, 0x9DCB);
			return;
		}
		cpu.cycles += 7 + 7;           // fled: karma
		cpu.a = 2; cpu.c = 1;
		call(0x7E00);
		cpu.cycles += 7 + 7 + 10;
		cpu.a = 2; cpu.c = 3;
		task_call(0x7E00, 0x9DCB);
		return;
	}
	say_inline_v(0x9F4A);              // "Victory!"
	cpu.cycles += 13;
	cpu.a = mem_read(0xC97D);
	call(0x7E75);
	cpu.cycles += 7;
	if (cpu.f & FC) {
		call(0x7ED8);
		cpu.cycles += 7 + 7;
		op_and(1);
		cpu.c = 2;
		call(0x7E00);
	} else cpu.cycles += 5;
	uint8_t from = mem_read(0xC975), foe = mem_read(0xC97D);
	cpu.cycles += 13 + 7;
	cpu.a = from;
	op_cp(4);
	if (from == 4) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 13 + 7;
	op_cp(1);
	if (from == 1) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 13 + 7 + 10;
	cpu.a = foe;
	op_cp(0x80);
	if (foe == 0x80) { task_call(0xBA0B, 0x9DCB); return; }   // jp $BA0B
	cpu.cycles += 13 + 7;
	cpu.a = from;
	op_cp(2);
	if (from == 2) {                   // in a dungeon: a chest on the cell
		cpu.cycles += 7 + 13 + 7;
		cpu.a = foe;
		static const uint8_t keep[3] = { 0x94, 0xB4, 0xA0 };
		for (int i = 0; i < 3; i++) {
			op_cp(keep[i]);
			if (foe == keep[i]) { cpu.cycles += 11; return; }
			cpu.cycles += 5 + (i < 2 ? 7 : 0);
		}
		cpu.cycles += 16;
		SET_HL(rd16(0xC97A));
		room_cell();
		cpu.cycles += 7 + 4 + 5;
		cpu.a = mem_read(HL);
		op_or(cpu.a);
		if (cpu.a) { cpu.cycles += 6; return; }
		cpu.cycles += 10 + 10;
		mem_write(HL, 0x40);
		return;
	}
	cpu.cycles += 12 + 13 + 7 + 7;     // $9F9A
	cpu.a = foe;
	op_cp(0x90);
	int chest = 0;
	if (foe >= 0x90) cpu.cycles += 5;
	else {
		cpu.cycles += 7;
		op_cp(0x20);
		cpu.cycles += 7;
		if (foe < 0x20) { cpu.cycles += 5; }
		else {
			cpu.cycles += 7 + 7;
			op_cp(0x2F);
			if (foe < 0x2F) { cpu.cycles += 5; chest = 1; }
		}
		if (!chest) {
			cpu.cycles += 7 + 5;       // $9FA9
			op_cp(0x50);
			if (foe < 0x50) { cpu.cycles += 6; return; }
			cpu.cycles += 7 + 5;
			op_cp(0x5F);
			if (foe >= 0x5F) { cpu.cycles += 6; return; }
		}
	}
	static const uint8_t none[5] = { 0x94, 0xA0, 0xB8, 0xB4, 0xDC };   // $9FAF
	for (int i = 0; i < 5; i++) {
		cpu.cycles += 7 + 5;
		op_cp(none[i]);
		if (foe == none[i]) { cpu.cycles += 6; return; }
	}
	cpu.cycles += 13;
	cpu.a = mem_read(0xC97C);
	call(0x7E99);
	cpu.cycles += 5;
	if (cpu.f & FC) { cpu.cycles += 6; return; }
	cpu.cycles += 10;
	task_call(0xBA4C, 0x9DCB);         // jp $BA4C: a chest on the map
}

static void combat_end(void) {
	tcall(0x7F4D, 0x9DBF);
	yield_at(0x9DC2);
	cpu.cycles += 17 + 13 + 4 + 7;     // $9FC8: is anyone still standing?
	uint8_t n = mem_read(0xC957);
	cpu.a = n; cpu.b = n; cpu.c = 0;
	for (;;) {
		call(0x7ED2);
		if (!(cpu.f & FC)) { cpu.cycles += 11; break; }
		cpu.cycles += 5 + 4;
		cpu.c++;
		if (--cpu.b) cpu.cycles += 13;
		else { cpu.cycles += 8 + 10; task_call(0x7EC9, 0x9DC8); break; }   // all down
	}
	cpu.cycles += 17 + 13 + 7 + 10;    // call $9E66
	cpu.a = mem_read(MAP_MODE);
	op_cp(5);
	if (cpu.a == 5) leave_room();
	else outcome();
	tcall(0x0605, 0x9DCB);
	tcall(0x7EA2, 0x9DCE);
	cpu.cycles += 13 + 7 + 7 + 7;
	uint8_t mode = (uint8_t)(mem_read(0xC975) + 0x11);
	cpu.a = mode;
	op_cp(0x15);
	if (mode == 0x15) { cpu.cycles += 7 + 13 + 7; cpu.a = mem_read(0xC9B7); op_and(0x7F); }
	else cpu.cycles += 5;
	cpu.cycles += 13 + 10;
	mem_write(0xC600, cpu.a);
}
Native native_combat_end = { "combat_end", 0x9DBF, 3, combat_end, 0, 0, 0, .task = 1 };
