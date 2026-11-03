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

## [2026.11.1] - 2026-11-03

**The Windows build wizard works again, and your saves can travel.**
2026.11.0's wizard could stop partway on Windows; this patch fixes it
and brings a mouse-friendly menu along.

- **Windows wizard fixed.** Builds on a Windows PC with Git's default
  settings finish again: the toolkit's shader files stay LF, so the
  package step no longer refuses `shaders/param-kinds.toml`, text is
  written as UTF-8 whatever the system code page, and Retry works after
  a failed step.
- **Move your saves.** The menu's new Saves page exports your saves to a
  zip and imports one on another PC. Your current saves are backed up
  first, and the game closes so its next start loads the imported ones.
- **Share your settings.** The Files page exports your settings or a
  shader profile and imports one back, with a file browser in the menu.
- **The menu takes the mouse.** Hover, click, drag sliders and scroll;
  an on-screen keyboard pops up for renames on a touch screen.
- **A tidier menu.** Compact and expanded sizes (R3), a toolbar that
  folds to icons and button glyphs in the footer.

### For developers

- **Pins:** xboxrecomp-cli v0.20.0 and toolkit blinx2-v0.13.0, up from
  v0.19.0 and blinx2-v0.12.0, the same as BLiNX 2's 2026.11.1. The CLI
  writes every text file as UTF-8, clones the toolkit with
  `core.autocrlf=false` and heals an older CRLF clone; the toolkit's
  `.gitattributes` keeps packaged text LF. The bootstrap `blinx1.py` is the
  CLI's refreshed template.
- **Why a patch release:** the fix lives in the CLI and the toolkit, and
  the wizard runs the pinned CLI, so the pins had to move before players
  could build again. The toolkit's menu changes come along.
- **Mod API 7** (`glyph_name`, the `{ctl:}` token, a per-mod status
  panel) and **deterministic mode** (`RECOMP_DEBUG=deterministic`, off: a
  virtual clock, a fixed thread order and OHCI pads on that clock, so a
  pad replay repeats on one host) are in the toolkit; stock runs read
  the clock as before.

## [2026.11.0] - 2026-11-01

**Real console sound, cleaner CRT shaders and a friendlier menu.**

- **The real audio DSP, on by default.** The game's own sound programs now
  run on the emulated Xbox audio DSP (reverb, effect sends, the game's own
  mix) instead of a plain stereo fold. `RECOMP_APU_DSP=0` in
  `config/launch.env` brings the old sound back.
- **CRT shaders fixed at high scales.** The shader picture sits in the
  game's frame, and the scanlines no longer band at render scale 2 or 3.
- **Shader profiles.** Chain up to 8 shaders, tune them, save them by
  name on the Shaders page and switch between them. On/off effects are
  toggles now, not sliders.
- **A friendlier menu.** Changed rows are marked, Save and Revert work
  from any page, leaving with unsaved changes asks first, and the
  selected row explains itself in the footer.
- **Widescreen, one step closer.** With a wide `display.aspect` the seam
  at the old 4:3 edge is gone and the camera's lens flare reaches across
  the whole screen. The default stays 4:3 until wider ratios are checked
  through the game.

### For developers

- **Pins:** xboxrecomp-cli v0.19.0 and toolkit blinx2-v0.12.0, up from
  v0.16.0 and blinx2-v0.9.0, the same as BLiNX 2's release.
- **Hor+ fixes:** the toolkit no longer takes a post pass that reads the
  padded frame for a movie and puts it back at 4:3 (the seam), and the
  game's lens-flare wrapper draws over the side bands.
- **Lifter:** the toolkit lifts the 8- and 16-bit one-operand `mul`,
  `imul`, `div` and `idiv` as the CPU runs them. 21 sites in gen change;
  in one, a function was handed a float that the old lift had turned
  into 0 or NaN.
- **Why the DSP is on:** it is the faithful path; the stub skipped the
  game's effects mix. The default now lives in the toolkit for every
  game.
- **The RAM cache is now the toolkit's default for every game.**
  BLiNX 1 keeps `ram_cache = true` in its `config.default.toml`; nothing
  changes for players.
- **Per-title hard disk:** dev runs without `RECOMP_HDD_DIR` now use
  `<data>/xboxrecomp/<title id>`, copying old saves once from the shared
  root. Installed builds are unchanged.
- **Extraction:** `blinx1 extract` writes the owner's stages to a local
  Godot project from the `[extract]` table in `game.toml`; nothing it
  writes is shared.

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

[Unreleased]: https://github.com/apexJCL/blinx1-recomp/compare/v2026.11.1...HEAD
[2026.11.1]: https://github.com/apexJCL/blinx1-recomp/compare/v2026.11.0...v2026.11.1
[2026.11.0]: https://github.com/apexJCL/blinx1-recomp/compare/v2026.10.0...v2026.11.0
[2026.10.0]: https://github.com/apexJCL/blinx1-recomp/releases/tag/v2026.10.0
