#!/bin/sh
# Rebuild the Ghidra project (ghidra/u4sms.gpr) from the ROM, the WLA-DX symbols
# and the emulator coverage, then write Ghidra's decompilation to decomp/.
set -e
cd "$(dirname "$0")"
GHIDRA=${GHIDRA:-../tools/ghidra_12.1.3_PUBLIC}
HERE=$(pwd -W 2>/dev/null || pwd)
mkdir -p ghidra build
head -c 16384 rom/u4.sms > build/bank00.bin
rm -rf ghidra/u4sms.gpr ghidra/u4sms.rep
"$GHIDRA/support/analyzeHeadless.bat" "$HERE/ghidra" u4sms -import "$HERE/build/bank00.bin" \
  -loader BinaryLoader -processor z80:LE:16:default -noanalysis \
  -scriptPath "$HERE/tools/ghidra" \
  -postScript U4Setup.java "$HERE/rom/u4.sms" "$HERE/build/u4.sym" "$HERE/src/xrefs.txt" "$HERE/coverage/cov.bin" \
  -postScript U4Decompile.java "$HERE/decomp" 2>&1 | grep -E "U4|ERROR|Exception|functions|seeds" | grep -v "^INFO  SCRIPT"
