/*
 * far_cull.c - the draw_distance mod's maths (far_cull.h; design D3).
 */
#include <string.h>
#include "far_cull.h"

uint32_t far_cull_bits(double distance)
{
    float f;
    uint32_t bits;

    /* NaN fails both tests: stock. */
    if (!(distance >= FAR_CULL_DIST_MIN))
        distance = FAR_CULL_DIST_MIN;
    else if (distance > FAR_CULL_DIST_MAX)
        distance = FAR_CULL_DIST_MAX;
    f = (float)(FAR_CULL_STOCK * distance);
    memcpy(&bits, &f, sizeof bits);
    return bits;
}

void far_cull_state_init(struct far_cull_state *st)
{
    st->checked = 0;
    st->dead = 0;
    st->have = FAR_CULL_STOCK_BITS;
}

enum far_cull_result far_cull_step(struct far_cull_state *st,
                                   volatile uint32_t *word, uint32_t want)
{
    uint32_t now;

    if (st->dead)
        return FAR_CULL_DEAD;
    if (want == st->have)
        return FAR_CULL_SAME;
    now = *word;
    if (!st->checked) {
        if (now != FAR_CULL_STOCK_BITS) {
            st->dead = 1;
            return FAR_CULL_NOT_STOCK;
        }
        st->checked = 1;
    } else if (now != st->have) {
        /* A patch, a cheat or a later fix writing the constant wins: the
         * mod neither overwrites nor "restores" a value it did not put
         * there. */
        st->dead = 1;
        return FAR_CULL_FOREIGN;
    }
    *word = want;
    st->have = want;
    return FAR_CULL_WROTE;
}
