## Why

With `display.aspect` wider than 4:3 (Hor+), BLiNX 1 pads the 4:3 render
targets out to the display ratio on the host, so the screen shows NDC x out
to +-A/(4/3). The game's own visibility test still stops at the 4:3 edge:
objects in the new side bands are culled and pop in only when they cross the
old edge. BLiNX 2 had the same bug and fixed it in the tracker item (cat
`17e6f3d`) by widening its main-view side planes.

## What changes

- `src/recomp_manual.c` wraps `sub_0009AD80`, the bounding-sphere
  visibility test the model and node draws call before drawing. With a wide
  `display.aspect` the wrapper scales the stored projection's `_11`
  (0x3AE6A0) by (4/3)/A for the call and restores it after, so the test
  keeps whatever the padded screen shows. It logs one `[HORCULL]` line.
- At 4:3, at narrower ratios and in builds without the enhancements layer
  the wrapper calls the generated body alone: stock behaviour is unchanged.

## Impact

- No new env key or config key: `display.aspect` (`RECOMP_DISPLAY_ASPECT`)
  already drives it.
- The side bands draw more objects at wide ratios (batches per flip up; see
  design.md for the numbers).
- The GPU projection is not touched: the device keeps its own copy of the
  matrix.
