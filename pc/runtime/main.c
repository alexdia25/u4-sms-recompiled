// PC front end: SDL2 window, keyboard/gamepad input, sound, battery save.
// The game's data comes from an asset pack built once from the player's ROM
// (python tools/make_assets.py rom.sms u4.pak); the exe never reads the ROM.
//   u4 rom.sms                           first run: build u4.pak from your ROM, then play
//   u4 [u4.pak]                          play (default: u4.pak next to the exe)
//   u4 --import rom.sms                  only build u4.pak
//   u4 u4.pak --headless N [--shot f.ppm] [--keys script]
//                                        run N frames without a window (testing)
//   options: --mute (no sound), --quit-after N (frames), --state s.bin,
//            --verify-pack rom.sms (compare the pack's rebuilt image with a ROM)
// Keys: arrows = D-pad, Z = button 1, X = button 2, Enter = Pause, Esc = quit.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sms.h"
#include "z80.h"
#include "native.h"
#include "assets.h"
#include "import.h"

#ifndef U4_HEADLESS_ONLY
#define SDL_MAIN_HANDLED
#include <SDL.h>
#endif

static uint8_t *load_file(const char *path, long *size) {
	FILE *f = fopen(path, "rb");
	if (!f) return NULL;
	fseek(f, 0, SEEK_END);
	*size = ftell(f);
	fseek(f, 0, SEEK_SET);
	uint8_t *p = malloc(*size);
	if (fread(p, 1, *size, f) != (size_t)*size) { fclose(f); free(p); return NULL; }
	fclose(f);
	return p;
}

static char save_path[1024];

static void load_save(const char *rom_path) {
	snprintf(save_path, sizeof save_path, "%s", rom_path);
	char *dot = strrchr(save_path, '.');
	if (dot) *dot = 0;
	strncat(save_path, ".sav", sizeof save_path - strlen(save_path) - 1);
	FILE *f = fopen(save_path, "rb");
	if (f) { size_t n = fread(sms.sram, 1, sizeof sms.sram, f); (void)n; fclose(f); }
}

static void write_save(void) {
	if (!sms.sram_dirty) return;
	FILE *f = fopen(save_path, "wb");
	if (f) { fwrite(sms.sram, 1, sizeof sms.sram, f); fclose(f); }
	sms.sram_dirty = 0;
}

static void write_ppm(const char *path) {
	FILE *f = fopen(path, "wb");
	if (!f) return;
	fprintf(f, "P6\n%d %d\n255\n", SCREEN_W, SCREEN_H);
	for (int i = 0; i < SCREEN_W * SCREEN_H; i++) {
		uint8_t px[3] = { (uint8_t)(sms.rgb[i] >> 16), (uint8_t)(sms.rgb[i] >> 8), (uint8_t)sms.rgb[i] };
		fwrite(px, 1, 3, f);
	}
	fclose(f);
}

// --keys script: lines "<frame> <buttons>" (buttons from U D L R 1 2, or - for none,
// P for Pause), applied from that frame on; "<frame> =C9AB:FF" writes RAM
// U4_LOG_FALLBACK=file: list ROM addresses (bank:addr) that ran on the interpreter
static uint8_t fallback_seen[32][0x4000];
static void log_fallback(uint16_t pc) {
	int b = rom_bank_at(pc);
	if (b >= 0 && b < 32) fallback_seen[b][pc & 0x3FFF] = 1;
}

// the RAM without the stack ($DE00-$DFEF): it holds stale return addresses
// that depend on exact timing and on how rewritten routines call the
// original ones; equal state-hashes mean the game is in the same state
static uint32_t state_hash(void) {
	uint32_t g = 2166136261u;
	for (int i = 0; i < 0x2000; i++)
		if (i < 0x1E00 || i >= 0x1FF0) g = (g ^ sms.ram[i]) * 16777619u;
	return g;
}

