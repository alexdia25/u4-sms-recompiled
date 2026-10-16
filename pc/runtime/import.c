// Build u4.pak from the player's ROM (mirrors tools/make_assets.py and
// tools/pack_formats.py). Every region is cut into its entry, then rebuilt
// with the loader's own rules and compared with the ROM; a region that
// doesn't come back exactly is stored raw instead, so the pack is faithful.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "import.h"
#include "assets.h"

enum { BANK = 0x4000 };
static const char *EXPECTED_SHA1 = "a90e21e5961bcf2e10b715a009c04e7c2017a3b1";

// ------------------------------------------------------------- byte buffer --
typedef struct { uint8_t *p; uint32_t n, cap; } Buf;
static void put(Buf *b, const void *src, uint32_t n) {
	if (b->n + n > b->cap) { b->cap = (b->n + n) * 2 + 256; b->p = realloc(b->p, b->cap); }
	memcpy(b->p + b->n, src, n);
	b->n += n;
}
static void put8(Buf *b, uint8_t v) { put(b, &v, 1); }
static void put16(Buf *b, uint16_t v) { uint8_t t[2] = { (uint8_t)v, (uint8_t)(v >> 8) }; put(b, t, 2); }
static void put32(Buf *b, uint32_t v) { put16(b, (uint16_t)v); put16(b, (uint16_t)(v >> 16)); }

// ------------------------------------------------------------------ SHA-1 --
static void sha1(const uint8_t *data, size_t len, char out[41]) {
	uint32_t h[5] = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0 };
	size_t total = ((len + 8) / 64 + 1) * 64;
	uint8_t *m = calloc(total, 1);
	memcpy(m, data, len);
	m[len] = 0x80;
	uint64_t bits = (uint64_t)len * 8;
	for (int i = 0; i < 8; i++) m[total - 1 - i] = (uint8_t)(bits >> (8 * i));
#define ROL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
	for (size_t c = 0; c < total; c += 64) {
		uint32_t w[80];
		for (int i = 0; i < 16; i++)
			w[i] = (uint32_t)m[c + 4 * i] << 24 | m[c + 4 * i + 1] << 16 | m[c + 4 * i + 2] << 8 | m[c + 4 * i + 3];
		for (int i = 16; i < 80; i++) w[i] = ROL(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
		uint32_t a = h[0], b = h[1], cc = h[2], d = h[3], e = h[4];
		for (int i = 0; i < 80; i++) {
			uint32_t f, k;
			if (i < 20) { f = (b & cc) | (~b & d); k = 0x5A827999; }
			else if (i < 40) { f = b ^ cc ^ d; k = 0x6ED9EBA1; }
			else if (i < 60) { f = (b & cc) | (b & d) | (cc & d); k = 0x8F1BBCDC; }
			else { f = b ^ cc ^ d; k = 0xCA62C1D6; }
			uint32_t t = ROL(a, 5) + f + e + k + w[i];
			e = d; d = cc; cc = ROL(b, 30); b = a; a = t;
		}
		h[0] += a; h[1] += b; h[2] += cc; h[3] += d; h[4] += e;
	}
#undef ROL
	free(m);
	for (int i = 0; i < 5; i++) sprintf(out + 8 * i, "%08x", h[i]);
}

// ------------------------------------------------------------ run-length --
// Split a stream at blk[a] (a bank) into values and tokens; returns the
// offset after its $00, or -1.
static long rle_split(const uint8_t *blk, long a, Buf *values, Buf *tokens) {
	for (;;) {
		if (a >= BANK) return -1;
		uint8_t t = blk[a++];
		if (t == 0) return a;
		put8(tokens, t);
		if (t & 0x80) {
			if (a + (t & 0x7F) > BANK) return -1;
			put(values, blk + a, t & 0x7F);
			a += t & 0x7F;
		} else {
			if (a >= BANK) return -1;
			for (int i = 0; i < t; i++) put8(values, blk[a]);
			a++;
		}
	}
}

