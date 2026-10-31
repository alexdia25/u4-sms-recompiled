// The sound driver (bank $1F), called each frame and to start tunes and
// sounds. Eight channels of $28 bytes at $DD5D (IX), each a little
// sequencer: notes, rests and commands from its song data, with volume
// envelopes ($9035), modulation ($9016), slides, and the PSG (and, if
// fitted, the FM unit's) registers written at the end. While a channel is
// worked on, the alternate registers hold its flags: D' = +$00, E' = +$0B,
// C' = +$0C. It runs with interrupts on (the frame interrupt's end), so
// the C lets the line interrupts in as it goes (native_sync). Cycle counts
// are the original's.
#include <stdio.h>
#include <stdlib.h>
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

static void start_sound(void);
static void silence(void);
#define IXR(d)      mem_read((uint16_t)(cpu.ix + (d)))
#define IXW(d, v)   mem_write((uint16_t)(cpu.ix + (d)), (uint8_t)(v))

static int alt_e_bit(int n) {          // exx / bit n,e / exx: the channel's flags +$0B
	cpu.cycles += 4 + 8 + 4;
	native_sync();
	return (cpu.e_ >> n) & 1;
}

// $87AA: the PSG's volumes and the noise off (and the FM unit's keys)
static void silence(void) {
	cpu.cycles += 4 + 13 * 4 + 4 + 13 + 13 + 4;
	native_sync();
	cpu.a = 0;
	op_sub8(cpu.a, 0, 1);
	mem_write(0xDD05, 0); mem_write(0xDD08, 0); mem_write(0xDD0B, 0); mem_write(0xDD0D, 0);
	cpu.a = op_dec8(cpu.a);
	mem_write(0xDD5B, 0xFF);
	cpu.a = mem_read(0xDD58);
	op_and(cpu.a);
	if (!(cpu.a & 0x80)) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 7 + 7 + 8 + 13 + 13 + 4 + 13 * 9 + 4 + 13 * 3 + 10;
	native_sync();
	cpu.a &= 0x40;
	op_xor(0x40);
	cpu.a = op_rot(7, cpu.a);          // srl a
	mem_write(0xDD3F, cpu.a);
	mem_write(0xDD40, cpu.a);
	for (int i = 0; i < 9; i++) mem_write(0xDD1B + i, 0);
	for (int i = 0; i < 3; i++) mem_write(0xDD2A + i, 0xFF);
	cpu.a = 0xFF;
	op_dec8(0);
}
Native native_snd_silence = { "snd_silence", 0x87AA, 0x1F, silence, 0, .long_calls = 1 };

// $86A9: BC = the tone of note A (0: none) for this channel: the PSG's table
// ($8E2D) or the FM's ($8D6D), transposed by +$04 and tuned by +$21; a noise
// channel (bit 3 of +$00) keeps the note in +$1D instead
static void note_tone(void) {
	cpu.cycles += 10 + 4;
	native_sync();
	SET_BC(0);
	op_and(cpu.a);
	if (!cpu.a) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 20 + 7;
	native_sync();
	if (IXR(0) & 0x08) {
		cpu.cycles += 5 + 4 + 19 + 19 + 10;
		native_sync();
		cpu.c = cpu.a;
		op_add8(IXR(4), 0);
		IXW(0x1D, cpu.a);
		return;
	}
	cpu.cycles += 19 + 4 + 4 + 19 + 4 + 7;
	native_sync();
	op_add8(IXR(4), 0);
	op_add8(cpu.a, 0);
	cpu.c = cpu.a;
	cpu.a = IXR(9);
	op_and(cpu.a);
	uint16_t table;
	if (cpu.a) { cpu.cycles += 10 + 10; table = 0x8D6D; }
	else { cpu.cycles += 5 + 10; table = 0x8E2D; }
	cpu.cycles += 11 + 7 + 6 + 7 + 19 + 4;
	native_sync();
	SET_HL(op_add16(table, BC));
	cpu.c = mem_read(HL);
	SET_HL(HL + 1);
	cpu.b = mem_read(HL);
	cpu.a = IXR(0x21);
	op_and(cpu.a);
	if (!cpu.a) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 7 + 10;
	native_sync();
	cpu.h = 0;
	if (cpu.a & 0x80) { cpu.cycles += 4; cpu.h = 0xFF; }
	cpu.cycles += 4 + 11 + 4 + 4 + 10;
	native_sync();
	cpu.l = cpu.a;
	SET_HL(op_add16(HL, BC));
	cpu.c = cpu.l;
	cpu.b = cpu.h;
}
Native native_snd_note_tone = { "snd_note_tone", 0x86A9, 0x1F, note_tone, OUT_BC, .long_calls = 1 };

// $8715: an envelope's or modulation's command byte A at HL (index C): $81
// back by the next byte, $82 back to the start, $83 hold (or, released,
// go on), else the next pair (or, released, like $81)
static void table_command(void) {
	cpu.cycles += 7 + 10;
	native_sync();
	op_sub8(0x81, 0, 0);
	if (cpu.a == 0x81) {
	back:
		cpu.cycles += 6 + 4 + 7 + 15 + 4 + 4 + 6 + 7 + 10;
		native_sync();
		SET_HL(HL + 1);
		cpu.a = cpu.c;
		cpu.c = mem_read(HL);
		op_sbc16(BC);
		op_sub8(cpu.c, 0, 1);
		cpu.c = cpu.a;
		SET_HL(HL - 1);
		cpu.a = mem_read(HL);
		return;
	}
	cpu.cycles += 7 + 10;
	native_sync();
	op_sub8(0x82, 0, 0);
	if (cpu.a == 0x82) {
		cpu.cycles += 15 + 7 + 7 + 10;
		native_sync();
		op_sbc16(BC);
		cpu.a = mem_read(HL);
		cpu.c = 0;
		return;
	}
	cpu.cycles += 7 + 10;
	native_sync();
	if (cpu.a == 0x83) {
		cpu.cycles += 7;
		native_sync();
		if (alt_e_bit(2)) {
			cpu.cycles += 6 + 7 + 4 + 10;
			native_sync();
			SET_HL(HL - 1);
			cpu.a = mem_read(HL);
			cpu.c--;
		} else {
			cpu.cycles += 5 + 4 + 6 + 7 + 10;
			native_sync();
			cpu.c++;
			SET_HL(HL + 1);
			cpu.a = mem_read(HL);
		}
		return;
	}
	op_sub8(0x83, 0, 0);               // (no carry, for the sbc of $871A)
	cpu.cycles += 7;
	native_sync();
	if (alt_e_bit(2)) { cpu.cycles += 5; goto back; }
	cpu.cycles += 4 + 4 + 6 + 6 + 7 + 10;
	native_sync();
	cpu.c += 2;
	SET_HL(HL + 2);
	cpu.a = mem_read(HL);
}
Native native_snd_table_command = { "snd_table_command", 0x8715, 0x1F, table_command, OUT_A | OUT_BC | OUT_HL, .long_calls = 1 };

// $86E4: the channel's volume this frame, from envelope A/2 ($9035) at step
// +$1E, plus its attenuation +$01, into $DD5A
static void envelope(void) {
	cpu.cycles += 10 + 4 + 7 + 11 + 7 + 6 + 7 + 4 + 19 + 11 + 7 + 4 + 10;
	native_sync();
	cpu.c = cpu.a;
	cpu.b = 0;
	SET_HL(op_add16(0x9035, BC));
	cpu.e = mem_read(HL);
	SET_HL(HL + 1);
	cpu.d = mem_read(HL);
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); }
	cpu.c = IXR(0x1E);
	SET_HL(op_add16(HL, BC));
	cpu.a = mem_read(HL);
	op_and(cpu.a);
	if (cpu.a & 0x80) {
		cpu.cycles += 7 + 7;
		native_sync();
		if (cpu.a == 0x80) {           // the end: hold the last
			op_sub8(0x80, 0, 0);
			cpu.cycles += 6 + 7 + 10;
			native_sync();
			SET_HL(HL - 1);
			cpu.a = mem_read(HL);
			goto add;
		}
		op_sub8(0x80, 0, 0);
		cpu.cycles += 5;
		native_sync();
		cpu.cycles += 17;
		native_sync();
		table_command();
	}
	cpu.cycles += 4 + 19;
	native_sync();
	cpu.c++;
	IXW(0x1E, cpu.c);
add:
	cpu.cycles += 19 + 7 + 10;
	native_sync();
	op_add8(IXR(1), 0);
	op_sub8(0x0F, 0, 1);
	if (cpu.f & FC) { cpu.cycles += 4; cpu.a = 0; op_sub8(0, 0, 1); }
	cpu.cycles += 13 + 10;
	native_sync();
	mem_write(0xDD5A, cpu.a);
}
Native native_snd_envelope = { "snd_envelope", 0x86E4, 0x1F, envelope, 0, .long_calls = 1 };

// $874E: the pitch's modulation, step +$1F of table A/2 ($9016), its sign
// by bit 7 of +$03 (bit 6: clamp), added to the tone +$16/+$17
static void modulation(void) {
	cpu.cycles += 10 + 4 + 7 + 11 + 7 + 6 + 7 + 4 + 19 + 11 + 7 + 4 + 10;
	native_sync();
	cpu.c = cpu.a;
	cpu.b = 0;
	SET_HL(op_add16(0x9016, BC));
	cpu.e = mem_read(HL);
	SET_HL(HL + 1);
	cpu.d = mem_read(HL);
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); }
	cpu.c = IXR(0x1F);
	SET_HL(op_add16(HL, BC));
	cpu.a = mem_read(HL);
	op_and(cpu.a);
	if (cpu.a & 0x80) {
		cpu.cycles += 7 + 10;
		native_sync();
		op_sub8(0x85, 0, 0);
		if (cpu.f & FC) { cpu.cycles += 17; table_command(); }
	}
	cpu.cycles += 4 + 19 + 19 + 8 + 10;
	native_sync();
	cpu.c++;
	IXW(0x1F, cpu.c);
	cpu.e = IXR(3);
	if (cpu.e & 0x80) { cpu.cycles += 4 + 4; op_cpl(); cpu.a = op_inc8(cpu.a); }
	cpu.cycles += 4;
	native_sync();
	op_and(cpu.a);
	if (!cpu.a) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 7 + 4 + 10;
	native_sync();
	cpu.b = 0;
	cpu.c = cpu.a;
	if (cpu.a & 0x80) { cpu.cycles += 4; cpu.b = 0xFF; }
	cpu.cycles += 19 + 19 + 4 + 4;
	native_sync();
	cpu.l = IXR(0x16);
	cpu.h = IXR(0x17);
	cpu.a = cpu.h;
	op_or(cpu.l);
	if (!cpu.a) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 4 + 15 + 8 + 10;
	native_sync();
	cpu.a = cpu.h;
	op_sbc16(BC);
	if (cpu.e & 0x40) {                // clamped: not past the top or bottom
		cpu.cycles += 4 + 7;
		native_sync();
		op_sub8(cpu.h, 0, 0);
		if (!(cpu.f & FZ)) {
			cpu.cycles += 8 + 7;
			native_sync();
			if (cpu.b & 0x80) { cpu.cycles += 19 + 10; IXW(0x16, 0xFF); }
			else { cpu.cycles += 5 + 19 + 10; IXW(0x16, 0x00); }
			return;
		}
		cpu.cycles += 5;
		native_sync();
	}
	cpu.cycles += 19 + 19 + 10;
	native_sync();
	IXW(0x16, cpu.l);
	IXW(0x17, cpu.h);
}
Native native_snd_modulation = { "snd_modulation", 0x874E, 0x1F, modulation, 0, .long_calls = 1 };

