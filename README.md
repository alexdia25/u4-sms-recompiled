# Ultima IV: Quest of the Avatar (Sega Master System): rebuildable source and PC port

This repository rebuilds the Master System game of *Ultima IV: Quest of the
Avatar* from source, and builds a native PC version of it. In the spirit of
[u4remasteredA2](https://github.com/sean-gugler/u4remasteredA2) and
[u4-decompiled](https://github.com/ergonomy-joe/u4-decompiled), it holds code,
names, notes and tools only. **None of the game's data is in it.** You supply
your own ROM, and the build takes what it needs from that.

The ROM has to be this release:

| | |
| --- | --- |
| Game | Ultima IV: Quest of the Avatar (Sega Master System, Europe/USA) |
| Size | 512 KB |
| SHA1 | `a90e21e5961bcf2e10b715a009c04e7c2017a3b1` |

From it you can build:

- **`build/u4.sms`**, the ROM reassembled from `src/`. Unchanged source gives a
  byte-identical copy of the original; changed source gives your own version
  of the game.
- **`pc/out/u4.exe`**, the PC version. It needs neither an emulator nor the ROM
  once it has been set up.

## What you need

| Tool | For | Tested with |
| --- | --- | --- |
| Python 3 | everything (no extra packages) | 3.13 |
| a POSIX shell | the build scripts (on Windows: Git Bash or MSYS2) | Git Bash |
| [WLA-DX](https://github.com/vhelin/wla-dx) (`wla-z80`, `wlalink`) | the SMS ROM | 10.7 |
| [Zig](https://ziglang.org/download/), used as the C compiler | the PC port | 0.17.0 |
| [SDL2](https://github.com/libsdl-org/SDL/releases), MinGW development package (`SDL2-devel-2.x.x-mingw.zip`) | the PC port | 2.32.10 |

The scripts look for each tool, in this order:

- **WLA-DX:** `$WLA` (the folder holding `wla-z80`), then `../tools/wla-dx/*/`, then `PATH`.
- **Zig:** `$ZIG` (the `zig` executable), then `../tools/zig/zig-*/zig`, then `PATH`.
- **SDL2:** `$SDL` (the package's `x86_64-w64-mingw32` folder), then `../tools/sdl2/SDL2-*/x86_64-w64-mingw32`.

The PC build is set up for 64-bit Windows. The runtime itself is portable C
(its fibers fall back to `ucontext` off Windows), but `pc/build.sh` only knows
the Windows target.

## Build the SMS ROM from your ROM

```sh
python tools/setup.py path/to/Ultima4.sms
```

Setup does the following:

1. Checks the ROM's SHA1.
2. Splits the ROM into `build/rom/bankNN.bin`. The source's `.INCBIN` lines read the game's data from there.
3. Assembles `src/` into `build/u4.sms` (`./build.sh`) and checks it is identical to your ROM.
4. Extracts the data in readable form to `build/data/`: dialogue, messages, maps, pictures, stats, shops and spells.
5. Builds `build/u4.pak`, the asset pack the PC version runs from.

After changing anything in `src/`, rebuild the ROM with:

```sh
./build.sh
```

The check then prints `DIFFERS` and exits with status 1. That is expected,
and `build/u4.sms` is your modified game. It runs in any SMS emulator.

## Build the PC port

```sh
python tools/setup.py path/to/Ultima4.sms --pc     # or, after setup: pc/build.sh
pc/out/u4.exe
```

`pc/build.sh` regenerates `pc/gen/recomp.c` from the disassembly and compiles
it, together with the runtime and the C rewrites, into `pc/out/u4.exe`.
`SDL2.dll` and `u4.pak` are placed next to it. The first build takes a few
minutes because the generated C is large; later builds reuse its object file.
To build the test version without a window or sound, run:

```sh
pc/build.sh headless
```

That gives `pc/out/u4_headless.exe`.

**Controls:**

| Key | Does |
| --- | --- |
| Arrows | D-pad |
| Z | button 1 |
| X | button 2 |
| Enter | Pause |
| Esc | quit |

A gamepad also works. The game's battery save is kept in `u4.sav` next to the exe.

### Making a release for players

```sh
pc/package.sh 1.0
```

This zips `u4.exe` and `SDL2.dll` into `pc/release/`, with no game data. A
player runs `u4.exe their-rom.sms` once: the exe checks the ROM, builds
`u4.pak` itself (`pc/runtime/import.c`), and starts. From then on `u4.exe`
alone runs the game.

## How it works

- **`src/`** is a WLA-DX disassembly of the whole ROM, labelled from
  `tools/hints.txt`. In the labels, `fn_BB_AAAA` is a function in bank BB at
  CPU address AAAA, and `loc_` marks a jump target. Data stays out of the
  source as `.INCBIN` lines.
- **`pc/gen/recomp.c`** (generated, not in git) is a static translation of the
  ROM's code to C, one block per Z80 instruction. `tools/recomp.py` writes it.
  It runs on `pc/runtime/`, which provides the CPU state, video, sound, mapper
  and the SDL2 front end. Every instruction keeps the console's cycle count,
  so timing matches the original.
- **`pc/native/`** holds readable C rewrites of the game's routines, which
  replace the translated originals one by one through `native` lines in
  `tools/hints.txt`:
  - text printing and the text window;
  - menus, number entry and yes/no prompts;
  - talking to people;
  - every kind of shop;
  - map loading and walking;
  - the player's turn, moving, and the commands Talk, Get Chest, Ready, Search and Camp;
  - combat and karma rules;
  - the task system.

  Each rewrite takes exactly the original's time and leaves the machine
  exactly as the original does.
- **`docs/`**:
  - `architecture.md` covers the main loop, tasks, how C rewrites wait across frames, and timing.
  - `data-formats.md` covers the ROM's data.
  - `rules.md` covers game rules found in the code: combat, karma, talking and shops.

### Tests

```sh
tests/run.sh             # all cases (about 8 minutes)
tests/run.sh food_buy    # one case
```

Each case is a key script that starts from power-on (`tests/start.keys`, which
plays through creating a character to Moonglow) and then shops, talks to
townsfolk or plays at random (`tests/make_cases.py` writes them). Every case
must end in the same state with the C rewrites as without, and in shadow mode
every call of a rewrite must match the original, time included. It needs only
`build/u4.pak` from setup.

### Checking rewrites against the original

| Setting | Effect |
| --- | --- |
| `U4_NATIVE=off` | runs the translated original everywhere |
| `U4_NATIVE_ONLY=a,b` | turns on only the named rewrites |
| `U4_NATIVE=shadow` | runs each rewrite, then rewinds and runs the original on the interpreter, and reports any difference in memory, video, sound or registers (`U4_SHADOW_CYCLES=1` also compares the time taken) |
| `U4_NATIVE=timed` | runs the rewrites but charges each the original's measured time |

The headless exe prints a `state-hash` at the end of a run. Equal hashes with
and without the rewrites mean the game ended in the same state:

```sh
pc/out/u4_headless.exe build/u4.pak --headless 8632 --keys script.keys
```

Here `script.keys` holds lines of the form `<frame> <buttons>`, with buttons
from `UDLR12` or `-` for none. A line `<frame> =C9AB:FF` writes RAM instead.
`U4_RANDOM=seed` plays random input. `tools/cycles.py` lists original code
with its cycle counts.

## Other tools

| Command | Does |
| --- | --- |
| `python tools/disasm.py build/original.sms src coverage/cov.bin` | regenerates `src/` after editing `tools/hints.txt` (names flow into the source and the generated C) |
| `python tools/disasm.py build/original.sms out coverage/cov.bin --inline-data` | writes a copy with the data inline, for reading only; never commit it |

`coverage/cov.bin` records which ROM bytes run as code and which are read as
data; the disassembler and the C generator are driven by it.

## Legal

Ultima IV is © Origin Systems / Electronic Arts, and the Master System version
is © Sega. This project contains no game data. You need your own copy of the
ROM to build anything from it.
