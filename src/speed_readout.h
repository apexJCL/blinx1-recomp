/*
 * speed_readout.h - the speed_readout mod: Blinx's run speed on the mod
 * status card, a debug readout for the diagonal speed (openspec change
 * speed-readout). Off by default.
 *
 * Without the layer, or with a toolkit before mod API v7 (the status
 * card), speed_readout_mod is NULL and the game is never read.
 */
#ifndef BLINX_SPEED_READOUT_H
#define BLINX_SPEED_READOUT_H

/* The table entry main.c registers before xbox_enhance_init; NULL when
 * the mod API v7 is not built or this release lacks the player names. */
const void *speed_readout_mod(void);

#endif
