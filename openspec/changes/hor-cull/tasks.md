## 1. Find the test

- [x] 1.1 Walk up from the model draw to `sub_0009CE20` and its
  `sub_0009AD80` call; list the readers of the stored projection
- [x] 1.2 Check that the test's callee tree reads no `_11` and makes no
  indirect calls; check that the far cull (0x3AE708) is independent

## 2. Wrapper

- [x] 2.1 `src/recomp_manual.c`: `sub_0009AD80` around `sub_0009AD80_gen`,
  `cull_init` from `nv2a_host_opts()`, one `[HORCULL]` line
- [x] 2.2 `blinx1 analyze && blinx1 recomp`: recomp does the `_gen`
  rename, and the gen key covers `recomp_manual.c`, so the build refuses
  until both have run

## 3. Gates

- [x] 3.1 Mac build, enhancements ON and OFF
- [x] 3.2 `uv run pytest scripts`, ruff check and format
- [x] 3.3 Metal A/B at 4:3, 16:9 and 21:9: side bands, batches per flip,
  flips/s (design.md)
- [x] 3.4 BLiNX 1 goldens: none exist yet (no golden.json), so no golden
  gate applies
- [x] 3.5 Proton run on the Deck at a wide ratio: `[HORCULL]` line, no
  crash, batches per flip as on Metal

## 4. Follow-ups

- [ ] 4.1 `packaging/config.default.toml` still calls wider ratios untested
  on BLiNX; revisit once Hor+ has been played through on BLiNX 1
- [ ] 4.2 The wide dumps show a faint vertical seam at the old 4:3 edges,
  with and without the fix. It is probably a fullscreen pass that still
  covers only 4:3, like the one BLiNX 2's `hor-plus-fullscreen-fx` fixed.
  It is not part of this change
- [ ] 4.3 CPU backend: under Hor+ the CPU rasteriser draws 4:3 only, but
  the cull still widens, so it processes 9-19% more batches that are
  clipped away. A cost, not a visual change; the CPU backend is outside the
  routine gates
- [ ] 4.4 The camera lens light effect (lens flare) is cut at the old 4:3
  edges at wide ratios (the user, 2026-10-30). Find whether it is a
  screen-space sprite pass sized to 4:3, a render-target sample (the
  `hor-plus-fullscreen-fx` rule) or a cull on the light's projected x, and
  fix it with 4.2, since both show at the 4:3 columns
