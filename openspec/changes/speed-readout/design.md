## Context

NTSC addresses; PAL v3 in `config/releases/pal-v3.toml`.

- **The player.** `sub_000D3D50` (the player update) takes its object as
  its stack argument (`mov esi, [esp+0x2C0]` at `0xD3D59`) and picks its
  pad with `cmp esi, [0x4FB244]` at `0xD3DB7`: the object `[0x4FB244]`
  points to reads the active pad, any other the next one. So `[0x4FB244]`
  is the player, named `player_ptr`. Four writers set it (`0xAA1BA`,
  `0xAAF95`, `0xAB76A`, `0xAB8EF`); the first stores the static object
  `0x4FB260`, which is where the probes read +0x30 (`0x4FB290`).
- **The run speed.** Past the 0.4 dead ring the update computes
  `s = (m - 0.4) x 5/3` (`0xD4A1C..0xD4A2B`, `0x198A40` 0.4, `0x199C1C`
  5/3) and stores it at player+0x30 (`0xD4D46`). 1.00 is a full straight
  push; the square's corner gives 1.69036.
- **PAL v3.** The update is at `0xD4C10`, the same code with other
  address operands: `cmp esi, [0x4EBC24]` at `0xD4C71`, the speed from
  `0x18A920` (0.4) and `0x18AC28` (5/3), stored to `[esi+0x30]` at
  `0xD5BDD`. `0x4EBC24` has four writers and 188 references, as `0x4FB244`
  has in NTSC.

## D1. Its own mod

The toolkit drops a mod's status card while the mod is off, and the
readout is wanted with `diag_clamp` off (the stock 1.69). So it is a mod
of its own, off by default.

## D2. Where: the mod frame

The readout runs in `on_frame`, from `xbox_mod_frame()` after
`pad_poll_frame`, once per main-loop iteration: each frame it reads what
the last update stored. No hook, no gen change.

## D3. Read only, and guarded

The mod reads `[player_ptr]`, then the float at +`player_run_speed`. It
shows `now -` and empties the peak when the pointer is zero, not 4-byte
aligned or outside the console's 64 MB, or when the value is not a speed
(NaN, negative, past 10). The guard is `speed_meter_ptr_ok` in
`speed_meter.c`, pure C and tested. It never writes guest memory, calls
guest code or hooks anything; off, `on_frame` returns before any read.

In practice `now -` shows only before the first stage. The pointer is zero
until the game makes its first player; from then on it keeps pointing at
the static player object (`0x4FB260` in NTSC), so in menus and between
stages the card shows the last value that object holds.

## D4. The text

Two lines, the first the card's title: `Speed`, then
`now 1.69  peak 1.69`. The peak is the top of the last 30 samples, one a pad
poll: 30 polls, about a second in an NTSC stage (the tracker item's runs: 150 polls
in 5 s), 1.2 s PAL. The card is set again only when the text changes, so
a steady run costs one string compare a poll. While Blinx moves the speed
changes on most polls, so the card is set, and redrawn, on most polls
too: acceptable for a debug mod.

## D5. The card needs the menu

The toolkit starts the status panels from the menu's init
(`enhance_mod_status_start`, called in `enhance_menu.c`), and their tick
runs on the actions hook, which `menu.enabled` arms. With the menu off
(`RECOMP_MENU=0`) the mod runs and sets its text, but no card is drawn:
the first Metal run (`runs/b1-speed-readout/off`) shows none. The menu
is on by default, so players see it; scripted runs set `RECOMP_MENU=1`.