// $8295: a slide: +$20 >= $40: by notes (the FM's, or the PSG's tone tables
// stepped +$20 & $3F at a time); below, the tone moved by its own >> +$20
static void slide(void) {
	cpu.cycles += 19 + 7 + 10;
	native_sync();
	cpu.a = IXR(0x20);
	op_sub8(0x40, 0, 0);
	if (cpu.a >= 0x40) {
		cpu.cycles += 7 + 4 + 13 + 8 + 7;
		native_sync();
		op_and(0x3F);
		cpu.c = cpu.a;
		cpu.a = mem_read(0xDD02);
		if (cpu.a & 0x80) {            // FM
			cpu.cycles += 19 + 4 + 7;
			native_sync();
			cpu.a = IXR(0x13);
			op_and(cpu.a);
			if (cpu.a) { cpu.cycles += 19; op_add8(IXR(4), 0); }
			else cpu.cycles += 5;
			cpu.cycles += 19 + 10;
			native_sync();
			IXW(0x1B, cpu.a);
			return;
		}
		cpu.cycles += 5 + 19 + 4;
		native_sync();
		cpu.a = IXR(0x1B);
		op_and(cpu.a);
		if (!cpu.a) { cpu.cycles += 11; return; }
		cpu.cycles += 5;
		native_sync();
		int down = alt_e_bit(6);
		cpu.cycles += 7;
		native_sync();
		if (!down) { cpu.cycles += 4 + 12; op_sub8(cpu.c, 0, 1); }
		else { cpu.cycles += 5 + 4; op_add8(cpu.c, 0); }
		cpu.cycles += 7;
		native_sync();
		if (!cpu.a) { cpu.cycles += 5; goto clear; }
		cpu.cycles += 19 + 8 + 7;
		native_sync();
		cpu.b = op_rot(0, IXR(9));     // rlc b
		uint16_t table;
		if (cpu.b) {
			cpu.cycles += 10 + 7 + 12;
			native_sync();
			table = 0x8D6D;
			op_sub8(0x61, 0, 0);
		} else {
			cpu.cycles += 5 + 10 + 7 + 7;
			native_sync();
			table = 0x8E2D;
			op_sub8(0x19, 0, 0);
			if (cpu.f & FC) { cpu.cycles += 5; goto zero; }
			cpu.cycles += 7;
			native_sync();
			op_sub8(0x79, 0, 0);
		}
		SET_HL(table);
		cpu.cycles += 7;
		native_sync();
		if (!(cpu.f & FC)) { cpu.cycles += 5; goto zero; }
		cpu.cycles += 19 + 4 + 4 + 7 + 11 + 7 + 6 + 7 + 19 + 19 + 10;
		native_sync();
		IXW(0x1B, cpu.a);
		op_add8(cpu.a, 0);
		cpu.e = cpu.a;
		cpu.d = 0;
		SET_HL(op_add16(HL, DE));
		cpu.e = mem_read(HL);
		SET_HL(HL + 1);
		cpu.d = mem_read(HL);
		IXW(0x16, cpu.e);
		IXW(0x17, cpu.d);
		return;
	zero:
		cpu.cycles += 4;
		native_sync();
		cpu.a = 0;
		op_sub8(0, 0, 1);
	clear:
		cpu.cycles += 19 + 19 + 19 + 10;
		native_sync();
		IXW(0x1B, cpu.a);
		IXW(0x16, cpu.a);
		IXW(0x17, cpu.a);
		return;
	}
	cpu.cycles += 19 + 19 + 4 + 4;     // $82FB
	native_sync();
	cpu.l = IXR(0x16);
	cpu.h = IXR(0x17);
	cpu.b = cpu.h;
	cpu.c = cpu.l;
	do {
		cpu.cycles += 8 + 8 + 4 + 10;
		native_sync();
		cpu.b = op_rot(7, cpu.b);      // srl b
		cpu.c = op_rot(3, cpu.c);      // rr c
		cpu.a = op_dec8(cpu.a);
	} while (cpu.a);
	int down = alt_e_bit(6);
	cpu.cycles += 7;
	native_sync();
	if (!down) {
		cpu.cycles += 11 + 4 + 7 + 7;
		native_sync();
		SET_HL(op_add16(HL, BC));
		cpu.a = cpu.h;
		op_sub8(4, 0, 0);
		if (cpu.f & FC) { cpu.cycles += 5; goto store; }
		cpu.cycles += 10;
		native_sync();
	} else {
		cpu.cycles += 5 + 4 + 15 + 7;
		native_sync();
		op_and(cpu.a);
		op_sbc16(BC);
		if (!(cpu.f & FC)) { cpu.cycles += 5; goto store; }
	}
	cpu.cycles += 10;
	native_sync();
	SET_HL(0);
store:
	cpu.cycles += 19 + 19 + 10;
	native_sync();
	IXW(0x16, cpu.l);
	IXW(0x17, cpu.h);
}
Native native_snd_slide = { "snd_slide", 0x8295, 0x1F, slide, 0, .long_calls = 1 };

static void out_port(uint8_t port, uint8_t v) {   // out (port),a
	cpu.cycles += 11;
	native_sync();
	io_out((uint16_t)(cpu.a << 8 | port), v);
}

// $824C: the PSG's registers from $DD03: three tones (tone, volume), then
// the noise's mode (if changed) and volume
static void psg_out(void) {
	cpu.cycles += 10 + 7;
	native_sync();
	SET_HL(0xDD03);
	cpu.b = 0x80;
	for (;;) {
		cpu.cycles += 7 + 7 + 4;
		native_sync();
		cpu.a = (uint8_t)((mem_read(HL) & 0x0F) | cpu.b);
		op_or(0);
		out_port(0x7F, cpu.a);
		cpu.cycles += 6 + 7 + 7;
		native_sync();
		SET_HL(HL + 1);
		cpu.a = mem_read(HL);
		op_and(0x3F);
		out_port(0x7F, cpu.a);
		cpu.cycles += 6 + 4 + 7 + 4 + 7 + 4 + 7 + 4;
		native_sync();
		SET_HL(HL + 1);
		cpu.b = (uint8_t)(cpu.b + 0x10);
		cpu.a = (uint8_t)((~mem_read(HL) & 0x0F) | cpu.b);
		op_or(0);
		out_port(0x7F, cpu.a);
		cpu.cycles += 6 + 4 + 7 + 4 + 7 + 10;
		native_sync();
		SET_HL(HL + 1);
		cpu.a = (uint8_t)(cpu.b + 0x10);
		cpu.b = cpu.a;
		op_sub8(0xE0, 0, 0);
		if (cpu.a == 0xE0) break;
	}
	cpu.cycles += 13 + 7 + 8 + 7 + 13 + 10;
	native_sync();
	cpu.a = mem_read(0xDD5C);
	op_sub8(mem_read(HL), 0, 0);
	int same = (cpu.f & FZ) != 0;
	cpu.a &= ~0x08;
	mem_write(HL, cpu.a);
	mem_write(0xDD5C, cpu.a);
	if (!same) {
		cpu.cycles += 7 + 7;
		native_sync();
		cpu.a = (uint8_t)((cpu.a & 0x07) | 0xE0);
		op_or(0);
		out_port(0x7F, cpu.a);
	}
	cpu.cycles += 6 + 7 + 4 + 7 + 7;
	native_sync();
	SET_HL(HL + 1);
	cpu.a = (uint8_t)((~mem_read(HL) & 0x0F) | 0xF0);
	op_or(0);
	out_port(0x7F, cpu.a);
	cpu.cycles += 10;
	native_sync();
}
Native native_snd_psg_out = { "snd_psg_out", 0x824C, 0x1F, psg_out, 0, .long_calls = 1 };

static void fm_mask_off(void) {        // call $8970: the channel's bit out of $DD3F and $DD40
	cpu.cycles += 17 + 7 + 7 + 4 + 13 + 4 + 13 + 13 + 10;
	native_sync();
	cpu.a = mem_read(HL) ^ 0xFF;
	cpu.b = cpu.a;
	cpu.a = mem_read(0xDD3F);
	op_and(cpu.b);
	mem_write(0xDD3F, cpu.a);
	mem_write(0xDD40, cpu.a);
}

