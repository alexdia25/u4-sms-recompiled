// Tasks rewritten in C (see docs/architecture.md, "Tasks").
//
// A task runs in steps: run_tasks jumps to the slot's resume address with IX
// pointing at the slot, and the step ends by yielding back to run_tasks. A
// rewritten step is a native routine registered at that resume address. It
// does the step's work and says where the task continues next time
// (task_yield / task_sleep / task_end), exactly as the original's yield would
// leave the slot, so rewritten and original steps can follow each other.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

// task slot fields (IX = the current slot)
#define SLOT          cpu.ix
#define SLOT_WAIT     (SLOT + 0x01)   // frames until the next step
#define SLOT_RESUME   (SLOT + 0x02)   // 16-bit
#define SLOT_SIZE     0x30

static void task_sleep(uint8_t frames, uint16_t resume) {   // sleep ($0B16)
	mem_write(SLOT_WAIT, frames);
	wr16(SLOT_RESUME, resume);
}

static void task_end(void) {                                // end_task ($0B5A)
	for (int i = 0; i < SLOT_SIZE; i++) mem_write(SLOT + i, 0);
}

// sound routines not rewritten yet
static void stop_music(void) { call_original(0x0605); }
static void play_music(uint8_t tune) { cpu.a = tune; call_original(0x0621); }

// music_later ($33D4, started by play_music_later at $33AF): stop the music,
// wait 20 frames, then start the tune stored in the slot (+$1A) and end.
#define MUSIC_LATER_TUNE (SLOT + 0x1A)

// (cycle counts: the original's, around the calls to the sound routines)
static void music_later_start(void) {   // $33D4
	cpu.cycles += 17;
	stop_music();
	task_sleep(20, 0x33DC);
	cpu.cycles += 7 + 17 + 19 + 58;      // sleep and yield
}

static void music_later_play(void) {    // $33DC
	cpu.cycles += 19 + 17;
	play_music(mem_read(MUSIC_LATER_TUNE));
	task_end();
	cpu.cycles += 10 + 1049;             // jp end_task, which clears the slot with ldir
}

Native native_music_later_start = { "music_later_start", 0x33D4, 0, music_later_start, 0, 0, 0 };
Native native_music_later_play = { "music_later_play", 0x33DC, 0, music_later_play, 0, 0, 0 };

// ----------------------------------------------------------------------------
// frame_tick ($2FFB): what every waiting task does once per frame, then yield.
// In combat the current member's figure is outlined and colour $1E of the
// palette cycles through 9 shades (the selection blink); then the text cursor
// position is updated ($335C) and the screen's moving parts are refreshed
// ($129D). The task continues after its call next frame.
#define MAP_MODE        0xC99F
#define CURRENT_MEMBER  0xC97F
#define BLINK_STEP      0xC63C
#define PALETTE_SHADOW  0xC023   // 32 colours, copied to the VDP when $C009 is set
#define PALETTE_DIRTY   0xC009
static const uint16_t BLINK_COLOURS = 0x30D1;   // 9 colours in bank 0

static void set_palette_entry(uint8_t index, uint8_t colour) {   // $0843
	mem_write(PALETTE_SHADOW + index, colour);
	mem_write(PALETTE_DIRTY, 0xFF);
	cpu.a = 0xFF;
}

void frame_tick_work(void) {
	uint8_t mode = mem_read(MAP_MODE);
	cpu.a = mode;
	if (mode == 3 || mode == 5) {   // combat
		uint8_t member = mem_read(CURRENT_MEMBER);
		cpu.cycles += (mode == 3 ? 32 : 41) + 302 + 24 * member;   // up to the call
		cpu.iy = 0xC8B9 + member * 5;            // the member's figure (member_object)
		SET_HL(rd16(cpu.iy));                    // its x, y
		cpu.a = 1;
		call_original(0x30B6);                   // outline it
		uint8_t step = mem_read(BLINK_STEP) + 1;
		cpu.cycles += 234 + (step < 9 ? 12 : 11);
		if (step >= 9) step = 0;
		mem_write(BLINK_STEP, step);
		uint8_t colour = mem_read(BLINK_COLOURS + step);
		cpu.h = colour;
		cpu.l = 0x1E;
		set_palette_entry(0x1E, colour);
	} else {
		cpu.cycles += 46;
	}
	cpu.cycles += 17;
	call_original(0x335C);
	cpu.cycles += 17;
	call_original(0x129D);
}

// The cycle counts added here and above are the original's (entry to ret,
// leaving out what call_original runs), added before each call_original so
// the calls start at the same time as the original's.
#define YIELD_CYCLES (10 + 58)   // jp yield, and yield itself

