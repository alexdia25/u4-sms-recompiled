// Map loading, rewritten from the original routines in bank 0.
// Each function documents the Z80 routine it replaces; see src/banks/bank00.asm.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"

// RAM variables (tools/hints.txt)
#define PARTY_X        0xC953
#define PARTY_Y        0xC954
#define VIEW_X         0xC985
#define VIEW_Y         0xC986
#define VIEW_CHUNK_X   0xC987
#define VIEW_CHUNK_Y   0xC988
#define MAP_BUFFER     0xD000   // 64 x 32 cells of 2 bytes: four 16x16 chunks
#define MAPPER_SLOT1   0xFFFE   // writing here also updates the RAM shadow at $DFFE
#define BANK_SHADOW_1  0xDFFE   // slot 1 / slot 2 bank shadows ($DFFE, $DFFF)

static const uint16_t bank_switch_scratch[] = { 0xC0C2, 0xC0C3 };   // used by the bank-switch helpers

// ----------------------------------------------------------------------------
// decode_rle ($06FF): run-length decoder used for dungeons and other data.
//   in:  HL = source, DE = destination (both CPU addresses, current mapping)
//   out: HL = byte after the terminator, DE = end of the output, A = 0
// Codes: $00 ends; $01-$7F n = next byte repeated n times; $80+n = n literal bytes.
static uint16_t decode_rle(uint16_t src, uint16_t *dest) {
	uint16_t out = *dest;
	for (;;) {
		uint8_t code = mem_read(src++);
		if (code == 0) { cpu.cycles += 28; break; }   // the original's cycles, per code
		if (code & 0x80) {
			cpu.cycles += 55 + 21 * (code & 0x7F);
			for (int n = code & 0x7F; n > 0; n--) mem_write(out++, mem_read(src++));
		} else {
			cpu.cycles += 54 + 26 * code;
			uint8_t value = mem_read(src++);
			for (int n = code; n > 0; n--) mem_write(out++, value);
		}
	}
	*dest = out;
	return src;
}

static void nat_decode_rle(void) {
	uint16_t de = DE;
	uint16_t hl = decode_rle(HL, &de);
	SET_HL(hl);
	SET_DE(de);
	cpu.a = 0;
}
Native native_decode_rle = { "decode_rle", 0x06FF, 0, nat_decode_rle, OUT_A | OUT_DE | OUT_HL, 0, 0 };

// decode_rle_far ($06F6): the same with bank A paged into slot 2 for the call.
static void nat_decode_rle_far(void) {
	uint8_t saved = mem_read(0xDFFF);
	mem_write(0xFFFF, cpu.a);
	uint16_t de = DE;
	cpu.cycles += 223;                    // bank_push, the call, bank_pop
	uint16_t hl = decode_rle(HL, &de);
	mem_write(0xFFFF, saved);
	SET_HL(hl);
	SET_DE(de);
	cpu.a = 0;
}
Native native_decode_rle_far = { "decode_rle_far", 0x06F6, 0, nat_decode_rle_far,
	OUT_A | OUT_DE | OUT_HL, bank_switch_scratch, 2 };

// ----------------------------------------------------------------------------
// load_world_chunk ($0EE8): unpack one 16x16 chunk of the world map into the
// map buffer. Banks 4 and 5 are paged into slots 1 and 2 while it runs.
//   (cx, cy): chunk coordinates, 0-15 (higher bits are ignored)
//   dest: where the chunk's top-left cell goes in the map buffer; each cell is
//         2 bytes and rows are $40 bytes apart
// The chunk has 16 row pointers at $4000 + cx*$200 + cy*$20. A row byte below
// $80 is one tile; $80+n is followed by a tile repeated n times (a run that
// would pass the end of the row is cut off). Each tile id is turned into the
// 2-byte cell value through the table at $6000 (bank 4).
static void load_world_chunk(uint8_t cx, uint8_t cy, uint16_t dest) {
	uint16_t saved_banks = rd16(BANK_SHADOW_1);
	wr16(MAPPER_SLOT1, 0x0504);   // slot 1 = bank 4, slot 2 = bank 5
	uint16_t row_table = 0x4000 | ((cx & 0x0F) << 9) | ((uint8_t)(cy << 4) << 1);
	cpu.cycles += 135 + 36;                // the original's cycles: set-up and end,
	for (int row = 0; row < 16; row++) {
		uint16_t src = rd16(row_table + 2 * row);
		uint16_t out = dest + row * 0x40;
		cpu.cycles += 45 + (((out + 0x20) & 0xFF) + 0x20 > 0xFF ? 68 : 69);   // per row,
		int x = 0;
		while (x < 16) {
			uint8_t code = mem_read(src++);
			int count = 1;
			uint8_t tile = code;
			if (code & 0x80) {
				count = code & 0x7F;
				tile = mem_read(src++);
				int k = count < 16 - x ? count : 16 - x;   // and per run or tile
				cpu.cycles += k == 16 - x ? 42 + 71 * k : 60 + 71 * k;
			} else {
				cpu.cycles += 89;
			}
			for (; count > 0 && x < 16; count--, x++) {
				uint16_t cell = 0x6000 + (uint8_t)(tile * 2);
				mem_write(out++, mem_read(cell));
				mem_write(out++, mem_read(cell + 1));
			}
		}
	}
	wr16(MAPPER_SLOT1, saved_banks);
}

