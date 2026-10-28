## ADDED Requirements

### Requirement: The toolkit USB pad reads a game-installed source
The toolkit SHALL let a game install a pad source with
`usb_gamepad_set_source`. With a source installed, the emulated pad's
report SHALL carry the source's state for that pad, then the toolkit's
`RECOMP_PAD_PRESS`, `RECOMP_PAD_SCRIPT` and `RECOMP_PAD_LIVE` presses on top.
With none installed, every report, plug and rumble SHALL be as before.

#### Scenario: No source
- **WHEN** a title never calls `usb_gamepad_set_source`
- **THEN** pad 1 is plugged once the controller runs, `RECOMP_USB_PADS` pads in all, and reports come from `xbox_InputGetState`

#### Scenario: Toolkit script with a source
- **WHEN** a source is installed and `RECOMP_PAD_SCRIPT` names pad 1 and pad 2
- **THEN** both pads are plugged in even if the source reports neither, and the script's presses reach the reports

### Requirement: Pads come and go with the source
With a source installed, the root hub SHALL plug pad `d` on the root port
XAPI maps to controller port `d + 1` when the source reports it present,
one enumeration at a time, and SHALL unplug it after it has been absent for
about 250 ms, resetting its address and configuration.

#### Scenario: A second host pad
- **WHEN** a second host pad comes up while pad 1 is configured
- **THEN** pad 2 arrives on its port, and when the pad goes, pad 2 is removed

### Requirement: BLiNX 1 has BLiNX 2's input layer
BLiNX 1 SHALL read `RECOMP_INPUT_SCRIPT` (presets, files or inline steps in
BLiNX 2's grammar), `RECOMP_INPUT_STRICT` and `RECOMP_HOST_PAD`, map host pads
to ports with pad_slots, and merge the keyboard and mouse on port 0 through
the toolkit's KBM calls.

#### Scenario: Scripted boot
- **WHEN** `RECOMP_INPUT_SCRIPT=@attract` runs headless
- **THEN** the log shows the steps matching the title's file opens, and no host pad is read

#### Scenario: Strict
- **WHEN** `RECOMP_INPUT_STRICT=1` and the script names an unknown button
- **THEN** the run exits with status 2 before the guest starts