// $88C5: the channel's tone DE and volume ($DD5A) into the chips' shadow
// registers: the FM's ($DD12 + C, if +$09) or the PSG's ($DD03 + 3C); its
// rhythm or noise part too (bit 1 of +$00)
static void write_channel(void) {
	cpu.cycles += 19 + 4 + 10;
	native_sync();
	cpu.a = IXR(9);
	op_and(cpu.a);
	if (cpu.a) {                       // FM
		cpu.cycles += 8 + 10;
		native_sync();
		if (cpu.b & 1) {
			cpu.cycles += 10 + 7 + 11 + 7 + 4 + 4 + 7;
			native_sync();
			cpu.b = 0;
			SET_HL(op_add16(0xDD12, BC));
			cpu.c = 9;
			cpu.a = cpu.d;
			op_or(cpu.e);
			if (!cpu.a) {
				cpu.cycles += 11 + 10 + 11 + 10 + 10;
				native_sync();
				SET_HL(HL + 9);
				mem_write(HL, 0);
				SET_HL(HL + 9);
				mem_write(HL, 0x1F);
				goto rhythm;
			}
			cpu.cycles += 5 + 7 + 11 + 19 + 7 + 4 + 7;
			native_sync();
			mem_write(HL, cpu.e);
			SET_HL(HL + 9);
			cpu.a = (uint8_t)((IXR(0x0A) & 0x20) | cpu.d | 0x80);
			op_or(0);
			if (alt_e_bit(2)) { cpu.cycles += 7 + 8; cpu.a |= 0x10; }
			else cpu.cycles += 12;
			cpu.cycles += 7 + 13 + 8 + 7;
			native_sync();
			mem_write(HL, cpu.a);
			cpu.a = mem_read(0xDD02);
			if (cpu.a & 0x80) {
				cpu.cycles += 8 + 7;
				native_sync();
				if (cpu.a & 0x40) { cpu.cycles += 15; mem_write(HL, mem_read(HL) | 0x40); }
				else cpu.cycles += 5;
			} else cpu.cycles += 5;
			cpu.cycles += 11 + 19 + 4 + 7 + 7;
			native_sync();
			SET_HL(HL + 9);
			cpu.a = IXR(9);
			cpu.b = cpu.a;
			op_sub8(0x10, 0, 0);
			int below = cpu.a < 0x10;
			if (below) cpu.cycles += 5;
			else { cpu.cycles += 4; cpu.a = 0; op_sub8(0, 0, 1); }
			cpu.cycles += 16 + 4 + 7;
			native_sync();
			cpu.a = (uint8_t)(cpu.a << 4 | cpu.a >> 4);
			cpu.c = cpu.a;
			if (!below) {              // an instrument of its own: loaded when it changes
				cpu.cycles += 13 + 4 + 7;
				native_sync();
				cpu.a = mem_read(0xDD56);
				op_sub8(cpu.b, 0, 0);
				if (cpu.a == cpu.b) cpu.cycles += 5;
				else {
					cpu.cycles += 4 + 8 + 13;
					native_sync();
					cpu.a = (uint8_t)(cpu.b | 0x80);
					mem_write(0xDD56, cpu.a);
				}
			} else cpu.cycles += 5;
			cpu.cycles += 13 + 7 + 4 + 7;
			native_sync();
			cpu.a = (uint8_t)((mem_read(0xDD5A) ^ 0x0F) | cpu.c);
			op_or(0);
			mem_write(HL, cpu.a);
		}
	rhythm:
		cpu.cycles += 4 + 8 + 4;       // $892A
		native_sync();
		if (!(cpu.d_ & 2)) { cpu.cycles += 11; return; }
		cpu.cycles += 5 + 19 + 7 + 7 + 7 + 7;
		native_sync();
		cpu.a = IXR(0x0A) & 0x07;
		op_sub8(5, 0, 0);
		cpu.b = 0;
		if (cpu.a >= 5) {
			cpu.cycles += 4 + 4 + 7;
			native_sync();
			cpu.a = cpu.e;
			op_or(cpu.d);
			if (!cpu.a) { cpu.cycles += 13 + 10; mem_write(0xDD0D, 0); }
			else {
				cpu.cycles += 5 + 13 + 8 + 19 + 7;
				native_sync();
				int fm = mem_read(0xDD02) & 0x80;
				cpu.a = IXR(0x1D);
				if (fm) cpu.cycles += 5; else { cpu.cycles += 8; cpu.a &= ~0x08; }
				cpu.cycles += 13 + 13 + 13;
				native_sync();
				mem_write(0xDD5C, cpu.a);
				cpu.a = mem_read(0xDD5A);
				mem_write(0xDD0D, cpu.a);
			}
			cpu.cycles += 7 + 13 + 10;
			native_sync();
			cpu.a = 0xFF;
			mem_write(0xDD5B, 0xFF);
			return;
		}
		cpu.cycles += 5 + 4 + 10 + 11 + 4 + 4 + 7;   // $895F
		native_sync();
		cpu.c = cpu.a;
		SET_HL(op_add16(0x89CD, BC));
		cpu.a = cpu.e;
		op_or(cpu.d);
		if (!cpu.a) {
			fm_mask_off();
			cpu.cycles += 7 + 10;
			native_sync();
			cpu.e = 0x0F;
		} else {
			cpu.cycles += 5;
			native_sync();
			if (!alt_e_bit(2)) {
				cpu.cycles += 7;
				native_sync();
				fm_mask_off();
				cpu.cycles += 10;
				native_sync();
			} else {
				cpu.cycles += 12 + 13 + 7 + 13 + 13 + 8 + 7;
				native_sync();
				cpu.a = mem_read(0xDD3F) | mem_read(HL);
				mem_write(0xDD3F, cpu.a);
				cpu.a = mem_read(0xDD02);
				if (!(cpu.a & 0x80)) {
					cpu.cycles += 13 + 7 + 13;
					native_sync();
					cpu.a = mem_read(0xDD40) | mem_read(HL);
					mem_write(0xDD40, cpu.a);
				} else cpu.cycles += 5;
			}
			cpu.cycles += 13 + 7 + 4;  // $89A0
			native_sync();
			cpu.a = mem_read(0xDD5A) ^ 0x0F;
			cpu.e = cpu.a;
		}
		cpu.cycles += 10 + 4 + 7 + 7;  // $89A6
		native_sync();
		SET_HL(0xDD2A);
		cpu.a = cpu.c;
		if (cpu.a >= 3) { cpu.cycles += 6; SET_HL(HL + 1); } else cpu.cycles += 5;
		cpu.cycles += 7 + 7;
		native_sync();
		if (cpu.a >= 1) { cpu.cycles += 6 + 4; SET_HL(HL + 1); cpu.a++; } else cpu.cycles += 5;
		cpu.cycles += 7 + 7;
		native_sync();
		op_and(1);
		if (!cpu.a) {
			cpu.cycles += 7 + 7 + 4 + 7 + 10;
			native_sync();
			cpu.a = (uint8_t)((mem_read(HL) & 0xF0) | cpu.e);
			mem_write(HL, cpu.a);
		} else {
			cpu.cycles += 5 + 7 + 7 + 32 + 4 + 7 + 10;
			native_sync();
			cpu.e = (uint8_t)(cpu.e << 4 | cpu.e >> 4);
			cpu.a = (uint8_t)((mem_read(HL) & 0x0F) | cpu.e);
			mem_write(HL, cpu.a);
		}
		return;
	}
	cpu.cycles += 4 + 4 + 7;           // $89D2: the PSG
	native_sync();
	cpu.a = cpu.e;
	op_or(cpu.d);
	if (!cpu.a) { cpu.cycles += 13; mem_write(0xDD5A, 0); }
	else cpu.cycles += 5;
	cpu.cycles += 8 + 10;
	native_sync();
	if (cpu.b & 2) {                   // the noise
		cpu.cycles += 13 + 8 + 19 + 7;
		native_sync();
		int fm = mem_read(0xDD02) & 0x80;
		cpu.a = IXR(0x1D);
		if (fm) cpu.cycles += 5; else { cpu.cycles += 8; cpu.a &= ~0x08; }
		cpu.cycles += 13 + 13 + 13 + 4 + 13 + 7 + 7;
		native_sync();
		mem_write(0xDD5C, cpu.a);
		cpu.a = mem_read(0xDD5A);
		mem_write(0xDD0D, cpu.a);
		cpu.a = cpu.c;
		mem_write(0xDD5B, cpu.a);
		op_sub8(2, 0, 0);
		if (cpu.a == 2) { cpu.cycles += 5; goto tone_on; }
		cpu.cycles += 8;
		native_sync();
		if (!(cpu.b & 1)) { cpu.cycles += 11; return; }
		cpu.cycles += 5 + 10;
		native_sync();
		goto store;
	}
	cpu.cycles += 13 + 4 + 7;          // $8A01
	native_sync();
	cpu.a = mem_read(0xDD5B);
	op_sub8(cpu.c, 0, 0);
	if (cpu.a == cpu.c) {
		cpu.cycles += 7 + 7;
		native_sync();
		op_sub8(2, 0, 0);
		if (cpu.a == 2) {
			cpu.cycles += 13 + 7 + 7 + 7;
			native_sync();
			cpu.a = mem_read(0xDD5C) & 3;
			op_sub8(3, 0, 0);
			if (cpu.a == 3) {          // the tone the noise follows, silent
				cpu.cycles += 4 + 13 + 4 + 13;
				native_sync();
				mem_write(0xDD0D, 0);
				cpu.a = 0xFF;
				mem_write(0xDD5B, 0xFF);
			} else cpu.cycles += 5;
		} else cpu.cycles += 5;
	} else cpu.cycles += 5;
tone_on:
	cpu.cycles += 8 + 7;               // $8A1C
	native_sync();
	if (!(cpu.b & 1)) {
		cpu.cycles += 4 + 13;
		native_sync();
		cpu.a = 0;
		mem_write(0xDD5A, 0);
	} else cpu.cycles += 5;
store:
	cpu.cycles += 4 + 7;               // $8A24
	native_sync();
	cpu.a = cpu.c;
	op_sub8(3, 0, 0);
	if (cpu.a == 3) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 4 + 4 + 4 + 7 + 10 + 11 + 7 + 6 + 64 + 7 + 6 + 13 + 7 + 10;
	native_sync();
	cpu.a = (uint8_t)(cpu.c * 3);
	cpu.c = cpu.a;
	cpu.b = 0;
	SET_HL(op_add16(0xDD03, BC));
	mem_write(HL, cpu.e);
	SET_HL(HL + 1);
	SET_DE((uint16_t)(DE << 4));
	mem_write(HL, cpu.d);
	SET_HL(HL + 1);
	cpu.a = mem_read(0xDD5A);
	mem_write(HL, cpu.a);
}
Native native_snd_write_channel = { "snd_write_channel", 0x88C5, 0x1F, write_channel, 0, .long_calls = 1 };

static void push(uint16_t v) { cpu.cycles += 11; push16(v); }
static uint16_t pop(void) { cpu.cycles += 10; return pop16(); }

// $87F3: the channel's sound this frame out to the shadow registers: off
// when muted (bit 1 of +$0C); faded by $DD11 (bit 3 of +$0B); with a second
// voice from the table at +$22 (bit 0 of +$0B): its volume, chip, and the
// tone shifted
static void channel_out(void) {
	cpu.cycles += 4 + 4 + 4 + 4 + 8;
	native_sync();
	cpu.a = cpu.d_;
	cpu.b = cpu.a;
	if (cpu.a & 0x20) { cpu.cycles += 17; silence(); }
	else cpu.cycles += 10;
	cpu.cycles += 4 + 4 + 8 + 4;
	native_sync();
	cpu.a = cpu.e_;
	if (cpu.c_ & 2) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 8 + 7;
	native_sync();
	if (cpu.a & 0x08) {                // fading
		cpu.cycles += 20 + 13 + 7 + 7;
		native_sync();
		SET_DE(rd16(0xDD11));
		cpu.a = mem_read(0xDD5A);
		op_sub8(0x10, 0, 0);
		if (cpu.a < 0x10) {
			cpu.cycles += 5 + 4 + 7 + 7;
			native_sync();
			op_add8(cpu.e, 0);
			op_sub8(0x0F, 0, 1);
			if (!(cpu.f & FC)) { cpu.cycles += 5; goto keep; }
		} else {
			cpu.cycles += 13 + 7 + 10;
			native_sync();
			cpu.a = mem_read(0xDD11);
			op_sub8(0x0F, 0, 0);
			if (cpu.f & FC) goto tone;
			cpu.cycles += 10;
			native_sync();
		}
		cpu.cycles += 4;               // $8821
		native_sync();
		cpu.a = 0;
		op_sub8(0, 0, 1);
	keep:
		cpu.cycles += 13;
		native_sync();
		mem_write(0xDD5A, cpu.a);
	} else cpu.cycles += 5;
tone:
	cpu.cycles += 19 + 19 + 19 + 4 + 8 + 4 + 10;   // $8825
	native_sync();
	cpu.e = IXR(0x16);
	cpu.d = IXR(0x17);
	cpu.c = IXR(0x06);
	if (!(cpu.e_ & 1)) { write_channel(); return; }
	cpu.cycles += 19 + 4;              // $8835: two voices
	native_sync();
	cpu.a = IXR(0x13);
	op_and(cpu.a);
	if (!cpu.a) { cpu.cycles += 11; return; }
	cpu.cycles += 5;
	native_sync();
	push(BC);
	cpu.cycles += 19 + 13 + 4;
	native_sync();
	cpu.h = IXR(9);
	cpu.a = mem_read(0xDD5A);
	cpu.l = cpu.a;
	push(HL);
	cpu.cycles += 19 + 19;
	native_sync();
	cpu.l = IXR(0x22);
	cpu.h = IXR(0x23);
	push(HL);
	cpu.cycles += 7 + 4 + 10;
	native_sync();
	cpu.a = mem_read(HL);
	op_and(cpu.a);
	if (cpu.a & 0x80) {                // the first voice's own volume
		int fade = alt_e_bit(3);
		cpu.cycles += 19 + 10;
		native_sync();
		cpu.a = IXR(1);
		if (fade) {
			cpu.cycles += 16 + 4 + 7 + 10;
			native_sync();
			SET_HL(rd16(0xDD11));
			op_add8(cpu.l, 0);
			op_sub8(0x0F, 0, 1);
			if (cpu.f & FC) { cpu.cycles += 4; cpu.a = 0; op_sub8(0, 0, 1); }
		}
		cpu.cycles += 13;
		native_sync();
		mem_write(0xDD5A, cpu.a);
	}
	push(DE);                          // $8866
	cpu.cycles += 17;
	native_sync();
	write_channel();
	SET_DE(pop());
	SET_HL(pop());
	SET_BC(pop());
	cpu.cycles += 7 + 4 + 7 + 7 + 7;
	native_sync();
	cpu.a = (uint8_t)((mem_read(HL) + cpu.c) & 0x7F);
	op_sub8(0x0F, 0, 1);
	if (cpu.f & FC) { cpu.cycles += 4; cpu.a = 0; op_sub8(0, 0, 1); }
	else cpu.cycles += 5;
	cpu.cycles += 13 + 6 + 7 + 19 + 6 + 7 + 6 + 4 + 7 + 10;
	native_sync();
	mem_write(0xDD5A, cpu.a);
	SET_HL(HL + 1);
	cpu.a = mem_read(HL);
	IXW(9, cpu.a);
	SET_HL(HL + 1);
	cpu.c = mem_read(HL);
	SET_HL(HL + 1);
	op_and(cpu.a);
	int fm = cpu.a != 0;
	cpu.a = mem_read(HL);
	if (!fm) {                         // the PSG: the tone shifted by octaves
		cpu.cycles += 7 + 7;
		native_sync();
		op_and(0xF0);
		if (cpu.a) {
			cpu.cycles += 10;
			native_sync();
			if (cpu.a & 0x80) {
				do {
					cpu.cycles += 8 + 8 + 7 + 7;
					native_sync();
					SET_DE((uint16_t)(DE << 1));
					op_add8(0x10, 0);
					if (cpu.a) cpu.cycles += 5;
				} while (cpu.a);
			} else {
				do {
					cpu.cycles += 8 + 8 + 7 + 7;
					native_sync();
					SET_DE((uint16_t)(DE >> 1));
					op_sub8(0x10, 0, 1);
					if (cpu.a) cpu.cycles += 5;
				} while (cpu.a);
			}
			cpu.cycles += 10;
			native_sync();
		} else cpu.cycles += 5;
	} else {                           // the FM: its block
		cpu.cycles += 7 + 16 + 4 + 7 + 4;
		native_sync();
		cpu.a = (uint8_t)(((cpu.a & 0xF0) >> 4) + cpu.d);
		op_and(0x0F);
		cpu.d = cpu.a;
	}
	cpu.cycles += 4 + 7 + 8 + 7;       // $88AC
	native_sync();
	cpu.a = cpu.b;
	cpu.b = 0;
	if (cpu.c & 0x80) { cpu.cycles += 4; cpu.b = 0xFF; }
	else cpu.cycles += 5;
	cpu.cycles += 4 + 11 + 4;
	native_sync();
	SET_DE(op_add16(DE, BC));
	SET_BC(pop());
	push((uint16_t)(cpu.a << 8 | cpu.f));
	cpu.cycles += 7 + 7 + 4 + 17;
	native_sync();
	cpu.c = mem_read(HL) & 0x0F;
	write_channel();
	uint16_t af = pop();
	cpu.a = (uint8_t)(af >> 8); cpu.f = (uint8_t)af;
	cpu.cycles += 19 + 10;
	native_sync();
	IXW(9, cpu.a);
}
Native native_snd_channel_out = { "snd_channel_out", 0x87F3, 0x1F, channel_out, 0, .long_calls = 1 };

