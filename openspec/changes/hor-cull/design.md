## Finding the test

BLiNX 2's fix scales the aspect argument of its projection builder for the
main view. BLiNX 1 has no separate cull frustum to widen, so the port starts
from the draw instead:

- The model draw (`sub_00094140`, reached through `sub_00099860` and
  `sub_0009AA50`) is called from `sub_0009CE20`, the model and node draw.
  Before drawing, `sub_0009CE20` calls `sub_0009AD80(x, y, z, r)` and skips
  to its end (loc_0009D082) when it returns 0.
- `sub_00092470` builds the projection: `sub_000F86E8(0x3AE6A0, fov,
  -4/3, 1, 2048)`, then `sub_00135D60(1, 0x3AE6A0)` (SetTransform, which
  copies the matrix into the device). Only four functions read the stored
  matrix's `_11` (0x3AE6A0), `_22` (0x3AE6B4) and `_34` (0x3AE6CC):
  `sub_00092470` itself, `sub_0009AD80`, and `sub_0009BF20`/`sub_0009BF3A`.

`sub_0009AD80` is the test. It builds the modelview (0x3AE660 = view
0x3AE620 x world) with `sub_00091460`, moves the sphere centre into view
space, scales the radius by a per-object factor (0x676300 + idx*4) and then:

- rejects the sphere when vz + r <= 0 (behind the eye);
- with inv = 1 / (_34 * vz): sx = _11 * vx * inv, sy = -_22 * vy * inv,
  rx = |_11 * r * 1.2 * inv|, ry = |_22 * r * 1.2 * inv|;
- keeps the sphere when -(1 + rx) <= sx <= 1 + rx and the same for y, then
  calls `sub_00090860` and returns 1; otherwise returns 0.
- between the two: when the flag at 0x3AE708 is set, a sphere that passed
  the side and top/bottom tests is still dropped when vz - r > 400
  (0x1D7394), a far cull (found by the tracker item, the LOD mod).

Its other callers are `sub_0009C6C0`, `sub_000C14E0`, `sub_000C1670` and
`sub_000D2D30`. All of them ask "is this on screen", so all of them want the
padded width.

## The change

Hor+ keeps the vertical extent and widens the horizontal one to A / (4/3).
With s = (4/3) / A, scaling `_11` by s for the call gives
|s * sx| <= 1 + s * rx, that is |sx| <= 1/s + rx: the padded screen edge,
with the margin scaled the same way. `_22` is left alone.

- The wrapper sets `MEMF(0x3AE6A0) = _11 * s`, calls `sub_0009AD80_gen`,
  and puts `_11` back only if it still holds the scaled value, so a
  projection rebuilt during the call is kept.
- The device already has its own copy of the projection, so the GPU sees no
  change. The test's callee tree (`sub_00090860`, `sub_00091370`,
  `sub_00091460`, `sub_00096B20`, `sub_000A3BF0`) reads no `_11` and makes
  no indirect calls. `sub_0009BF20`/`BF3A` place sprites by `_11 * 320`
  and run outside the call, so they keep the stock value.
- The far cull reads only vz and r, and runs only after the x and y tests
  pass, so the wider side planes leave it as it is: a side-band object
  past 400 is still dropped, as it would be in the 4:3 view. A mod that
  moves the far cull and this wrapper touch different inputs.
- `cull_init` uses enhance.c's own wide test (`aspect_w * 3 > aspect_h *
  4`), as BLiNX 2 does. Without the layer, `display.aspect` is 0 and the
  wrapper is the generated body alone.

The `_gen` rename comes from `blinx1 recomp` (`--exclude-manual`). The gen
key also covers `recomp_manual.c`, so every edit to it needs `blinx1 analyze
&& blinx1 recomp` before the build will run. The function itself is already
known, so no seed is needed.

### Alternatives

- Widening the projection `sub_00092470` builds (BLiNX 2's route) would
  change the GPU matrix and the sprite placement too.
- Patching the 1.0 bounds in the test would mean hand-editing generated code.

## Measured (Metal, Mac, headless attract demo, stage 1-1)

Each run used `RECOMP_INPUT_SCRIPT=@attract`. The demo opened at flip
1650/1651 in every run. The table covers the 3200 flips after demo start +
120. The A/B used one probe build with a temporary uncommitted toggle that
turned the wrapper off.

| display.aspect | fix | batches/flip | vs off | flips/s |
| --- | --- | ---: | ---: | ---: |
| 4:3 | (stock path) | 3917.3 | | 36.1 |
| 16:9 | off | 3920.7 | | 36.0 |
| 16:9 | on | 4287.8 | +9.4% | 36.2 |
| 21:9 | off | 3929.3 | | 36.2 |
| 21:9 | on | 4664.7 | +18.7% | 36.2 |

- The 4:3 centre of every host-size dump is byte-identical with and without
  the fix (mean difference 0.0).
- Only the side bands change. With the fix, objects such as the barrels,
  lamps with their glow, and walls appear there; without it they are absent
  until they cross the old edge.
- Flips/s does not change.

On the Deck (D3D11 under Proton, the same exe as the final code, 21:9,
130 s):

- one `[HORCULL]` line, no crash, 0 of 4573 presents mismatched, 35.2
  flips/s;
- over the 2500 flips after demo start + 120, 5970.2 batches per flip,
  against 5970.0 on Metal for the same window. The demo is the same on
  both backends, and so is the culling.
