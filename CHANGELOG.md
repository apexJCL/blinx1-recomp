# Changelog

What changed for players in each release, newest first, in the style of
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/). A release tag's
section is its GitHub release notes: before tagging v1.0.0, add
`## [1.0.0] - YYYY-MM-DD` with the changes (Added, Changed, Fixed,
Removed). .github/workflows/release-wizard.yml stops when the tag has none.

## [Unreleased]

### Added

- Options in `config.toml`: render scale (up to 4x), anisotropic filtering,
  the present filter and fullscreen. The game draws as stock until you set
  one.
- **An in-game menu**: F10, Guide+A or Back+L3 held for a moment (and an
  Elite's P1 on macOS) opens it over the game to change the options while
  you play, rebind the host actions and turn mods on or off. Save keeps
  only what you changed. The game keeps running behind it and gets no
  input. On a Steam Deck, SteamOS keeps Guide+A for itself: use Back+L3,
  or bind R4 to F10 once in Steam Input (Back Grip Buttons > R4 >
  Keyboard > F10).
- **A stats strip**: F3 shows fps, frame time, pacing, scale and backend in
  the window's corner; off until you press it.
- **Less disk wear**: the game's cache partition now lives in memory (up to
  750 MB of RAM). The game rewrites its cache all the time, about 7 GB an
  hour on the disk before this change. `ram_cache = false` under `[storage]`
  in `config/config.toml` brings the old behaviour back.

### Fixed

- Movies and the attract demo no longer run slow: the vblank timer ticked
  well under 60 Hz.
- The game no longer freezes with the sound stuck after a few minutes: the
  sound engine's voice lists were edited by two threads at once.
- The game no longer runs out of memory loading its first stage: its
  optional code and data sections load when the game asks for them, as on
  the Xbox, instead of all at boot.
- Boot no longer hangs waiting for the GPU, and memory blocks for the GPU
  and sound no longer overlap the game's own code and data.