static long put_stream(Buf *out, const uint8_t *blk, long a) {
	Buf v = {0}, t = {0};
	long e = rle_split(blk, a, &v, &t);
	if (e >= 0) { put16(out, (uint16_t)v.n); put(out, v.p, v.n); put16(out, (uint16_t)t.n); put(out, t.p, t.n); }
	free(v.p); free(t.p);
	return e;
}

static uint16_t w16(const uint8_t *blk, long o) { return (uint16_t)(blk[o] | blk[o + 1] << 8); }

// --------------------------------------------------------------- encoders --
static int enc_dialogue(const uint8_t *blk, const PackRegion *r, Buf *out) {
	long t = r->start, end = r->start + r->length;
	for (int i = 0; i < 16; i++) {
		long a = w16(blk, t + 2 * i) - 0x8000;
		long b = i < 15 ? w16(blk, t + 2 * i + 2) - 0x8000 : end;
		long h = a;
		while (h < b && blk[h] != 0xFF) h++;
		if (h >= b) return -1;
		put8(out, (uint8_t)(h - a));
		put(out, blk + a, (uint32_t)(h - a));
		put(out, blk + h + 1, (uint32_t)(b - h - 1));
	}
	return 0;
}

static int enc_map_graphics(const uint8_t *blk, const PackRegion *r, Buf *out) {
	int count = r->length / 128;
	for (int g = 0; g < count; g++) {
		uint8_t pix[192], masks[24];
		for (int row = 0; row < 12; row++)
			for (int col = 0; col < 2; col++) {
				const uint8_t *o = blk + g * 128 + col * 60 + row * 5;
				masks[row * 2 + col] = o[0];
				for (int x = 0; x < 8; x++) {
					uint8_t c = 0;
					for (int k = 0; k < 4; k++) c |= ((o[1 + k] >> (7 - x)) & 1) << k;
					pix[row * 16 + col * 8 + x] = c;
				}
			}
		for (int i = 0; i < 192; i += 2) put8(out, (uint8_t)(pix[i] << 4 | pix[i + 1]));
		put(out, masks, 24);
	}
	return 0;
}

static int cmp_u16(const void *a, const void *b) { return (int)*(const uint16_t *)a - (int)*(const uint16_t *)b; }

static int enc_world(const uint8_t *rom, Buf *out) {
#define RD(a) rom[((a) < 0x8000 ? 4 : 5) * BANK + ((a) & 0x3FFF)]
	static uint16_t ptrs[4096], order[4096];
	int k = 0;
	for (int cx = 0; cx < 16; cx++)
		for (int cy = 0; cy < 16; cy++)
			for (int row = 0; row < 16; row++) {
				uint32_t a = 0x4000 + cx * 0x200 + cy * 0x20 + 2 * row;
				ptrs[k++] = (uint16_t)(RD(a) | RD(a + 1) << 8);
			}
	memcpy(order, ptrs, sizeof ptrs);
	qsort(order, 4096, 2, cmp_u16);
	int n = 0;
	for (int i = 0; i < 4096; i++) if (i == 0 || order[i] != order[i - 1]) order[n++] = order[i];
	put16(out, (uint16_t)n);
	for (int i = 0; i < n; i++) {
		uint32_t p = order[i];
		uint8_t t[16];
		int x = 0;
		while (x < 16) {
			uint8_t b = RD(p); p++;
			if (b & 0x80) { uint8_t v = RD(p); p++; for (int j = 0; j < (b & 0x7F) && x < 16; j++) t[x++] = v; }
			else t[x++] = b;
		}
		put(out, t, 16);
	}
	for (int i = 0; i < 4096; i++) {
		uint16_t *f = bsearch(&ptrs[i], order, n, 2, cmp_u16);
		put16(out, (uint16_t)(f - order));
	}
	for (int i = 0; i < 256; i++) put8(out, RD(0x6000 + i));
#undef RD
	return 0;
}

