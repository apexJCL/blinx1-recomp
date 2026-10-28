/*
 * recomp_env_game.h - this game's own environment keys, added to the
 * toolkit's table (xboxrecomp src/platform/recomp_env.h, which includes this
 * file when RECOMP_ENV_HAVE_GAME_KEYS is defined; CMakeLists.txt sets it on
 * the recomp_env target). Same row format: X(ID, TIER, NAME, LEGACY, HELP).
 * Inventory with meanings: docs/env.md.
 */
#ifndef RECOMP_ENV_GAME_H
#define RECOMP_ENV_GAME_H

/* The pad layer (src/pad_input.c). */
#define RECOMP_ENV_GAME_KEYS(X) \
    X(INPUT_SCRIPT,      CONFIG, "RECOMP_INPUT_SCRIPT",      NULL, "scripted pad: @preset, file or inline script") \
    X(INPUT_STRICT,      CONFIG, "RECOMP_INPUT_STRICT",      NULL, "1: a bad input script ends the run") \
    X(HOST_PAD,          CONFIG, "RECOMP_HOST_PAD",          NULL, "0/1: host gamepads in or out") \
    X(INPUT_TRACE,       TRACE, "input",           "RECOMP_INPUT_TRACE",       "pad states, slots, rumble and file opens") \
    X(PAD_PEEK,          DEBUG, "pad_peek",        NULL,                       "=va:n[:x],...: guest dwords on every scripted pad poll")

/* RECOMP_DEBUG keys whose values hold commas. */
#define RECOMP_ENV_GAME_COMMA_KEYS "pad_peek",

/* Toolkit keys this game changes the default of; an explicit variable still
 * wins (recomp_env.h, RECOMP_ENV_GAME_DEFAULTS).
 *
 * DISPATCH_GATE: the XDK DirectSound in this title guards its voice lists
 * (this+0x488) with nothing but KeRaiseIrql(DISPATCH), which is exclusive
 * only on the console's one CPU. With guest threads running in parallel on
 * the host, a list can change under another user: most likely a game thread
 * edits it while the DSOUND DPC (the timer thread, through
 * KeSynchronizeExecution) walks it, or two threads edit it at once. A voice
 * is then left unlinked with Flink pointing at itself while the head still
 * reaches it, and the service loop sub_001868EB (restart from the head on a
 * self-linked node) spins at raised IRQL for good: the game freezes, audio
 * underruns, and OHCI logs "guest IRQL never dropped" (the watchdog
 * showed billions of calls to the voice method 0x001893FE). The gate makes
 * DISPATCH one-CPU again. */
#define RECOMP_ENV_GAME_DEFAULTS(D) \
    D(DISPATCH_GATE, "1")

#endif
