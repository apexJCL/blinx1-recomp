## Context

The game's stick pipeline (BLiNX 1, addresses in the XBE):

| Step | Where | What |
|---|---|---|
| read | `sub_00076E20` (XAPI, from `sub_000A2DE0`) | per axis: `f = (v + 0.5) * 3.0518e-5`; `|f| < 0.25` reads 0. Floats at pad+0x14..+0x20 (pad struct 0x1e4 bytes) |
| merge | `sub_000A2DE0` `pad_poll_frame` | ports merged into 0x3F09F4.. and each axis clamped to [-1, 1], copied back to the active port |
| run | `sub_000D3D50` at 0xD49A2..0xD4A40 | `m = sqrt(x² + y²)`; `m > 0.4` → `s = (m - 0.4) * 5/3` at player+0x30, no clamp; heading `atan2`; velocity +0x108 eases toward `s * dt * 4.5` at 0.125 |

The clamp at 0xD5045 only bounds `v / dt` for the run animation. So the
run speed follows the stick's length up to the square's corner: 1.0 on a
cardinal, 1.69 on a Duke diagonal, and 1.0 on a round pad's diagonal.

BLiNX 2 (cat) has the same read (`sub_0021C290`: `(v + 0.5) * 3.0518e-5`,
per-axis deadzone 0.4). However, its walk code (0x17C5D1, 0x17C712) sets
`m = 1` when `m > 1`, before `(m - 0.4) * 5/3 * 1.5`. A square diagonal
is therefore no faster there, and the mapping would only make partial
diagonals reach full speed sooner. BLiNX 2 stays as it is.

## Decisions

### D1. Toolkit, not a game mod
The cause is the host gate's shape, not the game's code. Any Xbox title
that takes a stick's length expects the Duke's square gate. A host-side
mapping fixes it for every game, beside the existing host-stick steps
(`RECOMP_PAD_DEADZONE`, `input.pad_deadzone_*`, `input.pad_invert_*`), with
no guest patch. A game mod could only patch BLiNX 1's speed formula, and
would have to be redone for each title.

### D2. The stretch keeps the angle
`k = max(1, min(r, 32767) / max(|x|, |y|))`, where `r = sqrt(x² + y²)`,
blended in at the rim as `k' = 1 + (k - 1) · t`, `t` a smoothstep of
`min(r, 32767) / 32767` from 0.7 to 1. The point keeps its
direction. At full tilt its larger axis becomes its length, so the unit
circle maps onto the unit square and each angle keeps its own speed. This
matches Steam Input's squared output and the official PC release only at
the rim: under 0.7 of full the stick keeps its true length, as a square
gate reads it near the centre, so drift stays in the game's 0.4 deadzone
and a half tilt runs at half speed.

- `k ≥ 1`: the mapping never shrinks an axis. Cardinals, (0, 0), a pad
  that already reports square corners, and -32768 pass unchanged.
- `min(r, 32767)`: a pad reporting outside the circle (most do, a
  little) never exceeds the corner. The result clamps to int16.

Rejected: per-axis rescaling (`x / cos`) and the elliptical grid mapping.
Both bend the angle, which the game's heading (atan2) would show.

### D3. Last in `xbox_PadSetupApply`
The mapping runs after the radial deadzone, the stick swap and the
alternate map, on the stick the guest sees:

- the deadzone stays round, as a drifting pad needs;
- a swapped stick is squared under its guest name, so
  `input.pad_square_left` always means the guest's left stick.

The flags live in `xbox_pad_setup`, so a setup with either flag on is not
the identity and is installed (`xbox_InputSetPadSetup`). With both off,
nothing changes: the stock path installs no setup.

### D4. Per stick, enhancements only
There are two flags, so a title can square the movement stick and leave
a camera stick round. They are enhancements-layer knobs, read in
`pad_init`, like the other `input.pad_*` knobs. They also have env rows,
bound in `enhance_cfg_bind_env`, because a run or a bench needs to switch
them without a file. The menu's Controls page has a toggle for each. A
toggle installs a copy of the setup in force with its one bit changed
(the poll reads it at the next frame) and saves like the other items.

### D5. Scripts and the keyboard
Scripted sticks (`RECOMP_INPUT_SCRIPT`, `pad_script`) feed the guest
directly and stay unmapped, so a script means the same values on every
build. The keyboard already reaches the corner (`INPUT_DIAGONAL_FULL`).

### D6. BLiNX 1's default
The official PC release turns the squared deadzone on by default. Turning
on `input.pad_square_left` in BLiNX 1's `packaging/config.default.toml`
would change the stock feel of the port, so it is left to the user. The
spike leaves it off.

## Risks

- Analogue precision in the rim band: the stretch rises from 1 at 0.7 of
  full to √2 at full, so near 45° the output length moves up to about 3x
  as fast as the input (around 0.87 of full). One host step becomes up to
  about 3 guest steps, which is still invisible at int16, but the run
  speed climbs steeply over that last part of the travel.
- Menus that read the stick through the auto-repeat (`sub_00076B60`) see
  larger diagonals, which is the Duke's behaviour.
- Rim saturation: only a pad whose rim reaches 32767 gets full diagonals.
  With the rim blend, a worn pad whose rim stops short of full now reads
  less on diagonals than under the full stretch, since `t` is below 1
  there: a rim at 0.9 of full gives about 0.83 per axis on a diagonal,
  where the full stretch gave 0.9. The real-pad check covers it.
  Rescaling the radius per pad would need a calibration the toolkit does
  not keep.
