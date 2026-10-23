#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sms.h"
#include "z80.h"

SMS sms;

enum { LINES = 262, CYCLES_PER_LINE = 228 };

// ---------------------------------------------------------------- VDP ----
static struct {
	uint8_t vram[0x4000], cram[32], reg[16];
	uint16_t addr;
	uint8_t code, latch, first, buffer, status, line_counter, line_pending;
	int line;
	uint8_t fb[SCREEN_W * SCREEN_H];  // CRAM index per pixel
} vdp;

static int vdp_irq(void) {
	return ((vdp.status & 0x80) && (vdp.reg[1] & 0x20)) || (vdp.line_pending && (vdp.reg[0] & 0x10));
}

static uint8_t vdp_read_data(void) {
	vdp.latch = 0;
	uint8_t v = vdp.buffer;
	vdp.buffer = vdp.vram[vdp.addr];
	vdp.addr = (vdp.addr + 1) & 0x3FFF;
	return v;
}
static void vdp_write_data(uint8_t v) {
	vdp.latch = 0;
	if (vdp.code == 3) vdp.cram[vdp.addr & 0x1F] = v & 0x3F;
	else vdp.vram[vdp.addr] = v;
	vdp.buffer = v;
	vdp.addr = (vdp.addr + 1) & 0x3FFF;
}
static uint8_t vdp_read_status(void) {
	uint8_t v = vdp.status | 0x1F;
	vdp.status = 0; vdp.line_pending = 0; vdp.latch = 0;
	return v;
}
static void vdp_write_ctrl(uint8_t v) {
	if (!vdp.latch) {
		vdp.latch = 1; vdp.first = v;
		vdp.addr = (vdp.addr & 0x3F00) | v;
		return;
	}
	vdp.latch = 0;
	vdp.code = v >> 6;
	vdp.addr = ((v & 0x3F) << 8) | vdp.first;
	if (vdp.code == 0) { vdp.buffer = vdp.vram[vdp.addr]; vdp.addr = (vdp.addr + 1) & 0x3FFF; }
	else if (vdp.code == 2) vdp.reg[v & 0x0F] = vdp.first;
}
static uint8_t vdp_vcounter(void) { return (uint8_t)(vdp.line <= 0xDA ? vdp.line : vdp.line - 6); }