static int enc_string_table(const uint8_t *blk, const PackRegion *r, uint16_t count, Buf *out) {
	long start = r->start + 2 * count, end = r->start + r->length;
	// the strings in storage order, and where each begins
	long *at = malloc(sizeof(long) * (r->length + 1));
	int n = 0;
	for (long a = start; a < end;) {
		at[n++] = a;
		while (a < end && blk[a] != 0) a++;
		a++;
	}
	put16(out, count);
	put16(out, (uint16_t)n);
	put(out, blk + start, (uint32_t)(end - start));
	int ok = 0;
	for (int i = 0; i < count; i++) {
		long p = w16(blk, r->start + 2 * i) - 0x8000;
		int found = -1;
		for (int s = 0; s < n; s++) if (at[s] == p) { found = s; break; }
		if (found < 0) { ok = -1; break; }
		put16(out, (uint16_t)found);
	}
	free(at);
	return ok;
}

static int enc_settlements(const uint8_t *blk, const PackRegion *r, Buf *out) {
	put16(out, w16(blk, r->start));
	for (int i = 1; i < 18; i++) {
		long o = w16(blk, r->start + 2 * i) - 0x8000;
		put(out, blk + o, 0x100);
		if (put_stream(out, blk, o + 0x100) < 0) return -1;
	}
	return 0;
}

static int enc_dungeon_bank(const uint8_t *blk, int nd, int extra, Buf *out) {
	put8(out, (uint8_t)nd);
	put8(out, (uint8_t)extra);
	for (int i = 0; i < nd * 17 + extra; i++) {
		int room = !(i < nd * 17 && i % 17 == 0);
		long tab = i < nd * 17 ? (i / 17) * 0x22 + 2 * (i % 17) : nd * 0x22 + 2 * (i - nd * 17);
		long o = w16(blk, tab) - 0x8000;
		if (room) { put(out, blk + o, 80); o += 80; }
		if (put_stream(out, blk, o) < 0) return -1;
	}
	return 0;
}

static int enc_arenas(const uint8_t *blk, const PackRegion *r, Buf *out) {
	put8(out, 24);
	for (int i = 0; i < 24; i++) {
		long o = w16(blk, r->start + 2 * i) - 0x8000;
		put(out, blk + o, 48);
		if (put_stream(out, blk, o + 48) < 0) return -1;
	}
	return 0;
}

static int enc_chunks(const uint8_t *blk, const PackRegion *r, const uint16_t *prm, Buf *out) {
	int n = prm[0];
	long a = r->start;
	put16(out, (uint16_t)n);
	for (int i = 0; i < n; i++) {
		int type = prm[1 + 3 * i], tag = prm[2 + 3 * i], len = prm[3 + 3 * i];
		put8(out, (uint8_t)type);
		put8(out, (uint8_t)tag);
		if (type == 0) { put16(out, (uint16_t)len); put(out, blk + a, len); a += len; }
		else if ((a = put_stream(out, blk, a)) < 0) return -1;
	}
	return 0;
}

