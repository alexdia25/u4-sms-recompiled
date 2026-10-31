// Talking to Lord British ($812B, bank 2), inside the Talk command's task.
// His words are bank $1A's messages (the table at $80EC). The first time
// ($C963) he greets the Avatar; else a dead Avatar is raised and the party
// healed, he greets them by number, and whoever has the experience rises
// a level ($82BD: double each step from 100, at most level 8, with 1-8
// more of each stat, 50 at most). Then a menu of topics, the ones at $C5E0:
// asking a topic adds those it leads to (the pairs at $8422); topic 1 ends
// the talk, 5 offers healing, 6 tells what is still to be done ($83A3).
// Cycle counts are the original's.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define SLOT cpu.ix
#define TOP   (SLOT + 0x1B)            // the first topic shown
#define ROW   (SLOT + 0x1C)            // the cursor's row in the menu
#define TOPIC (SLOT + 0x1D)
#define MESSAGE_BANK  0xC0CA
#define MESSAGE_TABLE 0xC0CB
#define PARTY         0xC751
#define PARTY_SIZE    0xC957
#define KARMA         0xC90B
#define TOPICS        0xC5E0

static void call(uint16_t addr) { cpu.cycles += 17; call_original(addr); }
static void tcall(uint16_t addr, uint16_t site) {
	cpu.cycles += 17;
	task_call(addr, (uint16_t)(site + 3));
}
static void msg_a(void) { call(0x5582); }
static void msg(uint8_t n) { cpu.cycles += 7; cpu.a = n; msg_a(); }
static void newline(void) { cpu.cycles += 17; text_newline(); }
static void quote(void) { cpu.cycles += 17 + 7; text_char('"'); }   // call $548D
static void avatar_name(void) { call(0x5650); }

// $8453, called at site: message A of many pages, a press of a button
// between them; its return kept in the slot (task_enter_3) meanwhile
static void pages(uint16_t site) {
	cpu.cycles += 17 + 17 + 77 + 4 + 13 + 16;
	wr16(SLOT + 0x28, (uint16_t)(site + 3));
	cpu.c = cpu.a;
	cpu.a = mem_read(MESSAGE_BANK);
	SET_HL(rd16(MESSAGE_TABLE));
	call(0x0807);                      // far_word
	cpu.cycles += 19 + 19;
	wr16(SLOT + 0x0E, HL);
	for (;;) {
		cpu.cycles += 13 + 19 + 19;
		cpu.a = mem_read(MESSAGE_BANK);
		SET_HL(rd16(SLOT + 0x0E));
		call(0x0815);                  // far_read
		cpu.cycles += 4 + 10;
		op_or(cpu.a);
		if (cpu.f & FZ) break;
		cpu.cycles += 13;
		cpu.a = mem_read(MESSAGE_BANK);
		call(0x5597);                  // the page; HL after it
		cpu.cycles += 19 + 19;
		wr16(SLOT + 0x0E, HL);
		task_wait_button(0x8482);
		cpu.cycles += 17 + 17;
		text_newline();
		text_newline();
		cpu.cycles += 12;
	}
	cpu.cycles += 42;                  // task_return_3
	SET_HL((uint16_t)(site + 3));
}

static void flash(uint16_t site) {     // $8411, called at site: a sound, the map inverted twice
	(void)site;
	cpu.cycles += 17 + 7;
	cpu.a = 0x15;
	call(0x05F1);
	tcall(0x041C, 0x8416);
	call(0x41A9);                      // invert
	tcall(0x041C, 0x841C);
	cpu.cycles += 10;
	call_original(0x41A9);
}

