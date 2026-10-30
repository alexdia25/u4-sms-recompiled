// A person's dialogue, for the talk task (bank 0).
// - talk_setup ($34DD): for person IY: the dialogue's number in the place
//   (place * 16 + the person's dialogue byte - 1, at IX+9), its bank (the
//   3-byte entries at $351D: bank, then the address of the place's table
//   of dialogues) kept at $C9A8; from the dialogue: its four question
//   topics to $C9A9, the address of its topic list at $C9AD, and that of
//   its strings (after the list's $FF) at $C9AF.
// - dialogue_string ($355F): HL = string B (from 1) of the dialogue, A =
//   its bank.
// - dialogue_print ($354D, $3553 without keeping HL): string B printed.
// - talk_topics ($3578): every topic in the list whose flag in $C5xx is
//   set marks the topic it leads to (the second byte, $10 more when it
//   is negative) as known; then the menu: the seven words at $3606, and
//   the two keywords ($0A, $0B) once known ($C595, $C596), eight at most
//   from IX+$1C, each a name from $360D or (from $0A) a dialogue string;
//   the cursor on row IX+$1D.
// - say_topic ($3642): the topic chosen said back (5 and 6: the person's
//   strings $0A, $0B; else its word at $360D).
// - name_has_article ($3666): Z when the person's name begins "a " or
//   "the " (in any case, $368C).
// - say_welcome ($AE20, bank 2): "Welcome to" ($BCC6), the place's name
//   at HL, "!", a new line, and the text at $BCCD.
// Cycle counts are the original's, instruction by instruction (I).
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define BANK        0xC9A8
#define QUESTIONS   0xC9A9
#define TOPIC_LIST  0xC9AD
#define STRINGS     0xC9AF
#define WORDS       0xC63D
#define VRAM_QUEUED 0xC063

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static void push(uint16_t v) { push16(v); I(11); }
static void call(uint16_t addr) { I(17); call_original(addr); }

static void add_a_to_hl(void) {        // add a,l / ld l,a / adc a,h / sub l / ld h,a
	op_add8(cpu.l, 0); I(4);
	cpu.l = cpu.a; I(4);
	op_add8(cpu.h, cpu.f & FC); I(4);
	op_sub8(cpu.l, 0, 1); I(4);
	cpu.h = cpu.a; I(4);
}
static void word_at(void) {            // $0097: HL = (HL)
	cpu.a = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	cpu.h = mem_read(HL); I(7);
	cpu.l = cpu.a; I(4);
	I(10);
}
static void rst20(void) {              // HL = the word HL[A]
	I(11); I(10);
	op_add8(cpu.a, 0); I(4);
	if (!(cpu.f & FC)) I(12);
	else { I(7); cpu.h = op_inc8(cpu.h); I(4); }
	add_a_to_hl();
	cpu.a = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	cpu.h = mem_read(HL); I(7);
	cpu.l = cpu.a; I(4);
	I(10);
}
static void rst28(void) {              // A = (HL + A)
	I(11);
	add_a_to_hl();
	cpu.a = mem_read(HL); I(7);
	I(10);
}
static void rst30(void) {              // HL += A
	I(11);
	op_add8(cpu.l, 0); I(4);
	cpu.l = cpu.a; I(4);
	if (!(cpu.f & FC)) { I(11); return; }
	I(5);
	cpu.h = op_inc8(cpu.h); I(4);
	I(10);
}
static void ldir(void) {
	while (op_block(6, 0)) I(21);
	I(16);
}

static void talk_setup(void) {         // $34DD
	wr16(0xC9A5, cpu.iy); I(20);
	call(0x33EE);                      // place_index
	cpu.b = cpu.a; I(4);
	for (int i = 0; i < 4; i++) { op_add8(cpu.a, 0); I(4); }
	cpu.c = mem_read((uint16_t)(cpu.iy + 7)); I(19);
	cpu.c = op_dec8(cpu.c); I(4);
	op_add8(cpu.c, 0); I(4);
	mem_write((uint16_t)(cpu.ix + 9), cpu.a); I(19);
	cpu.a = cpu.b; I(4);
	op_add8(cpu.a, 0); I(4);
	op_add8(cpu.b, 0); I(4);
	SET_HL(0x351D); I(10);
	rst30();
	cpu.a = mem_read(HL); I(7);
	mem_write(BANK, cpu.a); I(13);
	vram_bank_push(0x34FC);
	SET_HL(HL + 1); I(6);
	I(17); word_at();
	cpu.a = cpu.c; I(4);
	rst20();
	SET_DE(QUESTIONS); I(10);
	SET_BC(0x0004); I(10);
	ldir();
	wr16(TOPIC_LIST, HL); I(16);
	cpu.a = 0xFF; I(7);
	SET_BC(0x0000); I(10);
	while (op_block(6, 1)) I(21);      // cpir: past the list's $FF
	I(16);
	wr16(STRINGS, HL); I(16);
	vram_bank_pop();
}

