/*
 * diag_clamp.h - the diag_clamp mod: the left stick's length held to 1, so
 * a diagonal runs no faster than straight ahead, as in BLiNX 2 (openspec
 * change diagonal-speed-clamp). Off by default.
 *
 * Without the layer, or with a toolkit before mod API v3, diag_clamp_mod is
 * NULL and the pad states are never touched.
 */
#ifndef BLINX_DIAG_CLAMP_H
#define BLINX_DIAG_CLAMP_H

/* The table entry main.c registers before xbox_enhance_init; NULL when
 * the mod API v3 is not built or this release lacks the pad names. */
const void *diag_clamp_mod(void);

#endif
