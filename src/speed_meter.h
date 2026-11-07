/*
 * speed_meter.h - the readout of the speed_readout mod (speed_readout.c;
 * openspec change speed-readout), pure C so scripts/test_speed_readout.py
 * can drive it.
 *
 * The player update (sub_000D3D50) stores the run speed s = (m - 0.4) x
 * 5/3 at player+0x30, m the left stick's length: 1.00 for a full straight
 * push, 1.69 for the square's corner. The meter keeps the last
 * SPEED_METER_FRAMES samples: the mod frame runs once a pad poll, so 30
 * polls, about a second in an NTSC stage (the tracker item's runs), 1.2 s PAL. It writes the current one and their peak.
 */
#ifndef BLINX_SPEED_METER_H
#define BLINX_SPEED_METER_H

#include <stddef.h>
#include <stdint.h>

#define SPEED_METER_FRAMES 30
/* The card's text buffer, NUL included: far more than the longest text,
 * "Speed\nnow 10.00  peak 10.00". */
#define SPEED_METER_TEXT 64

struct speed_meter {
    float s[SPEED_METER_FRAMES];
    unsigned at;    /* next slot */
    unsigned n;     /* samples held, up to SPEED_METER_FRAMES */
};

void speed_meter_reset(struct speed_meter *m);

/* 1 when p, a guest pointer to the player, can be read at p + off as a
 * float in a guest RAM of ram bytes: nonzero (zero before the game makes
 * its first player), 4-byte aligned, and the float wholly inside. */
int speed_meter_ptr_ok(uint32_t p, uint32_t off, uint32_t ram);

/* One frame's speed. A value the update cannot store (not a number,
 * negative, or past 10) is not a speed, so it reads as no player: the
 * meter empties. */
void speed_meter_push(struct speed_meter *m, float s);

/* The card's text, title first: "Speed\nnow 1.69  peak 1.69", or
 * "Speed\nnow -" when empty. Returns snprintf's length. */
int speed_meter_text(const struct speed_meter *m, char *out, size_t cap);

#endif
