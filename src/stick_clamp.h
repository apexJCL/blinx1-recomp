/*
 * stick_clamp.h - the maths of the diag_clamp mod (diag_clamp.c; openspec
 * change diagonal-speed-clamp), pure C so scripts/test_diag_clamp.py can
 * drive it.
 *
 * The game reads each stick axis on its own (sub_00076E20: (v + 0.5) /
 * 32767.5, zeroed under 0.25) and pad_poll_frame clamps each axis to
 * [-1, 1], so a stick can reach the square [-1, 1]^2. The player update
 * (sub_000D3D50, 0xD49EE) runs at the stick's length less 0.4, times 5/3,
 * with no clamp: a full corner (1, 1) gives 1.69 where a cardinal gives
 * 1.0. BLiNX 2 sets the length to 1 past 1 before its own formula (its walk
 * code at 0x17C5D1 and 0x17C712). The mod does the same to the stick
 * itself: it keeps the angle and scales the length down to 1, which is what
 * a round-gate pad reports at the rim.
 */
#ifndef BLINX_STICK_CLAMP_H
#define BLINX_STICK_CLAMP_H

/* Scale (*x, *y) by 1 / max(1, |(x, y)|). Returns 1 when it scaled, 0 when
 * the stick was inside the unit circle or not a number (left as it is). */
int stick_clamp(float *x, float *y);

#endif
