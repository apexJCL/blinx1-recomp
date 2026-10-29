## 1. Measure

- [x] 1.1 Deck D3D11 stock and user config, rd12 (snow) and rd11; Metal rd12
- [x] 1.2 Method log of a snow flip (`pb_mlog`), chain counts

## 2. Toolkit (xboxrecomp `fix/snow-fps`)

- [x] 2.1 `nv2a_merge` helper in `nv2a_backend_common.c`
- [x] 2.2 Walker: hold, continue, flush (method, walk end, checkpoint)
- [x] 2.3 `merge_draws` on Metal and D3D11; off for CPU, null and Hor+
- [x] 2.4 `pb_merge` and `pb_mlog` debug rows; [PB-PERF] `draws`
- [x] 2.5 ctest `tests/pb_draw_merge`
- [x] 2.6 Mac build, `tests/posix_ctest.sh`
- [x] 2.7 Review: no merging for a batch that samples its own colour
      surface; counters added in `draw_primitive`; the open batch capped at
      256 (save buffer 256 entries); `pb_merge`/`pb_mlog` rows in cat and
      blinx `docs/env.md`; draw merging in `pushbuffer-executor.md`; ctest
      cases self, cap, verts

## 3. Verify

- [x] 3.1 Deck rd12 and rd11 after, stock; Metal rd12 after
- [ ] 3.2 Goldens: BLiNX 2 Metal and Deck D3D11 (BLiNX 1 has no golden.json yet); Metal snow frames merge vs pb_merge=0 look the same
- [x] 3.3 Follow-up: merging under Hor+
- [ ] 3.4 Follow-ups from the review, not done here: the backends'
      `skipped` and `culled` flip counters still count backend draws (add
      `d->batches`); cap the span relative to the chain's vertices (for
      example `span <= 4*verts + 64`) so far-apart strips don't fetch 4096
      vertices; more ctest cases (list-full split, inline and immediate
      batches, `pb_merge=0`, CPU backend, checkpoint flush, ARRAY_ELEMENT16,
      fan and quad chains); `merge_continues` ignores the subchannel
      (harmless: no other class has methods at 0x17FC or 0x1800-0x1810)
