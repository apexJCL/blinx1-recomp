## Why

In BLiNX 1 a diagonal can run 69% faster than straight ahead.
The player update `sub_000D3D50` runs at the left stick's length with no
clamp, `s = (m - 0.4) x 5/3`, and the stick the game sees is the square
[-1, 1]^2. A full corner (1, 1) gives s = 1.69; a cardinal gives 1.0.
Speedrunners use this "faster diagonal". It is reached by:

- the keyboard: with the stick ramp off (`input.stick_ramp_ms` 0, BLiNX 1's
  default) a diagonal holds both axes at full;
- a square-gate pad (the Duke);
- a round pad with the toolkit's `input.pad_square_left` (Controls >
  Square left stick).

BLiNX 2 sets the length to 1 before its own run formula (walk code at
0x17C5D1 and 0x17C712), so its diagonal is never faster. Players who want
BLiNX 1 to move like BLiNX 2, on any input, have no way to do so.

## What changes

- **The `diag_clamp` mod** (`src/diag_clamp.c`, mod API v3, default off),
  labelled "Clamp diagonal speed" on the Mods page. Switched on, it scales
  the left stick of each of the four pad states down to length 1, keeping
  its angle, right after the pad poll and before the frame's update reads
  it. Every input then runs as a round-gate pad does.
- **The clamp maths** (`src/stick_clamp.c`): pure C, driven by
  `scripts/test_diag_clamp.py`.
- Names `pad_states`, `pad_state_size`, `pad_left_x`, `pad_left_y` in
  `config/names.toml` and `config/releases/pal-v3.toml`.
- Env key `RECOMP_DIAG_CLAMP` (rows in `src/env/recomp_env_game.h` and
  `docs/env.md`) and a commented entry in `packaging/config.default.toml`.
- The menu help (the row's tooltip): "Caps diagonal speed at straight
  speed. Without it, full diagonals (keyboard, or a pad with Square left
  stick) run 1.69x as fast." The mod's About: "BLiNX 1 runs full
  diagonals 1.69x as fast as straight, on the keyboard and on a pad with
  Square left stick (on by default). This mod caps them at straight
  speed, as in BLiNX 2. Side effect: a diagonal tap no longer counts as a
  stick shake."

No toolkit change. The descriptor's summary is the mod page's About, shown
in full, and it carries the whole explanation. The switch's `help` (up to
159 bytes, the tooltip card) is set too, but the toolkit does not show it
yet: `detail_rows` in `src/enhance/enhance_menu.c` adds the "Enabled" row
without copying the setting's help, and the Mods list row has none either.
That held for every mod's switch; the toolkit fix is on
`fix/mod-toggle-help` (61c40406, design D6). No generated code and no `recomp_manual.c` change: the
mod frame already runs after `pad_poll_frame` (draw-distance-mod D4).

## Impact

- Off (the default), the pad states are never written and the game is
  stock.
- On, a keyboard, square-gate or squared-stick diagonal runs at 1.0, as a
  cardinal. A round pad's full diagonal reads 1.000016 long and is moved by
  0.002%; anything inside the unit circle is untouched.
- With `input.pad_square_left` on as well, the stick still reaches the
  corners (the toolkit's stretch), and the clamp then brings the length back
  to 1. The two compose in that order with no setting between them: the
  clamp wins.
- Only the left stick. The right stick (the camera) is the game's.
- The game's shake test (a stick move of more than 1.0 in a frame, which
  shortens some timers when Blinx shakes free) no longer counts a diagonal
  tapped from the centre and let go, as on a round pad; reversals still
  count (design D2). The About says so.
- Demo and replay input is played back from the game's own record and
  skips the clamp, so demos stay in sync (design D1).
