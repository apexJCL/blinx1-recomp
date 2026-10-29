## Context

`sub_0009AD80(x, y, z, r)` is BLiNX 1's bounding-sphere visibility test.
The model and node draws call it (`sub_0009C6C0`, `sub_0009CE20`,
`sub_000C14E0`, `sub_000C1670`, `sub_000D2D30`) and skip what it rejects.
After the four side-plane tests it has a far cull (`0x9AFD9..0x9B00A`):

    if ([0x3AE708] != 0 && vz - r > [0x1D7394]) return 0;

`0x1D7394` is a float in `.rdata` holding 400.0f (`0x43C80000`). The flag
at `0x3AE708` is set in the stage draw `sub_000BF100` (`0xBF163`) and in a
background draw (`sub_000BDCBC`), both only when the stage index
`[0x512cac]` is 20, 21, 22, 28, 29 or 30, and cleared in `sub_00094660`.
The stage file is `rd%1d%1d` with round `idx / 4 + 1` and stage
`idx % 4 + 1` (`0xBC5E1`), so the far cull runs only in rd61, rd62, rd63,
rd81, rd82 and rd83. Every other stage, the attract demo (rd11) included,
never reaches it, and there the mod changes nothing.

## D1. Write the constant, not a wrapper

The task allows a global change only if the constant has no other reader.
It has none:

- `fcomp dword ptr [0x1d7394]` at `0x9AFF3` is the only instruction in any
  code section that names the address (`analysis/` disassembly; in the
  generated code, `recomp_0013.c`, the one `MEMF(0x1D7394)`).
- A raw scan of every XBE section for the pointer value `0x001D7394` finds
  only that instruction's operand (`0x9AFF5`). No table holds the address,
  so nothing reads it through a pointer.
- The neighbours `0x1D738C`, `0x1D7390` (0.03125) and `0x1D7398` (0.00125)
  are separate constants with their own readers. The mod writes 4 bytes
  only.

The generated code reads the constant from guest memory on every call
(`MEMF`), so a write takes effect at the next test. A wrapper on
`sub_0009AD80` would also work, but the tracker item already wraps that function
(D5). Writing the datum keeps the two changes apart.

## D2. The far plane and fog stay

`sub_00092470` builds the projection with
`sub_000F86E8(0x3AE6A0, fov, -4/3, near 1.0, far 2048.0)` and loads it with
`sub_00135D60(1, 0x3AE6A0)`. The largest cutoff the mod writes is 1600
(distance 4). A sphere passes the cull only while `vz - r <= 1600`, so its
near side is inside the far plane and it is drawn. Only a sphere with a
radius over 448 could reach past 2048, and then its far part is clipped
the same way as in the stock game. So the far plane needs no change, and
the factor stays capped at 4.

Fog: the stage draw sets no fog that ends inside 400 to 1600. In rd61 with
the mod at 2 the frames show the same scene with no fog band or new
edge (`runs/blinx1-lod-mod/view/cmp_2500.png`); the pixels that differ
are rain, flame animation and the timer, which differ between any two
runs.

## D3. The guard

Ported from BLiNX 2's `lod` mod (cat b66f398, `lod_table.c`). The pure-C
state in `far_cull.c` is `{checked, dead, have}`, where `have` starts as
the stock bits:

- want == have: nothing is read or written. An off mod that never wrote
  never touches memory.
- Before the first write the constant must be stock (400.0f). If not, the
  state goes dead (`FAR_CULL_NOT_STOCK`).
- After a write, the constant must still hold `have`. If not, something
  else wrote it (a patch, a cheat, a later fix), and the state goes dead
  (`FAR_CULL_FOREIGN`).
- Dead means no writes for the rest of the run, restore included.

Off from the menu: the next mod frame asks for stock and writes 400 back.
Exit: `shutdown` asks for stock (best effort, as in cat).

## D4. The mod frame

