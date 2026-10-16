// Building the asset pack from the player's ROM inside the exe, so a release
// needs nothing but the exe and the ROM. Mirrors tools/make_assets.py; the
// region list (offsets, kinds, counts) is generated into pc/gen/pack_layout.c.
#ifndef U4_IMPORT_H
#define U4_IMPORT_H

#include <stdint.h>

typedef struct PackRegion {
	uint8_t bank;
	uint16_t start, length;
	const char *kind, *name;
	uint16_t param, nparams;   // into pack_params
} PackRegion;

extern const uint16_t pack_params[];
extern const PackRegion pack_regions[];
extern const int pack_region_count;

// Check the ROM and write the pack. 0 on success; messages go to stderr.
int import_rom(const char *rom_path, const char *pack_path);

#endif
