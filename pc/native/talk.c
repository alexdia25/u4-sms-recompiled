// Talking to a person, rewritten from bank 1 ($6DCE-$71A3) as a C task.
//
// The Talk command (bank 2) jumps to $6DCE once the player has picked the
// person in front of them. The conversation then runs as part of that task:
// a greeting, then a menu of topics (name, job, health, join, give, bye and
// the keywords learnt so far) that the player moves through with up/down,
// picks with button 2 and leaves with button 1. Each answer may unlock more
// topics; the person's own "question" topic asks a yes/no question that
// changes karma. Some people turn away when asked too often.
//
// The cycle counts are the original's (see tools/cycles.py), added where the
// original spends them. Routines not rewritten yet are run with call_original;
// none of them yields (only frame_tick and wait_button do, and those are in
// C: task_frame_tick, task_wait_button).
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define SLOT            cpu.ix
#define MENU_COUNT      (SLOT + 0x1B)   // topics in the menu (talk_topics)
#define MENU_TOP        (SLOT + 0x1C)   // first topic shown
#define MENU_ROW        (SLOT + 0x1D)   // cursor row, 0-7
#define TOWN_VIRTUE     (SLOT + 0x07)   // talk_join: the town's virtue; question: the answer
#define PERSON_KIND     (SLOT + 0x09)
#define TALK_FLAGS      0xC580          // 32 flags; +$80+topic: asked
#define MENU_TOPICS     0xC63D          // the topic numbers in the menu
#define TOPIC           0xC9A7          // the topic being answered
#define QUESTION_TOPIC  0xC9A9          // the topic that leads to the question
#define QUESTION_KARMA  0xC9AA
#define TURN_AWAY       0xC9AB
#define PERSON          0xC9A5          // the person's figure
#define LOCATION        0xC9A0
#define PARTY_SIZE      0xC957
#define AVATAR_LEVEL    0xC75B
#define COMPANIONS      0xC749          // 8 "in town" flags
#define KARMA           0xC90B
#define LAST_KARMA      0xC96B          // move count / 16 when karma was last given this way ($33F6)
#define JOY_PRESSED     0xC0B0

// the original's ways of calling things, with their cycles
static void call(uint16_t addr) { cpu.cycles += 17; call_original(addr); }
static void say(uint16_t text) { cpu.cycles += 10 + 17; text_print(text); }   // ld hl,text / call print_text_hl
static void say_hl(void) { cpu.cycles += 17; text_print(HL); }                 // call print_text_hl
static void say_last(uint16_t text) { cpu.cycles += 10 + 10; text_print(text); }   // ld hl,text / jp print_text_hl
static void say_string(uint8_t n) { cpu.cycles += 7; cpu.b = n; call(0x354D); }  // ld b,n / call: the person's string n
static void blank_line(void) {      // call $54CB: two print_newlines
	cpu.cycles += 17 + 17;
	text_newline();
	text_newline();
}
static uint8_t rst28(uint16_t table, uint8_t i) {   // ld hl,table / rst $28: A = table[i]
	cpu.cycles += 10 + 48;
	SET_HL(table + i);
	cpu.a = mem_read(HL);
	return cpu.a;
}
static uint16_t rst20(uint16_t table, uint8_t i) {  // ld hl,table / rst $20: HL = word table[i]
	cpu.cycles += 10 + (i < 0x80 ? 91 : 90);
	uint16_t at = (uint16_t)(table + 2 * i);
	SET_HL(rd16(at));
	cpu.a = mem_read(at);
	return HL;
}
static void sound(uint16_t which) { call(which); }   // $40C9 select, $40D1 leave, $40D9 move
static void exit_talk(void) { cpu.cycles += 10; command_end_at(0x8084); }   // jp $8084

// $6E24: A = the cursor row's screen line (row * 2 + 1)
static void row_line(void) {
	cpu.cycles += 17 + 37;
	cpu.a = (uint8_t)(mem_read(MENU_ROW) * 2 + 1);
}

// switch_on_a ($334A) over the menu keys {$01, $02, $10, $20}: which one, or 4
static int menu_key(uint8_t key) {
	static const uint8_t keys[4] = { 0x01, 0x02, 0x10, 0x20 };
	for (int i = 0; i < 4; i++)
		if (key == keys[i]) { cpu.cycles += 58 * i + 79; return i; }
	cpu.cycles += 58 * 4 + 22 + 12;   // no match: ret, jr back
	return 4;
}