static void start_sound(void);

// $8A94: B of the FM's registers from C: 0, and the shadow copies at DE
// and HL cleared ($00 and $FF)
static void fm_clear(void) {
	do {
		cpu.cycles += 4;
		cpu.a = cpu.c;
		out_port(0xF0, cpu.a);
		cpu.cycles += 4;
		cpu.a = 0;
		op_sub8(0, 0, 1);
		out_port(0xF1, cpu.a);
		cpu.cycles += 7 + 10 + 6 + 6 + 4 + 17 + 10;   // ... call $8294 (a delay)
		mem_write(DE, 0);
		mem_write(HL, 0xFF);
		SET_DE(DE + 1); SET_HL(HL + 1);
		cpu.c++;
		if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
	} while (1);
	cpu.cycles += 10;
}

// $8A5A: the FM's rhythm instruments (6 pairs at $8F31), and its rhythm
// volumes ($36-$38) at $FF
static void fm_rhythm_init(void) {
	cpu.cycles += 10 + 10;
	SET_HL(0x8F31); SET_BC(0x06F0);
	do {
		cpu.cycles += 7 + 6;
		cpu.a = mem_read(HL);
		SET_HL(HL + 1);
		cpu.cycles += 12;
		io_out(BC, cpu.a);
		cpu.cycles += 4 + 7;
		cpu.c++;
		cpu.a = mem_read(HL);
		cpu.cycles += 12;
		io_out(BC, cpu.a);
		cpu.cycles += 6 + 4 + 17 + 11 + 10 + 10;   // ... call $8292 (a delay)
		SET_HL(HL + 1);
		cpu.c--;
		if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
	} while (1);
	cpu.cycles += 10 + 10;
	SET_BC(0x0336); SET_HL(0xDD53);
	do {
		cpu.cycles += 4;
		cpu.a = cpu.c;
		out_port(0xF0, cpu.a);
		cpu.cycles += 7;
		cpu.a = 0xFF;
		out_port(0xF1, cpu.a);
		cpu.cycles += 7 + 4 + 6 + 17 + 11 + 10 + 10;
		mem_write(HL, 0xFF);
		cpu.c++;
		SET_HL(HL + 1);
		if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
	} while (1);
	cpu.cycles += 7 + 10;
	cpu.b = 6;
}

enum { GO_ON, CHANNEL_DONE };   // how a command ends: back to $8332, or the channel left (jp $8107)

static void jump(void) {               // $83A8: on at the address A, (DE)
	cpu.cycles += 4 + 6 + 7 + 4 + 4 + 6 + 10;
	cpu.l = cpu.a;
	SET_DE(DE + 1);
	cpu.h = mem_read(DE);
	cpu.a = cpu.h;
	uint16_t at = DE;
	SET_DE((uint16_t)(HL - 1));
	SET_HL(at);
}

static void skip(void) { cpu.cycles += 6 + 10; SET_DE(DE + 1); }   // inc de / ret

