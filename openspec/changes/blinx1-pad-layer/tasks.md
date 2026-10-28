## 1. Toolkit hook (xboxrecomp `feat/ohci-pad-layer`)

- [x] 1.1 `usb_gamepad.h`: `UsbPadSource`, `usb_gamepad_set_source`,
  `usb_gamepad_present`, `usb_gamepad_detach`, `usb_gamepad_rumble`,
  `usb_gamepad_flips`
- [x] 1.2 `usb_gamepad.c`: the report reads the source; the toolkit
  script's pads join the present mask; rumble routing
- [x] 1.3 `ohci.c`: a plugged mask; hot-plug passes with a source; DEVICE
  NOT RESPONDING for a removed pad; rumble through `usb_gamepad_rumble`
- [x] 1.4 ctest `tests/usb_pad_source`: report bytes, present mask, rumble,
  plug and unplug on the root hub ports
- [x] 1.5 Mac build and `tests/posix_ctest.sh`

## 2. BLiNX 1 layer (blinx `feat/pad-layer`)

- [x] 2.1 `src/pad_slots.{c,h}` and `scripts/test_pad_slots.py`
- [x] 2.2 `src/pad_input.c`: the engine, the source, presets
- [x] 2.3 `src/env/recomp_env_game.h`, CMake wiring, `docs/env.md`
- [x] 2.4 `main.c`: `blinx1_pad_init()` before `xbox_OhciInit()`
- [x] 2.5 Build against the toolkit branch; `uv run pytest`, ruff

## 3. Runs

- [x] 3.1 Headless Mac run, a script to the title and the attract demo
- [x] 3.2 Host-pad path: no script, keyboard on, a run that reaches the title
  with the pad present
- [x] 3.3 `RECOMP_PAD_SCRIPT` alone still plugs pad 1 and presses
- [x] 3.4 JSRF and BLiNX 2 unaffected (no source installed / no OHCI)

## Follow-ups

- Lift the script engine and pad_slots into the toolkit so BLiNX 1 and
  BLiNX 2 share one copy, presets passed in by each game.
- Hands-on hot-plug with real pads on the Mac and the Deck.
- The `RECOMP_INPUT_DIAG` probe in usb_gamepad.c still reads
  `xbox_InputGetState` with a source installed.
- Without a source on the Mac (no script, no keyboard, no host pad) no pad
  is plugged, where the model used to plug pad 1 always: a pad plugged
  later arrives through the hot-plug path.
