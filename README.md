# BLiNX: the time sweeper static recompilation

> [!TIP]
> **The easy way:** from the first release on, download `blinx1-wizard-*` for
> your system from the [latest release](https://github.com/apexJCL/blinx1-recomp/releases/latest),
> point it at your own disc image, and it builds and installs the game for you.
> The by-hand route is under [Build the game](#build-the-game).

*BLiNX: the time sweeper* was an original Xbox game. This project turns your
own copy of it into a native program for your PC, Steam Deck or Mac: the
game's x86 code is lifted to C by the
[xboxrecomp](https://github.com/apexJCL/xboxrecomp) toolkit and compiled
together with a replacement Xbox kernel, GPU (NV2A) and audio (APU) runtime,
driven by [xboxrecomp-cli](https://github.com/apexJCL/xboxrecomp-cli). No
emulator, no disc in a drive: one command builds a private, installable
bundle from your disc dump.

**No game data is included or distributed.** You need your own copy of the
game; everything built from it stays on your own machines.

**Early days.** The boot logos, the title screen and the attract demo run
on macOS (Metal) and on the Steam Deck (Proton). Play past the title is not
checked yet: expect crashes.

**Contents:** [For players](#for-players) (what you need, platforms, build
and install, options, the in-game menu, help) |
[For developers](#for-developers) (build from source, the toolchain and its
pins, contributing, legal).

---

# For players

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
is pillarboxed, never stretched. `scale` draws it sharper (see
[Options](#options)).

**Sister project.** [BLiNX 2: Masters of Time & Space](https://github.com/apexJCL/blinx2-recomp)
is recompiled with the same toolkit and build wizard, and is much further
along. Neither repository publishes game files, frames, audio or saves.

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

## Getting help

Open an issue on this repository. Say which platform and bundle version you
run (the version is in the bundle's name and in `logs/`), what you did, and
attach the newest `logs/game-*.log` and `logs/xbox_kernel.log` from the
data folder. Never attach your game files, a save or the bundle itself.
Problems with the wizard or the `./blinx1` commands themselves go to
[xboxrecomp-cli's issues](https://github.com/apexJCL/xboxrecomp-cli/issues).

---

# For developers

## Build from source

```sh
./blinx1 setup      # once: .venv, llvm-mingw, the toolkit (each download sha256-pinned)
./blinx1 all        # analyze (parse, disasm, funcid, abi), recomp, build
./blinx1 doctor     # what this machine has, and what is next
```

Windows: `blinx1` (the `blinx1.cmd` wrapper) or `py -3 blinx1.py`, from a
short checkout path such as `C:\g\blinx1`.

The build is `build-win/blinx1_recomp.exe`, cross-compiled with llvm-mingw
on any host, or `build/blinx1_recomp` with Apple clang on a Mac (Metal).
`src/recomp_manual.c` takes hand-written overrides,
`config/seed_functions.json` the function starts the disassembler missed,
and `./blinx1 analyze` then `./blinx1 recomp` regenerate `src/recomp/gen/`
(never edit it by hand). The CLI's `docs/porting.md` has the loop;
`game.toml` has every setting, documented in the CLI's `docs/manifest.md`.
This game's own environment keys and the toolkit defaults it changes are in
[docs/env.md](docs/env.md).

## The toolchain and its pins

| Component | Pinned at | Where |
|---|---|---|
| xboxrecomp-cli (`xbr`) | tag `v0.17.0`, commit `af5649d` | `game.toml` `[cli]` |
| xboxrecomp toolkit (fork) | tag `blinx2-v0.10.0`, commit `a0c51a5`, branch `blinx2/portability` | `game.toml` `[toolkit]` |
| llvm-mingw | `20260922` | `game.toml` `[toolchain]`, hashes in `config/setup-pins.json` |

`./blinx1 doctor` reports what is in use and whether it is the pin.

## Contributing

Changes are tracked with [OpenSpec](https://github.com/Fission-AI/OpenSpec)
under `openspec/changes/<name>/` (proposal, design, tasks, spec deltas).
Opt-in features default off and never change stock behaviour; generated code
is never hand-edited; environment variables go through the table and
`docs/env.md`. Toolkit changes go to
[apexJCL/xboxrecomp](https://github.com/apexJCL/xboxrecomp), branch
`blinx2/portability`; CLI changes to
[apexJCL/xboxrecomp-cli](https://github.com/apexJCL/xboxrecomp-cli). The
checks (`uv run pytest scripts`, `uv run ruff check .`, `uv run ruff format
--check .`) run on every push.

## Legal

This repository contains **no game data and no game code**: no game files,
assets or saves, and no lifted (recompiled) game code. `src/recomp/gen/` is
generated on your machine by the pipeline and is never committed.

To build it you need your own legally obtained copy of BLiNX: the time
sweeper. The game and its assets belong to their respective owners; this
project is not affiliated with them. A bundle made by `blinx1 package`
contains your copy of the game and code generated from it: it is for your
own machines only, and is not yours to redistribute.

## License and credits

MIT, see [LICENSE](LICENSE). Parts of `src/main.c`, `src/recomp_manual.c`
and `CMakeLists.txt` started from xboxrecomp's MIT-licensed new-game
templates; the license file carries that notice too. The toolkit that the
build links in statically carries LGPL-2.1 APU code extracted from xemu: if
you distribute a built binary, read [NOTICE](NOTICE) first.

Built on [xboxrecomp](https://github.com/sp00nznet/xboxrecomp) by sp00nz.
The pad layer and the in-game menu come from the BLiNX 2 project.
