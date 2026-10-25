// The calls into the sound driver from bank 0: each pages bank $1F into
// slot 2, calls one of the driver's entries and puts the old bank back.
// - sound_reset ($05C7): $8009, with the FM unit's flag ($C002) as A.
// - sound_stop ($05E3): $8003; sound_silence ($0605): $800C;
//   sound_tick ($0613): $800F.
// - play_sound ($05F1): $8006 with A (kept at $C0B9).
// - play_music ($0621): A as the tune, unless it is already playing
//   ($C0B8): $8003, then $8006. After $8003, $8009 and $800C no tune is
//   playing ($C0B8 = $FF).
// - read_pad1 ($0646): the first pad's six buttons, 1 = held.
// Cycle counts are the original's, instruction by instruction (I).
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define SLOT2_BANK     0xDFFF
#define MUSIC_PLAYING  0xC0B8
#define SOUND_ARG      0xC0B9

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static void call(uint16_t addr) { I(17); call_original(addr); }

static void page_driver(void) {        // ld a,(slot2_bank) / push af / ld a,$1F / ld ($FFFF),a
	cpu.a = mem_read(SLOT2_BANK); I(13);
	push16((uint16_t)(cpu.a << 8 | cpu.f)); I(11);
	cpu.a = 0x1F; I(7);
	mem_write(0xFFFF, cpu.a); I(13);
}
static void restore_bank(void) {       // $05DE: pop af / ld ($FFFF),a / ret
	uint16_t af = pop16(); cpu.a = af >> 8; cpu.f = (uint8_t)af; I(10);
	mem_write(0xFFFF, cpu.a); I(13);
	I(10);
}
static void no_music(void) {           // $05D9
	cpu.a = 0xFF; I(7);
	mem_write(MUSIC_PLAYING, cpu.a); I(13);
	restore_bank();
}

static void sound_reset(void) {        // $05C7
	page_driver();
	cpu.a = mem_read(0xC002); I(13);
	op_rrca(); I(4);
	cpu.a = op_rot(5, cpu.a); I(8);    // sra a
	call(0x8009);
	no_music();
}

static void sound_stop(void) { page_driver(); call(0x8003); I(12); no_music(); }      // $05E3
static void sound_silence(void) { page_driver(); call(0x800C); I(12); no_music(); }   // $0605
static void sound_tick(void) { page_driver(); call(0x800F); I(12); restore_bank(); }  // $0613

static void play_sound(void) {         // $05F1
	mem_write(SOUND_ARG, cpu.a); I(13);
	page_driver();
	cpu.a = mem_read(SOUND_ARG); I(13);
	call(0x8006);
	I(12);
	restore_bank();
}

static void play_music(void) {         // $0621
	push16(HL); I(11);
	SET_HL(MUSIC_PLAYING); I(10);
	op_sub8(mem_read(HL), 0, 0); I(7);
	mem_write(HL, cpu.a); I(7);
	SET_HL(pop16()); I(10);
	if (cpu.f & FZ) { I(11); return; }
	I(5);
	page_driver();
	call(0x8003);
	cpu.a = mem_read(MUSIC_PLAYING); I(13);
	call(0x8006);
	I(12);
	restore_bank();
}

static void read_pad1(void) {          // $0646
	cpu.a = io_in((uint16_t)(cpu.a << 8 | 0xDC)); I(11);
	op_cpl(); I(4);
	op_and(0x3F); I(7);
	I(10);
}

#define SOUND_CALL(name, addr, out) Native native_##name = { #name, addr, 0, name, out, .long_calls = 1 };
SOUND_CALL(sound_reset, 0x05C7, OUT_A | OUT_F)
SOUND_CALL(sound_stop, 0x05E3, OUT_A | OUT_F)
SOUND_CALL(play_sound, 0x05F1, OUT_A | OUT_F)
SOUND_CALL(sound_silence, 0x0605, OUT_A | OUT_F)
SOUND_CALL(sound_tick, 0x0613, OUT_A | OUT_F)
SOUND_CALL(play_music, 0x0621, OUT_A | OUT_F)
SOUND_CALL(read_pad1, 0x0646, OUT_A | OUT_F)