static int headless(int frames, const char *shot, const char *keys) {
	FILE *k = keys ? fopen(keys, "r") : NULL;
	int next_frame = -1;
	char btn[32] = "";
	if (k && fscanf(k, "%d %31s", &next_frame, btn) != 2) next_frame = -1;
	unsigned random_seed = getenv("U4_RANDOM") ? (unsigned)atoi(getenv("U4_RANDOM")) : 0;
	int random_from = getenv("U4_RANDOM_FROM") ? atoi(getenv("U4_RANDOM_FROM")) : 0;   // after a script
	FILE *wav = getenv("U4_WAV") ? fopen(getenv("U4_WAV"), "wb") : NULL;
	for (int f = 0; f < frames; f++) {
		if (random_seed && f >= random_from) {   // U4_RANDOM=seed: random play
			static int hold;
			if (hold-- <= 0) {
				random_seed = random_seed * 1103515245u + 12345u;
				unsigned r = (random_seed >> 8) % 100;
				static const uint8_t dirs[4] = { PAD_UP, PAD_DOWN, PAD_LEFT, PAD_RIGHT };
				sms.pad1 = r < 45 ? dirs[(random_seed >> 20) & 3] : r < 75 ? PAD_2 : r < 88 ? PAD_1 : 0;
				hold = 3 + (int)((random_seed >> 12) % 20);
			}
		}
		while (next_frame == f) {
			unsigned addr, val;
			if (sscanf(btn, "=%x:%x", &addr, &val) == 2) {   // "=ADDR:VAL" pokes RAM (for tests)
				mem_write((uint16_t)addr, (uint8_t)val);
				if (fscanf(k, "%d %31s", &next_frame, btn) != 2) next_frame = -1;
				continue;
			}
			sms.pad1 = 0;
			for (char *c = btn; *c; c++) {
				switch (*c) {
				case 'U': sms.pad1 |= PAD_UP; break; case 'D': sms.pad1 |= PAD_DOWN; break;
				case 'L': sms.pad1 |= PAD_LEFT; break; case 'R': sms.pad1 |= PAD_RIGHT; break;
				case '1': sms.pad1 |= PAD_1; break; case '2': sms.pad1 |= PAD_2; break;
				case 'P': sms_pause(); break;
				}
			}
			if (fscanf(k, "%d %31s", &next_frame, btn) != 2) next_frame = -1;
		}
		sms_run_frame();
		if (wav) {   // U4_WAV=file: raw 16-bit mono 44.1 kHz sound
			int16_t buf[2048];
			fwrite(buf, 2, (size_t)sms_audio(buf, 2048), wav);
		}
		if (getenv("U4_DUMP_FRAME") && atoi(getenv("U4_DUMP_FRAME")) == f + 1) {
			FILE *df = fopen("ram_c.bin", "wb"); fwrite(sms.ram, 1, 0x2000, df); fclose(df);
		}
		if (getenv("U4_FRAME_HASH")) {
			uint32_t fh = 2166136261u;
			for (int i = 0; i < 0x2000; i++) fh = (fh ^ sms.ram[i]) * 16777619u;
			printf("%d %08X\n", f + 1, fh);
		}
	}
	if (k) fclose(k);
	if (wav) fclose(wav);
	if (getenv("U4_LOG_FALLBACK")) {
		FILE *lf = fopen(getenv("U4_LOG_FALLBACK"), "w");
		for (int b = 0; b < 32; b++)
			for (int o = 0; o < 0x4000; o++)
				if (fallback_seen[b][o]) fprintf(lf, "%02X:%04X\n", b, (b == 0 ? 0 : b == 1 ? 0x4000 : 0x8000) + o);
		fclose(lf);
	}
	if (shot) write_ppm(shot);
	uint32_t h = 2166136261u;
	for (int i = 0; i < 0x2000; i++) h = (h ^ sms.ram[i]) * 16777619u;
	printf("frames %d pc %04X ram-hash %08X state-hash %08X interpreted %llu\n", frames, cpu.pc, h,
		state_hash(), (unsigned long long)recomp_fallbacks);
	// U4_SETTLE=n: n more frames with no input, printing the state at each
	// pass of the task scheduler (native.c). A frame can end while a
	// rewritten routine has done at once work the original is half way
	// through; at the scheduler neither is in the middle of anything.
	int settle = getenv("U4_SETTLE") ? atoi(getenv("U4_SETTLE")) : 0;
	native_settle = settle > 0;
	for (int f = 0; f < settle; f++) {
		sms.pad1 = 0;
		sms_run_frame();
	}
	return 0;
}

