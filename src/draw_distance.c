/*
 * draw_distance.c - the draw_distance mod (draw_distance.h; openspec change
 * draw-distance-mod).
 *
 * The game rejects every object whose bounding sphere starts more than 400
 * units in front of the camera (the far cull in sub_0009AD80, far_cull.h).
 * At 480 lines the cutoff hid pop-in; at render scale 2x and 3x it shows.
 * The mod writes 400 x distance over that constant.
 *
 * Fog and the projection's far plane stay the game's: the far plane is 2048
 * (sub_00092470), past the largest cutoff the mod writes (design D2).
 *
 * Every write runs on the guest's thread from the mod frame (on_frame and
 * the on_enable callback both come from xbox_mod_frame), after the pad poll
 * and before the frame's draws.
 */
#include <stddef.h>
#include "draw_distance.h"
#include "far_cull.h"

#if defined(RECOMP_ENV_HAVE_ENHANCE_KEYS) && defined(__has_include)
#  if __has_include("mod.h")
#    include "mod.h"
#    if XBOX_MOD_API_VERSION >= 3
#      define DRAW_DISTANCE_MOD 1
#    endif
#  endif
#endif

#ifdef DRAW_DISTANCE_MOD

#include "enhance_menu.h"

enum { SET_ENABLE, SET_DISTANCE, SET_N };

static const char *const k_offon[] = { "off", "on", NULL };

static const struct xbox_mod_setting k_set[SET_N] = {
    /* name, key, env, aliases, label, help, unit, kind, live, flags,
     * def, lo, hi, step, choices */
    [SET_ENABLE] = { "enable", NULL, "RECOMP_DRAW_DISTANCE", NULL, "Longer draw distance",
      "Objects stay on screen further away instead of popping in at the"
      " game's 400-unit cutoff. Costs more draws.",
      NULL, XBOX_SET_ENUM, ENHANCE_LIVE, XBOX_SET_ENABLE, 0, 0, 0, 0, k_offon },
    [SET_DISTANCE] = { "distance", NULL, "RECOMP_DRAW_DISTANCE_SCALE", NULL, "Draw distance",
      "How far objects are drawn, as a multiple of the game's cutoff:"
      " 2 draws them twice as far.",
      "x", XBOX_SET_FLOAT, ENHANCE_LIVE, 0, 2, FAR_CULL_DIST_MIN, FAR_CULL_DIST_MAX,
      0.25, NULL },
};

static const struct xbox_mod_desc k_desc = {
    XBOX_MOD_DESC_VERSION, sizeof k_desc, "Draw distance",
    "Objects are drawn further away than the game's cutoff", NULL, "1",
    k_set, SET_N, NULL, 0
};

static const struct xbox_mod s_mod;
static const struct xbox_mod_api *s_api;

/* Guest thread only (init, then the mod frame), but for the best-effort
 * restore at exit (shutdown_mod). */
static int s_on;
static struct far_cull_state s_state;

/* The constant to want when it differs from the last write (far_cull_step:
 * the guards of design D3), and the log line that says so. */
static void put(uint32_t want, double dist)
{
    volatile uint32_t *w;

    if (!s_api->mem)
        return;
    w = (volatile uint32_t *)(s_api->mem + FAR_CULL_VA);
    switch (far_cull_step(&s_state, w, want)) {
    case FAR_CULL_SAME:
    case FAR_CULL_DEAD:
        break;
    case FAR_CULL_WROTE:
        if (want != FAR_CULL_STOCK_BITS)
            s_api->log(&s_mod, "far cutoff %g (distance %g)",
                       (double)XBOX_MOD_MEMF(s_api, FAR_CULL_VA), dist);
        else
            s_api->log(&s_mod, "stock far cutoff back");
        break;
    case FAR_CULL_NOT_STOCK:
        s_api->log(&s_mod, "far cutoff at 0x%X is not the stock 400 (0x%08X);"
                   " left alone", FAR_CULL_VA, (unsigned)*w);
        break;
    case FAR_CULL_FOREIGN:
        s_api->log(&s_mod, "far cutoff at 0x%X changed outside the mod (0x%08X);"
                   " left alone", FAR_CULL_VA, (unsigned)*w);
        break;
    }
}

static void on_frame(void *user)
{
    double dist;

    (void)user;
    if (s_state.dead)
        return;
    if (!s_on) {
        put(FAR_CULL_STOCK_BITS, 1.0);  /* a no-op unless the mod wrote last */
        return;
    }
    dist = s_api->setting(&k_desc, SET_DISTANCE);
    put(far_cull_bits(dist), dist);
}

static void on_enable(int on, void *user)
{
    (void)user;
    if (on == s_on)
        return;
    s_on = on;
    s_api->log(&s_mod, "%s", on ? "on" : "off");
    /* The frame that follows (this one: on_enable runs from the same
     * xbox_mod_frame, before the frame hooks) writes or restores. */
}

static int init(const struct xbox_mod_api *api)
{
    if (api->size < offsetof(struct xbox_mod_api, on_setting))
        return 1;   /* no v3 setting getter: leave the constant alone */
    s_api = api;
    far_cull_state_init(&s_state);
    /* Inited dormant (switched off, the menu able to turn it on) or on. */
    s_on = api->setting(&k_desc, SET_ENABLE) != 0;
    api->on_frame(on_frame, NULL);
    api->on_enable(on_enable, NULL);
    if (s_on)
        api->log(&s_mod, "on: distance %g", api->setting(&k_desc, SET_DISTANCE));
    return 0;
}

/* Exit: the stock cutoff back, best effort. This can run on the exit hook's
 * thread while a mod frame is still in flight, which may then write its
 * value again or log a "changed outside the mod" line; neither matters,
 * since guest memory goes with the process. s_on is left to the guest
 * thread. */
static void shutdown_mod(void)
{
    put(FAR_CULL_STOCK_BITS, 1.0);
}

static const struct xbox_mod s_mod = { "draw_distance", init, shutdown_mod, &k_desc };

const void *draw_distance_mod(void)
{
    return &s_mod;
}

void draw_distance_frame(void)
{
    xbox_mod_frame();
}

#else /* no mod API v3: stock */

const void *draw_distance_mod(void)
{
    return NULL;
}

void draw_distance_frame(void)
{
}

#endif
