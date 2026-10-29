## Context

`nv2a_pb_exec_method` decodes one method at a time. A batch ends at
`SET_BEGIN_END(0)`, which calls `draw_primitive`, which hands a
`struct nv2a_pb_draw` to the backend. Metal and D3D11 both turn the batch
into a triangle list with `nv2a_prim_to_list` and fetch the vertex range
lo..hi of its indices.

## Decisions

- **Merge in the walker, not in each backend.** Both GPU backends already
  convert to a list; giving them a list of several batches keeps the two in
  step (the backends-agree rule) and needs no backend code but the opt-in.
- **Only when nothing came between.** The held batches draw with the state
  as it was at their END. Holding them across any other method, even a
  re-send of the same value, would need the walker to know which methods
  leave draw state alone. On the snow flip that gains nothing: allowing
  same-value re-sends of 0x0308, 0x039C and 0x035C merged no more batches.
- **Flush before the method is applied.** The first method that does not
  continue the chain draws it, so the backend sees the draws in their
  stream order. When that method comes inside a batch that has started
  (only indices so far), that batch's indices are set aside and put back.
- **Flush at the end of each walk and at a run-ahead checkpoint.** Nothing
  is held when GET moves.
- **Limits.** A batch joins with up to 256 indices; the list holds 0x6000;
  the chain spans at most 0x1000 vertices, since the backend fetches the
  whole range lo..hi. A batch that does not fit draws the chain and starts a
  new one.
- **A chain of one is drawn as it came,** with its own primitive, so a lone
  batch reaches the backend exactly as before.
- **Counters stay per guest batch.** `draw_primitive` adds the batches a
  draw stands for to `s_gpu.draws`, the [PB-PERF] `batches`, the pacing and
  write-back counts, drawn or skipped; `draws` on [PB-PERF] is the new count
  of backend draws.
- **Hor+ off.** With `display.aspect` set, the backends decide each batch's
  placement (2D, full width, screen quad, band, flat layer) from its own
  vertices; a merged list would change those calls. Merging there is the
  follow-up.
- **Not for a batch that samples its own target.** Both GPU backends copy
  the colour surface per draw for a batch whose enabled texture stage
  samples it (Metal `rt_texture`, D3D11 `rt_self_copy`), so each batch of a
  run sees what the one before it wrote. Such a batch is never held: an
  enabled stage (CONTROL0 bit 30) whose resolved offset matches the colour
  surface's in its low 27 bits, the test `rt_texture` uses (review).
- **The open batch stays under the cap.** An index method continues a
  chain only while the open batch stays within 256 indices, so the indices
  set aside at a flush fit a 256-entry buffer, not one of NV_MAX_INDICES.
- **Default on.** It is an internal change that keeps the frame, not an
  enhancement; `pb_merge=0` is the A/B switch.
- **Counters stay per guest batch.** A merged draw carries the number of
  guest batches it stands for (`nv2a_pb_draw.batches`). The backends add
  that number to the "batches" figure of the `[METAL]`/`[D3D11]` flip
  lines, because golden anchors find their screens by it. The first BLiNX 2
  Metal golden counted backend draws there, and it anchored attract-title
  and story-tsedit hundreds of flips late. `[PB-PERF]` gives the real
  backend draw count as `draws`.

## Risks

- A backend rule keyed to one batch's size or position (debug dumps keyed to
  one batch, `D3D11_DEBUG_PS_SKIP_POST`) sees merged lists; these are debug
  only.
- Frame identity rests on the backends drawing a list as they would the
  strips; the goldens check it on Metal and on the Deck's D3D11.
