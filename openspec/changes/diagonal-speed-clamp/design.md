## Context

The left stick from pad to player (NTSC addresses; PAL v3 in
`config/releases/pal-v3.toml`):

- **Read.** `sub_00076E20` takes each axis on its own as
  `(v + 0.5) x 3.0518e-5` (`0x198A00` 0.5, `0x1998A0` 3.051804e-5), so
  -32768 and 32767 give -1 and 1. An axis under 0.25 (`0x199898`, a double)
  is set to 0: the deadzone zeroes that axis; it does not rescale the rest.
  The floats land in the pad state at +0x14 (LX), +0x18 (LY), +0x1C and
  +0x20 (right stick).
- **Poll.** `pad_poll_frame` (`sub_000A2DE0`) calls the read for the
  ports, then fills the global pad `0x3F09E0`: `cmp [0x3F0BC4], 0; je
  0xA2FFB` sums all four plugged-in pads (`+0x88` set) when the flag is
  zero, and copies the active pad (`[0x3FACAC]`) when it is set. It clamps
  each axis to [-1, 1] (`0xA3242..0xA3348`, `0x198A50` -1, `0x1989B8` 1)
  and writes LX and LY back to the active pad's state, `[0x3F09C0] + port
  x 0x1E4` (`0xA33EB`, `0xA3406`). The globals `0x3F09F4`/`0x3F09F8` are
  read only inside this function. An unplugged port's state is not
  rewritten: it keeps its last stick.
- **Player.** `sub_000D3D50` takes its pad pointer at `0xD3E07..0xD3E15`
  (the active pad, or `(active + 1) & 3` for a second player object) and
  reads LX, LY at `0xD49A6..0xD49A9`. At `0xD49EE..0xD4A03` it takes the
  length `m = sqrt(LY^2 + LX^2)` (`sub_000A3BF0`). At `0xD4A08`
  (`fcom [0x198A40]`, 0.4) a length at or under 0.4 jumps to `0xD4AEC`:
  speed 0, and the idle counter +0x5A counts up. Past 0.4 the speed is
  `(m - 0.4) x [0x199C1C]` (5/3), stored to player+0x30 at `0xD4D46`. The
  velocity +0x108 eases toward `s x [0x1ED104]` (1.0) `x [0x198B84]`
  (4.5). Nothing clamps `m` or `s`.
- **Other readers of player+0x30.** Only `sub_000D3D50` itself: `0xD44EB`
  (a change to state 0x13) and `0xD4CA1` (the run animation starts at
  s > 0.4).
- **Other readers of the left stick.** `0xD3E7F..0xD3EC8` (a shake test,
  `|stick - previous (+0x3C/+0x40)| > [0x1989B8]` 1.0, sets the byte
  +0x24; see D2); `sub_00045BC0` (from `sub_00084A30`, a stick-driven look
  offset into `0x9ECE34/38`); `sub_00088F20` (a zero test, and stick to
  angle at `0x895C8`); `sub_000E3300` (a sign test, menu); `sub_000AAC80`
  (the demo and replay recorder: it records the pad and plays it back
  into `0x63EC54/58`).
- **Demo and replay input.** When `[0x51A104]` is nonzero the game reads
  its pad from `0x63EC40`, which `sub_000AAC80` fills from its record, not
  from the pad states.

## D1. Where: after pad_poll_frame

The mod clamps the pad states in its `on_frame`. That runs from
`xbox_mod_frame()`, which the existing `sub_000A2DE0` wrapper in
`src/recomp_manual.c` calls after the generated body (draw-distance-mod
D4), once per main-loop iteration and before the frame's update reads the
pads. So:

- no gen hook, no new wrapper, no `recomp_manual.c` change;
- a toggle from the menu takes effect at the next poll;
- the next poll rewrites every plugged-in pad's state from the hardware,
  so turning the mod off needs nothing put back. An unplugged port's state
  keeps its last stick, clamped; it is one a round pad could have left,
  and it is rewritten when the pad comes back;
- demo and replay input (`[0x51A104]` nonzero, read from `0x63EC40`, which
  `sub_000AAC80` plays back from its record) does not pass through the
  pad states, so it skips the clamp. That is what keeps a recorded demo in
  sync: it replays the stick it was recorded with.

The four states at `[pad_states] + i x pad_state_size` are all clamped:
the player reads the active one, and a second player object the next. A
pointer that is 0 (before the game's pad init) or outside the 64 MB is left
alone, the latter logged once.

A hook on the length in `sub_000D3D50` would also work, but would mean a
gen-side override of a 0x1000-byte function for one `fsqrt`, and would
leave the other readers on the square.

## D2. Every reader sees a round pad

Clamping the stick, not the speed, gives every reader above exactly what a
round-gate pad gives at the same angle: a round pad cannot report a length
past about 1.000016 (its full diagonal, (23170, 23170)). The look offset,
the angle and the menu's sign test keep working; an angle is kept, and a
sign never changes. The clamp touches nothing inside the unit circle, so
the 0.4 dead ring, the walk speeds between 0.4 and 1 and the deadzone are
stock. Demo and replay input skips the clamp (D1).

The shake test does change, to what a round pad gives:
`sub_000D3D50` sets +0x24 when the stick moved more than 1.0 since the
last frame. Stock, a keyboard diagonal tapped from the centre (or let go)
jumps 1.414 and counts; clamped, it jumps 1.0 and does not, as on a round
pad. A reversal (a jump near 2) still counts. What the flag drives, read
from the code (not confirmed in play):

- **+0x62**, a word countdown: while it is above 0 the update takes its
  locked branch (`0xD3D6A`, `0xD3ECC`), which takes 1 off a frame and 4
  more on a frame with +0x24 set (`0xD3EE9`), stopping at 0. It is set to
  300 by `sub_000DA8D0` (called from `sub_000C2900` with the player
  `sub_000D2860` returns) and to 600 in `sub_000C2900` when `[0x512CAC]` is 0x27.
  Shaking the stick every frame ends the lock up to five times sooner.
- **+0x64**, a float timer set to 30 (`0xD69AC`, `0xD909E`) or 60
  (`0xD69FC`, `0xD8E45`): a frame with +0x24 set takes 4 off it
  (`0xD470B..0xD4712`, `[0x1989C8]`), stopping at 0.

So where the game lets Blinx shake free sooner, a keyboard or square-pad
player tapping a diagonal on and off gets no credit with the mod on; they
must rock the stick (or the keys) from side to side, as a round-pad player
must stock. The About says so. The mod does not change the test: its
threshold is the game's, and keeping the 1.414 jump would bring back the
corner it removes.

## D3. Square left stick

`input.pad_square_left` is the toolkit's host-pad stretch (in
`xbox_PadSetupApply`), before the guest reads the pad. The mod runs after
the guest's own poll. With both on, a round pad reaches the corner, then
the clamp brings it back to length 1: the clamp wins, and the help says
so. No setting links the two: each is valid alone.

## D4. Measurements

Metal, Mac, headless, the user route to stage 1 (`runs/b1-diag-clamp/
run.sh`), the speed read as player+0x30 (`pad_peek` at `0x4FB290`):

| run (`runs/b1-diag-clamp/`) | input held 150 polls | mod | speed (148 polls) |
|---|---|---|---|
| `kbd-off` | keyboard W+D | off | 1.69036 |
| `kbd-on` | keyboard W+D | on | 1.00000 |
| `square-on` | LX=LY=32767 (square gate, or Square left stick) | on | 1.00000 |
| `cardinal-on` | LX=32767 | on | 1.00000 |
| `round-off` | LX=LY=23170 (round gate) | off | 1.00003 |
| `round-on` | LX=LY=23170 | on | 1.00000 |

The keyboard runs use `RECOMP_KEYBOARD=1` and the script's `key W; key D`
steps, with `input.stick_ramp_ms` at its default 0: stock, the keyboard's
diagonal is the 1.69 corner. With the mod on the log has `[MOD diag_clamp]
on: left stick held to length 1` and `[MOD] 1 running: diag_clamp`.

The user route itself (`route-1650.pad`) holds a stick past the unit circle
on the way in: stock it reaches 1.11438 at folded state 105, which with the
mod on runs at 1.00000. So with the mod on the route drifts from stock, as
it should; the off gate (D5) is what compares to stock.

## D5. Stock with the mod off

The stage 1 route with a round diagonal, mod off (`runs/b1-diag-clamp/
round-off`), against the baseline (`runs/fdm/review-round`):
Blinx's state at `0x4FB290` (17 floats, consecutive repeats folded)
repeats exactly for all 1024 states of the baseline; no [CRASH]. The log
shows `[MOD] 0 running`: with the menu off, an off mod is not even inited.
With the menu on it is inited dormant, and its frame returns before it
reads guest memory.

Deck/Proton, mod off: the same route and script, D3D11 under GE-Proton,
the Windows exe built natively on the PC (`BENCH_BUILD_HOST=bench host-win`,
exe `33ed88be04b9`, blinx 6d3ff5a, toolkit 7775ac0), 85 s
(`bench-logs/20261105-094331`): 2883 flips, 0 present mismatches, no
[CRASH]. All 802 folded states of the tracker item Deck run
(`bench-logs/20261105-070622`) repeat exactly, and all 969 states of this
run match the Metal off run (with the CRT's `-nan(ind)` read as `nan`).
The diagonal holds at 1.00003, as stock.

## D6. Where the help shows

The Mods page lists "Clamp diagonal speed" with its toggle
(`runs/b1-diag-clamp/mods-page-list.png`). Enter opens the mod page: the
title, v1, About (the summary, wrapped to the width), State, Enabled and
Reset this mod (`runs/b1-diag-clamp/mods-page-detail.png`). The About says
it in three plain sentences: BLiNX 1 runs full diagonals 1.69x as fast as
straight on the keyboard and with Square left stick (on by default); the
mod caps them at straight speed, as in BLiNX 2; and the shake change (D2).
With Square left stick off a round pad runs its diagonal at 1.00, so the
About names only the inputs that reach 1.69.

The switch's own `help` is the tooltip card under the Mods list row and
the mod page's Enabled row. The integration toolkit (7775ac0) does not
show it: the toggle slot never took the enable setting's help, and
`detail_rows` (`enhance_menu.c`, `row_add(R_SLOT, t, 1, "Enabled")`) and
the Mods list's `R_MOD` rows copied none, so no mod's switch got a card.
The toolkit fix is on `fix/mod-toggle-help` (tip 61c40406, off
`posix-host/portability`): the toggle slot carries the enable setting's
help and both rows copy it, for every mod (a switch with no help stays
empty); `tests/enhance_menu` has the case. This change needs nothing more:
the help is already in the descriptor and fits the card (159 bytes,
`scripts/test_diag_clamp.py`). With that toolkit the card shows
(`runs/b1-diag-clamp/mods-page-tooltip.png`).
