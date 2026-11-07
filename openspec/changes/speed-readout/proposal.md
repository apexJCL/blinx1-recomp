## Why

The diagonal speed fix (diagonal-speed-clamp) is checked by
reading the player's run speed in a probe log (`pad_peek` on player+0x30).
The user wants to see it live while playing instead: straight runs should
read 1.00, a full keyboard or Square left stick diagonal about 1.69, and
1.00 with the clamp on. The
toolkit's mod status card (mod API v7, `status`) can show it. The card
belongs to one mod and is dropped while that mod is off, so the readout
cannot live in `diag_clamp`: it has to show with the clamp off too.

## What changes

- **The `speed_readout` mod** (`src/speed_readout.c`, mod API v7, default
  off), "Speed readout" on the Mods page. Switched on, each mod frame it
  reads the player's run speed and shows it on the status card at the top
  left: `now 1.69  peak 1.69`, the peak being the top of the last 30
  polls (about a second in an NTSC stage, 1.2 s PAL). Before the first
  stage, with no player yet, it shows `now -`.
- **The readout** (`src/speed_meter.c`): pure C, driven by
  `scripts/test_speed_readout.py`.
- Names `player_ptr` and `player_run_speed` in `config/names.toml` and
  `config/releases/pal-v3.toml`; gen regenerated (only `recomp_names.h`
  changes).
- Env key `RECOMP_SPEED_READOUT` (rows in `src/env/recomp_env_game.h` and
  `docs/env.md`) and a commented entry in `packaging/config.default.toml`.
- Help: "Shows Blinx's run speed on screen: 1.00 straight, up to 1.69 on
  a full diagonal (keyboard, or a pad with Square left stick)." About:
  "Debug readout of Blinx's run speed. 1.00 is a full straight push; a
  diagonal at full tilt reaches 1.69 on the keyboard or a pad with Square
  left stick. Peak is the top of the last 30 polls (about a second).
  Outside a stage it shows the last value."

No toolkit change, no hook and no `recomp_manual.c` change.

## Impact

- Off (the default), the mod reads nothing and shows nothing.
- On or off, it never writes guest memory, so the game runs as stock.
- A toolkit before mod API v7 builds the game without the mod.
