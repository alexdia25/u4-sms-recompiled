#!/bin/sh
# Make a release folder/zip of the PC version: the exe and SDL2.dll only.
# No game data goes in; players give the exe their own ROM once.
#   pc/package.sh [version]   -> pc/release/u4-pc-<version>.zip
set -e
cd "$(dirname "$0")/.."
VER=${1:-$(git describe --always --dirty)}
[ -f pc/out/u4.exe ] || { echo "build first: pc/build.sh"; exit 1; }
DIR=pc/release/u4-pc-$VER
rm -rf "$DIR"
mkdir -p "$DIR"
cp pc/out/u4.exe pc/out/SDL2.dll "$DIR"/
cat > "$DIR/README.txt" <<'EOF'
Ultima IV: Quest of the Avatar - PC version rebuilt from the Sega Master System game

This contains no game data. You need your own copy of the Master System ROM
(SHA1 a90e21e5961bcf2e10b715a009c04e7c2017a3b1).

First run:   u4.exe path\to\your-rom.sms
  This checks the ROM, builds u4.pak next to the exe, and starts the game.
  The ROM is not needed after that.
Then:        u4.exe

Controls: arrows = D-pad, Z = button 1, X = button 2, Enter = Pause, Esc = quit.
A gamepad also works. Your save is kept in u4.sav.
EOF
( cd pc/release && rm -f "u4-pc-$VER.zip" && python -c "import shutil,sys; shutil.make_archive(sys.argv[1], 'zip', '.', sys.argv[1])" "u4-pc-$VER" )
echo "pc/release/u4-pc-$VER.zip"