static void heal_all(void) {           // $8296: everyone alive good, at full hit points
	cpu.cycles += 17 + 14 + 10 + 13 + 4 + 7;
	cpu.iy = PARTY;
	SET_DE(0x001D);
	cpu.b = mem_read(PARTY_SIZE);
	cpu.a = 0x44;
	do {
		cpu.cycles += 19;
		op_sub8(mem_read(cpu.iy + 0x1C), 0, 0);
		if (cpu.f & FZ) cpu.cycles += 12;
		else {
			cpu.cycles += 7 + 19 + 19 + 19 + 19 + 19;
			mem_write(cpu.iy + 0x1C, 0x47);
			cpu.l = mem_read(cpu.iy + 2);
			cpu.h = mem_read(cpu.iy + 3);
			mem_write(cpu.iy + 0, cpu.l);
			mem_write(cpu.iy + 1, cpu.h);
		}
		cpu.cycles += 15;
		cpu.iy = op_add16(cpu.iy, DE);
		cpu.cycles += --cpu.b ? 13 : 8;
	} while (cpu.b);
	cpu.cycles += 10;
}

static uint8_t raised(uint8_t stat) {  // $834C: 1-8 more, 50 at most
	cpu.cycles += 17;
	call(0x2BBF);
	cpu.cycles += 7 + 4 + 4 + 7;
	op_and(0x07);
	cpu.a = op_inc8(cpu.a);
	op_add8(stat, 0);
	op_sub8(0x33, 0, 0);
	if (cpu.f & FC) { cpu.cycles += 11; return cpu.a; }
	cpu.cycles += 5 + 7 + 10;
	cpu.a = 0x32;
	return cpu.a;
}

static void level_up(void) {           // $82BD
	cpu.cycles += 17 + 14 + 20;
	cpu.iy = PARTY;
	SET_BC(rd16(PARTY_SIZE));
	for (;;) {
		cpu.cycles += 19 + 19 + 10 + 7;
		cpu.e = mem_read(cpu.iy + 4);
		cpu.d = mem_read(cpu.iy + 5);
		SET_HL(0xE700);
		cpu.b = 8;
		int reached = 0;
		for (;;) {
			cpu.cycles += 11 + 11 + 10;
			uint16_t hl = HL;
			op_add16(hl, DE);
			if (((uint32_t)hl + DE) > 0xFFFF) { cpu.f |= FC; cpu.cycles += 12; reached = 1; break; }
			cpu.f &= ~FC;
			cpu.cycles += 7 + 8 + 8;
			cpu.h = op_rot(5, cpu.h);
			cpu.l = op_rot(3, cpu.l);
			if (--cpu.b) { cpu.cycles += 13; continue; }
			cpu.cycles += 8 + 12;
			break;
		}
		if (reached) {
			cpu.cycles += 19 + 4;
			cpu.a = mem_read(cpu.iy + 0x0A);
			op_sub8(cpu.b, 0, 0);
			if (!(cpu.f & FC)) { cpu.cycles += 12; reached = 0; }
			else cpu.cycles += 7;
		}
		if (reached) {
			cpu.cycles += 19 + 4 + 7;
			mem_write(cpu.iy + 0x0A, cpu.b);
			cpu.l = cpu.b;
			cpu.h = 0x64;
			call(0x2AA1);              // multiply_8x8
			cpu.cycles += 19 + 19 + 19 + 19 + 19 + 19;
			mem_write(cpu.iy + 0, cpu.l);
			mem_write(cpu.iy + 1, cpu.h);
			mem_write(cpu.iy + 2, cpu.l);
			mem_write(cpu.iy + 3, cpu.h);
			mem_write(cpu.iy + 0x1C, 0x47);
			cpu.d = mem_read(cpu.iy + 6);
			uint8_t v = raised(cpu.d);
			cpu.cycles += 19 + 19;
			mem_write(cpu.iy + 6, v);
			cpu.d = mem_read(cpu.iy + 7);
			v = raised(cpu.d);
			cpu.cycles += 19 + 19;
			mem_write(cpu.iy + 7, v);
			cpu.d = mem_read(cpu.iy + 8);
			v = raised(cpu.d);
			cpu.cycles += 19;
			mem_write(cpu.iy + 8, v);
			quote();
			cpu.cycles += 15 + 10 + 10 + 11;
			SET_DE(0x000D);
			SET_HL((uint16_t)(cpu.iy + 0x0D));
			call(0x5519);              // the name
			msg(0x2B);
			cpu.cycles += 19 + 7 + 17;
			cpu.a = (uint8_t)(mem_read(cpu.iy + 0x0A) + 0x30);
			text_char(cpu.a);
			uint16_t hl = HL;
			cpu.cycles += 17 + 19 + 17;
			text_print(0x8334);        // .", a new line
			cpu.cycles += 19 + 10;
			SET_HL(hl);
			flash(0x8338);
			call(0x4324);
		}
		cpu.cycles += 10 + 15 + 4 + 10;
		SET_DE(0x001D);
		cpu.iy = op_add16(cpu.iy, DE);
		cpu.c = op_dec8(cpu.c);
		if (cpu.f & FZ) break;
	}
	cpu.cycles += 7 + 10;
	cpu.a = 0x2C;
	call_original(0x5582);
}

