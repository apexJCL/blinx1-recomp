/*
 * stick_clamp.c - the length clamp of the diag_clamp mod (stick_clamp.h).
 */
#include <math.h>
#include "stick_clamp.h"

int stick_clamp(float *x, float *y)
{
    double fx = *x, fy = *y, m = sqrt(fx * fx + fy * fy);

    /* m > 1 is false for a NaN axis, and an infinite one would give 0 and
     * NaN: neither is a stick the pad read can make, so both stay as the
     * game wrote them. */
    if (!(m > 1.0) || !isfinite(m))
        return 0;
    *x = (float)(fx / m);
    *y = (float)(fy / m);
    return 1;
}