int main(int argc, char **argv) {
	const char *pack_path = "u4.pak", *import_path = NULL, *verify_rom = NULL, *shot = NULL, *keys = NULL, *state = NULL;
	int frames = -1, mute = 0, quit_after = -1;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--headless") && i + 1 < argc) frames = atoi(argv[++i]);
		else if (!strcmp(argv[i], "--shot") && i + 1 < argc) shot = argv[++i];
		else if (!strcmp(argv[i], "--keys") && i + 1 < argc) keys = argv[++i];
		else if (!strcmp(argv[i], "--mute")) mute = 1;
		else if (!strcmp(argv[i], "--state") && i + 1 < argc) state = argv[++i];
		else if (!strcmp(argv[i], "--quit-after") && i + 1 < argc) quit_after = atoi(argv[++i]);
		else if (!strcmp(argv[i], "--verify-pack") && i + 1 < argc) verify_rom = argv[++i];
		else if (!strcmp(argv[i], "--import") && i + 1 < argc) import_path = argv[++i];
		else pack_path = argv[i];
	}
	// The pack (and save) live next to the exe unless a path is given.
	static char default_pack[1024];
	if (!strcmp(pack_path, "u4.pak")) {
		snprintf(default_pack, sizeof default_pack, "%s", argv[0]);
		char *slash = strrchr(default_pack, '/'), *bslash = strrchr(default_pack, '\\');
		if (bslash > slash) slash = bslash;
		if (slash) { strcpy(slash + 1, "u4.pak"); pack_path = default_pack; }
	}
	// Given a ROM instead of a pack (or --import), build the pack from it first.
	size_t pl = strlen(pack_path);
	if (pl > 4 && !strcmp(pack_path + pl - 4, ".sms")) {
		import_path = pack_path;
		snprintf(default_pack, sizeof default_pack, "%s", argv[0]);
		char *slash = strrchr(default_pack, '/'), *bslash = strrchr(default_pack, '\\');
		if (bslash > slash) slash = bslash;
		if (slash) strcpy(slash + 1, "u4.pak"); else strcpy(default_pack, "u4.pak");
		pack_path = default_pack;
	}
	if (import_path) {
		if (import_rom(import_path, pack_path)) return 1;
		if (frames < 0 && argc == 3 && !strcmp(argv[1], "--import")) return 0;   // import only
	}
	uint8_t *image;
	int size;
	if (assets_load(pack_path, &image, &size)) {
		fprintf(stderr, "cannot load %s: give the exe your Ultima IV ROM once (u4 rom.sms) to create it\n", pack_path);
		return 1;
	}
	if (verify_rom) {   // check the image built from the pack against a ROM
		long rsize;
		uint8_t *rom = load_file(verify_rom, &rsize);
		if (!rom || rsize != size) { fprintf(stderr, "cannot read %s or size differs\n", verify_rom); return 1; }
		long diffs = 0;
		for (long i = 0; i < size; i++) if (rom[i] != image[i] && diffs++ < 5) fprintf(stderr, "differs at $%05lX\n", i);
		printf("%ld bytes differ\n", diffs);
		return diffs != 0;
	}
	sms_init(image, size);
	load_save(pack_path);
	if (state && sms_load_state(state)) { fprintf(stderr, "cannot load state %s\n", state); return 1; }
	if (getenv("U4_LOG_FALLBACK")) recomp_on_fallback = log_fallback;
	if (frames >= 0) { int r = headless(frames, shot, keys); native_report(); return r; }