static uint8_t topic_here(uint16_t table) {   // ld a,(ix+$1B) / add a,(ix+$1C) / (ld hl,table / rst $28)
	cpu.cycles += 19 + 19;
	cpu.a = mem_read(TOP);
	op_add8(mem_read(ROW), 0);
	if (table) {
		cpu.cycles += 10 + 11;
		SET_HL(table);
		call_original(0x0028);
	}
	return cpu.a;
}

static void row_step(int by) {         // the cursor one row on
	cpu.cycles += 17 + 19 + 4 + 4 + 10;
	cpu.a = (uint8_t)(mem_read(ROW) * 2 + 1);
	call(0x49E0);                      // cursor_off
	cpu.cycles += 23;
	mem_write(ROW, (uint8_t)(mem_read(ROW) + by));
	cpu.cycles += 17 + 19 + 4 + 4 + 10;
	cpu.a = (uint8_t)(mem_read(ROW) * 2 + 1);
	call(0x49D5);                      // cursor_on
	cpu.cycles += 12;
}

// asked about a topic: those it leads to join the list ($8422: topic,
// topic it adds; 0 ends)
static void topic_leads_on(void) {
	cpu.cycles += 10;
	SET_DE(0x8422);
	for (;;) {
		cpu.cycles += 7 + 6 + 4 + 10;
		cpu.a = mem_read(DE);
		SET_DE(DE + 1);
		op_or(cpu.a);
		if (cpu.f & FZ) return;
		cpu.cycles += 19;
		op_sub8(mem_read(TOPIC), 0, 0);
		if (!(cpu.f & FZ)) cpu.cycles += 12;
		else {
			cpu.cycles += 7 + 7 + 4 + 10;
			cpu.a = mem_read(DE);
			cpu.b = cpu.a;
			SET_HL(TOPICS);
			for (;;) {
				cpu.cycles += 7 + 4;
				cpu.a = mem_read(HL);
				op_or(cpu.a);
				if (cpu.f & FZ) {
					cpu.cycles += 12 + 7;
					mem_write(HL, cpu.b);
					break;
				}
				cpu.cycles += 7 + 4;
				op_sub8(cpu.b, 0, 0);
				if (cpu.f & FZ) { cpu.cycles += 12; break; }
				cpu.cycles += 7 + 6 + 12;
				SET_HL(HL + 1);
			}
		}
		cpu.cycles += 6 + 12;
		SET_DE(DE + 1);
	}
}