// recenter_map_view ($0E22): after the party moved to a new map position,
// work out which 2x2 chunks surround it and load them into the map buffer.
// The view window is 32x32 cells; the party sits at view_x/view_y (8-23)
// inside it, chunk view_chunk_x/view_chunk_y being the top-left one.
static void center_axis(uint8_t pos, uint16_t view, uint16_t chunk) {
	uint8_t c = (pos >> 4) & 0x0F;
	uint8_t v = pos & 0x0F;
	cpu.cycles += v < 8 ? 200 : 123;
	if (v < 8) {          // keep the party away from the window's edge
		v += 16;
		c = (c - 1) & 0x0F;
	}
	mem_write(chunk, c);
	mem_write(view, v);
}

static void nat_recenter_map_view(void) {
	center_axis(mem_read(PARTY_X), VIEW_X, VIEW_CHUNK_X);
	center_axis(mem_read(PARTY_Y), VIEW_Y, VIEW_CHUNK_Y);
	uint8_t cx = mem_read(VIEW_CHUNK_X), cy = mem_read(VIEW_CHUNK_Y);
	load_world_chunk(cx, cy, MAP_BUFFER);
	load_world_chunk(cx + 1, cy, MAP_BUFFER + 0x20);
	load_world_chunk(cx, cy + 1, MAP_BUFFER + 0x400);
	load_world_chunk(cx + 1, cy + 1, MAP_BUFFER + 0x420);
	cpu.cycles += 319;    // the four load_chunk_* calls
}
Native native_recenter_map_view = { "recenter_map_view", 0x0E22, 0, nat_recenter_map_view, 0, 0, 0 };

// ----------------------------------------------------------------------------
// Settlements (towns, villages, castles): location 1-17.
#define LOCATION          0xC9A0
#define LOCATION_PEOPLE   0xC651   // 256-byte block of people/objects
#define PARTY_SIZE        0xC957
#define PARTY             0xC751   // 29-byte member records
#define COMPANIONS_IN_TOWN 0xC749  // 8 flags, one per companion, cleared below

// The byte the game keeps next to each tile in the map buffer: from the
// "special tile" table in bank 16 ($8100 + tile). decode_location_map pages
// bank 16 in for each lookup and puts the previous bank back.
static uint8_t tile_attribute(uint8_t tile) {
	uint8_t saved = mem_read(0xFFFF);   // reads the RAM shadow ($DFFF)
	mem_write(0xFFFF, 0x10);
	uint8_t attr = mem_read(0x8100 + tile);
	mem_write(0xFFFF, saved);
	return attr;
}

// decode_location_map ($116B): unpack a 32x32 settlement map to `dest`,
// 2 bytes per cell (tile, attribute). $00 ends; $01-$7F n = next tile n
// times; $80+n = n literal tiles. Returns the address after the data.
// The cycle counts are the original's: per code, then per tile.
static uint16_t decode_location_map(uint16_t src, uint16_t dest) {
	cpu.cycles += 98;
	for (;;) {
		uint8_t code = mem_read(src++);
		if (code == 0) { cpu.cycles += 103; return src; }
		if (code & 0x80) {
			cpu.cycles += 52 + 119 * (code & 0x7F);
			for (int n = code & 0x7F; n > 0; n--) {
				uint8_t tile = mem_read(src++);
				mem_write(dest++, tile);
				mem_write(dest++, tile_attribute(tile));
			}
		} else {
			uint8_t tile = mem_read(src++);
			uint8_t attr = tile_attribute(tile);
			cpu.cycles += 136 + 47 * code;
			for (int n = code; n > 0; n--) {
				mem_write(dest++, tile);
				mem_write(dest++, attr);
			}
		}
	}
}

// mark_shorelines ($11C5): for every water cell (tile 0-2) of the 32x32 map at
// $D000, set a bit in its attribute for each neighbour that is land, so the
// renderer can draw the shore: bit 0 above, bit 1 below, bit 2 left, bit 3
// right. Neighbours outside the map, other water, bridges/docks (tiles $17,
// $19, $1A, $48) and the shore pieces that already face that way don't count.
static int is_water_like(uint8_t t) { return t == 0x17 || t == 0x19 || t == 0x1A || t == 0x48; }