BLiNX 1 had no `xbox_mod_frame()` call. The call has to run once per guest
frame on the guest thread, before the draws. The pad poll `sub_000A2DE0`
(which calls the XAPI reader `sub_00076E20`) fits. The main-loop phases in
`sub_000A9CE0` (`0xA9D66`, `0xA9EAB`, `0xA9F94`) and the loop around
`0xAFC02` / `0xAFC4D` call it once per iteration, after the vblank wait
(`sub_00138620`). `src/recomp_manual.c` wraps it: the generated body, then
`draw_distance_frame()`, which is `xbox_mod_frame()` with the mod API and
nothing without it. Measured in rd61 with an auto-continue
lldb breakpoint: 54 hits over 58 flips in a 22 s window (the breakpoint
slows the game to about 2.6 flips/s, and the window edges are fuzzy), so
once per flip.

## D5. With the tracker item (Hor+ cull)

The tracker item wraps `sub_0009AD80` and scales `_11` of the stored projection
(`0x3AE6A0`) for the call, so the side planes match the padded width. This
mod does not touch `sub_0009AD80` or `0x3AE6A0`. It writes only the
constant that the far cull reads after the side-plane tests. Neither
change reads what the other writes, so they compose in any order and
either can land alone. The only shared edit is in `src/recomp_manual.c`:
both replace the template's `extern uint32_t g_eax` with
`#include "gen/recomp_types.h"`. Whichever merges second keeps one copy.

## D6. Settings

- `mods."draw_distance.enable"` (`RECOMP_DRAW_DISTANCE`): `off` / `on`,
  default `off`, live.
- `mods."draw_distance.distance"` (`RECOMP_DRAW_DISTANCE_SCALE`): 1 to 4,
  step 0.25, default 2, live. The cutoff is `400 x distance`.

The default factor is 2 because the mod at factor 1 is the stock game.

## Measurements

Metal, Mac, `RECOMP_PRESENT_PACING=spin`, silent. Raw logs in
`runs/blinx1-lod-mod/`.

Attract demo (rd11, flag never set), 72.5 s window:

| run | batches/flip | flips/s |
| --- | ---: | ---: |
| 1x off | 5803 | 30.08 |
| 1x on (2) | 5800 | 30.10 |
| 2x off | 5802 | 30.08 |
| 2x on (2) | 5803 | 30.09 |

rd61 (round 6-1, far cull active), reached from the user save's stage
select by swapping the chosen index `0x51a568` from 1 to 20 before the
loader `sub_000BC580` reads it (`route-rd61.txt`), player idle at the
start, 107 s window from 7 s after `rd61.mit` opens:

| run | batches/flip | flips/s | PB walk ms |
| --- | ---: | ---: | ---: |
| 1x off | 9509 | 29.99 | 17.08 |
| 1x on (2) | 9572 | 30.00 | 16.88 |
| 1x on (4) | 9571 | 30.00 | 14.35 |
| 2x off | 9511 | 30.00 | 17.58 |
| 2x on (2) | 9572 | 30.00 | 15.70 |

The mod adds about 63 batches per flip (0.7%) at the start of rd61, and
nothing more at 4 than at 2 there: the start view holds nothing between
800 and 1600. Flips/s stays at the game's 30 cap; no drop near the 25%
flag. The walk times vary with the other agents' load on the Mac and
show no trend.

## Stock check

BLiNX 1 has no goldens yet (`game.toml` sets no `golden.json`). With the mod
off nothing is written: `far_cull_step` is never called (the unit tests
check that off never writes), the log has no `far cutoff` line, and the
only added runtime work is the `xbox_mod_frame()` call after the pad poll,
which with no mod running only checks the Mods page for a toggle. In the
deterministic attract demo the off and on runs draw the same batches per
flip, so the hook itself changes nothing the game draws. A frame A/B against `81d461b` was not run:
the main checkout's build uses another toolkit pin, so its frames would
differ for unrelated reasons, and rd61's rain and timer differ between any
two runs.

After the rebase onto the tracker item (main `49276c0`, both `sub_0009AD80` and
`sub_000A2DE0` wrapped in `recomp_manual.c`, gen regenerated): rd61 at 1x
with the mod on (2) logs `far cutoff 800`, draws 9571 batches per flip at
30.00 flips/s (9572 before), and its flip 2500 frame shows the same scene
as before the rebase, apart from rain, flame and timer. The Deck attract
run with the mod off, on an exe the Windows PC built (sha256 `f148dd10`),
ran the 130 s with 5138 flips (39.5/s), 0 present mismatches, no crash and
`[MOD] 0 running` (bench-logs `20261028-205135`).
