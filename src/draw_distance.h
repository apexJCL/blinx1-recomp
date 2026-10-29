/*
 * draw_distance.h - the draw_distance mod: the game's far object cutoff
 * pushed out (openspec change draw-distance-mod). Off by default.
 *
 * Without the layer, or with a toolkit before mod API v3, draw_distance_mod
 * is NULL and the constant is never touched.
 */
#ifndef BLINX_DRAW_DISTANCE_H
#define BLINX_DRAW_DISTANCE_H

/* The table entry main.c registers before xbox_enhance_init; NULL when
 * the mod API v3 is not built. */
const void *draw_distance_mod(void);

/* Once per guest frame, on the guest thread: the toolkit's mod frame
 * (xbox_mod_frame), or nothing without the mod API. Called from the pad
 * poll wrapper in recomp_manual.c. */
void draw_distance_frame(void);

#endif