static void vdp_render_line(int y) {
	uint8_t *row = vdp.fb + y * SCREEN_W, *reg = vdp.reg, *vram = vdp.vram;
	uint8_t backdrop = 16 + (reg[7] & 0x0F);
	if (!(reg[1] & 0x40)) { memset(row, backdrop, SCREEN_W); return; }
	uint8_t prio[SCREEN_W], opaque[SCREEN_W], drawn[SCREEN_W];
	int name_base = (reg[2] & 0x0E) << 10;
	int sx = (y < 16 && (reg[0] & 0x40)) ? 0 : reg[8];
	for (int x = 0; x < SCREEN_W; x++) {
		int lock_v = (reg[0] & 0x80) && x >= 192;
		int vy = lock_v ? y : (y + reg[9]) % 224;
		int px = (x - sx) & 0xFF;
		int ent = name_base + (((vy >> 3) * 32 + (px >> 3)) << 1);
		int w = vram[ent] | (vram[ent + 1] << 8);
		int tile = w & 0x1FF, ty = vy & 7, tx = px & 7;
		if (w & 0x400) ty = 7 - ty;
		if (!(w & 0x200)) tx = 7 - tx;
		int a = (tile << 5) + (ty << 2);
		int c = ((vram[a] >> tx) & 1) | (((vram[a + 1] >> tx) & 1) << 1) |
			(((vram[a + 2] >> tx) & 1) << 2) | (((vram[a + 3] >> tx) & 1) << 3);
		row[x] = (uint8_t)(((w & 0x800) ? 16 : 0) + c);
		opaque[x] = c != 0;
		prio[x] = (w & 0x1000) != 0;
	}
	int sat = (reg[5] & 0x7E) << 7;
	int tall = reg[1] & 0x02, zoom = reg[1] & 0x01;
	int h = (tall ? 16 : 8) << (zoom ? 1 : 0);
	int pat_base = (reg[6] & 0x04) ? 256 : 0;
	memset(drawn, 0, sizeof drawn);
	int count = 0;
	for (int i = 0; i < 64; i++) {
		int sy = vram[sat + i];
		if (sy == 0xD0) break;
		sy += 1; if (sy > 240) sy -= 256;
		if (y < sy || y >= sy + h) continue;
		if (++count > 8) { vdp.status |= 0x40; break; }
		int sxp = vram[sat + 128 + i * 2];
		if (reg[0] & 0x08) sxp -= 8;
		int n = vram[sat + 129 + i * 2];
		if (tall) n &= 0xFE;
		int ly = (y - sy) >> (zoom ? 1 : 0);
		int a = ((pat_base + n + (ly >> 3)) << 5) + ((ly & 7) << 2);
		for (int k = 0; k < (zoom ? 16 : 8); k++) {
			int x = sxp + k;
			if (x < 0 || x > 255) continue;
			int bit = 7 - (zoom ? k >> 1 : k);
			int c = ((vram[a] >> bit) & 1) | (((vram[a + 1] >> bit) & 1) << 1) |
				(((vram[a + 2] >> bit) & 1) << 2) | (((vram[a + 3] >> bit) & 1) << 3);
			if (!c) continue;
			if (drawn[x]) { vdp.status |= 0x20; continue; }
			drawn[x] = 1;
			if (prio[x] && opaque[x]) continue;
			row[x] = (uint8_t)(16 + c);
		}
	}
	if (reg[0] & 0x20) memset(row, backdrop, 8);
}

static void vdp_end_line(int line) {
	if (line < SCREEN_H) vdp_render_line(line);
	if (line <= 192) {
		if (vdp.line_counter == 0) { vdp.line_counter = vdp.reg[10]; vdp.line_pending = 1; }
		else vdp.line_counter--;
	} else vdp.line_counter = vdp.reg[10];
	if (line == 192) vdp.status |= 0x80;
}

// ---------------------------------------------------------------- PSG ----
static struct {
	uint16_t tone[3];
	uint8_t vol[4], noise, latch;
	uint16_t lfsr;
	double counter[4];
	int out[4];
	int rate;
} psg = { .vol = {15, 15, 15, 15}, .lfsr = 0x8000, .rate = 44100 };

static void psg_write(uint8_t v) {
	if (v & 0x80) {
		psg.latch = (v >> 4) & 7;
		int ch = psg.latch >> 1;
		if (psg.latch & 1) psg.vol[ch] = v & 0x0F;
		else if (ch == 3) { psg.noise = v & 7; psg.lfsr = 0x8000; }
		else psg.tone[ch] = (psg.tone[ch] & 0x3F0) | (v & 0x0F);
	} else {
		int ch = psg.latch >> 1;
		if (psg.latch & 1) psg.vol[ch] = v & 0x0F;
		else if (ch == 3) { psg.noise = v & 7; psg.lfsr = 0x8000; }
		else psg.tone[ch] = (psg.tone[ch] & 0x0F) | ((v & 0x3F) << 4);
	}
}

void sms_audio_rate(int rate) { psg.rate = rate; }