// ---------------------------------------------------------------- topics --
// The answers to some topics are code of their own (the table at $6F45,
// called with rst $18). They return to the menu, or end the conversation.
enum { GO_ON, LEAVE };

static int topic_join(void);
static int topic_give(void);

static int answer_topic(uint8_t topic) {
	cpu.cycles += 13 + 10 + 73;        // ld a,(topic) / ld hl,$6F45 / rst $18
	cpu.a = topic;
	switch (topic) {
	case 0:                            // bye ($6F57)
		cpu.cycles += 10;              // pop af
		say(0x720E);
		cpu.cycles += 10;              // jp $6F42
		return LEAVE;
	case 7: return topic_join();
	case 8: return topic_give();
	default: {                         // the others answer and return
		uint16_t entry = rd16(0x6F45 + 2 * topic);
		SET_HL(entry);
		cpu.a = (uint8_t)entry;
		call_original(entry);
		return GO_ON;
	}
	}
}

// join ($6F96): only the first person of a companion's home town can join,
// when the Avatar's karma in that town's virtue is 0 (won) or at least 40
// and the party is smaller than the Avatar's level
static int topic_join(void) {
	cpu.cycles += 20 + 19 + 7;
	cpu.iy = rd16(PERSON);
	int refuse = 1;
	if (mem_read(cpu.iy + 7) == 1) {
		cpu.cycles += 7 + 13 + 7 + 7;
		uint8_t town = (uint8_t)(mem_read(LOCATION) - 6);
		if (town < 8) {
			cpu.cycles += 7 + 19 + 13 + 19 + 7;
			mem_write(TOWN_VIRTUE, town);
			if (mem_read(0xC76C) != town) { cpu.cycles += 5; refuse = 0; }
		} else cpu.cycles += 5;
	} else cpu.cycles += 5;
	if (refuse) {                      // $6FB5
		call(0x71B1);
		say_last(0x7216);
		return GO_ON;
	}
	cpu.cycles += 19;                  // $6FBE
	uint8_t karma = rst28(KARMA, mem_read(TOWN_VIRTUE));
	cpu.cycles += 4 + 7;
	if (karma != 0) {
		cpu.cycles += 7 + 7;
		if (karma < 0x28) {            // not virtuous enough yet
			say(0x722B);
			cpu.cycles += 19;
			rst20(0x734E, mem_read(TOWN_VIRTUE));
			say_hl();
			say_last(0x723A);
			return GO_ON;
		}
		cpu.cycles += 5;
	} else cpu.cycles += 5;
	cpu.cycles += 13 + 10 + 7 + 7;     // $6FE2: party_size - level
	if (mem_read(PARTY_SIZE) >= mem_read(AVATAR_LEVEL)) {
		say(0x722B);
		say(0x7258);
		say_last(0x723A);
		return GO_ON;
	}
	cpu.cycles += 5;                   // $6FFD: joins
	say(0x7264);
	cpu.cycles += 10 + 10;
	SET_HL(COMPANIONS); SET_BC(0x0800);
	call(0x333B);
	cpu.cycles += 14 + 10 + 19;
	cpu.iy = 0xC76E;
	SET_DE(0x001D);
	cpu.a = mem_read(TOWN_VIRTUE);
	for (;;) {                         // the companion's record among the 8
		cpu.cycles += 19 + 7;
		if (mem_read(cpu.iy + 0x1B) == cpu.a) { cpu.cycles += 5; break; }
		cpu.cycles += 15 + 12;
		cpu.iy += 0x1D;
	}
	cpu.cycles += 15 + 10 + 13 + 4;    // $701F
	SET_HL(cpu.iy);
	cpu.c = mem_read(PARTY_SIZE);
	cpu.a = cpu.c;
	call(0x48B6);                      // member_record -> IY
	cpu.cycles += 15 + 10 + 7;
	SET_DE(cpu.iy);
	cpu.b = 0x1D;
	call(0x3340);                      // swap the records
	cpu.cycles += 10 + 11 + 10 + 10;   // inc (party_size), pop af, jp $6F42
	mem_write(PARTY_SIZE, mem_read(PARTY_SIZE) + 1);
	return LEAVE;
}

