// Asset pack loader: builds the memory image the original code reads from
// the entries written by tools/make_assets.py, so the game never needs the
// ROM file. Each region of each bank is rebuilt by the rule of its kind
// (mirroring tools/pack_formats.py); "raw" regions are copied as they are.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assets.h"

typedef struct { char *name; uint8_t *data; uint32_t size; } Entry;
static Entry *entries;
static uint32_t nentries;

static const Entry *find(const char *name) {
	for (uint32_t i = 0; i < nentries; i++)
		if (!strcmp(entries[i].name, name)) return &entries[i];
	return NULL;
}

const uint8_t *asset(const char *name, uint32_t *size) {
	const Entry *e = find(name);
	if (!e) return NULL;
	if (size) *size = e->size;
	return e->data;
}

static uint16_t u16(const uint8_t *p) { return (uint16_t)(p[0] | p[1] << 8); }
static uint32_t u32(const uint8_t *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }

// dialogue: 16 records of (header length, header, 11 zero-terminated
// strings); rebuilt as 16 pointers followed by the records with the $FF
// header terminator put back.
static int rebuild_dialogue(const uint8_t *d, uint32_t n, uint8_t *out, uint16_t start, uint16_t length) {
	uint32_t p = 0;
	uint16_t addr = 0x8000 + start + 0x20;
	uint8_t *rec = out + 0x20;
	for (int i = 0; i < 16; i++) {
		if (p >= n) return -1;
		uint8_t hl = d[p++];
		uint8_t *begin = rec;
		memcpy(rec, d + p, hl); rec += hl; p += hl;
		*rec++ = 0xFF;
		for (int s = 0; s < 11; s++) {
			do { if (p >= n) return -1; *rec = d[p++]; } while (*rec++ != 0);
		}
		out[2 * i] = (uint8_t)addr;
		out[2 * i + 1] = (uint8_t)(addr >> 8);
		addr += (uint16_t)(rec - begin);
	}
	return rec - out == length ? 0 : -1;
}

// map_graphics: graphics of 16x12 pixels (128 per full bank), each 96 bytes of pixels (two
// per byte, high nibble first) and 24 mask bytes; rebuilt as the ROM keeps
// them: two 8-pixel columns of 12 rows of (mask, 4 bit planes), 128 bytes.
static int rebuild_map_graphics(const uint8_t *d, uint32_t n, uint8_t *out, uint16_t length) {
	int count = (int)(n / 120);
	if (n != (uint32_t)count * 120 || length != count * 128) return -1;
	memset(out, 0, length);
	for (int g = 0; g < count; g++) {
		const uint8_t *src = d + g * 120;
		for (int r = 0; r < 12; r++) {
			for (int col = 0; col < 2; col++) {
				uint8_t *o = out + g * 128 + col * 60 + r * 5;
				o[0] = src[96 + r * 2 + col];
				for (int x = 0; x < 8; x++) {
					int i = r * 16 + col * 8 + x;
					uint8_t c = (i & 1) ? src[i / 2] & 15 : src[i / 2] >> 4;
					for (int k = 0; k < 4; k++)
						if (c & (1 << k)) o[1 + k] |= (uint8_t)(0x80 >> x);
				}
			}
		}
	}
	return 0;
}

// world_map: u16 row count, the distinct 16-tile rows, 4096 u16 row numbers
// (chunk by chunk), the 256-byte cell table. Rebuilt as the ROM lays it out
// from 4:$4000: pointer tables, cell table at $6000, then the rows run-length
// coded (runs of 3 or more as $80+n, tile) from $6100 on, into bank 5.
enum { WORLD_END = 0x4000 + 0x24DF };