// the command A ($80-$A9) at DE - 1, its first parameter in A
static int command(uint8_t code) {
	switch (code) {
	case 0x80: jump(); return GO_ON;
	case 0x81:                         // a loop: the counter +$10 (or +$11 for $11)
		cpu.cycles += 6 + 7 + 7;
		SET_DE(DE + 1);
		op_sub8(0x11, 0, 0);
		{
			int which = cpu.a == 0x11 ? 0x11 : 0x10;
			if (which == 0x11) cpu.cycles += 5;
			cpu.cycles += 23 + 7 + 10;
			uint8_t n = op_dec8(IXR(which));
			IXW(which, n);
			cpu.a = mem_read(DE);
			if (n) { jump(); return GO_ON; }
			skip();
		}
		return GO_ON;
	case 0x86:                         // the end: its done flag into $DD00
		cpu.cycles += 19 + 13;
		cpu.a = IXR(0x14);
		mem_write(0xDD00, cpu.a);
		/* fall through */
	case 0x82:
		cpu.cycles += 19 + 10 + 10 + 4 + 10;
		IXW(0, 0);
		pop16(); SET_HL(pop16());       // pop hl, twice
		op_exx();
		return CHANNEL_DONE;
	case 0x83: cpu.cycles += 19 + 10; IXW(3, cpu.a); return GO_ON;
	case 0x84: case 0x89:              // transpose (by, or $80: none)
		cpu.cycles += 7 + 7;
		op_sub8(0x80, 0, 0);
		if (cpu.a == 0x80) { cpu.cycles += 5 + 19 + 10; IXW(4, 0); return GO_ON; }
		cpu.cycles += 19 + 19 + 10;
		op_add8(IXR(4), 0);
		IXW(4, cpu.a);
		return GO_ON;
	case 0x85: case 0x96: cpu.cycles += 19 + 10; IXW(5, cpu.a); return GO_ON;
	case 0x87: cpu.cycles += 19 + 10; IXW(1, cpu.a); return GO_ON;
	case 0x88: cpu.cycles += 19 + 10; IXW(2, cpu.a); return GO_ON;
	case 0x8A: {                       // volume: up or down, within 0-$0F
		cpu.cycles += 19 + 4 + 10;
		cpu.c = IXR(1);
		op_and(cpu.a);
		if (cpu.a & 0x80) {
			cpu.cycles += 4 + 7;
			op_add8(cpu.c, 0);
			if (cpu.f & FC) { cpu.cycles += 5; }
			else { cpu.cycles += 19 + 10; IXW(1, 0); return GO_ON; }
		} else {
			cpu.cycles += 4 + 7 + 7;
			op_add8(cpu.c, 0);
			op_sub8(0x10, 0, 0);
			if (cpu.f & FC) cpu.cycles += 5;
			else { cpu.cycles += 7; cpu.a = 0x0F; }
		}
		cpu.cycles += 19 + 10;
		IXW(1, cpu.a);
		return GO_ON;
	}
	case 0x8B: {                       // a sound started from the song
		cpu.cycles += 19 + 11 + 17;
		cpu.b = IXR(0x14);
		push16(BC);
		start_sound();
		cpu.cycles += 10 + 4 + 19;
		SET_BC(pop16());
		cpu.a = cpu.b;
		op_sub8(IXR(0x14), 0, 0);
		if (cpu.a == IXR(0x14)) { cpu.cycles += 11; return GO_ON; }
		cpu.cycles += 5 + 10 + 10 + 10 + 4 + 10;   // jp $83D0
		pop16(); SET_HL(pop16());       // pop hl, twice
		op_exx();
		return CHANNEL_DONE;
	}
	case 0x8C: cpu.cycles += 6 + 10; SET_DE(DE - 1); return GO_ON;
	case 0x8D:                         // +A of the channel = the next byte
		cpu.cycles += 4 + 15 + 10 + 11 + 6 + 7 + 7 + 10;
		cpu.c = cpu.a;
		SET_HL(op_add16(cpu.ix, BC));
		SET_DE(DE + 1);
		cpu.a = mem_read(DE);
		mem_write(HL, cpu.a);
		return GO_ON;
	case 0x8E: case 0x8F: cpu.cycles += 19 + 10; IXW(0x1D, cpu.a); return GO_ON;
	case 0x90: cpu.cycles += 19 + 10; IXW(0, cpu.a); return GO_ON;
	case 0x91:
		cpu.cycles += 10 + 7 + 7 + 10;
		SET_HL(0xDD00);
		op_or(mem_read(HL));
		mem_write(HL, cpu.a);
		return GO_ON;
	case 0x92: {                       // the FM's rhythm mode (6: off)
		cpu.cycles += 10 + 12;
		SET_HL(0xDD58);
		op_bit(7, mem_read(HL));
		if (!(cpu.f & FS)) { cpu.cycles += 11; return GO_ON; }
		cpu.cycles += 5 + 7 + 7;
		uint8_t a = cpu.a;
		op_sub8(6, 0, 0);
		if (a != 6) {
			cpu.cycles += 12;
			op_bit(6, mem_read(HL));
			if (!(cpu.f & FZ)) { cpu.cycles += 11; return GO_ON; }
			cpu.cycles += 5 + 15 + 7 + 11 + 10 + 10 + 7 + 17;
			mem_write(HL, mem_read(HL) | 0x40);
			cpu.c = 0x16;
			push16(DE);
			SET_DE(0xDD47); SET_HL(0xDD50);
			cpu.b = 3;
			fm_clear();
			cpu.cycles += 10 + 4 + 13 + 10;
			SET_DE(pop16());
			cpu.a = 0;
			op_sub8(0, 0, 1);
			mem_write(0xDD3F, 0);
			return GO_ON;
		}
		cpu.cycles += 5 + 12;
		op_bit(6, mem_read(HL));
		if (cpu.f & FZ) { cpu.cycles += 11; return GO_ON; }
		cpu.cycles += 5 + 15 + 11 + 17;
		mem_write(HL, mem_read(HL) & ~0x40);
		push16(DE);
		fm_rhythm_init();
		cpu.cycles += 10 + 7 + 13 + 10;
		SET_DE(pop16());
		cpu.a = 0x20;
		mem_write(0xDD3F, 0x20);
		return GO_ON;
	}
	case 0x93: {                       // vibrato: depth, speed (0: off)
		cpu.cycles += 19 + 8 + 8 + 4 + 10;
		cpu.c = IXR(0x0B) & ~0x12;
		op_and(cpu.a);
		if (cpu.a & 0x80) { cpu.cycles += 8; cpu.c |= 0x10; }
		cpu.cycles += 7 + 19 + 6 + 7 + 19 + 19 + 8 + 4 + 7;
		cpu.a &= 0x0F;
		IXW(0x1A, cpu.a);
		SET_DE(DE + 1);
		cpu.a = mem_read(DE);
		IXW(0x18, cpu.a);
		IXW(0x19, cpu.b);
		cpu.c |= 0x20;
		op_and(cpu.a);
		if (!cpu.a) { cpu.c &= ~0x20; cpu.cycles += 8; }
		else cpu.cycles += 5;
		cpu.cycles += 19 + 10;
		IXW(0x0B, cpu.c);
		return GO_ON;
	}
	case 0x94: {                       // $8467: a slide (bit 7 of A: down; 0: off)
		cpu.cycles += 19 + 8 + 4 + 10;
		cpu.b = IXR(0x0B) & ~0x40;
		op_and(cpu.a);
		if (cpu.a & 0x80) { cpu.cycles += 8; cpu.b |= 0x40; }
		cpu.cycles += 8 + 7 + 7;
		cpu.b |= 0x80;
		op_and(0x7F);
		if (cpu.a) { cpu.cycles += 19; IXW(0x20, cpu.a); }
		else { cpu.cycles += 5 + 8 + 12; cpu.b &= ~0x80; }
		cpu.cycles += 19 + 10;
		IXW(0x0B, cpu.b);
		return GO_ON;
	}
	case 0x95:                         // jumps unless bit 1 of +$0B
		cpu.cycles += 20 + 10;
		if (!(IXR(0x0B) & 2)) { jump(); return GO_ON; }
		skip();
		return GO_ON;
	case 0x9E:                         // jumps if bit 2 of +$0C
		cpu.cycles += 20 + 10;
		if (IXR(0x0C) & 4) { jump(); return GO_ON; }
		skip();
		return GO_ON;
	case 0xA1: case 0xA2: {            // jumps with (A1) or without (A2) the FM
		cpu.cycles += 13 + 4 + 7 + 10;
		int fm = (mem_read(0xDD58) & 0x80) != 0;
		op_and(mem_read(0xDD58));
		cpu.a = mem_read(DE);
		if (fm == (code == 0xA1)) { jump(); return GO_ON; }
		skip();
		return GO_ON;
	}
	case 0x97: cpu.cycles += 19 + 19 + 10; op_add8(IXR(0x21), 0); IXW(0x21, cpu.a); return GO_ON;
	case 0x98: cpu.cycles += 19 + 10; IXW(6, cpu.a); return GO_ON;
	case 0x99:
		cpu.cycles += 19 + 7 + 19 + 6 + 10;
		cpu.a = IXR(0) ^ 0x10;
		op_xor(0);
		IXW(0, cpu.a);
		SET_DE(DE - 1);
		return GO_ON;
	case 0x9A:                         // a call: the return kept in +$26
		cpu.cycles += 4 + 6 + 7 + 19 + 19 + 4 + 4 + 6 + 10;
		cpu.c = cpu.a;
		SET_DE(DE + 1);
		cpu.a = mem_read(DE);
		IXW(0x26, cpu.e);
		IXW(0x27, cpu.d);
		cpu.e = cpu.c;
		cpu.d = cpu.a;
		SET_DE(DE - 1);
		return GO_ON;
	case 0x9B: cpu.cycles += 19 + 19 + 10; cpu.e = IXR(0x26); cpu.d = IXR(0x27); return GO_ON;
	case 0x9C:                         // transpose by the table at (DE) - +$10
		cpu.cycles += 19 + 4 + 6 + 7 + 4 + 6 + 7 + 4 + 11 + 7 + 19 + 19 + 10;
		op_sub8(IXR(0x10), 0, 1);
		cpu.c = cpu.a;
		SET_DE(DE + 1);
		cpu.l = mem_read(DE);
		SET_DE(DE + 1);
		cpu.h = mem_read(DE);
		cpu.a = cpu.h;
		SET_HL(op_add16(HL, BC));
		cpu.a = mem_read(HL);
		op_add8(IXR(4), 0);
		IXW(4, cpu.a);
		return GO_ON;
	case 0x9D: cpu.cycles += 19 + 10; IXW(0x15, cpu.a); return GO_ON;
	case 0x9F: {                       // the fade: $FF stops it, else channels by mask, speed, depth
		cpu.cycles += 10 + 7 + 7;
		SET_HL(0xDD02);
		op_sub8(0xFF, 0, 0);
		if (cpu.a == 0xFF) {
			cpu.cycles += 15 + 7 + 13 + 10 + 10 + 7;
			mem_write(0xDD02, mem_read(0xDD02) & ~0x20);
			mem_write(0xDD11, 0x0F);
			SET_BC(0x0028);
			SET_HL(0xDD68);
			cpu.a = 7;
			do {
				cpu.cycles += 15 + 11 + 4 + 7;
				mem_write(HL, mem_read(HL) & ~0x08);
				SET_HL(HL + 0x28);
				cpu.a = op_dec8(cpu.a);
				if (cpu.a) cpu.cycles += 5;
			} while (cpu.a);
			cpu.cycles += 10;
			return GO_ON;
		}
		cpu.cycles += 5 + 4 + 6 + 7 + 4 + 7 + 13 + 15 + 15 + 8 + 7;
		cpu.c = cpu.a;
		SET_DE(DE + 1);
		cpu.a = mem_read(DE);
		cpu.b = cpu.a;
		op_and(0x0F);
		mem_write(0xDD10, cpu.a);
		uint8_t f = (uint8_t)((mem_read(0xDD02) & ~0x10) | 0x20);
		if (cpu.b & 0x80) { cpu.cycles += 15; f |= 0x10; }
		else cpu.cycles += 5;
		mem_write(0xDD02, f);
		cpu.cycles += 4 + 13 + 6 + 7 + 13 + 6 + 7 + 4 + 10;
		mem_write(0xDD0F, 0);
		SET_DE(DE + 1);
		mem_write(0xDD0E, mem_read(DE));
		SET_DE(DE + 1);
		cpu.a = mem_read(DE);
		op_and(cpu.a);
		if (!(cpu.a & 0x80)) { cpu.cycles += 13; mem_write(0xDD11, cpu.a); }
		cpu.cycles += 11 + 4 + 7 + 4 + 4 + 12 + 7 + 4 + 7 + 10 + 11 + 7 + 6 + 7 + 10 + 11 + 10;
		push16(DE);
		cpu.b = cpu.c & 0x0F;
		cpu.a = (uint8_t)(((cpu.c >> 3) | (cpu.c << 5)) & 0x1E);
		cpu.e = cpu.a;
		cpu.d = 0;
		SET_HL(op_add16(0x8C53, DE));
		cpu.e = mem_read(HL);
		SET_HL(HL + 1);
		cpu.d = mem_read(HL);
		SET_HL(op_add16(0x000B, DE));
		SET_DE(0x0028);
		for (;;) {                     // these channels fade
			cpu.cycles += 15 + 11 + 4 + 10;
			mem_write(HL, mem_read(HL) | 0x08);
			SET_HL(op_add16(HL, DE));
			cpu.b = op_dec8(cpu.b);
			if (cpu.b & 0x80) break;
		}
		cpu.cycles += 10 + 10;
		SET_DE(pop16());
		return GO_ON;
	}
	case 0xA0:                         // a second voice: on, from the table here (3 bytes)
		cpu.cycles += 4 + 7;
		op_and(cpu.a);
		if (!cpu.a) { cpu.cycles += 23 + 10; IXW(0x0B, IXR(0x0B) & ~1); return GO_ON; }
		cpu.cycles += 5 + 23 + 19 + 19 + 18 + 10;
		IXW(0x0B, IXR(0x0B) | 1);
		IXW(0x22, cpu.e);
		IXW(0x23, cpu.d);
		SET_DE(DE + 3);
		return GO_ON;
	case 0xA3:
		cpu.cycles += 10 + 12 + 7;
		SET_HL(0xDD58);
		if (mem_read(HL) & 0x80) { cpu.cycles += 19; IXW(9, cpu.a); }
		else cpu.cycles += 5;
		cpu.cycles += 10;
		return GO_ON;
	case 0xA4: cpu.cycles += 19 + 10; IXW(0x0A, cpu.a); return GO_ON;
	case 0xA5: case 0xA6: {            // one byte on, or one back, by the FM
		cpu.cycles += 13 + 4 + 10;
		int fm = (mem_read(0xDD58) & 0x80) != 0;
		cpu.a = mem_read(0xDD58);
		op_and(cpu.a);
		int on = code == 0xA5 ? fm : !fm;
		if (code == 0xA6 && fm) cpu.cycles += 12;
		cpu.cycles += 6 + 10;
		SET_DE(on ? DE + 1 : DE - 1);
		return GO_ON;
	}
	case 0xA7:                         // two bytes skipped unless on chip +$06 = A
		cpu.cycles += 19 + 7;
		op_sub8(IXR(6), 0, 0);
		if (cpu.a == IXR(6)) cpu.cycles += 5;
		else { cpu.cycles += 6 + 6; SET_DE(DE + 2); }
		cpu.cycles += 10;
		return GO_ON;
	case 0xA8: {                       // jumps if on chip +$06 = A
		cpu.cycles += 6 + 19 + 7 + 10;
		SET_DE(DE + 1);
		int eq = cpu.a == IXR(6);
		op_sub8(IXR(6), 0, 0);
		cpu.a = mem_read(DE);
		if (eq) { jump(); return GO_ON; }
		skip();
		return GO_ON;
	}
	case 0xA9:                         // an envelope of its own: on (3 bytes), or off
		cpu.cycles += 4 + 10;
		op_and(cpu.a);
		if (cpu.a) {
			cpu.cycles += 19 + 23 + 6 + 7 + 19 + 6 + 7 + 19 + 10;
			IXW(2, cpu.a);
			IXW(1, IXR(1) | 0x10);
			SET_DE(DE + 1);
			IXW(0x24, mem_read(DE));
			SET_DE(DE + 1);
			cpu.a = mem_read(DE);
			IXW(0x25, cpu.a);
		} else {
			cpu.cycles += 23 + 6 + 7 + 19 + 6 + 10;
			IXW(1, IXR(1) & ~0x10);
			SET_DE(DE + 1);
			cpu.a = mem_read(DE);
			IXW(2, cpu.a);
			SET_DE(DE + 1);
		}
		return GO_ON;
	}
	fprintf(stderr, "sound: command %02X at %04X is not in any song%c", code, DE, 10);
	exit(2);
}

