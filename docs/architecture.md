# How the game is put together

Names refer to labels in `src/` (from `tools/hints.txt`); addresses are CPU
addresses (`bank:$addr` when paged).

## Start-up and the main loop

`reset` ($0042) sets the mapper (bank 0, 1, 2 in slots 0–2), clears RAM, sets the
stack to $DFF0, initialises the hardware and jumps to `warm_restart` ($0CC4).

`main_loop` ($0CD2) runs once per frame:

1. remembers the frame counter, reads the joypad (`update_joypad`);
2. looks at `game_mode` ($C600):
   - **bit 7 clear** — a mode is being entered: its setup routine is called through
     `game_mode_table` ($0D55, three bytes per mode: bank, address). The setup draws
     the screen, starts the mode's tasks and sets bit 7.
   - **bit 7 set** — the mode is running: `run_tasks` ($0AE5) gives each task its turn;
3. waits for the next vertical blank and loops.

The loop and the scheduler are rewritten in C (`pc/native/loop.c`),
instruction by instruction: after each instruction's cycles the C calls
`native_boundary()`, which lets the machine catch up exactly where the
translated code would stop (at the first instruction that reaches the line's
end), so interrupts come between the same instructions. The frame wait spins
on the frame count the interrupt changes, so a coarser step would change when
it ends.

Mode numbers seen so far: `$11` world, `$12` settlement, `$13` dungeon, `$14`
combat, `$15` camping. Their setup routines: `$11` 2:$AC74, `$12` 2:$AC80, `$13` $69D8 (bank 1), `$14` and up through bank 3's vector table ($801B …).

## Tasks — how menus, talking and combat wait without blocking

Most of the game's behaviour runs as **tasks**: up to 10 cooperative threads that
each get a turn per frame. They are what let a routine "wait for a button" or
"pause for half a second" while the rest of the screen keeps animating.

- Task slots: 10 × 48 bytes at $C300. While a task runs, **IX points at its slot**.
  - `+0` kind (who started it), `+1` frames until it runs again (0 = slot free),
  - `+2/+3` where it resumes, `+4` the bank it runs in,
  - `+$14/+$15` resume point while waiting for a flag,
  - `+$24` … `+$2B` a four-level call stack (see below), the rest is the task's own
    variables.
- `spawn_task` ($0B33, C = kind, DE = entry): takes the first free slot, sets it to
  run next frame in the current bank; returns the slot in IY (carry set if all 10 are
  busy).
- `run_tasks` ($0AE5): for each busy slot, count `+1` down; at zero, page in `+4` and
  jump to `+2/+3` with IX = the slot.
- `yield` ($0B19): pops its own return address into `+2/+3` and returns to
  `run_tasks` — the task continues right after the call next frame.
  `sleep` ($0B16, A = frames) does the same after setting `+1`.
- `end_task` ($0B5A): clears the slot.
- Nested routines can't keep a return address on the shared Z80 stack across a
  yield, so a task routine that will yield starts with `task_enter_1..4` ($0AA5,
  $0AB5, $0AC5, $0AD5), which move its return address into `+$24`, `+$26`, `+$28` or
  `+$2A`, and ends by jumping to the matching `task_return_1..4` ($0AAE … $0ADE).
- `wait_flag` ($0A82) yields every frame until `$C0C7` is set, then resumes at the
  address it saved in `+$14`.

**Rewritten tasks** (`pc/native/tasks.c`): each step of a task — from a resume
point to its next yield — is a native routine registered at that resume address.
It does the step's work and leaves the slot as the original yield would
(`task_sleep`, `task_end`), so rewritten and original steps mix freely and the
shadow test compares each step. Parts not rewritten yet are reached with
`call_original`. The first one is `music_later` (two steps, both matching).

**Rewritten code that waits for frames** (marked `.blocking = 1`) runs on a
fiber: `wait_vblank()` parks the CPU at `PARK_PC` and returns to the machine,
which keeps running interrupts; the dispatcher resumes the fiber there each time
slice. These can't be compared step by step, so `U4_NATIVE_ONLY=<name>` runs
just that rewrite and the final state is compared with a run without it.
The first one is `text_page_wait` (the "press a button" stop every 5 lines).

**C tasks** (`Native.task`) are whole task routines in C that wait across turns,
like the conversation (`pc/native/talk.c`). Each runs on the fiber of its task
slot. `task_frame_tick(site)` / `task_wait_button(site)` do what the original's
`call frame_tick` / `call wait_button` at that address do, leaving the slot as
the original would (resume address, `+$2A`), end the turn and return to
`run_tasks`; when `run_tasks` jumps to that resume address with IX on the slot,
the dispatcher resumes the C code instead. `end_task` and `clear_tasks` are
watched (hook natives) so a C task whose slot the game clears is dropped. A C
task calls only original routines that don't yield (`call_original`), since
their stack would otherwise be left behind. Tests: key scripts can poke RAM
(`<frame> =C9AB:FF`) to steer a conversation into each of its paths, the same
way in both runs.