static int rebuild_world(const uint8_t *d, uint32_t n, uint8_t *full) {
	if (n < 2) return -1;
	uint32_t rows = u16(d);
	if (n != 2 + 16 * rows + 8192 + 256) return -1;
	uint16_t *where = malloc(rows * sizeof *where);
	uint32_t addr = 0x6100;
	for (uint32_t r = 0; r < rows; r++) {
		const uint8_t *t = d + 2 + 16 * r;
		where[r] = (uint16_t)addr;
		for (int i = 0; i < 16;) {
			int j = i;
			while (j < 16 && t[j] == t[i]) j++;
			int len = j - i;
			if (addr + 2 > 0x4000 + WORLD_END) { free(where); return -1; }
			if (len >= 3) { full[addr++ - 0x4000] = (uint8_t)(0x80 | len); full[addr++ - 0x4000] = t[i]; }
			else for (int k = 0; k < len; k++) full[addr++ - 0x4000] = t[i];
			i = j;
		}
	}
	const uint8_t *idx = d + 2 + 16 * rows;
	for (int k = 0; k < 4096; k++) {
		uint16_t w = where[u16(idx + 2 * k)];
		full[2 * k] = (uint8_t)w;
		full[2 * k + 1] = (uint8_t)(w >> 8);
	}
	memcpy(full + 0x2000, idx + 8192, 256);
	free(where);
	return 0;
}

// string_table: u16 pointer count, u16 string count, the strings (each
// zero-terminated) in storage order, then per pointer the string's number.
// Rebuilt as the pointer table followed by the strings.
static int rebuild_string_table(const uint8_t *d, uint32_t n, uint8_t *out, uint16_t start, uint16_t length) {
	if (n < 4) return -1;
	uint32_t count = u16(d), nstr = u16(d + 2), p = 4;
	uint16_t *where = malloc((nstr ? nstr : 1) * sizeof *where);
	uint32_t pool = 2 * count;   // where the strings go, from the region start
	for (uint32_t s = 0; s < nstr; s++) {
		where[s] = (uint16_t)(0x8000 + start + pool);
		do {
			if (p >= n || pool >= length) { free(where); return -1; }
			out[pool++] = d[p];
		} while (d[p++] != 0);
	}
	for (uint32_t i = 0; i < count; i++) {
		if (p + 2 > n) { free(where); return -1; }
		uint16_t s = u16(d + p); p += 2;
		if (s >= nstr) { free(where); return -1; }
		out[2 * i] = (uint8_t)where[s];
		out[2 * i + 1] = (uint8_t)(where[s] >> 8);
	}
	free(where);
	return pool == length ? 0 : -1;
}

// Re-encode run-length data from its values and token layout (see
// tools/pack_formats.py, rle_join): a token n < $80 repeats the next value
// n times, $80+n copies n values; $00 ends. Returns bytes written or -1.
static long rle_join(const uint8_t *values, uint32_t nvalues, const uint8_t *tokens, uint32_t ntokens,
                     uint8_t *out, long room) {
	long o = 0;
	uint32_t p = 0;
	for (uint32_t i = 0; i < ntokens; i++) {
		uint8_t t = tokens[i];
		uint32_t len = t & 0x7F;
		if (t & 0x80) {
			if (o + 1 + len > room || p + len > nvalues) return -1;
			out[o++] = t;
			memcpy(out + o, values + p, len); o += len; p += len;
		} else {
			if (o + 2 > room || p >= nvalues) return -1;
			out[o++] = t;
			out[o++] = values[p];
			p += len;
		}
	}
	if (o + 1 > room) return -1;
	out[o++] = 0;
	return o;
}

// settlements: u16 table entry 0, then 17 records of (256 people bytes,
// u16 n, n map tiles, u16 k, k tokens); rebuilt as the 18-word table at
// $8077 followed by the records (people, run-length coded map).
static int rebuild_settlements(const uint8_t *d, uint32_t n, uint8_t *out, uint16_t start, uint16_t length) {
	if (n < 2 || length < 36) return -1;
	out[0] = d[0]; out[1] = d[1];
	uint32_t p = 2;
	long o = 36;
	for (int r = 0; r < 17; r++) {
		uint16_t addr = (uint16_t)(0x8000 + start + o);
		out[2 + 2 * r] = (uint8_t)addr;
		out[3 + 2 * r] = (uint8_t)(addr >> 8);
		if (p + 0x100 + 2 > n || o + 0x100 > length) return -1;
		memcpy(out + o, d + p, 0x100); o += 0x100; p += 0x100;
		uint32_t nt = u16(d + p); p += 2;
		const uint8_t *tiles = d + p; p += nt;
		if (p + 2 > n) return -1;
		uint32_t nk = u16(d + p); p += 2;
		if (p + nk > n) return -1;
		long w = rle_join(tiles, nt, d + p, nk, out + o, length - o);
		if (w < 0) return -1;
		o += w; p += nk;
	}
	return o == length ? 0 : -1;
}

