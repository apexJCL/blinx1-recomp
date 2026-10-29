## Why

BLiNX 1 drops below 30 fps in stage 2 (Round 1, the snow) on the Deck.
The Deck's D3D11 path under Proton, stock 1x, rd12, measured over
180 s: wall p50 43.3 ms (about 23 fps), every flip over 34 ms. The walk takes
36.6 ms of it (backend draws 23.7, decode 12.2). The same place without snow
(rd11) takes 33.3 ms, walk 27.3 ms. The user's config (2x, aniso 16, 16:10)
is no slower (40.7 ms): the cost is the CPU walk, not the GPU.

What the snow adds is batches: 18,560 a flip against 14,800, with almost no
more constant loads (1,531 against 1,471). The method log of one Metal snow
flip shows why: every batch is BEGIN, one DRAW_ARRAYS of a 3- to 8-vertex
triangle strip, END. 10,884 of 18,645 batches follow the previous END with no
method at all in between. The game draws its particles and much of its scene
as single strips, and each costs one backend draw call (about 1.3 us on the
Deck) and the walker's per-batch work.

## What changes

- **Toolkit: draw merging in the walker** (`nv2a_pb_exec.c`, helper
  `nv2a_merge` in `nv2a_backend_common.c`). A batch made only of
  DRAW_ARRAYS or ARRAY_ELEMENT indices, of a triangle primitive, is held at
  its END. The next batches with the same primitive and no other method in
  between join it. The first other method, or the end of the walk, draws the
  chain as one TRIANGLES list made by `nv2a_prim_to_list` from each batch in
  turn. The triangles, their order, winding and last vertex are those the
  batches would have drawn, so the frame is the same.
- A backend opts in with `merge_draws` (Metal and D3D11 do; the CPU and null
  backends do not). Merging is off while `display.aspect` is set, because
  Hor+ places each batch from its own vertices (`nv2a_aspect_place`).
- `RECOMP_DEBUG=pb_merge=0` turns it off for A/B runs. The [PB-PERF] line
  gains `draws`: the backend draws, against `batches`, which still counts
  guest batches. `RECOMP_DEBUG=pb_mlog=<flip>` logs one flip's methods
  (the tool that found this).

## Impact

- Every game on a GPU backend at stock aspect: fewer, larger draws; the
  frame should be unchanged (goldens).
- Hor+ (16:10 on the user's Deck) is not merged by this change: a follow-up
  has to make merging and per-batch placement agree.
