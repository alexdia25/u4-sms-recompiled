// Asset pack (tools/make_assets.py): the game's data, built once from the
// player's ROM. The exe runs from it alone.
#ifndef U4_ASSETS_H
#define U4_ASSETS_H

#include <stdint.h>

// Load the pack and build the memory image the original code reads.
// 0 on success; *image is malloc'd, *size its length.
int assets_load(const char *path, uint8_t **image, int *size);

// An entry of the loaded pack (for rewritten routines that read structured
// data directly), or NULL.
const uint8_t *asset(const char *name, uint32_t *size);

// Rebuild one region from its pack entry (used by the importer to check itself).
int assets_rebuild(const char *kind, const uint8_t *d, uint32_t n, int bank, uint16_t start, uint16_t length, uint8_t *out);

#endif
