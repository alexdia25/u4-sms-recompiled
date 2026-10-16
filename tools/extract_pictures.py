"""Write the full-screen pictures and tile sets as PNGs (build/data/pictures).

    python extract_pictures.py <rom> <outdir>

A picture is a 16-colour palette, 4 bit planes (each run-length coded; the
k-th byte of every tile row comes from plane k) and a name table in two
coded layers, tile numbers then attributes (see tools/pack_formats.py). The
intro scenes are 22x15 tiles (copied to the screen 22 tiles per row by
$937A). Tile sets have no palette or layout; they are drawn as a sheet with
the map palette.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pack_formats import BANK, PICTURE_BANKS, _picture_chunks  # noqa: E402
from extract_maps import rom_palette  # noqa: E402


def rgb(c):
    return ((c & 3) * 85, ((c >> 2) & 3) * 85, ((c >> 4) & 3) * 85)


def tile_pixel(planes, tile, y, x):
    i = tile * 8 + y
    if i >= len(planes[0]):
        return 0
    return sum(((planes[k][i] >> (7 - x)) & 1) << k for k in range(4))


def main():
    from PIL import Image
    rom = open(sys.argv[1], 'rb').read()
    out = os.path.join(sys.argv[2], 'pictures')
    os.makedirs(out, exist_ok=True)
    map_pal = rom_palette(rom)
    count = 0
    for bank, start in PICTURE_BANKS.items():
        blk = rom[bank * BANK:(bank + 1) * BANK]
        chunks, _ = _picture_chunks(blk, start)
        i = 0
        while i < len(chunks):
            kind, x, tag = chunks[i]
            if tag == 1 and kind == 0:   # palette, 4 planes, 2 name layers
                pal = [rgb(c) for c in x]
                planes = [chunks[i + 1 + k][1][0] for k in range(4)]
                names = chunks[i + 5][1][0]
                attrs = chunks[i + 6][1][0]
                w = 22 if len(names) == 330 else 32
                h = (len(names) + w - 1) // w
                im = Image.new('RGB', (w * 8, h * 8))
                for n, t in enumerate(names):
                    tile = t | ((attrs[n] & 1) << 8)
                    for yy in range(8):
                        for xx in range(8):
                            im.putpixel(((n % w) * 8 + xx, (n // w) * 8 + yy), pal[tile_pixel(planes, tile, yy, xx)])
                im.save(os.path.join(out, 'picture_b%02d_%02d.png' % (bank, i)))
                count += 1
                i += 7
            elif tag == 2:   # tile set
                planes = [chunks[i + k][1][0] for k in range(4)]
                n = len(planes[0]) // 8
                cols = 16
                im = Image.new('RGB', (cols * 8, ((n + cols - 1) // cols) * 8))
                for t in range(n):
                    for yy in range(8):
                        for xx in range(8):
                            im.putpixel(((t % cols) * 8 + xx, (t // cols) * 8 + yy), map_pal[tile_pixel(planes, t, yy, xx)])
                im.save(os.path.join(out, 'tiles_b%02d_%02d.png' % (bank, i)))
                count += 1
                i += 4
            else:
                i += 1
    print('%d images' % count)


if __name__ == '__main__':
    main()