// $8329: the channel's next events from its song (+$07/+$08): commands,
// then a note (with its length) or an instrument and a note; ends with the
// channel's flags in the alternate registers. CHANNEL_DONE: the song
// ended (the main loop goes on at $8107, the registers exchanged)
static int sequencer(void) {
	cpu.cycles += 19 + 19 + 10;
	cpu.e = IXR(7);
	cpu.d = IXR(8);
	for (;;) {
		cpu.cycles += 7 + 4 + 10;      // $8333
		cpu.a = mem_read(DE);
		op_and(cpu.a);
		if (!(cpu.a & 0x80)) break;    // a note
		cpu.cycles += 7 + 10;
		op_sub8(0xC0, 0, 0);
		if (cpu.a >= 0xC0) goto preset;
		uint8_t code = cpu.a;
		cpu.cycles += 6 + 10 + 11 + 10 + 7 + 4 + 4 + 11 + 7 + 6 + 7 + 4 + 7 + 4;
		SET_DE(DE + 1);
		push16(0x8332);
		cpu.b = 0;
		cpu.c = (uint8_t)(code << 1);
		SET_HL(rd16(0x8350 + cpu.c));
		cpu.a = mem_read(DE);
		int r = command(code);
		if (r != GO_ON) return r;
		cpu.cycles += 6;               // its ret to $8332: inc de
		pop16();
		SET_DE(DE + 1);
	}
	cpu.cycles += 6;                   // $863C
	SET_DE(DE + 1);
	goto note;
preset:                                // $85E6: an instrument, then the note
	cpu.cycles += 7 + 10;
	if (cpu.a >= 0xE0) {               // $85E0: the last note again
		cpu.cycles += 19 + 10;
		cpu.a = IXR(0x13);
		goto note;
	}
	cpu.cycles += 7 + 7 + 4 + 4 + 4 + 7 + 19 + 4 + 7;
	cpu.a = (uint8_t)((cpu.a - 0xC0) * 4);
	cpu.c = cpu.a;
	cpu.b = 0;
	cpu.a = IXR(9);
	op_and(cpu.a);
	uint16_t table;
	if (cpu.a) { cpu.cycles += 10 + 10; table = 0x8FB1; }
	else { cpu.cycles += 5 + 10; table = 0x8F9D; }
	cpu.cycles += 11 + 7 + 19 + 6 + 7 + 4 + 6 + 7 + 4 + 7 + 19 + 19 + 4 + 7 + 12 + 4 + 19 + 7 + 4 + 19 + 12 + 7;
	SET_HL(op_add16(table, BC));
	IXW(2, mem_read(HL));
	SET_HL(HL + 1);
	cpu.c = mem_read(HL);
	SET_HL(HL + 1);
	cpu.b = mem_read(HL);
	cpu.a = cpu.b & 0x1F;
	IXW(0x1D, cpu.a);
	IXW(0x0A, cpu.a);
	cpu.b = (uint8_t)(((cpu.b & 0x60) << 3 | (cpu.b & 0x60) >> 5) & 0xFF);
	cpu.a = (uint8_t)((IXR(0) & 0xFC) | cpu.b);
	IXW(0, cpu.a);
	if (!(mem_read(HL) & 0x80)) {
		cpu.cycles += 23 + 10;
		IXW(0x0B, IXR(0x0B) & ~0x80);
	} else {
		cpu.cycles += 5 + 4 + 17;
		cpu.a = cpu.c;
		command(0x94);                 // call $8467: a slide
		cpu.cycles += 7;
		cpu.c = 0;
	}
	cpu.cycles += 19 + 6 + 7 + 6;
	IXW(3, cpu.c);
	SET_HL(HL + 1);
	cpu.a = mem_read(HL);
	SET_DE(DE + 1);
note:                                  // $863D: note A
	cpu.cycles += 4 + 19 + 4 + 4 + 19 + 17;
	{
		uint8_t note = cpu.a;
		uint8_t last = IXR(0x13);
		int retrigger = last == 0;
		IXW(0x13, note);
		cpu.a = note;
		note_tone();
		cpu.cycles += 19 + 19 + 7 + 7 + 10;
		IXW(0x17, cpu.b);
		IXW(0x16, cpu.c);
		cpu.a = mem_read(DE);
		op_sub8(0xE0, 0, 0);
		if (cpu.a < 0xE0) {            // the same length as the last
			cpu.cycles += 19 + 10;
			cpu.a = IXR(0x12);
		} else {
			cpu.cycles += 7 + 10;
			cpu.a = (uint8_t)(cpu.a - 0xE0);
			if (!cpu.a) {              // $E0: the length is the next byte
				cpu.cycles += 6 + 7 + 10;
				SET_DE(DE + 1);
				cpu.a = mem_read(DE);
			} else {
				cpu.cycles += 10 + 4 + 7 + 11 + 7;
				SET_BC(cpu.a);
				SET_HL(op_add16(0x8FC4, BC));
				cpu.a = mem_read(HL);
			}
			cpu.cycles += 6;
			SET_DE(DE + 1);
		}
		cpu.cycles += 19 + 19 + 10 + 4 + 19 + 19 + 4 + 7;   // $866E
		IXW(0x0E, cpu.a);
		IXW(0x12, cpu.a);
		SET_HL(0xDD02);
		cpu.a = 0;
		op_sub8(0, 0, 1);
		IXW(0x0D, 0);
		IXW(0x1F, 0);
		if (!retrigger) {
			cpu.cycles += 20 + 7;
			if (IXR(0) & 0x10) { cpu.cycles += 5; goto tied; }
		} else cpu.cycles += 5;
		cpu.cycles += 19 + 23 + 15;    // $8687: struck
		IXW(0x1E, 0);
		IXW(0x0B, IXR(0x0B) | 0x04);
		mem_write(HL, mem_read(HL) | 0x40);
	tied:
		cpu.cycles += 23 + 15 + 19 + 19 + 4 + 19 + 19 + 19 + 4 + 10;
		IXW(0x0C, IXR(0x0C) & ~0x02);
		mem_write(HL, mem_read(HL) | 0x80);
		IXW(8, cpu.d);
		IXW(7, cpu.e);
		cpu.a = 0;                     // ex af,af': the last note's, tested; sub a's kept
		op_sub8(0, 0, 1);
		cpu.a_ = 0;
		cpu.f_ = cpu.f;
		cpu.a = last;
		op_and(last);
		op_exx();
		cpu.d = IXR(0);
		cpu.e = IXR(0x0B);
		cpu.c = IXR(0x0C);
		op_exx();
	}
	return GO_ON;
}

static void ldir(void) {
	uint16_t n = BC;
	cpu.cycles += 21 * (n - 1) + 16;
	for (; n; n--) { mem_write(DE, mem_read(HL)); SET_HL(HL + 1); SET_DE(DE + 1); }
	SET_BC(0);
	cpu.f &= ~(FH | FN | FP);
}

// $8B64: tune or sound A from the table at $8FD6: its header gives a count
// of channels (bit 7: FM and PSG versions) and for each the channel and its
// first 9 (PSG) or 11 (FM) bytes; a channel held by a sound (bit 6) is not
// taken by a tune
static void start_song(void) {
	cpu.cycles += 4 + 4 + 4 + 7 + 10 + 11 + 7 + 6 + 7 + 4 + 7 + 6 + 4 + 7;
	cpu.c = cpu.a;
	cpu.e = (uint8_t)(cpu.a * 2);
	cpu.d = 0;
	SET_HL(op_add16(0x8FD6, DE));
	cpu.e = mem_read(HL);
	SET_HL(HL + 1);
	cpu.d = mem_read(HL);
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); }
	cpu.a = mem_read(HL);
	SET_HL(HL + 1);
	op_and(cpu.a);
	if (!cpu.a) { fprintf(stderr, "sound: song %02X runs code%c", cpu.c, 10); exit(2); }
	cpu.cycles += 5 + 4 + 10;          // $8BA4
	cpu.b = cpu.a;
	if (cpu.a & 0x80) {                // two versions: the FM's here, the PSG's at the pointer
		cpu.cycles += 13 + 4 + 10;
		cpu.a = mem_read(0xDD58);
		op_and(cpu.a);
		if (cpu.a & 0x80) { cpu.cycles += 6 + 10; SET_HL(HL + 1); }
		else {
			cpu.cycles += 7 + 6 + 7 + 4 + 7;
			uint16_t at = rd16(HL);
			SET_HL(at);
			cpu.a = (uint8_t)at;
			cpu.b = mem_read(HL);
		}
		cpu.cycles += 6 + 4 + 7 + 10;  // $8BB8
		SET_HL(HL + 1);
		cpu.a = cpu.b & 0x7F;
		op_and(0x7F);
		if (!cpu.a) { fprintf(stderr, "sound: song %02X runs code%c", cpu.c, 10); exit(2); }
	}
	for (;;) {                         // $8BC0: a channel
		cpu.cycles += 7 + 6 + 11 + 4 + 4 + 7 + 10 + 11 + 7 + 6 + 7 + 10 + 7 + 4 + 7;
		uint8_t ch = mem_read(HL);
		SET_HL(HL + 1);
		cpu.e = (uint8_t)(ch * 2);
		cpu.d = 0;
		uint16_t at = HL;
		SET_HL(op_add16(0x8C53, DE));
		cpu.e = mem_read(HL);
		SET_HL(HL + 1);
		cpu.d = mem_read(HL);
		SET_HL(at);
		cpu.a = mem_read(HL);
		op_and(cpu.a);
		if (!cpu.a) {                  // $8C49: none for it: the channel off
			cpu.cycles += 5 + 7 + 6;
			mem_write(DE, 0);
			SET_HL(HL + 1);
		} else {
			cpu.cycles += 7 + 8 + 5;
			cpu.a = mem_read(DE);
			op_bit(6, cpu.a);
			if (cpu.a & 0x40) { cpu.cycles += 6; return; }   // held by a sound
			cpu.cycles += 11 + 14 + 19 + 4 + 7;
			cpu.ix = DE;
			IXW(0x0C, 0);
			op_and(cpu.a);
			int same = 0;
			if (cpu.a) {
				cpu.cycles += 4 + 19 + 7;
				cpu.a = cpu.c;
				op_sub8(IXR(0x14), 0, 0);
				same = cpu.a == IXR(0x14);
				if (same) { cpu.cycles += 23 + 10; IXW(0x0C, IXR(0x0C) | 0x04); }
				else cpu.cycles += 5;
			} else cpu.cycles += 5;
			if (!same) { cpu.cycles += 19; IXW(0x14, cpu.c); }
			cpu.cycles += 13 + 4 + 10;     // $8BF1
			cpu.a = mem_read(0xDD58);
			op_and(cpu.a);
			int fm = 0;
			if (cpu.a & 0x80) {
				cpu.cycles += 8 + 7;
				if (cpu.b & 0x80) fm = 1; else cpu.cycles += 5;
			}
			if (fm) { cpu.cycles += 7; cpu.c = 0x0B; }
			else {
				cpu.cycles += 19 + 19 + 7 + 10;
				IXW(9, 0);
				IXW(0x0A, 0);
				cpu.c = 9;
			}
			cpu.cycles += 4 + 7;           // $8C0B: its first bytes
			cpu.a = cpu.b;
			cpu.b = 0;
			ldir();
			cpu.cycles += 4 + 19 + 13 + 4 + 19 + 7;
			cpu.b = cpu.a;
			cpu.c = IXR(0x14);
			cpu.a = mem_read(0xC001);
			op_or(cpu.a);
			int keep = cpu.a != 0;
			cpu.a = IXR(5);
			if (!keep) {               // its tempo by the table at $8C6F
				cpu.cycles += 11 + 7 + 4 + 7 + 4 + 4 + 7 + 10 + 19;
				cpu.a = mem_read((uint16_t)(0x8C6F + cpu.a));
				IXW(5, cpu.a);
			} else cpu.cycles += 5;
			cpu.cycles += 8 + 19 + 4 + 19 * 5 + 19 + 4 + 19 + 12;
			op_neg();
			IXW(0x0F, cpu.a);
			IXW(0x0B, 0); IXW(0x0D, 0); IXW(0x15, 0); IXW(0x21, 0); IXW(0x13, 0);
			IXW(0x1D, 4);
			cpu.a = 1;
			op_inc8(0);
			IXW(0x0E, 1);
		}
		cpu.cycles += 4 + 4 + 7 + 10;      // $8C4B
		cpu.b--;
		cpu.a = cpu.b & 0x7F;
		op_and(0x7F);
		if (!cpu.a) break;
	}
	cpu.cycles += 10;
}
Native native_snd_start_song = { "snd_start_song", 0x8B64, 0x1F, start_song, 0 };

// $8AF3 (vector $8006): start tune or sound A, with interrupts off
static void start_sound(void) {
	cpu.cycles += 11 + 11 + 11 + 15 + 4 + 17;
	push16(BC); push16(DE); push16(HL); push16(cpu.ix);
	cpu.iff1 = cpu.iff2 = 0;
	start_song();
	cpu.cycles += 4 + 14 + 10 + 10 + 10 + 10;
	cpu.iff1 = cpu.iff2 = 1;
	cpu.ix = pop16(); SET_HL(pop16()); SET_DE(pop16()); SET_BC(pop16());
}
Native native_snd_start_sound = { "snd_start_sound", 0x8AF3, 0x1F, start_sound, 0 };

