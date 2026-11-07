## 1. Spec

- [x] 1.1 proposal, design, spec

## 2. Code (blinx `feat/diag-clamp`)

- [x] 2.1 `src/stick_clamp.c/.h`: the length clamp
- [x] 2.2 `src/diag_clamp.c/.h`: the mod (API v3), off by default, help and About text
- [x] 2.3 `src/main.c`: the mod in the table
- [x] 2.4 Names in `config/names.toml` and `config/releases/pal-v3.toml`; gen regenerated (only `recomp_names.h` changes)
- [x] 2.5 CMake sources, env row, `docs/env.md`, `config.default.toml`
- [x] 2.6 `scripts/test_diag_clamp.py`

## 3. Checks

- [x] 3.1 `uv run pytest scripts`, ruff
- [x] 3.2 Mac build
- [x] 3.3 Measured: keyboard diagonal stock 1.69; on, keyboard and squared-stick diagonals 1.0
- [x] 3.4 Off: stage 1 route state matches stock on Metal
- [x] 3.5 Off: Deck/Proton run (Windows exe built on the PC)
- [x] 3.6 Screenshots of the Mods page toggle the About, and the tooltip with the toolkit's fix/mod-toggle-help (D6)
- [x] 3.7 Review (df73d65): the shake test (D2) researched and named in the About; demo input (D1); Poll's sum/copy; changelog; PAL value and edge-case tests