// the original's cycles for checking one neighbour ($1243 and the others,
// with the call): the tests stop at the first that decides
static int neighbour_cycles(uint8_t t, uint8_t skip1, uint8_t skip2) {
	int c = 17 + 17;                         // the call, and its call to $1291
	if (t == 0x17) return c + 18 + 11;
	if (t == 0x19) return c + 30 + 11;
	if (t == 0x1A) return c + 42 + 11;
	c += 53;                                  // $1291 all the way
	if (t == 0x48) return c + 11;
	c += 5 + 7;
	if (t == skip1) return c + 11;
	c += 5 + 7;
	if (t == skip2) return c + 11;
	c += 5 + 7;
	if (t < 3) return c + 11;
	return c + 5 + 23 + 10;
}

static void mark_shorelines(void) {
	static const struct { int dx, dy; uint8_t skip1, skip2, bit; } side[4] = {
		{ 0, -1, 0x33, 0x34, 0x01 }, { 0, 1, 0x31, 0x32, 0x02 },
		{ -1, 0, 0x31, 0x33, 0x04 }, { 1, 0, 0x32, 0x34, 0x08 },
	};
	cpu.cycles += 86 + 54;
	for (int y = 0; y < 32; y++) {
		cpu.cycles += 4 + 4 + (y < 31 ? 12 : 7);
		for (int x = 0; x < 32; x++) {
			uint16_t cell = MAP_BUFFER + y * 0x40 + x * 2;
			cpu.cycles += 19 + 7 + 15 + (x < 31 ? 13 : 8);
			if (mem_read(cell) >= 3) { cpu.cycles += 12; continue; }
			cpu.cycles += 7 + 4 * (4 + 4) + 19 + 19;   // the tests, and the up/down reads
			for (int s = 0; s < 4; s++) {
				int nx = x + side[s].dx, ny = y + side[s].dy;
				if (nx < 0 || nx > 31 || ny < 0 || ny > 31) { cpu.cycles += 10; continue; }
				uint8_t t = mem_read(MAP_BUFFER + ny * 0x40 + nx * 2);
				cpu.cycles += neighbour_cycles(t, side[s].skip1, side[s].skip2) + (side[s].dx ? 19 : 0);
				if (is_water_like(t) || t == side[s].skip1 || t == side[s].skip2 || t < 3) continue;
				mem_write(cell + 1, mem_read(cell + 1) | side[s].bit);
			}
		}
	}
}

// load_location ($0E6C): set up the settlement in `location`. Its record in
// bank 15 (table at $8077) starts with the 256-byte people block, then the
// map. If a companion who lives in one of the eight towns (locations 6-13)
// is in the party and we are entering that town, the "companion is here"
// flags are cleared so they don't also appear among the townsfolk.
static void nat_load_location(void) {
	uint8_t saved = mem_read(0xDFFF);
	mem_write(0xFFFF, 0x0F);
	uint8_t loc = mem_read(LOCATION);
	uint16_t record = rd16(0x8077 + 2 * loc);
	for (int i = 0; i < 0x100; i++) mem_write(LOCATION_PEOPLE + i, mem_read(record + i));
	cpu.cycles += 5658;                       // bank_push, the lookup, the copy
	decode_location_map(record + 0x100, MAP_BUFFER);
	cpu.cycles += 17;
	mark_shorelines();
	uint8_t size = mem_read(PARTY_SIZE);
	uint8_t town = loc - 6;
	cpu.cycles += 13 + 4;
	if (size <= 1) cpu.cycles += 12;
	else if (town >= 8) cpu.cycles += 7 + 4 + 13 + 7 + 7 + 12;
	else {
		cpu.cycles += 7 + 4 + 13 + 7 + 7 + 7 + 10 + 10;
		int m = 1;
		for (; m < size; m++) {
			if (mem_read(PARTY + m * 29 + 0x1B) == town) {   // companion's home town
				cpu.cycles += 7 + 12 + 10 + 10 + 17 + 213;
				for (int i = 0; i < 8; i++) mem_write(COMPANIONS_IN_TOWN + i, 0);
				break;
			}
			cpu.cycles += 7 + 7 + 11 + (m < size - 1 ? 13 : 8);
		}
		if (m == size) cpu.cycles += 12;
	}
	mem_write(0xFFFF, saved);
	cpu.cycles += 10 + 77;                    // jp bank_pop
}
Native native_load_location = { "load_location", 0x0E6C, 0, nat_load_location, 0, bank_switch_scratch, 2 };

// ----------------------------------------------------------------------------
// Walking on the world map. The map buffer holds a 32x32-cell window made of
// four 16x16 chunks; view_x/view_y is the party's cell inside it. When the
// party comes within 5 cells of an edge the window slides by a whole chunk:
// the half that stays is copied across and the two new chunks are loaded.