static void task_yield(uint16_t resume) {   // yield ($0B19): continue at resume next frame
	wr16(SLOT_RESUME, resume);
	SET_HL(resume);   // yield leaves it in HL
}

static void frame_tick(void) {
	frame_tick_work();
	cpu.cycles += YIELD_CYCLES;
	task_yield(pop16());   // the caller's return address is where the task goes on
}
Native native_frame_tick = { "frame_tick", 0x2FFB, 0, frame_tick, 0, 0, 0, .long_calls = 1 };

// ----------------------------------------------------------------------------
// wait_button ($3033): run frame_tick every frame until button 1 or 2 is
// pressed, beep, and return to the caller. Because it waits across frames it
// keeps its caller's return address in the slot (task_enter_4, slot +$2A)
// instead of on the shared stack.
#define SLOT_RETURN_4   (SLOT + 0x2A)
#define JOY_PRESSED     0xC0B0
#define BUTTONS         0x30

static void wait_button_start(void) {   // $3033
	uint16_t caller = pop16();
	wr16(SLOT_RETURN_4, caller);       // task_enter_4
	cpu.cycles += 17 + 77 + 17;
	frame_tick_work();
	cpu.cycles += YIELD_CYCLES;
	task_yield(0x3039);
}

static void wait_button_check(void) {   // $3039, each following frame
	uint8_t pressed = mem_read(JOY_PRESSED) & BUTTONS;
	cpu.a = pressed;
	if (!pressed) {
		cpu.cycles += 13 + 7 + 12 + 17;
		frame_tick_work();
		cpu.cycles += YIELD_CYCLES;
		task_yield(0x3039);
		return;
	}
	cpu.cycles += 13 + 7 + 7 + 17;
	call_original(0x40D9);             // beep
	cpu.cycles += 10 + 42;
	uint16_t caller = rd16(SLOT_RETURN_4);
	SET_HL(caller);                    // task_return_4
	native_continue_at(caller);
}

Native native_wait_button_start = { "wait_button_start", 0x3033, 0, wait_button_start, 0, 0, 0, .long_calls = 1 };
Native native_wait_button_check = { "wait_button_check", 0x3039, 0, wait_button_check, 0, 0, 0, .long_calls = 1 };


// ----------------------------------------------------------------------------
// For C tasks: the same waits, as calls that end the task's turn and come
// back on its next one (task_wait_turn). The task's slot is left exactly as
// the original's would be: its resume address, and wait_button's return
// address in +$2A.

// "call frame_tick" at site: the task goes on after the call next turn
void task_frame_tick(uint16_t site) {
	cpu.cycles += 17;
	frame_tick_work();
	cpu.cycles += YIELD_CYCLES;
	SET_HL(site + 3);                  // yield leaves the resume address in HL
	task_wait_turn(site + 3);
}

// "call wait_button" at site ($3033): frame_tick every turn until button 1
// or 2 is pressed, then beep
void task_wait_button(uint16_t site) {
	uint16_t back = site + 3;
	cpu.cycles += 17 + 17 + 77;        // the call, and its call to task_enter_4
	wr16(SLOT_RETURN_4, back);
	for (;;) {
		task_frame_tick(0x3036);
		cpu.cycles += 13 + 7;
		uint8_t pressed = mem_read(JOY_PRESSED) & BUTTONS;
		cpu.a = pressed;
		if (pressed) break;
		cpu.cycles += 12;              // jr z back to the frame_tick call
	}
	cpu.cycles += 7 + 17;
	call_original(0x40D9);             // beep
	cpu.cycles += 10 + 42;             // jp task_return_4
	SET_HL(back);
}

// "ld a,n / call $3046" at site: refresh the screen's moving parts ($129D)
// and let n turns pass
void task_wait_frames(uint16_t site, uint8_t n) {
	cpu.cycles += 7 + 17 + 17 + 77 + 19;   // ld a, the call, task_enter_4, ld (ix+$19),a
	wr16(SLOT_RETURN_4, site + 3);
	mem_write(SLOT + 0x19, n);
	for (;;) {
		cpu.cycles += 17;
		call_original(0x129D);
		cpu.cycles += 17 + 58;         // call yield
		SET_HL(0x3052);
		task_wait_turn(0x3052);
		cpu.cycles += 23 + 7;
		uint8_t left = (uint8_t)(mem_read(SLOT + 0x19) - 1);
		mem_write(SLOT + 0x19, left);
		if (!left) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 10 + 42;             // jp task_return_4
	SET_HL(site + 3);
}