// dungeon_bank: u8 dungeons, u8 extra rooms, then per record (the dungeon,
// its 16 rooms, ..., then the extra rooms): [80 room bytes], u16 n, values,
// u16 k, tokens. Rebuilt as the pointer tables followed by the records.
static int rebuild_dungeon_bank(const uint8_t *d, uint32_t n, uint8_t *out, uint16_t length) {
	if (n < 2) return -1;
	int nd = d[0], extra = d[1], nrec = nd * 17 + extra;
	uint32_t p = 2;
	long o = 2 * nrec;
	for (int i = 0; i < nrec; i++) {
		int room = !(i < nd * 17 && i % 17 == 0);
		uint16_t addr = (uint16_t)(0x8000 + o);
		out[2 * i] = (uint8_t)addr;
		out[2 * i + 1] = (uint8_t)(addr >> 8);
		if (room) {
			if (p + 80 > n || o + 80 > length) return -1;
			memcpy(out + o, d + p, 80); o += 80; p += 80;
		}
		if (p + 2 > n) return -1;
		uint32_t nv = u16(d + p); p += 2;
		const uint8_t *values = d + p; p += nv;
		if (p + 2 > n) return -1;
		uint32_t nk = u16(d + p); p += 2;
		if (p + nk > n) return -1;
		long w = rle_join(values, nv, d + p, nk, out + o, length - o);
		if (w < 0) return -1;
		o += w; p += nk;
	}
	return o == length ? 0 : -1;
}

// arenas: u8 count, per record 48 bytes of places, u16 n, values, u16 k,
// tokens; rebuilt as the pointer table followed by the records.
static int rebuild_arenas(const uint8_t *d, uint32_t n, uint8_t *out, uint16_t start, uint16_t length) {
	if (n < 1) return -1;
	int count = d[0];
	uint32_t p = 1;
	long o = 2 * count;
	for (int i = 0; i < count; i++) {
		uint16_t addr = (uint16_t)(0x8000 + start + o);
		out[2 * i] = (uint8_t)addr;
		out[2 * i + 1] = (uint8_t)(addr >> 8);
		if (p + 48 + 2 > n || o + 48 > length) return -1;
		memcpy(out + o, d + p, 48); o += 48; p += 48;
		uint32_t nv = u16(d + p); p += 2;
		const uint8_t *values = d + p; p += nv;
		if (p + 2 > n) return -1;
		uint32_t nk = u16(d + p); p += 2;
		if (p + nk > n) return -1;
		long w = rle_join(values, nv, d + p, nk, out + o, length - o);
		if (w < 0) return -1;
		o += w; p += nk;
	}
	return o == length ? 0 : -1;
}

// chunks (picture banks): u16 count, then per chunk a type byte and a tag
// byte; type 0: u16 length + raw bytes; type 1: u16 n, values, u16 k,
// tokens (a run-length coded stream). Rebuilt by concatenation.
static int rebuild_chunks(const uint8_t *d, uint32_t n, uint8_t *out, uint16_t length) {
	if (n < 2) return -1;
	uint32_t count = u16(d), p = 2;
	long o = 0;
	for (uint32_t i = 0; i < count; i++) {
		if (p + 4 > n) return -1;
		uint8_t type = d[p];
		p += 2;
		uint32_t len = u16(d + p); p += 2;
		if (p + len > n) return -1;
		if (type == 0) {
			if (o + len > length) return -1;
			memcpy(out + o, d + p, len); o += len; p += len;
		} else {
			const uint8_t *values = d + p; p += len;
			if (p + 2 > n) return -1;
			uint32_t nk = u16(d + p); p += 2;
			if (p + nk > n) return -1;
			long w = rle_join(values, len, d + p, nk, out + o, length - o);
			if (w < 0) return -1;
			o += w; p += nk;
		}
	}
	return o == length ? 0 : -1;
}