int sms_audio(int16_t *out, int max) {
	static const int vol_table[16] = {
		8191, 6507, 5168, 4105, 3261, 2590, 2057, 1634, 1298, 1031, 819, 650, 516, 410, 326, 0 };
	int n = psg.rate / 60;
	if (n > max) n = max;
	double ticks = 3579545.0 / 16.0 / psg.rate;
	for (int s = 0; s < n; s++) {
		int mix = 0;
		for (int ch = 0; ch < 3; ch++) {
			int period = psg.tone[ch] ? psg.tone[ch] : 1;
			psg.counter[ch] -= ticks;
			while (psg.counter[ch] <= 0) { psg.counter[ch] += period; psg.out[ch] ^= 1; }
			if (period > 1) mix += psg.out[ch] ? vol_table[psg.vol[ch]] : -vol_table[psg.vol[ch]];
		}
		int nperiod = (psg.noise & 3) == 3 ? (psg.tone[2] ? psg.tone[2] : 1) : (0x10 << (psg.noise & 3));
		psg.counter[3] -= ticks;
		while (psg.counter[3] <= 0) {
			psg.counter[3] += nperiod;
			psg.out[3] ^= 1;
			if (psg.out[3]) {
				int fb = (psg.noise & 4) ? ((psg.lfsr & 1) ^ ((psg.lfsr >> 3) & 1)) : (psg.lfsr & 1);
				psg.lfsr = (uint16_t)((psg.lfsr >> 1) | (fb << 15));
			}
		}
		mix += (psg.lfsr & 1) ? vol_table[psg.vol[3]] : -vol_table[psg.vol[3]];
		if (mix > 32767) mix = 32767;
		if (mix < -32768) mix = -32768;
		out[s] = (int16_t)(mix / 2);
	}
	return n;
}

// ---------------------------------------------------------------- bus ----
int rom_bank_at(uint16_t a) {
	if (a < 0x400) return 0;
	if (a < 0x4000) return sms.mapper[1] % sms.rom_banks;
	if (a < 0x8000) return sms.mapper[2] % sms.rom_banks;
	if (a < 0xC000 && !(sms.mapper[0] & 0x08)) return sms.mapper[3] % sms.rom_banks;
	return -1;
}

uint8_t mem_read(uint16_t a) {
	if (a >= 0xC000) return sms.ram[a & 0x1FFF];
	if (a >= 0x8000 && (sms.mapper[0] & 0x08)) return sms.sram[((sms.mapper[0] & 0x04) << 12) | (a & 0x3FFF)];
	return sms.rom[(rom_bank_at(a) << 14) | (a & 0x3FFF)];
}

void mem_write(uint16_t a, uint8_t v) {
	if (a >= 0xC000) {
		sms.ram[a & 0x1FFF] = v;
		if (a >= 0xFFFC) sms.mapper[a - 0xFFFC] = v;
		return;
	}
	if (a >= 0x8000 && (sms.mapper[0] & 0x08)) {
		sms.sram[((sms.mapper[0] & 0x04) << 12) | (a & 0x3FFF)] = v;
		sms.sram_dirty = 1;
	}
}

uint8_t io_in(uint16_t port) {
	switch (port & 0xC1) {
	case 0x40: return vdp_vcounter();
	case 0x41: return 0;
	case 0x80: return vdp_read_data();
	case 0x81: { uint8_t v = vdp_read_status(); cpu.irq_line = (uint8_t)vdp_irq(); return v; }
	case 0xC0: return (uint8_t)~sms.pad1;
	case 0xC1: {
		uint8_t v = sms.reset ? 0xEF : 0xFF;   // bit 4: Reset
		// TH pins read back what the I/O control port drives
		if (!(sms.ioctrl & 0x02)) v = (v & ~0x40) | ((sms.ioctrl & 0x20) << 1);
		if (!(sms.ioctrl & 0x08)) v = (v & ~0x80) | (sms.ioctrl & 0x80);
		return v;
	}
	default: return 0xFF;
	}
}

void io_out(uint16_t port, uint8_t v) {
	switch (port & 0xC1) {
	case 0x01: sms.ioctrl = v; break;
	case 0x40: case 0x41: psg_write(v); break;
	case 0x80: vdp_write_data(v); break;
	case 0x81:
		vdp_write_ctrl(v);
		cpu.irq_line = (uint8_t)vdp_irq();
		if (cpu.irq_line && cpu.iff1) cpu.budget = cpu.cycles;  // take it after this instruction
		break;
	default: break;
	}
}

