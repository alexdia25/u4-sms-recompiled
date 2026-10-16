#!/bin/sh
# Shadow-test the rewritten routines (pc/native): run the PC build from every
# seed save state with random input, comparing each rewritten routine with the
# original on every call.  pc/shadowtest.sh [frames-per-seed]
cd "$(dirname "$0")/.."
FRAMES=${1:-3000}
EXE=pc/out/u4_headless.exe
[ -f "$EXE" ] || pc/build.sh headless
for s in explore/bin/*.bin; do
  echo "== $(basename $s .bin)"
  U4_NATIVE=shadow U4_RANDOM=7 "$EXE" build/u4.pak --state "$s" --headless "$FRAMES" 2>&1 | grep "shadow:"
done
