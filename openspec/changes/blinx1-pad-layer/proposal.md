## Why

BLiNX 1 reaches its pad through the toolkit's emulated USB controller
(`src/usb/ohci.c`): the title statically links XAPI, XAPI drives the OHCI
registers, and `usb_gamepad_report` answers the 20-byte report from
`xbox_InputGetState`. That path has a timed pad script (`RECOMP_PAD_SCRIPT`)
and nothing else. BLiNX 2, which overrides XInput instead, has the input layer
the bench and players rely on:

- scripts that wait on the game rather than on the clock (`wait open`,
  `wait mem`, `tap ... until`) and named presets;
- pad slots: the first host pad drives player 1 whatever its slot, a
  replugged pad keeps its port, Steam's inert virtual pad is skipped;
- one merge of the host pad, the script and the keyboard/mouse.

On BLiNX 1 the emulated pad 1 is always plugged in and reads host slot 0,
there is no hot-plug, and a script cannot wait for a screen.

## What changes

- **Toolkit (game-agnostic hook).** `usb_gamepad.h` gains a pad source a
  game can install (`usb_gamepad_set_source`): a state callback the report
  reads instead of `xbox_InputGetState`, a mask of the pads that are present
  now, and a rumble callback. With a source installed the OHCI root hub
  plugs and unplugs pads as that mask changes, one enumeration at a time,
  on the ports XAPI maps to controller ports 1-4. Without one, nothing
  changes. The toolkit's own `RECOMP_PAD_SCRIPT`, `RECOMP_PAD_LIVE`,
  `RECOMP_PAD_PRESS` and the pad recorder still apply on top of the report,
  and keep the pads they name plugged in.
- **BLiNX 1 (the layer).** `src/pad_input.c` and `src/pad_slots.c`, ported
  from BLiNX 2, implement the source: the step-script engine
  (`RECOMP_INPUT_SCRIPT`, BLiNX 1 presets), pad slots, the keyboard/mouse
  merge through the toolkit's `xbox_InputKbmPoll`/`xbox_InputMergeKbm`, the
  enhancements pad filter, and rumble to the slot behind a port. Game keys
  `RECOMP_INPUT_SCRIPT`, `RECOMP_INPUT_STRICT`, `RECOMP_HOST_PAD`,
  `RECOMP_TRACE=input` and `RECOMP_DEBUG=pad_peek` join the env table
  through `src/env/recomp_env_game.h`, each with a row in `docs/env.md`.

## Impact

- Toolkit: `src/usb/usb_gamepad.{h,c}`, `src/usb/ohci.c`, a new ctest
  `tests/usb_pad_source`. No change for a title that installs no source
  (JSRF, Burnout 3, SSX 3, any other OHCI title); BLiNX 2 does not use the
  OHCI model.
- BLiNX 1: new `src/pad_input.c`, `src/pad_slots.{c,h}`,
  `src/env/recomp_env_game.h`, `docs/env.md`, `scripts/test_pad_slots.py`;
  `main.c` installs the source before `xbox_OhciInit`; CMake wiring.
- Behaviour: pad 1 is present while it has a source (the keyboard, which the
  macOS launch defaults turn on, a script, or a host pad), as on BLiNX 2,
  instead of always. More pads appear and disappear with the host pads.
