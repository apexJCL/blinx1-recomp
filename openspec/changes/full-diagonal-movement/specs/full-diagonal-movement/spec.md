## ADDED Requirements

### Requirement: Square sticks are opt-in
`input.pad_square_left` and `input.pad_square_right` SHALL default to off.
With both off, the pad setup SHALL equal the identity, and the guest pad
SHALL be bit for bit what it was before the change.

#### Scenario: Default
- **WHEN** neither knob nor `RECOMP_INPUT_PAD_SQUARE_LEFT` / `_RIGHT` is set
- **THEN** no pad setup is installed for them and the sticks pass unchanged

### Requirement: A round diagonal reaches the corner
With a stick's flag on, the stick SHALL be scaled along its angle by
`k' = 1 + (k - 1) · t`, where `k = max(1, min(r, 32767) / max(|x|, |y|))`,
`r` is the stick's length and `t` is a smoothstep of `min(r, 32767) / 32767`
from 0.7 to 1, so the stretch acts only at the rim. The result SHALL clamp
to the int16 range.

#### Scenario: Partial tilt
- **WHEN** the stick's length is under 0.7 of full, at any angle
- **THEN** the guest reads the same values

#### Scenario: Full diagonal
- **WHEN** the host left stick reads (23170, 23170) with the flag on
- **THEN** the guest reads (32767, 32767)

#### Scenario: Cardinals and -32768
- **WHEN** the stick reads (32767, 0), (0, -32768) or (-32768, -32768)
- **THEN** the guest reads the same values

### Requirement: Square after the deadzone, per stick
The mapping SHALL run after the deadzone (`RECOMP_PAD_DEADZONE` or
`input.pad_deadzone_*`), the stick swap and the alternate map. Each flag
SHALL apply to its own guest stick only.

#### Scenario: Inside the deadzone
- **WHEN** the stick is inside the radial deadzone
- **THEN** the guest stick is 0 with the flag on as well as off

#### Scenario: One stick
- **WHEN** only `input.pad_square_left` is on
- **THEN** the right stick passes unchanged
