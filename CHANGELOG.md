# Changelog

What changed for players in each release of the BLiNX: the time sweeper
static recompilation, newest first, in the style of
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/). Releases are
tagged `vYEAR.MONTH.N`, with `-rc.N` for release candidates, and each
release's section (`## [YEAR.MONTH.N] - YYYY-MM-DD`, the tag without its
v) is its notes on the
[releases page](https://github.com/apexJCL/blinx1-recomp/releases);
.github/workflows/release-wizard.yml stops when the tag has none.
No release contains game data: you build the game from your own disc.

## [Unreleased]

## [2026.10.0] - 2026-10-29

**BLiNX: the time sweeper, on your PC, Steam Deck and Mac.** The first
release: the build wizard turns your own disc image into a native game for
Windows, macOS and Linux (Steam Deck included). Early days: the logos, the
title and the attract demo run on macOS and the Steam Deck, and play past
the title is not fully checked yet, so expect crashes and report them.

- **The build wizard.** Download `blinx1-wizard-*` for your system, point
  it at your disc image, and it builds and installs the game.
- **CRT looks.** Shadow mask, scanlines and a gentle curve on the final
  picture: `crt-lottes`, `crt-lottes-fast`, `crt-hyllian`, or AMD's `fsr`
  upscaler. Metal and Direct3D 11, with nothing extra to install on
  Windows.
- **A Shaders page.** Pick a shader in the menu and tune its curvature,
  mask, scanlines and bloom live, with a Reset to preset. The page shows
  only where this system can run the shaders.
- **An in-game menu, on by default.** F10, Guide+A or Back+L3 held for a
  moment (and an Elite's P1 on macOS) opens it over the game: change the
  options while you play, rebind the host actions, turn mods on or off.
  Save keeps only what you changed. On a Steam Deck, SteamOS keeps
  Guide+A: use Back+L3, or bind R4 to F10 once in Steam Input (Back Grip
  Buttons > R4 > Keyboard > F10).
- **A stats strip.** F3 shows fps, frame time, pacing, scale and backend
  in the corner.
- **Sharper pictures.** Render scale up to 4x, anisotropic filtering, the
  present filter and fullscreen, in `config.toml` or the menu. The game
  draws as stock until you set one.
- **Draw distance mod.** On the Mods page: distant objects stay on screen
  up to 4 times further, so less pops in at high render scales.
- **Less disk wear.** The game's cache partition lives in memory (up to
  750 MB of RAM) instead of rewriting about 7 GB an hour on your disk.
- **Fast snow stages.** The snow, thousands of tiny draws a frame, now
  costs a fraction of the frame.
- **Controllers.** Hot-plug and rumble through the game's own USB pad path,
  and an 8BitDo pad over Bluetooth on the Mac no longer drops input while
  it vibrates.
- **Fixed:** movies and the attract demo run at full speed; the game no
  longer freezes with the sound stuck after a few minutes, runs out of
  memory loading its first stage, or hangs at boot waiting for the GPU.

### For developers

- **Pins:** xboxrecomp-cli v0.16.0 and toolkit blinx2-v0.9.0, the same as
  BLiNX 2's release. The game builds with `-DXBOXRECOMP_ENHANCE=ON`, so the
  menu, the options above and the mods are in every build.
- **Version scheme:** releases are dated (`vYEAR.MONTH.N`), as in
  BLiNX 2. A first release is not a promise of stability; reports from
  players decide what comes next.
- **Why the RAM cache is on for BLiNX 1:** the game rewrites its cache
  partition all the time. On the same route, a Mac run wrote 0.3 MB with
  it on and 284 MB with it off, for about 565 MiB more RAM. The toolkit's
  default stays off, and so do the other games for now; `ram_cache =
  false` under `[storage]` brings the disk back.
- **Why the menu is on by default:** closed, it draws nothing and takes
  no input but the Guide+A and Back+L3 chords, and players should find F10
  without editing a file. `enabled = false` under `[menu]` turns it off.
- **Shaders:** `present.shader` (`RECOMP_PRESENT_SHADER`, default `none`)
  runs a RetroArch slang preset through librashader v0.12.0, fetched at
  build time, pinned by sha256 and loaded at run time: without it the game
  logs one line and looks stock. On x86-64 Windows it is the toolkit's
  own D3D11-only build, which imports only system DLLs, so it needs no
  D3DX9 or Visual C++ runtime. Overrides live in
  `[present.shader_params.<preset>]`; my wrapper presets calm the
  upstream curvature.
- **Licences:** every package carries the full third-party notices. The
  Windows exe's MinGW-w64 and llvm-mingw runtime terms are in `LICENSES/`
  with a `NOTICE` naming what each one links; the macOS app ships each
  bundled dylib's licence in `Resources/LICENSES` with
  `BUNDLED-LIBRARIES.md`; librashader's MPL-2.0 text and crate notices sit
  in `shaders/`.
- **Game defaults that differ from the toolkit**, each for a measured
  reason (`docs/env.md`, `src/main.c`):
  - `RECOMP_VBLANK=paced`: the plain tick ran at about 43 Hz on the Mac,
    so movies and gameplay were slow. Paced gives 60 Hz, as in BLiNX 2.
  - `RECOMP_DISPATCH_GATE=1`: the game's DirectSound guards its voice
    lists only by raising IRQL, so a game thread and the sound DPC could
    edit one list at once and spin forever. The gate takes one lock on
    every raise to DISPATCH. A route that froze at vblank 5400 without it
    ran to 12000 with no underrun.
  - XBE section paging: with every section loaded at boot, 64 MB left too
    little heap for the first stage. Non-preload sections now load on
    `XeLoadSection`, as on the console.
- **Under the hood:** the pads come through the toolkit's USB pad source
  (the game reads them through the OHCI controller), with BLiNX 2's pad
  layer on top; the PGRAPH fence follows the XDK 5558 layout; the snow
  fix merges back-to-back triangle batches with the same draw state into
  one draw.
- **Not yet:** widescreen. A Hor+ cull fix is in, but `display.aspect`
  stays 4:3 until wider ratios are checked on BLiNX 1.
- **Native Windows is tested:** the wizard's fresh and patch builds passed
  on a stock Windows 11 PC.

[Unreleased]: https://github.com/apexJCL/blinx1-recomp/compare/v2026.10.0...HEAD
[2026.10.0]: https://github.com/apexJCL/blinx1-recomp/releases/tag/v2026.10.0
