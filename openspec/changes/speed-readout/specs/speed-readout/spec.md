## ADDED Requirements

### Requirement: The speed readout is opt-in and read-only
The `speed_readout` mod SHALL default to off. It SHALL never write guest
memory, on or off.

#### Scenario: Default
- **WHEN** neither `RECOMP_SPEED_READOUT` nor `mods."speed_readout.enable"` is set
- **THEN** no card is shown and the game is stock

### Requirement: On, the card shows the run speed
Switched on, the mod SHALL show the player's run speed (player+0x30) on
its status card, with the peak of the last 30 polls, and SHALL show
`now -` when there is no player to read, which is only before the first
stage: from then on the player pointer stays on a static object, and
outside a stage the card shows the last value.

#### Scenario: Keyboard diagonal, clamp off
- **WHEN** the mod is on, `diag_clamp` is off and the player holds W+D
- **THEN** the card reads about 1.69, as a full diagonal with Square left stick does; with it off, a round pad's diagonal reads 1.00

#### Scenario: Keyboard diagonal, clamp on
- **WHEN** the mod and `diag_clamp` are on and the player holds W+D
- **THEN** the card reads 1.00

### Requirement: The menu explains it
The Mods page row SHALL be labelled "Speed readout". Its help SHALL say,
within the tooltip's 159 bytes, that it shows Blinx's run speed, 1.00
straight and up to 1.69 on a full diagonal (keyboard or Square left
stick). The mod's
About text SHALL say that it is a debug readout, what 1.00 and 1.69 mean,
that the peak covers the last 30 polls (about a second), and that outside
a stage it shows the last value.

#### Scenario: Round pad
- **WHEN** the mod is on and a round pad's stick is held at a full diagonal
- **THEN** the card reads 1.00, and nothing in the help or About says otherwise
