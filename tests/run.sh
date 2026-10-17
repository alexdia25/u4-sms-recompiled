#!/bin/sh
# Check the C rewrites against the original game, from power-on.
#   tests/run.sh            all cases
#   tests/run.sh food_buy   one case
#
# Needs build/u4.pak (tools/setup.py) and builds pc/out/u4_headless.exe if
# it is missing. For every case (tests/cases/*.keys, from tests/make_cases.py):
#   1. the game runs with every rewrite on and with none (U4_NATIVE=off); the
#      state-hash printed at the end must be the same
#   2. it runs in shadow mode with exact timing (U4_NATIVE=shadow,
#      U4_SHADOW_CYCLES=1): each call of a rewrite is compared with the
#      original on the same machine state; there must be no mismatch
#   3. the C tasks (which shadow mode can't check) must be entered at the
#      same cycles as the original code they replace (tests/entry_times.py)
# Prints one line per case and exits with 1 if any failed.
cd "$(dirname "$0")/.."
EXE=pc/out/u4_headless.exe
[ -f build/u4.pak ] || { echo "run tools/setup.py <your rom> first"; exit 1; }
[ -f "$EXE" ] || sh pc/build.sh headless >/dev/null || exit 1
[ -d tests/cases ] || python tests/make_cases.py
frames() { tail -1 "$1" | cut -d' ' -f1; }
TASKS=$(grep -h '\.task = 1' pc/native/*.c | sed 's/.*{ "\([a-z0-9_]*\)".*/\1/' | paste -sd, -)
# hash frames keys trace-file: the state-hash; the C tasks' entries go to the file
hash() { U4_TRACE=$TASKS "$EXE" build/u4.pak --headless "$1" --keys "$2" 2>"$3" | tail -1 | sed 's/.*state-hash \([0-9A-F]*\).*/\1/'; }

check() {
	k=tests/cases/$1.keys
	n=$(frames "$k")
	t=$(mktemp -d)
	on=$(hash "$n" "$k" "$t/on")
	off=$(U4_NATIVE=off hash "$n" "$k" "$t/off")
	bad=$(U4_NATIVE=shadow U4_SHADOW_CYCLES=1 "$EXE" build/u4.pak --headless "$n" --keys "$k" 2>&1 | grep -c "differs")
	late=$(python tests/entry_times.py "$t/off" "$t/on" 2>&1 | tail -1)
	rm -rf "$t"
	if [ "$on" = "$off" ] && [ "$bad" = 0 ] && [ -z "$late" ]; then echo "ok    $1"
	else echo "FAIL  $1 (state $on vs original $off, $bad shadow mismatches${late:+; $late})"; fi
}

if [ -n "$1" ]; then check "$1"; exit; fi
for k in tests/cases/*.keys; do basename "$k" .keys; done |
	xargs -P "${JOBS:-8}" -I{} sh tests/run.sh {} > tests/results.txt
sort tests/results.txt
! grep -q FAIL tests/results.txt