// ------------------------------------------------------------------ import --
int import_rom(const char *rom_path, const char *pack_path) {
	FILE *f = fopen(rom_path, "rb");
	if (!f) { fprintf(stderr, "cannot open %s\n", rom_path); return -1; }
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);
	uint8_t *rom = malloc(size);
	size_t got = fread(rom, 1, size, f);
	fclose(f);
	char digest[41];
	sha1(rom, got, digest);
	if ((long)got != size || strcmp(digest, EXPECTED_SHA1)) {
		fprintf(stderr, "%s is not the expected ROM (SHA1 %s, expected %s: Ultima IV, Sega Master System)\n",
		        rom_path, digest, EXPECTED_SHA1);
		free(rom);
		return -1;
	}
	Buf layout = {0}, entries = {0};
	uint32_t nentries = 0;
	char seen[64][96];
	int nseen = 0, raw = 0;
	put32(&layout, (uint32_t)pack_region_count);
	for (int i = 0; i < pack_region_count; i++) {
		const PackRegion *r = &pack_regions[i];
		const uint8_t *blk = rom + r->bank * BANK;
		const uint8_t *orig = blk + r->start;
		const uint16_t *prm = pack_params + r->param;
		Buf e = {0};
		int ok = -1;
		const char *kind = r->kind, *name = r->name;
		if (!strcmp(kind, "dialogue")) ok = enc_dialogue(blk, r, &e);
		else if (!strcmp(kind, "map_graphics")) ok = enc_map_graphics(blk, r, &e);
		else if (!strcmp(kind, "world_map")) ok = enc_world(rom, &e);
		else if (!strcmp(kind, "string_table")) ok = enc_string_table(blk, r, prm[0], &e);
		else if (!strcmp(kind, "settlements")) ok = enc_settlements(blk, r, &e);
		else if (!strcmp(kind, "dungeon_bank")) ok = enc_dungeon_bank(blk, prm[0], prm[1], &e);
		else if (!strcmp(kind, "arenas")) ok = enc_arenas(blk, r, &e);
		else if (!strcmp(kind, "chunks")) ok = enc_chunks(blk, r, prm, &e);
		if (ok == 0) {   // check it comes back exactly
			uint8_t *back = calloc(1, r->length);
			ok = assets_rebuild(kind, e.p, e.n, r->bank, r->start, r->length, back) == 0 &&
			     !memcmp(back, orig, r->length) ? 0 : -1;
			free(back);
		}
		char rawname[96];
		if (ok != 0) {   // raw (planned, or a decoder that didn't fit)
			if (strcmp(kind, "raw")) fprintf(stderr, "import: %s stored raw\n", name);
			snprintf(rawname, sizeof rawname, "raw/bank%02d_%04X", r->bank, r->start);
			kind = "raw"; name = rawname;
			e.n = 0;
			put(&e, orig, r->length);
			raw += r->length;
		}
		int dup = 0;
		for (int s = 0; s < nseen; s++) if (!strcmp(seen[s], name)) dup = 1;
		if (!dup) {
			if (nseen < 64) snprintf(seen[nseen++], 96, "%s", name);
			put16(&entries, (uint16_t)strlen(name)); put(&entries, name, (uint32_t)strlen(name));
			put32(&entries, e.n); put(&entries, e.p, e.n);
			nentries++;
		}
		put8(&layout, r->bank); put16(&layout, r->start); put16(&layout, r->length);
		put16(&layout, (uint16_t)strlen(kind)); put(&layout, kind, (uint32_t)strlen(kind));
		put16(&layout, (uint16_t)strlen(name)); put(&layout, name, (uint32_t)strlen(name));
		free(e.p);
	}
	FILE *o = fopen(pack_path, "wb");
	if (!o) { fprintf(stderr, "cannot write %s\n", pack_path); return -1; }
	fwrite("U4PK", 1, 4, o);
	uint8_t cnt[4] = { (uint8_t)(nentries + 1), (uint8_t)((nentries + 1) >> 8), 0, 0 };
	fwrite(cnt, 1, 4, o);
	uint8_t hdr[2] = { 6, 0 };
	fwrite(hdr, 1, 2, o); fwrite("layout", 1, 6, o);
	uint8_t ln[4] = { (uint8_t)layout.n, (uint8_t)(layout.n >> 8), (uint8_t)(layout.n >> 16), (uint8_t)(layout.n >> 24) };
	fwrite(ln, 1, 4, o); fwrite(layout.p, 1, layout.n, o);
	fwrite(entries.p, 1, entries.n, o);
	fclose(o);
	printf("wrote %s (%u entries, %d bytes stored raw)\n", pack_path, nentries + 1, raw);
	free(rom); free(layout.p); free(entries.p);
	return 0;
}
