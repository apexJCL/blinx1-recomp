## ADDED Requirements

### Requirement: Actionable Windows linker reports
The build guide SHALL identify the input XBE by SHA-256 and distinguish missing
generated definitions from definitions omitted from the link, without altering
game data or local changes.

#### Scenario: Unresolved generated wrapper body
- **WHEN** a Windows build reports an undefined `_gen` symbol
- **THEN** the guide requests the full log, XBE hash and source or wizard version
- **AND** it does not present a title ID match as proof of layout compatibility
