## ADDED Requirements

### Requirement: The draw-distance mod is opt-in
The `draw_distance` mod SHALL default to off. While it is off and has not
written, the far-cull constant at `0x1D7394` SHALL keep the XBE's value.

#### Scenario: Default
- **WHEN** neither `RECOMP_DRAW_DISTANCE` nor `mods."draw_distance.enable"` is set
- **THEN** the constant is never written and the game is stock

### Requirement: On, the far cutoff scales
Switched on, the mod SHALL write `400 x distance` (distance 1..4) over the
far-cull constant, from the mod frame on the guest thread.

#### Scenario: Enabled at 2
- **WHEN** the mod is on with `distance` 2
- **THEN** `0x1D7394` holds 800.0f and spheres up to 800 units away pass the far cull

### Requirement: The write is guarded
The mod SHALL write only over the stock value (400.0f). If the constant is
not stock before the first write, or no longer holds the mod's last write,
the mod SHALL stop for the run and write nothing more, restore included.

#### Scenario: Foreign change
- **WHEN** something else writes the constant after the mod
- **THEN** the mod logs it once and never writes the constant again

### Requirement: Stock comes back
Turning the mod off and exiting SHALL put 400.0f back, unless the guard
stopped the mod.

#### Scenario: Off from the menu
- **WHEN** the mod is on and the player turns it off on the Mods page
- **THEN** the next mod frame writes 400.0f back