// --------------------------------------------------------------- frame ----
void sms_init(const uint8_t *rom, int size) {
	memset(&sms, 0, sizeof sms);
	sms.rom = rom;
	sms.rom_banks = size >> 14;
	sms.mapper[1] = 0; sms.mapper[2] = 1; sms.mapper[3] = 2;
	sms.ioctrl = 0xFF;
	static const uint8_t init_regs[11] = {0x36, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFB, 0, 0, 0, 0xFF};
	memset(&vdp, 0, sizeof vdp);
	memcpy(vdp.reg, init_regs, sizeof init_regs);
	vdp.line_counter = 0xFF;
	z80_reset();
}

void sms_pause(void) { cpu.nmi_pending = 1; }

// Lines are timed on an absolute clock, so a rewritten routine that does
// many lines' worth of work at once (adding the original's cycle count) takes
// that many lines, as the original did.
static int64_t line_end;
void (*sms_on_line)(void);

void sms_run_frame(void) {
	const int64_t frame = (int64_t)CYCLES_PER_LINE * LINES;
	if (cpu.cycles - line_end > 2 * frame || line_end - cpu.cycles > 2 * CYCLES_PER_LINE)
		line_end = cpu.cycles;   // first frame, or a loaded state
	for (int line = 0; line < LINES; line++) {
		vdp.line = line;
		line_end += CYCLES_PER_LINE;
		cpu.budget = line_end;
		recomp_run();
		if (sms_on_line) sms_on_line();
		vdp_end_line(line);
		cpu.irq_line = (uint8_t)vdp_irq();
	}
	for (int i = 0; i < SCREEN_W * SCREEN_H; i++) {
		uint8_t c = vdp.cram[vdp.fb[i]];
		sms.rgb[i] = ((c & 3) * 85u << 16) | (((c >> 2) & 3) * 85u << 8) | ((c >> 4) & 3) * 85u;
	}
	sms.frame++;
}

// --------------------------------------------------- machine snapshots ----
// Used by the shadow tester (native.c) to run a rewritten routine and the
// original on the same machine state and compare the results.
struct Snap {
	Z80 cpu;
	uint8_t ram[0x2000], sram[0x8000], mapper[4], ioctrl;
	__typeof__(vdp) vdp;
	__typeof__(psg) psg;
};

Snap *snap_new(void) { return calloc(1, sizeof(Snap)); }

void snap_save(Snap *s) {
	s->cpu = cpu;
	memcpy(s->ram, sms.ram, sizeof s->ram);
	memcpy(s->sram, sms.sram, sizeof s->sram);
	memcpy(s->mapper, sms.mapper, 4);
	s->ioctrl = sms.ioctrl;
	s->vdp = vdp;
	s->psg = psg;
}

void snap_load(const Snap *s) {
	cpu = s->cpu;
	memcpy(sms.ram, s->ram, sizeof s->ram);
	memcpy(sms.sram, s->sram, sizeof s->sram);
	memcpy(sms.mapper, s->mapper, 4);
	sms.ioctrl = s->ioctrl;
	vdp = s->vdp;
	psg = s->psg;
}

const Z80 *snap_cpu(const Snap *s) { return &s->cpu; }