// give ($7039): only beggars ask; the player enters an amount of gold
static int topic_give(void) {
	cpu.cycles += 20 + 19 + 7 + 7;
	cpu.iy = rd16(PERSON);
	if (mem_read(cpu.iy + 3) != 0x58) {
		call(0x71B1);
		say_last(0x7282);
		return GO_ON;
	}
	cpu.cycles += 5 + 10;              // pop af: no return to the menu code
	say(0x72A4);
	cpu.cycles += 7;
	cpu.a = 2;
	call(0x4A66);                      // number entry
	do {
		task_frame_tick(0x7059);
		call(0x4A9F);
		cpu.cycles += 7;
		if (cpu.f & FC) cpu.cycles += 5;
	} while (cpu.f & FC);
	cpu.cycles += 4 + 4 + 10;
	if (HL == 0) return GO_ON + 2;     // nothing given: back to after the answer
	blank_line();
	call(0x4423);                      // take the gold, if there is enough
	cpu.cycles += 7;
	if (cpu.f & FC) {
		say(0x72AF);
		task_wait_button(0x7074);
		cpu.cycles += 10;
		return GO_ON + 2;
	}
	cpu.cycles += 5;
	call(0x71B1);
	say(0x72CD);
	task_wait_button(0x7083);
	cpu.cycles += 16;
	SET_HL(rd16(LAST_KARMA));
	call(0x33F6);
	cpu.cycles += 16 + 7 + 7 + 10;
	wr16(LAST_KARMA, HL);
	cpu.a = 2; cpu.c = 1;
	if (!(cpu.f & FZ)) { cpu.cycles += 7; call_original(0x4255); }   // call nz,karma_gain
	cpu.cycles += 10;
	return GO_ON + 2;
}

// ------------------------------------------------------------- question --
// $7099: the person's question; the yes/no answer changes karma. One of the
// shrine-related people then teaches a mantra-like choice ($7100 on).
static void question(void) {
	blank_line();
	cpu.cycles += 7 + 17;
	cpu.b = 7;
	call_original(0x6F8D);
	task_wait_button(0x70A1);
	say(0x731E);
	call(0x4B5E);                      // yes/no prompt
	do {
		task_frame_tick(0x70AD);
		call(0x4B69);
		cpu.cycles += 7;
		if (cpu.f & FC) cpu.cycles += 5;
	} while (cpu.f & FC);
	cpu.cycles += 17;
	text_newline();
	cpu.cycles += 20 + 7;
	uint8_t a, b;
	if (!(mem_read(TOWN_VIRTUE) & 1)) {
		cpu.cycles += 13 + 4 + 7 + 7 + 10;
		uint8_t k = mem_read(QUESTION_KARMA);
		cpu.a = 5; cpu.c = 7;
		if (k) { cpu.cycles += 7; call_original(0x426F); }   // karma_lose
		cpu.cycles += 7 + 7 + 12;
		a = 9; b = 8;
	} else {
		cpu.cycles += 5 + 13 + 4 + 7;
		if (mem_read(QUESTION_KARMA)) {
			cpu.cycles += 16;
			SET_HL(rd16(LAST_KARMA));
			call(0x33F6);
			cpu.cycles += 7 + 7 + 10;
			cpu.a = 0x0A; cpu.c = 7;
			if (!(cpu.f & FZ)) { cpu.cycles += 7; call_original(0x4255); }   // karma_gain
		} else cpu.cycles += 5;
		cpu.cycles += 7 + 7;
		a = 0x0A; b = 9;
	}
	cpu.cycles += 13 + 17;             // $70E6
	mem_write(TOPIC, a);
	cpu.a = a; cpu.b = b;
	call_original(0x6F8D);
	cpu.cycles += 16;
	SET_HL(rd16(LAST_KARMA));
	call(0x33F6);
	cpu.cycles += 16;
	wr16(LAST_KARMA, HL);
	task_wait_button(0x70F5);
	cpu.cycles += 13 + 7 + 10;
	if (mem_read(TOPIC) != 9) return;
	// $7100: some people then offer a choice of 8 words
	cpu.cycles += 19 + 14 + 10 + 7;
	cpu.a = mem_read(PERSON_KIND);
	cpu.iy = 0x73A7;
	SET_DE(0x0006);
	cpu.b = 6;
	for (;;) {
		cpu.cycles += 19 + 7;
		if (mem_read(cpu.iy) == cpu.a) { cpu.cycles += 5; break; }
		cpu.cycles += 15;
		cpu.iy += 6;
		if (--cpu.b == 0) { cpu.cycles += 8 + 10; return; }
		cpu.cycles += 13;
	}
	call(0x2E1B);
	cpu.cycles += 10 * 19;
	mem_write(SLOT + 0x1F, mem_read(cpu.iy + 1));
	mem_write(SLOT + 0x0E, mem_read(cpu.iy + 2));
	mem_write(SLOT + 0x0F, mem_read(cpu.iy + 3));
	mem_write(SLOT + 0x10, mem_read(cpu.iy + 4));
	mem_write(SLOT + 0x11, mem_read(cpu.iy + 5));
	SET_HL(rd16(cpu.iy + 4));
	cpu.cycles += 10 + 7;
	SET_DE(0x0213);
	cpu.b = 8;
	do {                               // draw the 8 choices
		cpu.cycles += 11;
		uint16_t de = DE;
		call(0x2D69);
		cpu.cycles += 10 + 4 + 4;
		SET_DE(de + 0x0200);
		cpu.cycles += 8;
		if (--cpu.b) cpu.cycles += 5;
	} while (cpu.b);
	call(0x2E54);
	blank_line();
	say(0x732A);
	cpu.cycles += 19;
	mem_write(SLOT + 0x1A, 0);
	for (;;) {                         // pick one
		task_frame_tick(0x7157);
		cpu.cycles += 7 + 10;
		cpu.c = 8;
		SET_DE(0x0102);
		call(0x49F9);
		cpu.cycles += 7;
		if (cpu.f & FZ) { cpu.cycles += 5; continue; }
		cpu.cycles += 7;
		if (cpu.f & FC) { cpu.cycles += 5; continue; }
		break;
	}
	cpu.cycles += 19 + 19 + 19 + 4 + 4;   // $7166: skip to the picked word
	uint16_t p = rd16(SLOT + 0x10);
	uint8_t d = (uint8_t)(mem_read(SLOT + 0x1A) + 1);
	cpu.a = 0;
	for (;;) {
		cpu.cycles += 4 + 7;
		if (--d == 0) { cpu.cycles += 5; break; }
		cpu.cycles += 10 + 12;
		int n = 0;                     // cpir: up to and including the 0
		while (mem_read(p++) != 0) n++;
		cpu.cycles += 21 * n + 16;
	}
	SET_HL(p);
	cpu.d = d;
	SET_BC(0);
	say_hl();
	blank_line();
	cpu.cycles += 19 + 19 + 7;
	if (mem_read(SLOT + 0x1A) == mem_read(SLOT + 0x1F)) {   // the right one
		cpu.cycles += 7 + 13 + 7 + 17;
		mem_write(TOPIC, 6);
		cpu.a = 6; cpu.b = 6;
		call_original(0x6F8D);
		cpu.cycles += 12;
	} else {
		cpu.cycles += 5 + 19 + 19;
		SET_HL(rd16(SLOT + 0x0E));
		say_hl();
	}
	task_wait_button(0x719E);
	cpu.cycles += 10;
}