// Rebuild one region of kind `kind` from its entry into out[0..length).
int assets_rebuild(const char *kind, const uint8_t *d, uint32_t n, int bank, uint16_t start, uint16_t length, uint8_t *out) {
	int ok = -1;
	if (!strcmp(kind, "raw")) { if (n == length) { memcpy(out, d, n); ok = 0; } }
	else if (!strcmp(kind, "dialogue")) ok = rebuild_dialogue(d, n, out, start, length);
	else if (!strcmp(kind, "map_graphics")) ok = rebuild_map_graphics(d, n, out, length);
	else if (!strcmp(kind, "string_table")) ok = rebuild_string_table(d, n, out, start, length);
	else if (!strcmp(kind, "settlements")) ok = rebuild_settlements(d, n, out, start, length);
	else if (!strcmp(kind, "dungeon_bank")) ok = rebuild_dungeon_bank(d, n, out, length);
	else if (!strcmp(kind, "arenas")) ok = rebuild_arenas(d, n, out, start, length);
	else if (!strcmp(kind, "chunks")) ok = rebuild_chunks(d, n, out, length);
	else if (!strcmp(kind, "world_map")) {
		static uint8_t *world;   // both banks' worth, built once
		if (!world) { world = calloc(1, WORLD_END); if (rebuild_world(d, n, world)) { free(world); world = NULL; } }
		if (world && (bank == 4 || bank == 5) && start == 0) {
			uint32_t from = bank == 4 ? 0 : 0x4000;
			if (from + length <= WORLD_END) { memcpy(out, world + from, length); ok = 0; }
		}
	}
	return ok;
}

int assets_load(const char *path, uint8_t **image, int *size) {
	FILE *f = fopen(path, "rb");
	if (!f) return -1;
	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	fseek(f, 0, SEEK_SET);
	uint8_t *buf = malloc(len);
	if (fread(buf, 1, len, f) != (size_t)len || len < 8 || memcmp(buf, "U4PK", 4)) { fclose(f); return -1; }
	fclose(f);
	nentries = u32(buf + 4);
	entries = calloc(nentries, sizeof *entries);
	uint8_t *p = buf + 8;
	for (uint32_t i = 0; i < nentries; i++) {
		uint16_t nl = u16(p); p += 2;
		entries[i].name = malloc(nl + 1);
		memcpy(entries[i].name, p, nl); entries[i].name[nl] = 0; p += nl;
		entries[i].size = u32(p); p += 4;
		entries[i].data = p; p += entries[i].size;
	}
	uint32_t lsize;
	const uint8_t *lay = asset("layout", &lsize);
	if (!lay) return -1;
	uint32_t nreg = u32(lay);
	const uint8_t *q = lay + 4;
	int banks = 0;
	// first pass: how big is the image
	for (uint32_t r = 0, o = 0; r < nreg; r++) {
		const uint8_t *e = q + o;
		if (e[0] + 1 > banks) banks = e[0] + 1;
		uint16_t kl = u16(e + 5), nl = u16(e + 7 + kl);
		o += 9 + kl + nl;
	}
	uint8_t *img = calloc(banks, 0x4000);
	for (uint32_t r = 0; r < nreg; r++) {
		uint8_t bank = q[0];
		uint16_t start = u16(q + 1), length = u16(q + 3), kl = u16(q + 5);
		char kind[32] = {0}, name[96] = {0};
		memcpy(kind, q + 7, kl < 31 ? kl : 31);
		uint16_t nl = u16(q + 7 + kl);
		memcpy(name, q + 9 + kl, nl < 95 ? nl : 95);
		q += 9 + kl + nl;
		uint32_t n;
		const uint8_t *d = asset(name, &n);
		uint8_t *out = img + bank * 0x4000 + start;
		int ok = d ? assets_rebuild(kind, d, n, bank, start, length, out) : -1;
		if (ok) { fprintf(stderr, "assets: cannot rebuild %s (%s)\n", name, kind); return -1; }
	}
	*image = img;
	*size = banks * 0x4000;
	return 0;
}
