/*
 * diag_clamp.c - the diag_clamp mod (diag_clamp.h; openspec change
 * diagonal-speed-clamp).
 *
 * The player update runs at the left stick's length (stick_clamp.h), so a
 * stick that reaches the square's corner runs 1.69 times as fast as one
 * pushed straight: the keyboard's diagonals, a square-gate pad, and a round
 * pad under the toolkit's input.pad_square_left. This is the faster
 * diagonal speedrunners use (Full Diagonal Movement). The mod
 * holds the stick to length 1, as BLiNX 2 does before its run formula, so
 * every input runs as a round-gate pad does.
 *
 * Where: pad_poll_frame (sub_000A2DE0) reads the pads into the array at
 * [pad_states], 0x1E4 bytes a pad, LX at +0x14 and LY at +0x18, clamps each
 * axis to [-1, 1] and writes the active pad's floats back to its state. The
 * mod frame runs right after it, from the wrapper in recomp_manual.c, and
 * before the frame's update reads the state (design D1). Each of the four
 * states is clamped: the player reads the active one, and a second player
 * object the next. Every reader of the stick then sees what a round pad
 * gives (design D2). The next poll rewrites every plugged-in pad's state,
 * so nothing has to be put back when the mod turns off; an unplugged
 * port's state keeps its last stick, clamped, which a round pad could
 * also have left there. Demo and replay input comes from the game's own
 * record, not these states, so it is never clamped and demos stay in sync.
 *
 * The right stick (the camera) is the game's.
 *
 * A release whose name map lacks pad_states has no such mod.
 */
#include <stddef.h>
#include <stdint.h>
#include "diag_clamp.h"
#include "stick_clamp.h"
#include "gen/recomp_names.h"

#if defined(RECOMP_ENV_HAVE_ENHANCE_KEYS) && defined(__has_include)
#  if __has_include("mod.h")
#    include "mod.h"
#    if XBOX_MOD_API_VERSION >= 3
#      define DIAG_CLAMP_MOD 1
#    endif
#  endif
#endif

#ifdef DIAG_CLAMP_MOD
#if XBR_HAVE(pad_states)   /* its three values are in every map beside it */

#include "enhance_menu.h"

#define PAD_STATES_VA XBR_VA(pad_states)       /* u32: the array's address */
#define PAD_COUNT     4u                       /* pad_poll_frame's loop */
/* The array must sit in the retail console's 64 MB. */
#define GUEST_RAM     0x04000000u

enum { SET_ENABLE, SET_N };

static const char *const k_offon[] = { "off", "on", NULL };

/* The help is the tooltip under the row, on the Mods list and the mod
 * page's Enabled row: three lines at most (the menu's D11 card), so the
 * whole story is in the summary, the mod page's About. A toolkit before
 * fix/mod-toggle-help (61c40406) shows no switch's help, only the About. */
static const struct xbox_mod_setting k_set[SET_N] = {
    /* name, key, env, aliases, label, help, unit, kind, live, flags,
     * def, lo, hi, step, choices */
    [SET_ENABLE] = { "enable", NULL, "RECOMP_DIAG_CLAMP", NULL, "Clamp diagonal speed",
      "Caps diagonal speed at straight speed. Without it, full diagonals"
      " (keyboard, or a pad with Square left stick) run 1.69x as fast.",
      NULL, XBOX_SET_ENUM, ENHANCE_LIVE, XBOX_SET_ENABLE, 0, 0, 0, 0, k_offon },
};

static const struct xbox_mod_desc k_desc = {
    XBOX_MOD_DESC_VERSION, sizeof k_desc, "Clamp diagonal speed",
    "BLiNX 1 runs full diagonals 1.69x as fast as straight, on the keyboard"
    " and on a pad with Square left stick (on by default). This mod caps"
    " them at straight speed, as in BLiNX 2."
    " Side effect: a diagonal tap no longer counts as a stick shake.",
    NULL, "1", k_set, SET_N, NULL, 0
};

static const struct xbox_mod s_mod;
static const struct xbox_mod_api *s_api;

/* Guest thread only: init, then the mod frame. */
static int s_on;
static int s_bad_logged;

static void on_frame(void *user)
{
    uint32_t base;

    (void)user;
    if (!s_on || !s_api->mem)
        return;
    base = XBOX_MOD_MEM32(s_api, PAD_STATES_VA);
    /* Zero before the game's pad init; anything outside RAM is not the
     * array, and the stick is left to the game. */
    if (!base || base > GUEST_RAM - PAD_COUNT * XBR_VAL(pad_state_size)) {
        if (base && !s_bad_logged) {
            s_bad_logged = 1;
            s_api->log(&s_mod, "pad states at 0x%08X: outside RAM, left alone",
                       (unsigned)base);
        }
        return;
    }
    for (uint32_t i = 0; i < PAD_COUNT; i++) {
        uint32_t p = base + i * XBR_VAL(pad_state_size);
        stick_clamp((float *)(s_api->mem + p + XBR_VAL(pad_left_x)),
                    (float *)(s_api->mem + p + XBR_VAL(pad_left_y)));
    }
}

static void on_enable(int on, void *user)
{
    (void)user;
    if (on == s_on)
        return;
    s_on = on;
    s_api->log(&s_mod, "%s", on ? "on: left stick held to length 1" : "off");
    /* This frame's on_frame (the same xbox_mod_frame, after this) clamps
     * the poll that just ran; off needs nothing put back, as the next poll
     * rewrites every plugged-in pad's state. */
}

static int init(const struct xbox_mod_api *api)
{
    if (api->size < offsetof(struct xbox_mod_api, on_setting))
        return 1;   /* no v3 setting getter: leave the stick alone */
    s_api = api;
    /* Inited dormant (switched off, the menu able to turn it on) or on. */
    s_on = api->setting(&k_desc, SET_ENABLE) != 0;
    api->on_frame(on_frame, NULL);
    api->on_enable(on_enable, NULL);
    if (s_on)
        api->log(&s_mod, "on: left stick held to length 1");
    return 0;
}

static const struct xbox_mod s_mod = { "diag_clamp", init, NULL, &k_desc };

const void *diag_clamp_mod(void)
{
    return &s_mod;
}

#else /* this release has no pad_states: no mod */
const void *diag_clamp_mod(void)
{
    return NULL;
}
#endif

#else /* no mod API v3: stock */

const void *diag_clamp_mod(void)
{
    return NULL;
}

#endif