// $83A3: what is still to be done, the first thing lacking
static void advice(void) {
	msg(0x1E);
	cpu.cycles += 10;
	SET_HL(0x8407);
	call(0xBBC6);                      // below_c8fd
	uint8_t say;
	do {
		cpu.cycles += 7;
		say = 0x1F;
		if (cpu.f & FC) { cpu.cycles += 12; break; }
		cpu.cycles += 7 + 13 + 7 + 7;
		cpu.a = mem_read(PARTY_SIZE);
		op_sub8(0x01, 0, 0);
		say = 0x20;
		if (cpu.f & FZ) { cpu.cycles += 12; break; }
		cpu.cycles += 7 + 13 + 4 + 7;
		cpu.a = mem_read(0xC956);
		op_or(cpu.a);
		say = 0x21;
		if (cpu.f & FZ) { cpu.cycles += 12; break; }
		cpu.cycles += 7 + 10 + 7;
		SET_HL(KARMA);
		cpu.b = 8;
		int won = 0;
		do {
			cpu.cycles += 7 + 4;
			cpu.a = mem_read(HL);
			op_or(cpu.a);
			if (cpu.f & FZ) { cpu.cycles += 12; won = 1; break; }
			cpu.cycles += 7 + 6;
			SET_HL(HL + 1);
			cpu.cycles += --cpu.b ? 13 : 8;
		} while (cpu.b);
		say = 0x22;
		if (!won) { cpu.cycles += 7 + 12; break; }
		cpu.cycles += 13 + 4 + 7;
		cpu.a = mem_read(0xC955);
		op_or(cpu.a);
		say = 0x23;
		if (cpu.f & FZ) { cpu.cycles += 12; break; }
		cpu.cycles += 7 + 10 + 4 + 7;
		SET_HL(KARMA);
		op_xor(cpu.a);
		cpu.b = 8;
		do {
			cpu.cycles += 7 + 6;
			op_or(mem_read(HL));
			SET_HL(HL + 1);
			cpu.cycles += --cpu.b ? 13 : 8;
		} while (cpu.b);
		cpu.cycles += 7;
		say = 0x24;
		if (!(cpu.f & FZ)) { cpu.cycles += 12; break; }
		cpu.cycles += 7 + 13 + 7 + 7 + 7;
		cpu.a = mem_read(0xC951) & 0x1C;
		op_sub8(0x1C, 0, 0);
		say = 0x25;
		if (!(cpu.f & FZ)) { cpu.cycles += 12; break; }
		cpu.cycles += 7 + 13 + 7 + 7 + 7;
		cpu.a = mem_read(0xC951) & 0xE0;
		op_sub8(0xE0, 0, 0);
		say = 0x26;
		if (!(cpu.f & FZ)) { cpu.cycles += 12; break; }
		cpu.cycles += 7 + 7;
		say = 0x27;
	} while (0);
	cpu.a = say;
	pages(0x8401);
}

// the menu, from $81B5; returns when the talk is over
static void topics(void) {
	static const uint8_t keys[4] = { 0x01, 0x02, 0x10, 0x20 };
top:
	cpu.cycles += 19 + 19;             // $81B5
	mem_write(TOP, 0);
	mem_write(ROW, 0);
list:
	call(0x36A3);                      // $81BD: the topics in the menu window
	for (;;) {
		task_frame_tick(0x81C0);
		cpu.cycles += 13 + 7 + 10 + 17;
		cpu.a = mem_read(0xC0B0) & 0x7F;
		uint8_t pressed = cpu.a;
		int k;
		for (k = 0; k < 4 && keys[k] != pressed; k++) cpu.cycles += 58;
		if (k == 4) {
			cpu.cycles += 7 + 4 + 11 + 12;
			cpu.b = 0;
			continue;
		}
		cpu.cycles += 41 + 38;
		cpu.b = (uint8_t)(keys[k] + 1);
		switch (k) {
		case 0:                        // up
			topic_here(0);
			if (cpu.f & FZ) { cpu.cycles += 12; continue; }
			cpu.cycles += 7;
			call(0x40D9);              // sound_1e
			cpu.cycles += 19 + 4;
			cpu.a = mem_read(ROW);
			op_or(cpu.a);
			if (!(cpu.f & FZ)) { cpu.cycles += 7; row_step(-1); continue; }
			cpu.cycles += 12 + 23 + 12;
			mem_write(TOP, (uint8_t)(mem_read(TOP) - 1));
			goto list;
		case 1:                        // down
			topic_here(0xC5E1);
			cpu.cycles += 4;
			op_or(cpu.a);
			if (cpu.f & FZ) { cpu.cycles += 12; continue; }
			cpu.cycles += 7;
			call(0x40D9);
			cpu.cycles += 19 + 7;
			cpu.a = mem_read(ROW);
			op_sub8(0x07, 0, 0);
			if (cpu.f & FC) { cpu.cycles += 7; row_step(1); continue; }
			cpu.cycles += 12 + 23 + 12;
			mem_write(TOP, (uint8_t)(mem_read(TOP) + 1));
			goto list;
		case 2:                        // button 1: back to the top
			topic_here(0);
			if (cpu.f & FZ) { cpu.cycles += 12; continue; }
			cpu.cycles += 7;
			call(0x40D9);
			cpu.cycles += 10;
			goto top;
		default:                       // button 2: this topic
			break;
		}
		call(0x40C9);                  // sound_0c
		uint8_t topic = topic_here(TOPICS);
		cpu.cycles += 19 + 7;
		mem_write(TOPIC, topic);
		cpu.a = (uint8_t)(topic + 0x3B);
		msg_a();
		cpu.cycles += 17 + 17;
		text_newline();
		text_newline();
		cpu.cycles += 19 + 7 + 10;
		cpu.a = topic;
		if (topic == 1) {              // $8359: farewell
			msg(0x36);
			cpu.cycles += 13 + 7 + 7;
			if (mem_read(PARTY_SIZE) >= 2) { cpu.cycles += 17; text_char(0x73); }
			else cpu.cycles += 10;
			msg(0x37);
			cpu.cycles += 7;
			cpu.a = 0x06;
			call(0x33AF);              // play_music_later
			cpu.cycles += 10;
			return;
		}
		cpu.cycles += 7 + 10;
		if (topic == 5) {              // $8375: healing?
			msg(0x28);
			call(0x4B5E);              // yes_no_start
			for (;;) {
				task_frame_tick(0x837D);
				call(0x4B69);          // yes_no_step
				cpu.cycles += 7;
				if (!(cpu.f & FC)) break;
				cpu.cycles += 5;
			}
			cpu.cycles += 20;
			if (mem_read(SLOT + 0x07) & 1) {
				cpu.cycles += 12;
				msg(0x2A);
				flash(0x8397);
				heal_all();
			} else {
				cpu.cycles += 7;
				msg(0x29);
				cpu.cycles += 12;
			}
			newline();
			cpu.cycles += 10;
		} else {
			cpu.cycles += 7 + 10;
			if (topic == 6) advice();
			else {
				cpu.cycles += 4;
				cpu.a = (uint8_t)(topic - 1);
				pages(0x826A);
				topic_leads_on();
			}
			if (topic == 6) cpu.cycles += 10;
		}
		msg(0x35);                     // $828E
		cpu.cycles += 10;
		goto list;
	}
}

