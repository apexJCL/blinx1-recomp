## 1. Spec

- [x] 1.1 proposal, design, spec (cause traced in BLiNX 1 and checked in BLiNX 2)

## 2. Code (toolkit `spike/fdm`)

- [x] 2.1 `src/input/pad_map.c/.h`: `xbox_InputSquareStick`; `square_left` / `square_right` in `xbox_pad_setup`, last in `xbox_PadSetupApply`
- [x] 2.2 `src/platform/recomp_env.h`: `RECOMP_INPUT_PAD_SQUARE_LEFT` / `_RIGHT` (enhance keys); `enhance_cfg.c` binds them
- [x] 2.3 `src/enhance/enhance_input.c`: knobs, `[INPUT] pad map:` names them
- [x] 2.4 `src/enhance/enhance_menu.c`: two Controls toggles, under a Sticks header on the Controls page (the review found the rows unreachable; toolkit d9528b89), with a `tests/enhance_menu` case: toggle, pending key, Save, a cfg re-read plus `enhance_input_reload`, a fresh start
- [x] 2.5 `docs/runtime/config.md`, `src/input/README.md`
- [x] 2.6 `tests/pad_map`: formula, cardinals and -32768, every gate point, deadzone order, per stick, identity

## 3. Checks

- [x] 3.1 `tests/posix_ctest.sh` 0 FAIL
- [x] 3.2 Mac build of BLiNX 1 against the toolkit worktree; Windows build natively on the PC (`notes/build-all.sh`, toolkit f8696e23: pass)
- [x] 3.3 BLiNX 1 stage 1: round diagonal vs squared diagonal vs cardinal, speed at player+0x30 (Blinx at 0x4FB260): 1.00 / 1.69 / 1.00, ground covered in 48 polls 38.2 / 64.5 units (`runs/fdm/fdm2-*`)
- [x] 3.4 Off: no `[INPUT] pad map:` line and no setup; `RECOMP_INPUT_PAD_SQUARE_LEFT=1` logs `square=l` (`runs/fdm/knob-*`)
- [x] 3.5 Review fixes (toolkit d9528b89 + docs 5541bd09): `tests/posix_ctest.sh` 152 pass, 0 FAIL (`runs/fdm/posix-ctest-review.log`). Metal, windowed: the Controls page shows Sticks with both rows; right on Square left stick turns it on with "1 unsaved", F5 writes `[input] pad_square_left = true` and says "Saved 1 change"; a restart on that file logs `[INPUT] pad map: square=l` and shows the row on (`runs/fdm/controls-{toggled,saved,restart}.png`, runs `runs/fdm/menu3`, `menu4`)
- [x] 3.6 Goldens with the knobs off: BLiNX 1 has no golden.json (`blinx1 golden` exits "game.toml sets no golden.json"), so the gate is the knobs-off stage 1 route with the round diagonal, compared by Blinx's state (0x4FB290, 17 floats, consecutive repeats folded):
  - Metal (Mac build on d9528b89, `runs/fdm/review-round`): the 946 states of the pre-review run `fdm2-round` repeat exactly, then the run idles on; no [CRASH].
  - Deck, Proton D3D11, exe built on the PC (sha256 c3e74624…, run c3e7462410a3, `bench-logs/20261105-070622`): ran to the 85 s limit, 2582 flips (30.4/s), 0/2582 present mismatches, no [CRASH], no `pad map` line; its 802 states equal the Mac's first 802, the diagonal walk and the stop included (NaN spelling folded).
- [ ] 3.7 Real host pad on the Mac or the PC with the knob on (needs a person at the pad)

## 4. Follow-ups

- [x] 4.1 `cat/docs/env.md` rows for the two env keys (cat `docs/fdm-env` b4ef3d68)
- [ ] 4.2 Decide whether BLiNX 1's `config.default.toml` turns `input.pad_square_left` on
