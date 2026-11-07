## ADDED Requirements

### Requirement: The diagonal clamp is opt-in
The `diag_clamp` mod SHALL default to off. While it is off, the pad states
SHALL never be written.

#### Scenario: Default
- **WHEN** neither `RECOMP_DIAG_CLAMP` nor `mods."diag_clamp.enable"` is set
- **THEN** the pad states are untouched and the game is stock

### Requirement: On, the left stick's length stops at 1
Switched on, the mod SHALL scale the left stick of each of the four pad
states to length 1 when it is longer, keeping its angle, after the pad poll
and before the player update reads it. A stick inside the unit circle, and
the right stick, SHALL be left as the game wrote them.

#### Scenario: Keyboard diagonal
- **WHEN** the mod is on and the player holds two keys of a diagonal
- **THEN** Blinx runs at speed 1.0, as straight ahead

#### Scenario: Square left stick
- **WHEN** the mod and `input.pad_square_left` are both on and the stick is at a diagonal
- **THEN** the stick reaches the corner on the host side and Blinx runs at speed 1.0

### Requirement: The menu explains it
The Mods page row SHALL be labelled "Clamp diagonal speed". Its help SHALL
say, within the tooltip's 159 bytes, that it caps diagonal speed at straight
speed and that, without it, full diagonals on the keyboard or with Square
left stick run 1.69x as fast.
The mod's About text SHALL say the same in a few plain sentences, say that
it matches BLiNX 2, and name the stick-shake side effect.

#### Scenario: Hovering the row
- **WHEN** the toolkit has `fix/mod-toggle-help` and the player selects the row on the Mods list, or the Enabled row on the mod's page
- **THEN** the switch's help shows as a tooltip under that row

#### Scenario: The About names the shake change
- **WHEN** the player opens the mod's page
- **THEN** the About says that a diagonal tapped from the centre no longer counts as a shake