// $8152-$824B: the FM unit's registers from their shadow copies: a changed
// instrument ($DD56), then for each of its 6 (9 without rhythm) channels the
// key/block/volume, tone and instrument registers when they changed, then
// the rhythm part ($0E, $36-$38). Not reached without an FM unit (and so
// not tested): a line-by-line translation.
static void fm_out(void) {
	cpu.cycles += 10 + 12 + 7;
	SET_HL(0xDD56);
	if (mem_read(HL) & 0x80) {         // the instrument
		cpu.cycles += 10 + 15 + 7 + 4 + 7 + 33 + 11 + 10;
		mem_write(HL, mem_read(HL) & 0x7F);
		cpu.a = mem_read(HL);
		SET_HL(op_add16((uint16_t)(cpu.a * 8), 0x8EBD));
		SET_BC(0x0800);
		do {
			cpu.cycles += 4;
			cpu.a = cpu.c;
			out_port(0xF0, cpu.a);
			cpu.cycles += 7;
			cpu.a = mem_read(HL);
			out_port(0xF1, cpu.a);
			cpu.cycles += 4 + 11 + 10 + 11 + 10 + 4 + 6;
			cpu.c++;
			SET_HL(HL + 1);
			if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
		} while (1);
	} else cpu.cycles += 5;
	cpu.cycles += 10 + 10 + 10 + 4 + 10 + 10 + 7 + 7 + 8 + 7;   // $8178
	SET_HL(0xDD12); SET_DE(0xDD1B); SET_BC(0xDD24);
	op_exx();
	SET_DE(0xDD41); SET_HL(0xDD4A);
	cpu.c = 0; cpu.b = 6;
	cpu.a = mem_read(0xDD58);
	if (!(cpu.a & 0x40)) { cpu.cycles += 7; cpu.b = 9; } else cpu.cycles += 5;
	for (;;) {                         // $8192: a channel
		cpu.cycles += 4 + 7 + 4;
		cpu.a = (uint8_t)(cpu.c | 0x30);
		op_exx();
		out_port(0xF0, cpu.a);
		cpu.cycles += 7 + 4 + 7;
		cpu.a = mem_read(DE);
		op_and(cpu.a);
		if (!cpu.a) {                  // off: volume $BF; the tone if changed
			cpu.cycles += 7;
			cpu.a = 0xBF;
			out_port(0xF1, cpu.a);
			cpu.cycles += 7 + 6 + 6 + 6 + 4 + 7 + 7;
			cpu.a = mem_read(DE);
			SET_BC(BC + 1); SET_DE(DE + 1); SET_HL(HL + 1);
			op_exx();
			op_sub8(mem_read(HL), 0, 0);
			if (cpu.f & FZ) { cpu.cycles += 5; goto skip3; }
			cpu.cycles += 7 + 4 + 7 + 6 + 6;
			mem_write(HL, cpu.a);
			cpu.a = (uint8_t)(cpu.c | 0x20);
			SET_DE(DE + 1); SET_HL(HL + 1);
			out_port(0xF0, cpu.a);
			cpu.cycles += 7;
			cpu.a = mem_read(HL);
			out_port(0xF1, cpu.a);
			cpu.cycles += 4 + 11 + 10 + 7 + 4 + 10;
			cpu.c++;
			op_and(0);
			goto next;
		}
		cpu.cycles += 5 + 7;           // $81C9: on: the volume
		cpu.a = mem_read(BC);
		out_port(0xF1, cpu.a);
		cpu.cycles += 7 + 4 + 7 + 10;
		cpu.a = mem_read(DE);
		op_exx();
		op_sub8(mem_read(HL), 0, 0);
		if (!(cpu.f & FZ)) {           // the block and key changed
			cpu.cycles += 8 + 4 + 4 + 7 + 4 + 7 + 7 + 4;
			op_bit(6, cpu.a);
			op_ex_af();
			cpu.a = (uint8_t)(cpu.c | 0x20);
			op_exx();
			op_and(0xFF); op_and(0xFF);
			out_port(0xF0, cpu.a);
			cpu.cycles += 4 + 7;
			op_ex_af();
			if (!(cpu.f & FZ)) {       // keyed again: off first
				cpu.a &= 0xEF;
				cpu.cycles += 7;
				out_port(0xF1, cpu.a);
				cpu.cycles += 7 + 17 + 11 + 10 + 11 + 10 + 10;
				cpu.a |= 0x10;
			} else cpu.cycles += 5;
			out_port(0xF1, cpu.a);
			cpu.cycles += 7 + 4 + 7;
			cpu.a &= 0x9F;
			op_and(0xFF);
			op_exx();
			mem_write(HL, cpu.a);
		}
		cpu.cycles += 7 + 4 + 7 + 10;  // $81F2: the tone
		cpu.a = mem_read(DE);
		op_exx();
		op_sub8(mem_read(HL), 0, 0);
		if (cpu.f & FZ) {              // $81BC
			cpu.cycles += 6 + 6 + 6 + 4;
			SET_BC(BC + 1); SET_DE(DE + 1); SET_HL(HL + 1);
			op_exx();
		skip3:
			cpu.cycles += 6 + 6 + 4;   // $81C0
			SET_DE(DE + 1); SET_HL(HL + 1);
			cpu.c++;
		next:
			cpu.cycles += 8;           // $81C3: djnz
			if (--cpu.b) { cpu.cycles += 5; continue; }
			cpu.cycles += 4 + 10;      // nop / jp $8212
			goto rhythm;
		}
		cpu.cycles += 4 + 4 + 7 + 4 + 4 + 4;
		op_exx();
		cpu.a = (uint8_t)(cpu.c | 0x10);
		op_exx();
		out_port(0xF0, cpu.a);
		cpu.cycles += 7;
		cpu.a = mem_read(HL);
		out_port(0xF1, cpu.a);
		cpu.cycles += 6 + 6 + 6 + 4 + 7 + 6 + 6 + 4 + 4 + 10;
		SET_BC(BC + 1); SET_DE(DE + 1); SET_HL(HL + 1);
		op_exx();
		mem_write(DE, cpu.a);
		SET_DE(DE + 1); SET_HL(HL + 1);
		cpu.c++;
		if (--cpu.b) continue;
		cpu.cycles += 7;               // $8210
		break;
	}
rhythm:
	cpu.cycles += 7;                   // $8212
	cpu.a = 0x0E;
	out_port(0xF0, cpu.a);
	cpu.cycles += 13;
	cpu.a = mem_read(0xDD40);
	out_port(0xF1, cpu.a);
	cpu.cycles += 4 + 13 + 4 + 10;
	cpu.c = cpu.a;
	cpu.a = mem_read(0xDD3F);
	op_sub8(cpu.c, 0, 0);
	if (!(cpu.f & FZ)) {
		cpu.cycles += 17 + 10 + 7 + 7 + 4;
		op_and(0xFF); op_and(0xFF);
		out_port(0xF1, cpu.a);
	}
	cpu.cycles += 4 + 7;               // $822D
	op_and(cpu.a);
	if (cpu.a) {                       // the rhythm volumes that changed
		cpu.cycles += 10 + 10 + 10;
		SET_BC(0x0336); SET_DE(0xDD2A); SET_HL(0xDD53);
		do {
			cpu.cycles += 7 + 7 + 7;
			cpu.a = mem_read(DE);
			op_sub8(mem_read(HL), 0, 0);
			if (!(cpu.f & FZ)) {
				cpu.cycles += 4 + 4;
				op_ex_af();
				cpu.a = cpu.c;
				out_port(0xF0, cpu.a);
				cpu.cycles += 4;
				op_ex_af();
				out_port(0xF1, cpu.a);
				cpu.cycles += 7 + 4 + 4;
				mem_write(HL, cpu.a);
			} else cpu.cycles += 5;
			cpu.cycles += 4 + 6 + 6;
			cpu.c++;
			SET_DE(DE + 1); SET_HL(HL + 1);
			if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
		} while (1);
	} else cpu.cycles += 5;
	psg_out();
}

// $814B: the chips' registers out: the FM's (and then the PSG's), or the PSG's
static void sound_out(void) {
	cpu.cycles += 13 + 4 + 10;
	cpu.a = mem_read(0xDD58);
	op_and(cpu.a);
	if (cpu.a & 0x80) fm_out(); else psg_out();
}
Native native_snd_out = { "snd_out", 0x814B, 0x1F, sound_out, 0, .long_calls = 1 };

// $84DA: the fade over (HL = $DD02): full volume, the channels' fade bits off
static void fade_over(void) {
	cpu.cycles += 15 + 7 + 13 + 10 + 10 + 7;
	mem_write(HL, mem_read(HL) & ~0x20);
	mem_write(0xDD11, 0x0F);
	SET_BC(0x0028);
	SET_HL(0xDD68);
	cpu.a = 7;
	do {
		cpu.cycles += 15 + 11 + 4 + 7;
		mem_write(HL, mem_read(HL) & ~0x08);
		SET_HL(HL + 0x28);
		cpu.a = op_dec8(cpu.a);
		if (cpu.a) cpu.cycles += 5;
	} while (cpu.a);
	cpu.cycles += 10;
}

// $8018 (vector $8000): the frame: each channel's tempo, the note's end
// (the next from the song), its volume, modulation, slide, output and
// vibrato; the fade; then the chips' registers. Bit 0 of $DD02: stop all
// first; bit 1 (or channel 8 playing): channel 8 alone
static void sound_frame(void) {
	cpu.cycles += 10 + 12;
	SET_HL(0xDD02);
	op_bit(2, mem_read(HL));
	if (mem_read(HL) & 0x04) { cpu.cycles += 11; return; }
	cpu.cycles += 5 + 17;
	silence();
	cpu.cycles += 15 + 12 + 10;
	mem_write(0xDD02, mem_read(0xDD02) | 0x04);
	SET_HL(0xDD02);
	if (mem_read(HL) & 0x01) {         // stop: song 0, every channel muted
		cpu.cycles += 15 + 11 + 4 + 17;
		mem_write(HL, mem_read(HL) & ~0x01);
		push16(HL);
		cpu.a = 0;
		op_sub8(0, 0, 1);
		start_song();
		cpu.cycles += 10 + 10 + 7;
		SET_HL(0xDD69); SET_DE(0x0028);
		cpu.b = 8;
		do {
			cpu.cycles += 15 + 11;
			mem_write(HL, mem_read(HL) | 0x02);
			SET_HL(HL + 0x28);
			if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
		} while (1);
		cpu.cycles += 10 + 12;
		SET_HL(pop16());
	}
	cpu.cycles += 12 + 7;              // $8047
	int only8 = 0;
	if (mem_read(HL) & 0x02) { cpu.cycles += 5; only8 = 1; }
	else {
		cpu.cycles += 13 + 4 + 7;
		cpu.a = mem_read(0xDE75);
		op_and(cpu.a);
		if (cpu.a) { cpu.cycles += 5; only8 = 1; }
	}
	if (only8) { cpu.cycles += 7 + 14 + 12; cpu.b = 1; cpu.ix = 0xDE75; }
	else { cpu.cycles += 7 + 14; cpu.b = 7; cpu.ix = 0xDD5D; }
	for (;;) {                         // $8057: a channel
		native_sync();
		cpu.cycles += 19 + 4 + 10;
		cpu.a = IXR(0);
		op_and(cpu.a);
		if (!cpu.a) goto next;
		cpu.cycles += 4 + 19 + 19 + 4;
		cpu.d = cpu.a;
		cpu.e = IXR(0x0B);
		cpu.c = IXR(0x0C);
		op_exx();
		cpu.cycles += 19 + 4 + 7;
		cpu.a = IXR(5);
		op_and(cpu.a);
		if (cpu.a) {                   // its tempo: a tick when the sum carries
			cpu.cycles += 19 + 19 + 7;
			op_add8(IXR(0x0F), 0);
			IXW(0x0F, cpu.a);
			if (!(cpu.f & FC)) { cpu.cycles += 5; goto sounding; }
		} else cpu.cycles += 5;
		cpu.cycles += 19;              // $8074: the note goes on
		cpu.b = IXR(0x0D);
		int struck = alt_e_bit(2);
		cpu.cycles += 7;
		if (struck) {                  // the strike ends at +$15 frames (bit 7: before the end)
			cpu.cycles += 19 + 4 + 7;
			cpu.a = IXR(0x15);
			op_and(cpu.a);
			int over = 0;
			if (cpu.a) {
				cpu.cycles += 10;
				if (cpu.a & 0x80) {
					cpu.cycles += 7 + 4 + 7;
					op_and(0x7F);
					op_sub8(cpu.b, 0, 0);
					if (cpu.f & FZ) { cpu.cycles += 5; over = 1; }
					else {
						cpu.cycles += 7;
						if (!(cpu.f & FC)) { cpu.cycles += 5; over = 0; }
						else over = 1;
					}
				} else {
					cpu.cycles += 4 + 7;
					op_add8(cpu.b, 0);
					if (cpu.f & FC) { cpu.cycles += 5; over = 1; }
					else {
						cpu.cycles += 19 + 7;
						op_sub8(IXR(0x0E), 0, 0);
						if (cpu.f & FC) { cpu.cycles += 5; over = 0; }
						else { cpu.cycles += 10; over = 1; }
					}
				}
			} else cpu.cycles += 5;
			if (over) {                // $8098
				cpu.cycles += 4 + 8 + 19 + 4;
				op_exx();
				cpu.e &= ~0x04;
				IXW(0x0B, cpu.e);
				op_exx();
			}
		} else cpu.cycles += 5;
		cpu.cycles += 4 + 19 + 4 + 19; // $809F: a frame of the note; at its end, the next
		cpu.b++;
		IXW(0x0D, cpu.b);
		cpu.a = cpu.b;
		op_sub8(IXR(0x0E), 0, 0);
		if (cpu.f & FZ) {
			cpu.cycles += 17;
			push16(0x80AA);
			if (sequencer() == CHANNEL_DONE) goto next;
			pop16();
		} else cpu.cycles += 10;
	sounding:
		native_sync();
		cpu.cycles += 19 + 4 + 7;      // $80AA: its volume
		cpu.a = IXR(2);
		op_add8(cpu.a, 0);
		int fixed = !cpu.a;
		if (!fixed) {
			cpu.cycles += 20 + 10;
			if (!(IXR(1) & 0x10)) { cpu.cycles += 17; envelope(); goto modulate; }
		} else cpu.cycles += 5;
		cpu.cycles += 19 + 13 + 10;    // $80B7
		cpu.a = IXR(1);
		mem_write(0xDD5A, cpu.a);
	modulate:
		cpu.cycles += 19 + 7 + 4;      // $80C3
		cpu.a = IXR(3);
		op_and(0x3F);
		op_add8(cpu.a, 0);
		if (cpu.a) { cpu.cycles += 17; modulation(); } else cpu.cycles += 10;
		native_sync();
		if (alt_e_bit(7)) { cpu.cycles += 17; slide(); } else cpu.cycles += 10;
		native_sync();
		cpu.cycles += 17;
		channel_out();
		native_sync();
		cpu.cycles += 4 + 8 + 10;      // $80D6: vibrato on the volume
		op_exx();
		if (cpu.e & 0x20) {
			cpu.cycles += 19 + 19 + 19 + 7;
			cpu.a = IXR(0x18);
			op_add8(IXR(0x19), 0);
			IXW(0x19, cpu.a);
			if (cpu.f & FC) {
				cpu.cycles += 19 + 19 + 10;
				cpu.a = IXR(1);
				op_sub8(IXR(0x1A), 0, 0);
				if (cpu.f & FZ) { cpu.cycles += 8 + 10; cpu.e |= 0x02; }
				else {
					cpu.cycles += 8 + 19 + 4 + 8 + 7;
					cpu.e &= ~0x02;
					cpu.a = op_dec8(IXR(1));
					if (cpu.e & 0x10) cpu.cycles += 5;
					else { cpu.cycles += 7; op_add8(2, 0); }
					cpu.cycles += 19;
					IXW(1, cpu.a);
				}
			} else cpu.cycles += 5;
		}
		cpu.cycles += 19;              // $8104
		IXW(0x0B, cpu.e);
	next:
		cpu.cycles += 13 + 7 + 13 + 10 + 15 + 4 + 10;   // $8107
		cpu.a = mem_read(0xDD02) & 0x3F;
		op_and(0xFF);
		mem_write(0xDD02, cpu.a);
		SET_DE(0x0028);
		cpu.ix += 0x28;
		if (--cpu.b) continue;
		break;
	}
	cpu.cycles += 10 + 8 + 7;          // $8118: the fade
	SET_HL(0xDD02);
	if (cpu.a & 0x20) {
		cpu.cycles += 20 + 13 + 4 + 13 + 7;
		SET_DE(rd16(0xDD0F));
		cpu.a = mem_read(0xDD0E);
		op_add8(cpu.e, 0);
		mem_write(0xDD0F, cpu.a);
		if (cpu.f & FC) {
			cpu.cycles += 13 + 4 + 7;
			cpu.a = mem_read(0xDD11);
			op_sub8(cpu.d, 0, 0);
			if (!(cpu.f & FZ)) {
				cpu.cycles += 12 + 7;
				if (mem_read(HL) & 0x10) {
					cpu.cycles += 5 + 4 + 13;
					cpu.a = op_dec8(cpu.a);
					mem_write(0xDD11, cpu.a);
				} else {
					cpu.cycles += 4 + 13 + 4 + 7;
					cpu.a = op_inc8(cpu.a);
					mem_write(0xDD11, cpu.a);
					op_sub8(cpu.d, 0, 0);
					if (cpu.f & FZ) { cpu.cycles += 17; fade_over(); cpu.cycles += 12; }
					else cpu.cycles += 5;
				}
			} else cpu.cycles += 5;
		} else cpu.cycles += 5;
	} else cpu.cycles += 5;
	cpu.cycles += 10 + 15;             // $8146
	SET_HL(0xDD02);
	mem_write(HL, mem_read(HL) & ~0x04);
	sound_out();
}
Native native_snd_frame = { "snd_frame", 0x8018, 0x1F, sound_frame, 0, .long_calls = 1 };

