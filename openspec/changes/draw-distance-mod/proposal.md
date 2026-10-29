## Why

BLiNX 1 drops whole objects at a fixed distance from the camera. The
visibility test `sub_0009AD80` (the bounding-sphere test the model and node
draws use) ends with a far cull: while the flag at `0x3AE708` is set, a
sphere whose near side is more than 400 units in front of the camera
(`vz - r > [0x1D7394]`, 400.0f) is rejected. On the console the cutoff hid
pop-in at 480 lines. At render scale 2x and 3x the cutoff is easy to see:
objects appear and vanish at the same distance while the rest of the
picture is sharp.

BLiNX 2 already has the pattern for this kind of change: an opt-in mod on
the in-game Mods page that writes a game constant under a guard (the `lod`
mod, cat b66f398).

## What changes

- **The `draw_distance` mod** (`src/draw_distance.c`, mod API v3, default
  off). Switched on, it writes `400 x distance` over the far-cull constant
  at `0x1D7394`. `distance` runs from 1 to 4 in steps of 0.25 (default 2).
  The write is guarded: it writes only over the stock value, stops for the
  run if anything else changed the constant, and puts 400 back when the
  setting is turned off and at exit.
- **The guard maths** (`src/far_cull.c`): pure C, driven by
  `scripts/test_draw_distance.py`.
- **The mod frame glue.** BLiNX 1 had no `xbox_mod_frame()` call. The pad
  poll `sub_000A2DE0` gets a wrapper in `src/recomp_manual.c` that calls it
  after the generated body, once per main-loop iteration. `main.c`
  registers the mod table before `xbox_enhance_init`.
- Env keys `RECOMP_DRAW_DISTANCE` and `RECOMP_DRAW_DISTANCE_SCALE` (rows in
  `src/env/recomp_env_game.h` and `docs/env.md`), and a commented `[mods]`
  block in `packaging/config.default.toml`.

Not changed: the projection's far plane (2048, past the largest cutoff of
1600), fog, the flag at `0x3AE708` and the rest of `sub_0009AD80`. No
toolkit change.

## Impact

- Off (the default), the game is stock: the constant is never written.
- On, more objects are drawn. Batches per flip and flips/s on Metal at 1x
  and 2x are recorded in design.md.
- The tracker item (Hor+ cull) wraps `sub_0009AD80` and scales its side planes.
  This change does not touch that function. The two compose with no
  ordering (design D5).
