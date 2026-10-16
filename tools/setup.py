"""Set up a working copy from your own ROM. Nothing from the game is in this
repository; everything that needs the game's bytes is made here.

    python tools/setup.py path/to/rom.sms [--pc]

1. checks the ROM is the expected release (SHA1)
2. splits it into banks for the assembler (build/rom/bankNN.bin; the
   source's .INCBIN lines read them)
3. assembles src/ and checks the result is identical to your ROM
4. extracts the game data to build/data (text, maps, tables, images)
5. builds the asset pack the PC version runs from (build/u4.pak)
6. with --pc: builds the PC version (pc/out/u4.exe), see pc/build.sh
"""
import hashlib
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXPECTED_SHA1 = 'a90e21e5961bcf2e10b715a009c04e7c2017a3b1'   # Ultima IV (Europe/USA), SMS
BANK = 0x4000


def run(*cmd):
    print('>', ' '.join(cmd))
    subprocess.run(cmd, cwd=ROOT, check=True)


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    rom_path = os.path.abspath(sys.argv[1])
    rom = open(rom_path, 'rb').read()
    sha1 = hashlib.sha1(rom).hexdigest()
    if sha1 != EXPECTED_SHA1:
        sys.exit('This ROM has SHA1 %s; the project expects %s (Ultima IV: Quest of the Avatar, '
                 'Sega Master System).' % (sha1, EXPECTED_SHA1))
    build = os.path.join(ROOT, 'build')
    os.makedirs(os.path.join(build, 'rom'), exist_ok=True)
    for b in range(len(rom) // BANK):
        with open(os.path.join(build, 'rom', 'bank%02d.bin' % b), 'wb') as f:
            f.write(rom[b * BANK:(b + 1) * BANK])
    with open(os.path.join(build, 'original.sms'), 'wb') as f:
        f.write(rom)
    run('sh', 'build.sh')
    py = sys.executable
    run(py, 'tools/extract_text.py', 'build/original.sms', 'build/data')
    run(py, 'tools/extract_maps.py', 'build/original.sms', 'build/data')
    run(py, 'tools/extract_stats.py', 'build/original.sms', 'build/data')
    run(py, 'tools/extract_pictures.py', 'build/original.sms', 'build/data')
    run(py, 'tools/make_assets.py', 'build/original.sms', 'build/u4.pak')
    if '--pc' in sys.argv:
        run('sh', 'pc/build.sh')
    print('done: build/u4.sms (rebuilt from source), build/data/, build/u4.pak')


if __name__ == '__main__':
    main()
