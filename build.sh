#!/bin/sh
# Rebuild u4.sms from the disassembly and check it against the original ROM.
set -e
cd "$(dirname "$0")"
# WLA-DX: $WLA (the folder with wla-z80 and wlalink), else ../tools/wla-dx/*, else PATH
if [ -z "$WLA" ]; then
  WLA=$(ls -d ../tools/wla-dx/*/ 2>/dev/null | head -1)
  [ -n "$WLA" ] || WLA=$(dirname "$(command -v wla-z80 || echo ./wla-z80)")
fi
if [ ! -f build/rom/bank00.bin ]; then echo "run tools/setup.py <your rom> first (the source reads the game data from it)"; exit 1; fi
"$WLA/wla-z80" -I src -I build/rom -o build/u4.o src/u4.asm
printf '[objects]\nbuild/u4.o\n' > build/u4.link
"$WLA/wlalink" -S build/u4.link build/u4.sms
EXPECT=a90e21e5961bcf2e10b715a009c04e7c2017a3b1
GOT=$(python -c "import hashlib,sys;print(hashlib.sha1(open(sys.argv[1],'rb').read()).hexdigest())" build/u4.sms)
if [ "$GOT" = "$EXPECT" ]; then echo "OK: build/u4.sms matches the original ROM ($GOT)"; else echo "DIFFERS: $GOT (original $EXPECT)"; exit 1; fi
