/*
 * far_cull.h - the maths of the draw_distance mod (draw_distance.c;
 * openspec change draw-distance-mod), pure C so
 * scripts/test_draw_distance.py can drive it.
 *
 * sub_0009AD80, the game's bounding-sphere test, ends with a far cull:
 * while the flag at 0x3AE708 is set it rejects a sphere whose near side is
 * more than [0x1D7394] (400.0f) in front of the camera. That fcomp at
 * 0x9AFF3 is the constant's only reader (design D1), so the mod writes the
 * constant itself.
 */
#ifndef BLINX_FAR_CULL_H
#define BLINX_FAR_CULL_H

#include <stdint.h>

#define FAR_CULL_VA          0x1D7394u     /* float, .rdata */
#define FAR_CULL_STOCK_BITS  0x43C80000u   /* 400.0f as the XBE ships it */
#define FAR_CULL_STOCK       400.0
#define FAR_CULL_DIST_MIN    1.0
#define FAR_CULL_DIST_MAX    4.0           /* 1600: still inside the far plane (2048) */

/* The constant's bits for this distance factor: 400 x distance, clamped
 * to [FAR_CULL_DIST_MIN, FAR_CULL_DIST_MAX]. A factor that is not a
 * number gives stock. */
uint32_t far_cull_bits(double distance);

/* The mod's hold on the constant (design D3). have is what it last wrote,
 * stock at the start, so an off mod that never wrote writes nothing. */
struct far_cull_state {
    int checked;        /* the stock value was seen */
    int dead;           /* not ours any more: no write this run */
    uint32_t have;
};

enum far_cull_result {
    FAR_CULL_SAME,      /* want == have: nothing read or written */
    FAR_CULL_WROTE,     /* the constant now holds want */
    FAR_CULL_DEAD,      /* stopped earlier: nothing read or written */
    FAR_CULL_NOT_STOCK, /* first write found a value that is not 400.0f */
    FAR_CULL_FOREIGN    /* the constant no longer holds have */
};

void far_cull_state_init(struct far_cull_state *st);

/* Bring the constant at word (the guest's FAR_CULL_VA) to want. Before
 * the first write it must be stock, after it it must still be have;
 * otherwise the state goes dead and the constant is never touched again,
 * restore included. */
enum far_cull_result far_cull_step(struct far_cull_state *st,
                                   volatile uint32_t *word, uint32_t want);

#endif
