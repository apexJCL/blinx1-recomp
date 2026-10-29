## 1. Spec

- [x] 1.1 proposal, design, spec

## 2. Code (blinx `feat/lod-mod`)

- [x] 2.1 `src/far_cull.c/.h`: cutoff bits and the guard
- [x] 2.2 `src/draw_distance.c/.h`: the mod (API v3), off by default
- [x] 2.3 `src/recomp_manual.c`: `sub_000A2DE0` wrapper calls the mod frame
- [x] 2.4 `src/main.c`: mod table before `xbox_enhance_init`
- [x] 2.5 CMake sources, env rows, `docs/env.md`, `config.default.toml`
- [x] 2.6 `scripts/test_draw_distance.py`

## 3. Checks

- [x] 3.1 `uv run pytest scripts`, ruff
- [x] 3.2 Mac build; the hook runs once per flip
- [x] 3.3 Fog and far plane with the mod on
- [x] 3.4 Metal batches per flip and flips/s, 1x and 2x, off and on
- [x] 3.5 Stock check with the mod off (no goldens: no-write proof and the off runs; see design)
- [x] 3.6 Deck/Proton run with the mod off (attract, 130 s: 5036 flips, 0 present mismatches, no crash, `[MOD] 0 running`; bench-logs/20261028-202620)