static void dialogue_string(void) {    // $355F
	cpu.a = mem_read(BANK); I(13);
	vram_bank_push(0x3562);
	SET_HL(rd16(STRINGS)); I(16);
	op_xor(cpu.a); I(4);
	for (;;) {
		cpu.b = op_dec8(cpu.b); I(4);
		if (cpu.f & FZ) { I(12); break; }
		I(7);
		do {                           // past one string, to its 0
			op_sub8(mem_read(HL), 0, 0); I(7);
			SET_HL(HL + 1); I(6);
			if (cpu.f & FZ) { I(7); break; }
			I(12);
		} while (1);
		I(12);
	}
	cpu.a = mem_read(BANK); I(13);
	vram_bank_pop();
}

static void dialogue_print_hl(void) {  // $3553
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: printing may wait for frames
	I(17); dialogue_string();
	vram_bank_push(0x3556);
	call(0x5519);                      // print_text_hl
	vram_bank_pop();
}

static void dialogue_print(void) {     // $354D
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: printing may wait for frames
	push(HL);
	I(17); dialogue_print_hl();
	SET_HL(pop16()); I(10);
	I(10);
}

static void talk_topics(void) {        // $3578
	// shadow mode: the menu's copies to VRAM may have to wait for room
	if (!native_on_fiber() && mem_read(VRAM_QUEUED) >= 0x18) { native_decline(); return; }
	cpu.a = mem_read(BANK); I(13);
	vram_bank_push(0x357B);
	SET_HL(rd16(TOPIC_LIST)); I(16);
	cpu.d = 0xC5; I(7);
	for (;;) {
		cpu.a = mem_read(HL); I(7);
		op_sub8(0xFF, 0, 0); I(7);
		if (cpu.f & FZ) { I(12); break; }
		I(7);
		SET_HL(HL + 1); I(6);
		cpu.e = cpu.a; I(4);
		cpu.a = mem_read(DE); I(7);
		op_or(cpu.a); I(4);
		if (cpu.f & FZ) I(12);
		else {
			I(7);
			cpu.a = mem_read(HL); I(7);
			op_or(cpu.a); I(4);
			I(10);                     // jp p
			if (cpu.f & FS) { op_add8(0x10, 0); I(7); }
			cpu.e = cpu.a; I(4);
			cpu.a = 0x01; I(7);
			mem_write(DE, cpu.a); I(7);
		}
		SET_HL(HL + 1); I(6);
		I(12);
	}
	SET_HL(0x3606); I(10);
	SET_DE(WORDS); I(10);
	SET_BC(0x0007); I(10);
	ldir();
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
	uint16_t count = (uint16_t)(cpu.ix + 0x1B);
	mem_write(count, 0x07); I(19);
	cpu.a = mem_read(0xC595); I(13);
	op_or(cpu.a); I(4);
	if (cpu.f & FZ) I(12);
	else {
		I(7);
		mem_write(HL, 0x0A); I(10);
		SET_HL(HL + 1); I(6);
		mem_write(count, op_inc8(mem_read(count))); I(23);
	}
	cpu.a = mem_read(0xC596); I(13);
	op_or(cpu.a); I(4);
	if (cpu.f & FZ) I(12);
	else {
		I(7);
		mem_write(HL, 0x0B); I(10);
		mem_write(count, op_inc8(mem_read(count))); I(23);
	}
	call(0x2E1B);                      // menu_window_clear
	SET_DE(0xCAA6); I(10);
	cpu.c = mem_read((uint16_t)(cpu.ix + 0x1C)); I(19);
	cpu.a = mem_read(count); I(19);
	cpu.b = cpu.a; I(4);
	op_sub8(0x08, 0, 0); I(7);
	if (cpu.f & FC) I(12);
	else { I(7); cpu.b = 0x08; I(7); }
	do {
		cpu.a = cpu.c; I(4);
		SET_HL(WORDS); I(10);
		rst28();
		op_sub8(0x0A, 0, 0); I(7);
		if (cpu.f & FC) {
			I(12);
			SET_HL(0x360D); I(10);
			rst20();
		} else {
			I(7);
			push(BC);
			cpu.b = cpu.a; I(4);
			I(17); dialogue_string();
			SET_BC(pop16()); I(10);
			I(12);
		}
		push(DE);
		call(0x2D6C);                  // text_chars
		SET_DE(pop16()); I(10);
		SET_HL(0x0080); I(10);
		SET_HL(op_add16(HL, DE)); I(11);
		{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
		cpu.c = op_inc8(cpu.c); I(4);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	cpu.a = mem_read((uint16_t)(cpu.ix + 0x1D)); I(19);
	op_add8(cpu.a, 0); I(4);
	cpu.a = op_inc8(cpu.a); I(4);
	call(0x49D5);                      // cursor_on
	call(0x2E54);                      // menu_window_show
	vram_bank_pop();
}

static void say_welcome(void) {        // $AE20
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: printing may wait for frames
	push(HL);
	SET_HL(0xBCC6); I(10);
	call(0x5519);                      // print_text_hl
	SET_HL(pop16()); I(10);
	call(0x5519);
	cpu.a = 0x21; I(7);
	call(0x548F);                      // print_char
	call(0x54CE);                      // print_newline
	SET_HL(0xBCCD); I(10);
	I(10);
	native_continue_at(0x5519);        // jp print_text_hl
}

// $3642: topic A said back: for 5 and 6 the person's own strings $0A and
// $0B, else the topic's word ($360D); then $54CB
static void say_topic(void) {
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: printing may wait for frames
	cpu.b = 0x0A; I(7);
	op_sub8(0x05, 0, 0); I(7);
	if (cpu.f & FZ) I(12);
	else {
		I(7);
		cpu.b = op_inc8(cpu.b); I(4);
		op_sub8(0x06, 0, 0); I(7);
		if (!(cpu.f & FZ)) {           // $365C
			I(12);
			SET_HL(0x360D); I(10);
			rst20();
			call(0x5519);              // print_text_hl
			I(10); native_continue_at(0x54CB);
			return;
		}
		I(7);
	}
	I(17); dialogue_string();          // $364D
	vram_bank_push(0x3650);
	call(0x5519);
	call(0x54CB);
	vram_bank_pop();
}

static void upcase(void) {             // $34D4: A in upper case
	I(17);
	op_sub8(0x61, 0, 0); I(7);
	if (cpu.f & FC) { I(11); return; }
	I(5);
	op_sub8(0x7B, 0, 0); I(7);
	if (!(cpu.f & FC)) { I(11); return; }
	I(5);
	op_add8(0xE0, 0); I(7);
	I(10);
}

static void starts_with(void) {        // $368C: Z if the B letters at HL are those at DE, any case
	I(17);
	push(BC); push(DE); push(HL);
	for (;;) {
		cpu.a = mem_read(HL); I(7);
		SET_HL(HL + 1); I(6);
		upcase();
		cpu.c = cpu.a; I(4);
		cpu.a = mem_read(DE); I(7);
		SET_DE(DE + 1); I(6);
		upcase();
		op_sub8(cpu.c, 0, 0); I(4);
		if (!(cpu.f & FZ)) { I(12); break; }
		I(7);
		if (--cpu.b) I(13); else { I(8); break; }
	}
	SET_HL(pop16()); I(10);
	SET_DE(pop16()); I(10);
	SET_BC(pop16()); I(10);
	I(10);
}

// $3666: Z when the person's name (string 1) begins "a " or "the "
static void name_has_article(void) {
	cpu.a = mem_read(0xC0CA); I(13);   // message_bank
	vram_bank_push(0x3669);
	cpu.b = 0x01; I(7);
	I(17); dialogue_string();
	SET_DE(0x3686); I(10);
	cpu.b = 0x02; I(7);
	starts_with();
	if (cpu.f & FZ) I(12);
	else {
		I(7);
		SET_DE(0x3688); I(10);
		cpu.b = 0x04; I(7);
		starts_with();
	}
	vram_bank_pop();
}

// $71A4: who the person is, by $C9AC ("He", "She", "It", ..., the table
// at $71BD); $71B1: and ' says, "' ($71E4) after it (bank 1)
static void pronoun_body(void) {
	push(HL);
	cpu.a = mem_read(0xC9AC); I(13);
	SET_HL(0x71BD); I(10);
	rst20();
	call(0x5519);                      // print_text_hl
	SET_HL(pop16()); I(10);
	I(10);
}
static void pronoun(void) {
	if (!native_on_fiber()) { native_decline(); return; }   // shadow mode: printing may wait for frames
	pronoun_body();
}
static void pronoun_says(void) {
	if (!native_on_fiber()) { native_decline(); return; }
	push(HL);
	I(17); pronoun_body();
	SET_HL(0x71E4); I(10);
	call(0x5519);
	SET_HL(pop16()); I(10);
	I(10);
}

#define DIALOGUE(name, addr, out) Native native_##name = { #name, addr, 0, name, out, .long_calls = 1 };
DIALOGUE(talk_setup, 0x34DD, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
DIALOGUE(dialogue_print, 0x354D, OUT_A | OUT_F | OUT_BC | OUT_DE)
DIALOGUE(dialogue_print_hl, 0x3553, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
DIALOGUE(dialogue_string, 0x355F, OUT_A | OUT_F | OUT_BC | OUT_HL)
DIALOGUE(talk_topics, 0x3578, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
DIALOGUE(say_topic, 0x3642, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
DIALOGUE(name_has_article, 0x3666, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL)
Native native_say_welcome = { "say_welcome", 0xAE20, 2, say_welcome, OUT_A | OUT_F | OUT_HL, .long_calls = 1 };
Native native_pronoun = { "pronoun", 0x71A4, 1, pronoun, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, .long_calls = 1 };
Native native_pronoun_says = { "pronoun_says", 0x71B1, 1, pronoun_says, OUT_A | OUT_F | OUT_BC | OUT_DE | OUT_HL, .long_calls = 1 };