// shift_map_buffer_x ($0FE3): copy the 16-cell-wide half at `from` (32 rows)
// to `to`.
static void copy_half_columns(uint16_t from, uint16_t to) {
	for (int row = 0; row < 32; row++)
		for (int i = 0; i < 0x20; i++)
			mem_write(to + row * 0x40 + i, mem_read(from + row * 0x40 + i));
	cpu.cycles += 7 + 32 * 566 + 10;
}

// shift_map_buffer_y ($1031): copy the 16-row half at `from` to `to`.
static void copy_half_rows(uint16_t from, uint16_t to) {
	for (int i = 0; i < 0x400; i++) mem_write(to + i, mem_read(from + i));
	cpu.cycles += 10 + 32 * 522 + 10;
}

// Move one cell along an axis. Returns 1 when the window had to slide; the
// chunk number has then been updated and the caller loads the new chunks.
// load_cycles: what the original's two load_chunk_* calls take besides the
// chunks themselves.
static int step_axis(uint16_t pos, uint16_t view, uint16_t chunk, int dir, int load_cycles) {
	mem_write(pos, mem_read(pos) + dir);
	uint8_t v = mem_read(view) + dir;
	mem_write(view, v);
	if (dir > 0 ? v < 0x1B : v >= 0x05) { cpu.cycles += 67; return 0; }
	cpu.cycles += 180 + load_cycles;
	mem_write(view, v - 16 * dir);
	mem_write(chunk, (mem_read(chunk) + dir) & 0x0F);
	return 1;
}

static void nat_step_east(void) {     // step_east ($0F4B)
	if (step_axis(PARTY_X, VIEW_X, VIEW_CHUNK_X, 1, 155)) {
		uint8_t cx = mem_read(VIEW_CHUNK_X), cy = mem_read(VIEW_CHUNK_Y);
		copy_half_columns(MAP_BUFFER + 0x20, MAP_BUFFER);
		load_world_chunk(cx + 1, cy, MAP_BUFFER + 0x20);
		load_world_chunk(cx + 1, cy + 1, MAP_BUFFER + 0x420);
	}
}

static void nat_step_west(void) {     // step_west ($0F71)
	if (step_axis(PARTY_X, VIEW_X, VIEW_CHUNK_X, -1, 157)) {
		uint8_t cx = mem_read(VIEW_CHUNK_X), cy = mem_read(VIEW_CHUNK_Y);
		copy_half_columns(MAP_BUFFER, MAP_BUFFER + 0x20);
		load_world_chunk(cx, cy, MAP_BUFFER);
		load_world_chunk(cx, cy + 1, MAP_BUFFER + 0x400);
	}
}

static void nat_step_south(void) {    // step_south ($0F97)
	if (step_axis(PARTY_Y, VIEW_Y, VIEW_CHUNK_Y, 1, 155)) {
		uint8_t cx = mem_read(VIEW_CHUNK_X), cy = mem_read(VIEW_CHUNK_Y);
		copy_half_rows(MAP_BUFFER + 0x400, MAP_BUFFER);
		load_world_chunk(cx, cy + 1, MAP_BUFFER + 0x400);
		load_world_chunk(cx + 1, cy + 1, MAP_BUFFER + 0x420);
	}
}

static void nat_step_north(void) {    // step_north ($0FBD)
	if (step_axis(PARTY_Y, VIEW_Y, VIEW_CHUNK_Y, -1, 157)) {
		uint8_t cx = mem_read(VIEW_CHUNK_X), cy = mem_read(VIEW_CHUNK_Y);
		copy_half_rows(MAP_BUFFER, MAP_BUFFER + 0x400);
		load_world_chunk(cx, cy, MAP_BUFFER);
		load_world_chunk(cx + 1, cy, MAP_BUFFER + 0x20);
	}
}

Native native_step_east = { "step_east", 0x0F4B, 0, nat_step_east, 0, 0, 0 };
Native native_step_west = { "step_west", 0x0F71, 0, nat_step_west, 0, 0, 0 };
Native native_step_south = { "step_south", 0x0F97, 0, nat_step_south, 0, 0, 0 };
Native native_step_north = { "step_north", 0x0FBD, 0, nat_step_north, 0, 0, 0 };

// the two steps of load_location on their own (also called from elsewhere?)
static void nat_decode_location_map(void) {   // $116B: HL = data, DE = map; keeps all registers
	decode_location_map(HL, DE);
}
Native native_decode_location_map = { "decode_location_map", 0x116B, 0, nat_decode_location_map, 0, 0, 0 };

static void nat_mark_shorelines(void) { mark_shorelines(); }   // $11C5, keeps all registers
Native native_mark_shorelines = { "mark_shorelines", 0x11C5, 0, nat_mark_shorelines, 0, 0, 0 };
