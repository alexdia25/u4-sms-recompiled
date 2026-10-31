#!/bin/sh
# Random play: the rewrites against the original, from where tests/start.keys
# ends, with the party given money and things and sent somewhere by seed.
#   tests/random_play.sh 1 40         seeds 1 to 40
#   tests/random_play.sh 7            one seed
# Prints one line per seed; a seed that fails can be replayed the same way.
# U4_RANDOM_FRAMES: how long (default 30000 frames). A seed fails if the states
# differ or any original instruction ran.
cd "$(dirname "$0")/.."
EXE=pc/out/u4_headless.exe
N=${U4_RANDOM_FRAMES:-30000}
if [ -n "$2" ]; then
	i=$1
	while [ "$i" -le "$2" ]; do echo $i; i=$((i + 1)); done |
		xargs -P "${JOBS:-8}" -I{} sh tests/random_play.sh {}
	exit
fi
seed=$1
t=$(mktemp -d)
python tests/random_start.py "$seed" > "$t/k"
compare() {
	U4_RANDOM=$seed U4_RANDOM_FROM=9000 U4_SETTLE=10 "$EXE" build/u4.pak --headless "$1" --keys "$t/k" > "$t/on" 2>/dev/null
	U4_NATIVE=off U4_RANDOM=$seed U4_RANDOM_FROM=9000 U4_SETTLE=10 "$EXE" build/u4.pak --headless "$1" --keys "$t/k" > "$t/off" 2>/dev/null
	state=$(python tests/same_state.py "$t/off" "$t/on" 2>&1 | tail -1)
	left=$(grep -o 'interpreted [0-9]*' "$t/on" | head -1 | cut -d' ' -f2)
}
compare "$N"
# a run can end where the scheduler has no pass to compare at: a little further
case "$state" in state*) compare $((N + 100)) ;; esac
rm -rf "$t"
if [ -z "$state" ] && [ "$left" = 0 ]; then echo "ok    $seed"; else echo "FAIL  $seed (${state:+$state, }$left instructions interpreted)"; fi
