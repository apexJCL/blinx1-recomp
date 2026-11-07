# Playing BLiNX: the time sweeper

The player's reference: what you need, building by hand, installing each
bundle, the options and the in-game menu, and what to do when something
goes wrong. The [README](../README.md) has the short route (the build
wizard); the developers' side is in [developers.md](developers.md).

## What you need

| Item | Why |
|---|---|
| Your own, legally obtained copy of BLiNX: the time sweeper: a disc image (`.iso`) or the extracted folder with `default.xbe` | The game's code and assets come from it. This project does not provide or link to them, and refuses other titles. |
| A build machine: a Mac (Apple silicon), a Linux PC or a Windows PC, with about 15 GB free | The bundle is built from your dump on your machine. A Mac can build every target; Linux and Windows build the Windows and SteamOS bundles. |
| The build wizard, or [uv](https://docs.astral.sh/uv/getting-started/installation/), git and Python 3.9 or newer | The wizard fetches what it needs itself. By hand, `./blinx1` fetches everything else (CMake, Ninja, the compiler, the toolkit), each download checked against a pinned sha256. |
| A controller, or the keyboard | A pad is player 1. |

## Platforms

| You play on | Bundle | Graphics | Audio | Status |
|---|---|---|---|---|
| Steam Deck (stock SteamOS), or a Linux PC with Proton | `steamos` (the Windows build, run under Proton through `umu-run`) | Direct3D 11 (DXVK) | XAudio2 (FAudio) | Boots to the title and the attract demo. |
| macOS (Apple silicon) | `macos` (`.dmg`) | Metal | SDL3 (CoreAudio) | Boots to the title and the attract demo. |
| Windows x86-64 | `windows` (a setup program) | Direct3D 11 | XAudio2 | Builds, but **untested on native Windows**. |

The game draws at 640x480 and 4:3, as on the Xbox; a wide window or screen
is pillarboxed, never stretched. The render scale draws it sharper (see [Options](#options)).

## Build the game

**The short route: the build wizard.** From the first release on, each
[release](https://github.com/apexJCL/blinx1-recomp/releases) has a small
program for Windows, macOS and Linux (Steam Deck included): download yours,
point it at your disc image, pick the options, and it builds and installs
the game for you. It runs the same commands as below, and offers a small
update build for a device that already has one. Windows SmartScreen and
macOS Gatekeeper warn about it (it is not signed): on Windows, More info >
Run anyway; on macOS, right-click > Open once.

**By hand.** Get this project, then go into its folder (in Terminal on macOS
and Linux, or in PowerShell on Windows):

```sh
git clone https://github.com/apexJCL/blinx1-recomp.git
cd blinx1-recomp
./blinx1 /path/to/your/BLiNX.iso
```

In PowerShell that is `.\blinx1 C:\path\to\BLiNX.iso`. On Windows, keep the
path short (`C:\b1`) and run `git config --global core.longpaths true` once
before the clone.

It checks that the disc is BLiNX: the time sweeper, puts its files in
`game_files/` in this folder (git ignores it), fetches the toolchain, lifts
the game and builds a bundle for this computer: macos on a Mac, steamos on
Linux, windows on Windows. The first run takes a while; later runs only
rebuild what changed. The bundle lands in `dist/`.

```sh
./blinx1 package windows|steamos|macos   # the same, for another target
./blinx1 doctor      # what this machine has, what it can package, and what to install first
```

Copy the bundle to the machine you play on and install it there:

| Target | Install | Saves, settings and logs |
|---|---|---|
| **Steam Deck / Linux** (`dist/BLiNX1-<v>-steamos.tar`) | In Desktop Mode: `tar -xf BLiNX1-<v>-steamos.tar`, `cd BLiNX1-<v>-steamos`, `./install.sh --steam` (Steam open: it adds "BLiNX1" as a non-Steam game). A stock Deck has no `umu-run`; the installer offers to download it, or run `./install.sh --fetch-umu`. The first launch downloads the Steam Linux Runtime and GE-Proton, which takes a few minutes. Later versions: `./install.sh`; `./install.sh rollback` goes back one. | `~/Games/BLiNX1/{hdd,config,logs}` |
| **macOS** (`dist/BLiNX1-<v>.dmg`) | Open the `.dmg` and drag `BLiNX1.app` to Applications (replace the old one to update). The app is signed ad hoc, not notarized: on another Mac, right-click > Open once, or `xattr -dr com.apple.quarantine /Applications/BLiNX1.app`. | `~/Library/Application Support/BLiNX1/{hdd,config,logs}` |
| **Windows** (`dist/BLiNX1-<v>-windows/`) | Keep the folder together and run `BLiNX1-<v>-setup.exe`. It installs for your user only, with a Start Menu entry. SmartScreen will warn (unsigned, built on your machine): More info > Run anyway. | `%LOCALAPPDATA%\BLiNX1\{hdd,config,logs}` |

Saves (`hdd/`), settings (`config/`) and logs (`logs/`) live outside the
program files and survive every update, rollback and uninstall.

**Updating to a new release.** Run the new release's wizard on the same
build folder, or by hand `git pull` and then `./blinx1` in this folder.

**A bundle is private.** It contains your copy of the game and code generated
from it. Keep it on your own machines; never share, upload or publish it.

## Options

The game reads its options from `config/config.toml` in the data folder
above. `config.default.toml` (beside the executable,
`packaging/config.default.toml` here) lists every key at its default; copy
a line into your `config.toml` to change it, for example `scale = 2` under
`[render]` to draw at 1280x960. The wizard writes the options you pick
there. Defaults that differ from the Xbox:

- `storage.ram_cache = true`: the game's cache partition lives in memory
  (up to 750 MB of RAM). The game rewrites its cache all the time, about
  7 GB an hour, so this saves your SSD. `ram_cache = false` under
  `[storage]` puts the files back on the disk.
- `menu.enabled = true`: the in-game menu below.
- `present.surface = "auto"`: in SteamOS Game Mode the game hands gamescope
  the frame to scale instead of a window (`"window"` restores that).

### The in-game menu

F10, Guide+A or Back+L3 held 0.4 s (and, on macOS, an Elite controller's
P1) opens a menu over the game to change these options while it runs,
rebind the host actions and turn mods on or off; Save keeps only what you
changed in `config/config.toml`. The game keeps running behind it and sees
no input. F3 toggles a stats strip (fps, frame ms, pacing, scale, backend)
in the window's top right corner. Press L3 first with Back+L3, or both at
once: the first button still reaches the game until both are down. On a
Steam Deck, SteamOS keeps Guide+A for itself: use Back+L3, or bind R4 to
F10 once in Steam Input (the game's controller settings > Back Grip
Buttons > R4 > Keyboard > F10). `[menu] enabled = false` turns the menu
off.

## Known issues

- Play past the title and the attract demo is not checked through the
  game yet: expect crashes, and report them.
- Widescreen (a wide aspect ratio) is in, but not yet checked through the
  game, so the default stays 4:3.

## Troubleshooting

- **The wizard or `./blinx1` says "another release".** Your disc is
  another region or revision than the NTSC-U, version 6 one this port is
  made from. The build may fail or the game misbehave; see the README's
  supported version box.
- **macOS: "the app is damaged" or cannot be opened.** Gatekeeper: right-click
  the app and pick Open, once, or `xattr -dr com.apple.quarantine
  /Applications/BLiNX1.app`.
- **Steam Deck: the first launch shows only a notice for a few minutes.**
  It is downloading the Steam Linux Runtime and GE-Proton; the game starts
  when that is done.
- **Windows: SmartScreen warns about the wizard or the setup program.**
  Neither is signed: More info > Run anyway.

## Reporting a bug

[Open a bug report](https://github.com/apexJCL/blinx1-recomp/issues/new?template=bug_report.yml) on this repository: the form asks for everything below. Say which platform and bundle version you
run (the version is in the bundle's name and in `logs/`), what you did, and
attach the newest `logs/game-*.log` and `logs/xbox_kernel.log` from the
data folder. Never attach your game files, a save or the bundle itself.
Problems with the wizard or the `./blinx1` commands themselves go to
[xboxrecomp-cli's issues](https://github.com/apexJCL/xboxrecomp-cli/issues).
