/*
 * speed_meter.c - the readout of the speed_readout mod (speed_meter.h).
 */
#include <stdio.h>
#include "speed_meter.h"

/* No input the game reads reaches past 1.69 (the corner); 10 leaves room
 * for anything a later state writes there and still rejects a pointer
 * that is not the player's. */
#define SPEED_MAX 10.0f

void speed_meter_reset(struct speed_meter *m)
{
    m->at = 0;
    m->n = 0;
}

int speed_meter_ptr_ok(uint32_t p, uint32_t off, uint32_t ram)
{
    /* Each term is checked before the next subtracts it, so nothing wraps. */
    return p && !(p & 3u) && ram >= 4u && off <= ram - 4u && p <= ram - 4u - off;
}

void speed_meter_push(struct speed_meter *m, float s)
{
    /* The negated test is also true for NaN. */
    if (!(s >= 0.0f && s <= SPEED_MAX)) {
        speed_meter_reset(m);
        return;
    }
    m->s[m->at] = s;
    m->at = (m->at + 1) % SPEED_METER_FRAMES;
    if (m->n < SPEED_METER_FRAMES)
        m->n++;
}

int speed_meter_text(const struct speed_meter *m, char *out, size_t cap)
{
    float now, peak = 0.0f;

    if (!m->n)
        return snprintf(out, cap, "Speed\nnow -");
    now = m->s[(m->at + SPEED_METER_FRAMES - 1) % SPEED_METER_FRAMES];
    for (unsigned i = 0; i < m->n; i++)
        if (m->s[i] > peak)
            peak = m->s[i];
    return snprintf(out, cap, "Speed\nnow %.2f  peak %.2f", (double)now, (double)peak);
}
