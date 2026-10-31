// Master System hardware for the PC build: Sega mapper with battery RAM,
// mode-4 VDP, SN76489 PSG and joypads.
#ifndef U4_SMS_H
#define U4_SMS_H

#include <stddef.h>
#include <stdint.h>

enum { SCREEN_W = 256, SCREEN_H = 192 };

// joypad 1 bits (1 = pressed)
enum { PAD_UP = 0x01, PAD_DOWN = 0x02, PAD_LEFT = 0x04, PAD_RIGHT = 0x08, PAD_1 = 0x10, PAD_2 = 0x20 };

typedef struct SMS {
	const uint8_t *rom;
	int rom_banks;
	uint8_t ram[0x2000];
	uint8_t sram[0x8000];
	int sram_dirty;
	uint8_t mapper[4];   // $FFFC..$FFFF
	uint8_t ioctrl;
	uint8_t pad1;        // pressed buttons
	uint8_t pad2;        // the second joypad's, the same bits
	uint8_t reset;       // the Reset button held
	uint32_t frame;
	uint32_t rgb[SCREEN_W * SCREEN_H];   // last finished frame, 0x00RRGGBB
} SMS;

extern SMS sms;

void sms_init(const uint8_t *rom, int size);
void sms_run_frame(void);     // one 60 Hz frame
extern void (*sms_on_line)(void);   // called after each line's code (profiling)
int sms_load_state(const char *path);   // a machine snapshot (--state, for tests)
void sms_pause(void);         // the console's Pause button (NMI)
// mix `n` mono samples at `rate` Hz of the sound generated during the last frame
int sms_audio(int16_t *out, int max);
void sms_audio_rate(int rate);

// run.c: run the rewrites (and the interpreter for anything else) until cpu.cycles >= cpu.budget
// (falls back to the interpreter where there is no translation)
void machine_run(void);
extern uint64_t machine_interpreted;
extern void (*machine_on_interpreted)(uint16_t pc);

// machine snapshots for the shadow tester
typedef struct Snap Snap;
Snap *snap_new(void);
void snap_save(Snap *s);
void snap_load(const Snap *s);
const struct Z80 *snap_cpu(const Snap *s);
int snap_compare(const Snap *a, const Snap *b, uint16_t stack_lo, uint16_t stack_hi,
                 const uint16_t *ignore, int nignore, char *msg, size_t n);

#endif
