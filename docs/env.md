# Environment variables

The runtime reads every variable through one table, `RECOMP_ENV_KEYS` in the
toolkit's `src/platform/recomp_env.h`, once at startup; the toolkit's own keys
are documented there (`RECOMP_TRACE=help` or `RECOMP_DEBUG=help` prints the
whole table). This page lists only this game's own keys, defined in
`src/env/recomp_env_game.h`, which the toolkit table pulls in through its
`RECOMP_ENV_GAME_KEYS` hook (CMakeLists.txt sets `RECOMP_ENV_HAVE_GAME_KEYS`
on the `recomp_env` target).

Tiers as in the toolkit: config keys are their own variables, trace keys go
in `RECOMP_TRACE=...`, debug keys in `RECOMP_DEBUG=...`.

## Toolkit defaults this game changes

`RECOMP_ENV_GAME_DEFAULTS` in `src/env/recomp_env_game.h` sets these toolkit
keys when neither the variable nor its old spelling is set; setting the
variable still wins.

| Name | Default here | Why |
|---|---|---|
| `RECOMP_DISPATCH_GATE` | `1` | DirectSound guards its voice lists with raised IRQL alone, which parallel guest threads break: a corrupted list hangs the game at raised IRQL |
| `RECOMP_STORAGE_RAM_CACHE` | `1`, through `storage.ram_cache = true` in `packaging/config.default.toml`, not this table | the game rewrites a ring of 300 cache files of 1.9 MB each, about 7 GB of host writes an hour; in memory it costs up to the stock 750 MB cap of RAM and writes nothing. It is a config default, not a `RECOMP_ENV_GAME_DEFAULTS` row, because a value from that row counts as the environment and would beat `ram_cache = false` in `config/config.toml` |

## Config

| Name | Meaning | Read at |
|---|---|---|
| `RECOMP_INPUT_SCRIPT` | scripted pad on port 1: `@preset` (`@skip-intro`: START through the logos, ends idle on the title; `@attract`: the same, then waits for the attract demo), a file of steps, or an inline script (`wait open microsoft.sfd; tap START until open h_title02a.sfd`); grammar in `src/pad_input.c`. A script turns the host pads and the keyboard and mouse off unless the environment names them | `src/pad_input.c:779` |
| `RECOMP_INPUT_STRICT` | `1`: a script that does not parse, or names an unknown preset, ends the run with exit status 2 before the guest starts | `src/pad_input.c:799` |
| `RECOMP_HOST_PAD` | `0`/`1`: host gamepads in or out. Unset: in, unless `RECOMP_INPUT_SCRIPT` is set. The first host pad up drives port 1 whatever its XInput or SDL slot, later ones the next free ports, a replugged one its old port; the first pad pressed takes the lowest port held by one never pressed, such as Steam's inert virtual pad (`[INPUT] ... pads=slot:port`, `active=`). A pad pulled from the host is pulled from the title | `src/pad_input.c:813` |
| `RECOMP_DRAW_DISTANCE` | `mods."draw_distance.enable"`: `off` (stock) or `on`; enhancements layer only. `on` turns on the `draw_distance` mod (`src/draw_distance.c`, openspec change draw-distance-mod): the game's far object cutoff, the constant at `0x1D7394` (400) that `sub_0009AD80` compares with the near side of each bounding sphere, becomes `400 x distance`. Written only over the stock value; a foreign change stops the mod for the run; turning it off or exiting puts 400 back. Set by env, the setting is read-only on the Mods page | `src/draw_distance.c:41` |
| `RECOMP_DRAW_DISTANCE_SCALE` | `mods."draw_distance.distance"`: `1` to `4` in steps of `0.25` (default `2`), the cutoff as a multiple of 400; out of range is clamped and snapped with one `[MOD]` line | `src/draw_distance.c:45` |

## Trace

| Old name | Key | Meaning | Read at |
|---|---|---|---|
| `RECOMP_INPUT_TRACE` | `input` | one `[INPUT] ... host port=` line per poll in which a host pad changed, one `[INPUT] rumble` line per change of the motor words, and one `[INPUT] open <path>` line per file the game opens (what a `wait open` can name). The `sources:`, `ports=`, scripted `buttons=` and `script done` lines print without it | `src/pad_input.c:891` |

## Debug

| Key | Meaning | Read at |
|---|---|---|
| `pad_peek` | `=va:n[:x],...`: guest dwords (float, or hex with `:x`) printed on every scripted pad poll; up to 8 ranges of 32 dwords | `src/pad_input.c:1025` |

Two toolkit debug keys (documented in the toolkit table) were made for
this game's stage 2 snow: `RECOMP_DEBUG=pb_merge=0` draws every
guest batch on its own, without merging back-to-back triangle batches into
one backend draw (the A/B arm for the snow's frame rate), and
`RECOMP_DEBUG=pb_mlog=N` prints every method of flip N as `[MLOG]` lines,
which showed the snow's strips follow each other with no method between.

The toolkit's `RECOMP_PAD_PRESS` and `RECOMP_DEBUG=pad_script=` (the older
`RECOMP_PAD_SCRIPT` spelling still works, with a deprecation line) press on
top of the game's layer, in the toolkit's USB pad model.
