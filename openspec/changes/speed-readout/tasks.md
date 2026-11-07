## 1. Spec

- [x] 1.1 proposal, design, spec

## 2. Code (blinx `fix/diag-clamp-text`)

- [x] 2.1 `src/speed_meter.c/.h`: the readout and its peak
- [x] 2.2 `src/speed_readout.c/.h`: the mod (API v7), off by default, help and About text
- [x] 2.3 `src/main.c`: the mod in the table
- [x] 2.4 Names `player_ptr`, `player_run_speed` in `config/names.toml` and `config/releases/pal-v3.toml`; gen regenerated (only `recomp_names.h` changes)
- [x] 2.5 CMake sources, env row, `docs/env.md`, `config.default.toml`, CHANGELOG
- [x] 2.6 `scripts/test_speed_readout.py`

## 3. Checks

- [x] 3.1 `uv run pytest scripts`, ruff
- [x] 3.2 Mac build
- [x] 3.3 Metal run: the card, keyboard W and W+D, with diag_clamp off (1.00, 1.69) and on (1.00, 1.00); screenshots. The card needs the menu enabled (closed): `enhance_menu.c` starts the status panels
- [x] 3.4 Readout off against on: player+0x30 matches poll for poll over 2719 polls (`runs/b1-speed-readout/readout-off` against `readout-on-2`, and against `off`, menu off); the `clamp-off` run, which took 17 window shots, shifts one 0.46 window of the route by a poll or two before the stage (polls 1132-1369) and matches from there on
- [x] 3.5 Review (88df862): precise texts (1.69x as fast; keyboard and square-stick only), `now -` only before the first stage, the pointer guard in `speed_meter.c` with tests