// ------------------------------------------------------------------ talk --
static void talk(void) {               // $6DCE
	call(0x34DD);                      // talk_setup: the person's dialogue
	cpu.cycles += 10 + 10;
	SET_HL(TALK_FLAGS); SET_BC(0x2000);
	call(0x333B);                      // clear the talk flags
	cpu.cycles += 19 + 19;
	mem_write(MENU_TOP, 0);
	mem_write(MENU_ROW, 0);
	say(0x71D9);
	say_string(2);                     // the person's description
	call(0x2BBF);                      // random
	cpu.cycles += 7 + 10;
	op_and(1);
	if (cpu.a) {                       // half the time they also say their name
		cpu.cycles += 17;
		text_newline();
		call(0x6F61);
	}
	task_wait_button(0x6DFB);
	int restart = 1;
	for (;;) {
		if (restart) say(0x71F3);   // $6DFE
		restart = 0;
		call(0x3578);                  // $6E04 talk_topics: the menu
		int redraw = 0;
		while (!redraw) {              // $6E07
			task_frame_tick(0x6E07);
			cpu.cycles += 13 + 7 + 10 + 17;
			uint8_t key = mem_read(JOY_PRESSED) & 0x7F;
			cpu.a = key;
			int top = mem_read(MENU_TOP), row = mem_read(MENU_ROW);
			switch (menu_key(key)) {
			case 0:                    // up
				cpu.cycles += 19 + 19 + 7;
				if (top + row == 0) { cpu.cycles += 5; break; }
				sound(0x40D9);
				cpu.cycles += 19 + 4 + 7;
				if (row == 0) {        // scroll the menu
					cpu.cycles += 5 + 23 + 12;
					mem_write(MENU_TOP, top - 1);
					redraw = 1;
					break;
				}
				row_line(); call(0x49E0);   // cursor off
				cpu.cycles += 23;
				mem_write(MENU_ROW, row - 1);
				row_line(); call(0x49D5);   // cursor on
				cpu.cycles += 12;
				break;
			case 1:                    // down
				cpu.cycles += 19 + 19 + 4 + 19 + 7;
				if ((uint8_t)(top + row + 1) >= mem_read(MENU_COUNT)) { cpu.cycles += 5; break; }
				sound(0x40D9);
				cpu.cycles += 19 + 7 + 7;
				if (row >= 7) {
					cpu.cycles += 5 + 23 + 12;
					mem_write(MENU_TOP, top + 1);
					redraw = 1;
					break;
				}
				row_line(); call(0x49E0);
				cpu.cycles += 23;
				mem_write(MENU_ROW, row + 1);
				row_line(); call(0x49D5);
				cpu.cycles += 12;
				break;
			case 2:                    // button 1: leave ($6F3C)
				sound(0x40D1);
				blank_line();
				exit_talk();
				return;
			case 3: {                  // button 2: ask about the topic ($6E7D)
				sound(0x40C9);
				cpu.cycles += 19 + 19;
				uint8_t topic = rst28(MENU_TOPICS, (uint8_t)(top + row));
				cpu.cycles += 7 + 7;
				if (topic < 0x0A) cpu.cycles += 5;
				else { cpu.cycles += 7; topic -= 5; }
				cpu.cycles += 13;
				mem_write(TOPIC, topic);
				cpu.a = topic;
				call(0x3642);          // "<topic>" said back
				call(0x2BBF);          // random
				cpu.cycles += 10 + 7 + 7;
				uint8_t r = cpu.a, limit = mem_read(TURN_AWAY);
				if (r < limit) {       // asked too much?
					uint8_t d = (uint8_t)(limit - r);
					cpu.cycles += 8 + 7 + 7 + 7;
					if (d < 0x40) {    // $6ECC: they don't answer
						cpu.cycles += 5;
						call(0x71A4);
						say(0x7311);
						cpu.cycles += 10;
					} else {           // they turn away for good
						call(0x3666);
						cpu.cycles += 7;
						if (!(cpu.f & FZ)) { cpu.cycles += 5; call(0x71B1); }
						else {
							say_string(1);
							say(0x71E4);
							cpu.cycles += 12;
						}
						say(0x7301);
						cpu.cycles += 20 + 19 + 10;
						cpu.iy = rd16(PERSON);
						mem_write(cpu.iy + 6, 0xFF);
					}
					sound(0x40D1);     // $6F3C
					blank_line();
					exit_talk();
					return;
				}
				cpu.cycles += 5;
				int how = answer_topic(topic);   // $6ED8
				if (how == LEAVE) { exit_talk(); return; }
				if (how == GO_ON) task_wait_button(0x6EDF);
				// $6EE2: after an answer
				for (;;) {
					cpu.cycles += 13 + 7 + 4 + 7 + 10 + 13 + 10 + 7 + 10;
					uint8_t t = mem_read(TOPIC);
					mem_write(0xC500 + (uint8_t)(t + 0x80), 1);   // asked
					if (t == mem_read(QUESTION_TOPIC)) { question(); continue; }
					cpu.cycles += 7 + 10;
					if (t != 5) break;
					cpu.cycles += 19 + 7 + 10;
					if (mem_read(PERSON_KIND) != 0xAC) break;
					blank_line();      // $6F03: a name to type
					say(0x732A);
					cpu.cycles += 7;
					cpu.a = 4;
					call(0x4D0A);
					for (;;) {
						task_frame_tick(0x6F11);
						call(0x4D2B);
						cpu.cycles += 7;
						if (cpu.f & FZ) { cpu.cycles += 5; continue; }
						cpu.cycles += 7;
						if (cpu.f & FC) { cpu.cycles += 5; continue; }
						break;
					}
					blank_line();
					cpu.cycles += 10 + 10;
					SET_HL(0xC065); SET_DE(0x734B);
					call(0x34BB);      // compare
					cpu.cycles += 7;
					if (cpu.f & FC) { cpu.cycles += 5; say(0x733A); }
					else {
						cpu.cycles += 7 + 17;
						cpu.b = 6;
						call_original(0x6F8D);
						cpu.cycles += 12;
					}
					task_wait_button(0x6F36);
					cpu.cycles += 10;
					break;
				}
				restart = 1;
				redraw = 1;
				break;
			}
			default:                   // other buttons: nothing
				break;
			}
		}
	}
}
Native native_talk = { "talk", 0x6DCE, 1, talk, 0, 0, 0, .task = 1 };
