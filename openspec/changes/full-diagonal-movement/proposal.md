## Why

BLiNX 1 runs faster on a diagonal than on a cardinal, but only with the
Duke's square gate. A modern pad's gate is round. Its full diagonal reads
about (0.71, 0.71), so on PC and on our port the player runs no faster on
a diagonal than straight. The official PC release gets "Full
Diagonal Movement" from a Steam Input setting: a squared deadzone on the
left stick, so that a diagonal reads about (1, 1).

The cause, from the code:

- `sub_00076E20` (the XAPI pad read, under `pad_poll_frame`
  `sub_000A2DE0`) normalises each axis on its own, as
  `(v + 0.5) * 3.0518e-5`, with a per-axis deadzone of 0.25.
- `pad_poll_frame` clamps each axis to [-1, 1]. The stick the game sees is
  the square [-1, 1]².
- The player update `sub_000D3D50` (at 0xD49EE) takes the length
  `m = sqrt(x² + y²)`. Past 0.4 it sets the run speed to
  `s = (m - 0.4) * 5/3` (player+0x30), and does not clamp it. The
  velocity +0x108 then approaches `s * dt * 4.5`.
- A cardinal gives s = 1.0. The corner of a square gate gives m = 1.414
  and s = 1.69, so 69% faster. A round gate caps both at 1.0.

Nothing here is BLiNX-specific: the guest reads a Duke, and the host pad
is round. The fix belongs in the toolkit's host-pad mapping, beside the
deadzone and the inversion, for every title that expected a square gate.

BLiNX 2 (cat, `sub_0021C290` and the player walk at 0x17C5D1 and
0x17C712) uses the same per-axis normalisation, with a 0.4 deadzone.
However, it clamps the length to 1 before the speed formula, so its
diagonal is no faster than a cardinal on any gate. It needs no change.

## What changes

- **Toolkit (`xboxrecomp`, branch `spike/fdm`)**:
  - `xbox_InputSquareStick` in `src/input/pad_map.c`: a circle-to-square
    stretch at the rim that keeps the angle.
  - Two per-stick flags on `xbox_pad_setup`, applied last in
    `xbox_PadSetupApply`.
- Knobs `input.pad_square_left` and `input.pad_square_right`:
  - enhancements layer, default off;
  - env rows `RECOMP_INPUT_PAD_SQUARE_LEFT` and
    `RECOMP_INPUT_PAD_SQUARE_RIGHT`;
  - two toggles on the menu's Controls page;
  - the `[INPUT] pad map:` line names them.
- Docs (`docs/runtime/config.md`, `src/input/README.md`) and a
  `tests/pad_map` case.
- BLiNX 1 needs no code change, only a config value. Whether its packaged
  `config.default.toml` should turn on `input.pad_square_left` is a
  product call (see design).

## Impact

- Off by default; the setup with both flags off is the identity, so a
  stock run takes the same path as before (no setup installed).
- Host pads only. Scripts, the keyboard (already full diagonal through
  `INPUT_DIAGONAL_FULL`) and the USB model's own presses are untouched.
- Follow-up: rows for the two env keys in `cat/docs/env.md` (it lists the
  toolkit keys; not edited from this branch).