#ifdef U4_HEADLESS_ONLY
	fprintf(stderr, "built without SDL; use --headless\n");
	return 1;
#else
	SDL_SetMainReady();
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0) {
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		return 1;
	}
	SDL_Window *win = SDL_CreateWindow("Ultima IV", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		SCREEN_W * 3, SCREEN_H * 3, SDL_WINDOW_RESIZABLE);
	SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_RenderSetLogicalSize(ren, SCREEN_W, SCREEN_H);
	SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_W, SCREEN_H);
	SDL_AudioSpec want = {0}, have;
	want.freq = 44100; want.format = AUDIO_S16SYS; want.channels = 1; want.samples = 1024;
	SDL_AudioDeviceID audio = mute ? 0 : SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
	if (audio) { sms_audio_rate(have.freq); SDL_PauseAudioDevice(audio, 0); }
	SDL_GameController *pad = NULL;
	for (int i = 0; i < SDL_NumJoysticks(); i++)
		if (SDL_IsGameController(i)) { pad = SDL_GameControllerOpen(i); break; }

	int running = 1;
	uint64_t freq = SDL_GetPerformanceFrequency(), next = SDL_GetPerformanceCounter();
	while (running) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) running = 0;
			if (e.type == SDL_KEYDOWN && !e.key.repeat) {
				if (e.key.keysym.sym == SDLK_ESCAPE) running = 0;
				if (e.key.keysym.sym == SDLK_RETURN) sms_pause();
			}
			if (e.type == SDL_CONTROLLERBUTTONDOWN && e.cbutton.button == SDL_CONTROLLER_BUTTON_START) sms_pause();
		}
		const uint8_t *ks = SDL_GetKeyboardState(NULL);
		uint8_t p = 0;
		if (ks[SDL_SCANCODE_UP]) p |= PAD_UP;
		if (ks[SDL_SCANCODE_DOWN]) p |= PAD_DOWN;
		if (ks[SDL_SCANCODE_LEFT]) p |= PAD_LEFT;
		if (ks[SDL_SCANCODE_RIGHT]) p |= PAD_RIGHT;
		if (ks[SDL_SCANCODE_Z]) p |= PAD_1;
		if (ks[SDL_SCANCODE_X]) p |= PAD_2;
		if (pad) {
			if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_UP)) p |= PAD_UP;
			if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_DOWN)) p |= PAD_DOWN;
			if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_LEFT)) p |= PAD_LEFT;
			if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) p |= PAD_RIGHT;
			if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_A)) p |= PAD_1;
			if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_B)) p |= PAD_2;
		}
		sms.pad1 = p;
		sms_run_frame();
		if (audio && SDL_GetQueuedAudioSize(audio) < (Uint32)have.freq / 10 * 2) {
			int16_t buf[2048];
			int n = sms_audio(buf, 2048);
			SDL_QueueAudio(audio, buf, n * 2);
		}
		SDL_UpdateTexture(tex, NULL, sms.rgb, SCREEN_W * 4);
		SDL_RenderClear(ren);
		SDL_RenderCopy(ren, tex, NULL, NULL);
		SDL_RenderPresent(ren);
		if ((sms.frame % 120) == 0) write_save();
		if (quit_after >= 0 && (int)sms.frame >= quit_after) running = 0;
		// hold 60 Hz even when vsync is faster or missing
		next += freq / 60;
		uint64_t now = SDL_GetPerformanceCounter();
		if (now < next) SDL_Delay((Uint32)((next - now) * 1000 / freq));
		else if (now - next > freq / 4) next = now;
	}
	write_save();
	if (shot) write_ppm(shot);
	SDL_Quit();
	return 0;
#endif
}
