/*
 * speed_readout.c - the speed_readout mod (speed_readout.h; openspec
 * change speed-readout).
 *
 * A debug readout for the diagonal speed (diag_clamp.c): the
 * player update runs at the left stick's length, so a straight push reads
 * 1.00 and a square corner 1.69, or 1.00 with diag_clamp on. The card has
 * to be its own mod: the toolkit drops a mod's card when the mod is off,
 * and the readout is wanted with the clamp off too.
 *
 * Where: [player_ptr] is the player object; the update sub_000D3D50 runs
 * for it with pad 1 (cmp esi, [player_ptr] at 0xD3DB7) and stores the run
 * speed at +player_run_speed (0xD4D46). The mod frame runs after
 * pad_poll_frame, so each frame reads what the last update stored. The
 * mod only reads: it never writes guest memory, and off it reads nothing.
 *
 * A release whose name map lacks player_ptr has no such mod.
 */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "speed_readout.h"
#include "speed_meter.h"
#include "gen/recomp_names.h"

#if defined(RECOMP_ENV_HAVE_ENHANCE_KEYS) && defined(__has_include)
#  if __has_include("mod.h")
#    include "mod.h"
#    if XBOX_MOD_API_VERSION >= 7
#      define SPEED_READOUT_MOD 1
#    endif
#  endif
#endif

#ifdef SPEED_READOUT_MOD
#if XBR_HAVE(player_ptr)   /* player_run_speed is in every map beside it */

#include "enhance_menu.h"

#define PLAYER_PTR_VA XBR_VA(player_ptr)       /* u32: the player object */
/* The object must sit in the retail console's 64 MB. */
#define GUEST_RAM     0x04000000u

enum { SET_ENABLE, SET_N };

static const char *const k_offon[] = { "off", "on", NULL };

static const struct xbox_mod_setting k_set[SET_N] = {
    /* name, key, env, aliases, label, help, unit, kind, live, flags,
     * def, lo, hi, step, choices */
    [SET_ENABLE] = { "enable", NULL, "RECOMP_SPEED_READOUT", NULL, "Speed readout",
      "Shows Blinx's run speed on screen: 1.00 straight, up to 1.69 on a"
      " full diagonal (keyboard, or a pad with Square left stick).",
      NULL, XBOX_SET_ENUM, ENHANCE_LIVE, XBOX_SET_ENABLE, 0, 0, 0, 0, k_offon },
};

static const struct xbox_mod_desc k_desc = {
    XBOX_MOD_DESC_VERSION, sizeof k_desc, "Speed readout",
    "Debug readout of Blinx's run speed. 1.00 is a full straight push; a"
    " diagonal at full tilt reaches 1.69 on the keyboard or a pad with Square"
    " left stick. Peak is the top of the last 30 polls (about a second)."
    " Outside a stage it shows the last value.",
    NULL, "1", k_set, SET_N, NULL, 0
};

static const struct xbox_mod s_mod;
static const struct xbox_mod_api *s_api;

/* Guest thread only: init, then the mod frame. */
static int s_on;
static struct speed_meter s_meter;
static char s_shown[SPEED_METER_TEXT];   /* the card's last text: set again only on a change */

/* The player's run speed, or -1 when there is no player to read. The
 * pointer is zero only before the first stage: from then on it keeps
 * pointing at a static player object (0x4FB260 in NTSC), so "now -" shows
 * only until then, and outside a stage the card keeps the last value.
 * Anything outside RAM is not the player. */
static float read_speed(void)
{
    uint32_t p = XBOX_MOD_MEM32(s_api, PLAYER_PTR_VA);
    float s;

    if (!speed_meter_ptr_ok(p, XBR_VAL(player_run_speed), GUEST_RAM))
        return -1.0f;
    memcpy(&s, (const void *)(s_api->mem + p + XBR_VAL(player_run_speed)), sizeof s);
    return s;
}

static void on_frame(void *user)
{
    char text[sizeof s_shown];

    (void)user;
    if (!s_on || !s_api->mem)
        return;
    speed_meter_push(&s_meter, read_speed());
    speed_meter_text(&s_meter, text, sizeof text);
    if (strcmp(text, s_shown) != 0 && s_api->status(&s_mod, text) == 0)
        memcpy(s_shown, text, sizeof s_shown);
}

static void on_enable(int on, void *user)
{
    (void)user;
    if (on == s_on)
        return;
    s_on = on;
    /* Off drops the card (the toolkit's); on starts a fresh second. */
    speed_meter_reset(&s_meter);
    s_shown[0] = 0;
    s_api->log(&s_mod, "%s", on ? "on" : "off");
}

static int init(const struct xbox_mod_api *api)
{
    if (api->size < offsetof(struct xbox_mod_api, status) + sizeof api->status)
        return 1;   /* no v7 status card: nothing to show it on */
    s_api = api;
    speed_meter_reset(&s_meter);
    /* Inited dormant (switched off, the menu able to turn it on) or on. */
    s_on = api->setting(&k_desc, SET_ENABLE) != 0;
    api->on_frame(on_frame, NULL);
    api->on_enable(on_enable, NULL);
    if (s_on)
        api->log(&s_mod, "on");
    return 0;
}

static const struct xbox_mod s_mod = { "speed_readout", init, NULL, &k_desc };

const void *speed_readout_mod(void)
{
    return &s_mod;
}

#else /* this release has no player_ptr: no mod */
const void *speed_readout_mod(void)
{
    return NULL;
}
#endif

#else /* no mod API v7: stock */

const void *speed_readout_mod(void)
{
    return NULL;
}

#endif
