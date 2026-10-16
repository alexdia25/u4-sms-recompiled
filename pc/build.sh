#!/bin/sh
# Build the PC version: regenerate the recompiled C, then compile with Zig's clang.
#   pc/build.sh            -> pc/out/u4.exe (+ SDL2.dll, u4.sms copied next to it)
#   pc/build.sh headless   -> pc/out/u4_headless.exe (no SDL, for tests)
# The generated recomp.c is large (one function, ~100k lines) and takes a few
# minutes to compile; its object file is reused while the C is unchanged.
set -e
cd "$(dirname "$0")/.."
# Zig: $ZIG, else $TOOLS/zig/zig-*/zig, else zig on PATH.
# SDL2 (MinGW development package): $SDL, else $TOOLS/sdl2/SDL2-*/x86_64-w64-mingw32.
TOOLS=${TOOLS:-../tools}
if [ -z "$ZIG" ]; then
  ZIG=$(ls -d $TOOLS/zig/zig-*/ 2>/dev/null | head -1)zig
  [ -x "$ZIG" ] || [ -x "$ZIG.exe" ] || ZIG=zig
fi
if [ -z "$SDL" ]; then
  SDL=$(ls -d $TOOLS/sdl2/SDL2-*/ 2>/dev/null | head -1)x86_64-w64-mingw32
fi
ROM=${ROM:-build/u4.sms}
[ -f "$ROM" ] || ./build.sh
python tools/recomp.py "$ROM" coverage/cov.bin pc/gen/recomp.c
mkdir -p pc/out
python tools/make_assets.py "$ROM" pc/out/u4.pak --layout-header pc/gen/pack_layout.c   # region list for the exe's importer
CFLAGS="-O1 -std=gnu11 -Wno-unused-label -Wno-gnu-label-as-value -target x86_64-windows-gnu"
if [ ! -f pc/out/recomp.o ] || [ pc/gen/recomp.c -nt pc/out/recomp.o ] || [ pc/runtime/z80.h -nt pc/out/recomp.o ]; then
  echo "compiling pc/gen/recomp.c (a few minutes)..."
  "$ZIG" cc $CFLAGS -c pc/gen/recomp.c -o pc/out/recomp.o
fi
SRC="pc/runtime/main.c pc/runtime/sms.c pc/runtime/z80.c pc/runtime/native.c pc/runtime/assets.c pc/runtime/import.c pc/gen/pack_layout.c $(ls pc/native/*.c) pc/out/recomp.o"
if [ "$1" = headless ]; then
  "$ZIG" cc $CFLAGS -DU4_HEADLESS_ONLY $SRC -o pc/out/u4_headless.exe
else
  "$ZIG" cc $CFLAGS -I"$SDL/include/SDL2" $SRC "$SDL/lib/libSDL2.dll.a" -o pc/out/u4.exe
  cp "$SDL/bin/SDL2.dll" pc/out/
  rm -f pc/out/u4.sms
fi
echo "built pc/out"