// Compare everything but the CPU registers. RAM in [stack_lo, stack_hi) and
// the listed scratch addresses are ignored. Returns the number of differences
// and describes the first few in msg.
int snap_compare(const Snap *a, const Snap *b, uint16_t stack_lo, uint16_t stack_hi,
                 const uint16_t *ignore, int nignore, char *msg, size_t n) {
	int diffs = 0;
	size_t used = 0;
	msg[0] = 0;
#define NOTE(...) do { if (diffs++ < 6 && used < n) used += snprintf(msg + used, n - used, __VA_ARGS__); } while (0)
	for (int i = 0; i < 0x2000; i++) {
		uint16_t addr = 0xC000 + i;
		if (a->ram[i] == b->ram[i]) continue;
		if (addr >= stack_lo && addr < stack_hi) continue;
		int skip = 0;
		for (int k = 0; k < nignore; k++) if (ignore[k] == addr) skip = 1;
		if (!skip) NOTE(" ram[%04X] %02X/%02X", addr, a->ram[i], b->ram[i]);
	}
	if (memcmp(a->sram, b->sram, sizeof a->sram)) NOTE(" battery RAM");
	if (memcmp(a->mapper, b->mapper, 4)) NOTE(" mapper");
	if (memcmp(a->vdp.vram, b->vdp.vram, sizeof a->vdp.vram)) NOTE(" VRAM");
	if (memcmp(a->vdp.cram, b->vdp.cram, sizeof a->vdp.cram)) NOTE(" CRAM");
	if (memcmp(a->vdp.reg, b->vdp.reg, sizeof a->vdp.reg) || a->vdp.addr != b->vdp.addr ||
	    a->vdp.code != b->vdp.code || a->vdp.latch != b->vdp.latch) NOTE(" VDP registers/address");
	if (memcmp(a->psg.tone, b->psg.tone, sizeof a->psg.tone) || memcmp(a->psg.vol, b->psg.vol, sizeof a->psg.vol) ||
	    a->psg.noise != b->psg.noise || a->psg.latch != b->psg.latch) NOTE(" PSG");
#undef NOTE
	return diffs;
}

// Load a machine snapshot (--state, for tests). 0 on success.
int sms_load_state(const char *path) {
	FILE *f = fopen(path, "rb");
	if (!f) return -1;
	uint8_t m[4], r[16], w[8], misc[8], v7[7], cram[32], reg[16];
	int64_t cyc;
	uint16_t vaddr;
	uint32_t frame;
	int ok = fread(m, 1, 4, f) == 4 && !memcmp(m, "U4ST", 4) && fread(r, 1, 16, f) == 16 &&
		fread(w, 1, 8, f) == 8 && fread(misc, 1, 8, f) == 8 && fread(&cyc, 8, 1, f) == 1 &&
		fread(sms.ram, 1, 0x2000, f) == 0x2000 && fread(sms.sram, 1, 0x8000, f) == 0x8000 &&
		fread(sms.mapper, 1, 4, f) == 4 && fread(&sms.ioctrl, 1, 1, f) == 1 &&
		fread(vdp.vram, 1, 0x4000, f) == 0x4000 && fread(cram, 1, 32, f) == 32 && fread(reg, 1, 16, f) == 16 &&
		fread(&vaddr, 2, 1, f) == 1 && fread(v7, 1, 7, f) == 7 && fread(&frame, 4, 1, f) == 1;
	fclose(f);
	if (!ok) return -1;
	cpu.a = r[0]; cpu.f = r[1]; cpu.b = r[2]; cpu.c = r[3]; cpu.d = r[4]; cpu.e = r[5]; cpu.h = r[6]; cpu.l = r[7];
	cpu.a_ = r[8]; cpu.f_ = r[9]; cpu.b_ = r[10]; cpu.c_ = r[11]; cpu.d_ = r[12]; cpu.e_ = r[13]; cpu.h_ = r[14]; cpu.l_ = r[15];
	cpu.ix = w[0] | w[1] << 8; cpu.iy = w[2] | w[3] << 8; cpu.sp = w[4] | w[5] << 8; cpu.pc = w[6] | w[7] << 8;
	cpu.i = misc[0]; cpu.r = misc[1]; cpu.iff1 = misc[2]; cpu.iff2 = misc[3]; cpu.im = misc[4];
	cpu.halted = misc[5]; cpu.ei_delay = misc[6]; cpu.nmi_pending = misc[7];
	cpu.cycles = cyc;
	memcpy(vdp.cram, cram, 32); memcpy(vdp.reg, reg, 16);
	vdp.addr = vaddr; vdp.code = v7[0]; vdp.latch = v7[1]; vdp.first = v7[2]; vdp.buffer = v7[3];
	vdp.status = v7[4]; vdp.line_counter = v7[5]; vdp.line_pending = v7[6];
	sms.frame = frame;
	cpu.irq_line = (uint8_t)vdp_irq();
	return 0;
}