**Timing.** Every rewritten routine adds the original's exact cycle count
(T-states, worked out from the original's instructions and checked against it
in shadow mode, which reports both per routine), added at the point where the
original would spend them, so the rest of the game sees the same timing.
`U4_PROFILE=<name>` in shadow mode writes the original's cycles per
instruction to `profile.txt`, which is how the counts were checked. Three
things in the runtime keep that timing:

- scan lines are timed on an absolute clock, so a routine that does many
  lines' worth of work at once takes that many lines;
- when a rewritten routine runs past its time slice, the CPU is parked at
  `PARK_PC` and the rest of its cycles are spent there slice by slice, with
  interrupts taken as usual;
- a rewrite marked `.long_calls = 1` (like `frame_tick`) runs on the fiber,
  so the original routines it calls run in the machine with interrupts on.

With that, a run with every rewrite on ends in the same state as the original
(`state-hash` in the headless output, which leaves out the stack's stale
return addresses). `U4_NATIVE=timed` runs the rewrites but charges each the
original's measured time, which separates "does the same thing" from "takes
the same time" when a run differs.

For the C rewrite this means: routines that never yield (`docs/rules.md`,
`pc/native/`) can be replaced one at a time; task code needs to become explicit
state machines (or C coroutines) that keep their resume point in the slot.

## Interrupts

The VDP interrupt (`$0038` → `irq_handler`, $02C0) saves all registers, counts
frames (`vblank_count`), reads the pause/reset button, and services the queues the
main code fills: VRAM uploads queued by `queue_vram_copy` ($0465: bank, source,
VRAM address, length at $C200…) and the sound driver (bank 31, `sound_vectors`).
NMI (`$0066`, the Pause button) only sets a flag.

The handler is rewritten in C (`pc/native/irq.c`). The queue turns
interrupts back on between its VDP address writes, so a line interrupt can
come in the middle of it; the C calls `native_irq_point()` after each `ei`
and the instruction after it, which lets an interrupt pending then be taken
there, as on the machine.

The Reset button (port $DD bit 4, `X` in a key script) makes the frame
interrupt jump to $0CB4 with a fresh stack; `native_reset()` drops the
rewritten code that was waiting (parked fibers, C tasks, time sinks). A
rewrite that works in bigger steps than single instructions is left as its
last step did, where the original stops at an instruction, so a Reset
during such a rewrite can leave different half-done work.

## Banks

- Slot 0: bank 0 (code, fixed). Slot 1: bank 1 (code) — temporarily banks 4/5 for the
  world map. Slot 2: paged through `$FFFF`; `$DFFF` is its RAM shadow.
  `bank_push` ($07CE) / `bank_pop` ($07DE) switch slot 2 for a call and back,
  keeping the old bank on the stack.
- Bank 1 ends with 126 `jp` vectors ($7E00) that the other banks call, bank 3 starts
  with 22 (`bank03_vectors`), bank 31 with 8 (`sound_vectors`).
- Code: banks 0–3; sound driver and music: 31; everything else is data
  (`docs/data-formats.md`).

## Drawing the map

The map window is drawn in software: map cells are 16×12 pixels, which don't fit
the console's 8×8 tiles, so `draw_map_window` ($1F56–$2097) composes each cell's
graphic (two 8-pixel columns of mask + 4 planes) into unique VRAM tiles each time
the view moves. The map itself sits in a 32×32-cell window at $D000 (2 bytes per
cell: tile, attribute) that slides by whole 16×16 chunks as the party walks
(`step_east` … `step_north`, `recenter_map_view`).

## How much is rewritten

The PC version has no translated code left: `pc/runtime/run.c` runs the rewrites
and hands anything else to the interpreter, which counts it (`interpreted` in the
headless output; `U4_LOG_FALLBACK=file` lists the addresses). None of the test
cases or the seeds of `tests/random_play.sh` interprets a single instruction, and
both fail if one does. With `U4_NATIVE=off` the whole game runs on the interpreter,
which is the original that everything is compared with.

At the last count, 1,008 cases reached all but about 840 of the original's real
instructions. About 690 of those have a rewrite that no test has run yet; 317 of
them are the sound driver's, which every song and sound (`song_*`, `sounds_*`, with
and without the FM unit) leave unused, so the game's music data never asks for them.
About 140 have no rewrite: if one ever ran, the interpreter would run it and count
it. Another 1,000 or so disassembled "instructions" nothing reaches are text and tables.
Run the cases with `U4_NATIVE=off` and `U4_LOG_FALLBACK` to list what they reach.