static void sequencer_entry(void) {
	if (sequencer() == CHANNEL_DONE) native_continue_at(0x8107);
}
Native native_snd_sequencer = { "snd_sequencer", 0x8329, 0x1F, sequencer_entry, 0, .long_calls = 1 };

// $8ABE (vector $8003): every channel off, the fade over, the chips quiet
static void stop_all(void) {
	push16(cpu.ix); cpu.cycles += 15; native_boundary();
	cpu.ix = 0xDD5D; cpu.cycles += 14; native_boundary();
	SET_DE(0x0028); cpu.cycles += 10; native_boundary();
	cpu.b = 8; cpu.cycles += 7; native_boundary();
	cpu.a = 0;
	op_sub8(0, 0, 1); cpu.cycles += 4; native_boundary();
	do {
		IXW(0, 0); cpu.cycles += 19; native_boundary();
		cpu.ix += 0x28; cpu.cycles += 15; native_boundary();
		if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; native_boundary(); break; }
		native_boundary();
	} while (1);
	cpu.ix = pop16(); cpu.cycles += 14; native_boundary();
	cpu.a = mem_read(0xDD02); cpu.cycles += 13; native_boundary();
	cpu.a &= ~0x20; cpu.cycles += 8; native_boundary();
	mem_write(0xDD02, cpu.a); cpu.cycles += 13; native_boundary();
	cpu.a = 0x0F; cpu.cycles += 7; native_boundary();
	mem_write(0xDD11, 0x0F); cpu.cycles += 13; native_boundary();
	cpu.iff1 = cpu.iff2 = 0; cpu.cycles += 4;
	cpu.cycles += 17;
	silence();
	cpu.cycles += 17;
	sound_out();
	cpu.cycles += 13 + 4;
	cpu.a = mem_read(0xDD58);
	op_and(cpu.a);
	if (cpu.a & 0x80) { cpu.cycles += 17; psg_out(); } else cpu.cycles += 10;
	cpu.iff1 = cpu.iff2 = 1; cpu.cycles += 4;
	native_irq_point();
	cpu.a = mem_read(0xDD58); cpu.cycles += 13;
	cpu.cycles += 10;
}
Native native_snd_stop_all = { "snd_stop_all", 0x8ABE, 0x1F, stop_all, 0, .long_calls = 1 };

// $8A4A (vector $8009): the driver set up for the FM unit (A: bit 7, bit 6
// without rhythm) or not; then everything stopped
static void sound_init(void) {
	cpu.cycles += 13 + 4 + 10;
	mem_write(0xDD58, cpu.a);
	op_and(cpu.a);
	if (cpu.a) {
		cpu.cycles += 7 + 8;
		cpu.b = 9;
		if (!(cpu.a & 0x40)) { cpu.cycles += 17; fm_rhythm_init(); } else cpu.cycles += 10;
		cpu.cycles += 12 + 17 + 7 + 10 + 10;   // jr / call $8A8C
		cpu.c = 0x10;
		SET_DE(0xDD41); SET_HL(0xDD4A);
		fm_clear();
		cpu.cycles += 12;
	}
	cpu.cycles += 4 + 13 + 13 + 4 + 7 + 10;   // $8AA6
	mem_write(0xDD56, 0);
	cpu.a = mem_read(0xDD58);
	op_and(cpu.a);
	int fm = (cpu.a & 0x80) != 0;
	cpu.a = 0;
	if (fm) { cpu.cycles += 4; cpu.a = 1; }
	out_port(0xF2, cpu.a);
	cpu.cycles += 4 + 13 + 4 + 13;
	mem_write(0xDD02, 0);
	cpu.a = 0xFF;
	mem_write(0xDD0C, 0xFF);
	stop_all();
}
Native native_snd_init = { "snd_init", 0x8A4A, 0x1F, sound_init, 0, .long_calls = 1 };

// $8B03: a fade of the channels playing, B of them, at speed A; bit 7 of C:
// up (from 0), else down to C
static void fade(void) {
	cpu.cycles += 4 + 11 + 11 + 15 + 13 + 4 + 13 + 14;
	cpu.iff1 = cpu.iff2 = 0;
	push16(DE); push16(HL); push16(cpu.ix);
	mem_write(0xDD0E, cpu.a);
	cpu.a = 0;
	op_sub8(0, 0, 1);
	mem_write(0xDD0F, 0);
	cpu.ix = 0xDD5D;
	do {
		cpu.cycles += 4 + 19 + 4 + 4 + 7;
		cpu.e = cpu.a;
		uint8_t on = IXR(0);
		op_and(on);
		cpu.a = cpu.e;
		if (on) {
			cpu.cycles += 19 + 23;
			op_or(IXR(0x0B));
			IXW(0x0B, IXR(0x0B) | 0x08);
		} else cpu.cycles += 5;
		cpu.cycles += 10 + 15;
		SET_DE(0x0028);
		cpu.ix += 0x28;
		if (--cpu.b) cpu.cycles += 13; else { cpu.cycles += 8; break; }
	} while (1);
	cpu.cycles += 14 + 11 + 10 + 8 + 8 + 7;
	cpu.ix = pop16();
	uint8_t a = cpu.a, f = cpu.f;
	SET_HL(0xDD02);
	int up = cpu.c & 0x80;
	cpu.c &= 0x7F;
	if (!up) {
		cpu.cycles += 15 + 4 + 13 + 7 + 12;
		mem_write(HL, mem_read(HL) & ~0x10);
		mem_write(0xDD10, cpu.c);
		cpu.c = 0;
	} else {
		cpu.cycles += 5 + 15 + 4 + 13 + 7;
		mem_write(HL, mem_read(HL) | 0x10);
		mem_write(0xDD10, cpu.c);
		cpu.c = 0x0F;
	}
	cpu.cycles += 10 + 12 + 15 + 7;
	cpu.a = a; cpu.f = f;
	int was = mem_read(HL) & 0x20;
	mem_write(HL, mem_read(HL) | 0x20);
	int keep = 0;
	if (was) {
		cpu.cycles += 8 + 7;
		if (cpu.a & 0x08) { cpu.cycles += 5; keep = 1; }
	} else cpu.cycles += 5;
	if (!keep) { cpu.cycles += 4 + 13; cpu.a = cpu.c; mem_write(0xDD11, cpu.c); }
	cpu.cycles += 10 + 10 + 4 + 10;
	SET_HL(pop16()); SET_DE(pop16());
	cpu.iff1 = cpu.iff2 = 1;
}
Native native_snd_fade = { "snd_fade", 0x8B03, 0x1F, fade, 0, .long_calls = 1 };

static void fade_up(void) {            // $8F1F (vector $800C): all seven channels up from silence
	cpu.cycles += 7 + 7 + 7 + 10;
	cpu.b = 7; cpu.a = 0x20; cpu.c = 0x80;
	fade();
}
Native native_snd_fade_up = { "snd_fade_up", 0x8F1F, 0x1F, fade_up, 0, .long_calls = 1 };

static void fade_down(void) {          // $8F28 (vector $800F): and down to silence
	cpu.cycles += 7 + 7 + 7 + 10;
	cpu.b = 7; cpu.a = 0x20; cpu.c = 0x0F;
	fade();
}
Native native_snd_fade_down = { "snd_fade_down", 0x8F28, 0x1F, fade_down, 0, .long_calls = 1 };

static void pause_on(void) { cpu.cycles += 7 + 13 + 10; cpu.a = 3; mem_write(0xDD02, 3); }   // $8B5A (vector $8012)
static void pause_off(void) { cpu.cycles += 7 + 12 + 13 + 10; cpu.a = 1; mem_write(0xDD02, 1); }   // $8B60 (vector $8015)
Native native_snd_pause_on = { "snd_pause_on", 0x8B5A, 0x1F, pause_on, 0 };
Native native_snd_pause_off = { "snd_pause_off", 0x8B60, 0x1F, pause_off, 0 };