void lord_british(void) {
	cpu.cycles += 7 + 13 + 10 + 16 + 7;
	mem_write(MESSAGE_BANK, 0x1A);
	wr16(MESSAGE_TABLE, 0x80EC);
	cpu.a = 0x07;
	call(0x33AF);                      // play_music_later
	cpu.cycles += 13 + 4;
	cpu.a = mem_read(0xC963);
	op_or(cpu.a);
	if (cpu.f & FZ) {                  // the first time
		cpu.cycles += 7 + 7 + 13;
		mem_write(0xC963, 0xFF);
		msg(0x2D);
		avatar_name();
		cpu.cycles += 7;
		cpu.a = 0x2E;
		pages(0x8150);
		msg(0x2F);
		cpu.cycles += 10;
	} else {
		cpu.cycles += 12 + 13 + 7;
		if (mem_read(0xC76D) == 0x44) {  // the Avatar is dead: raised
			cpu.cycles += 7;
			newline();
			cpu.cycles += 7 + 13;
			mem_write(0xC76D, 0x47);
			quote();
			avatar_name();
			msg(0x30);
			flash(0x8175);
			heal_all();
			call(0x4324);
			task_wait_button(0x817E);
			newline();
		} else cpu.cycles += 12;
		msg(0x31);
		avatar_name();
		cpu.cycles += 13 + 4;
		uint8_t size = mem_read(PARTY_SIZE);
		if (size == 1) cpu.cycles += 12;
		else {
			cpu.cycles += 7 + 4;
			if (size == 2) {
				cpu.cycles += 12;
				msg(0x33);
				cpu.cycles += 10;
				SET_HL(0xC77B);        // the second member's name
				call(0x5519);
				msg(0x34);
			} else {
				cpu.cycles += 7;
				msg(0x32);
				cpu.cycles += 12;
			}
		}
		task_wait_button(0x81AC);
		newline();
		level_up();
	}
	topics();
	command_end_at(0x8084);
}
