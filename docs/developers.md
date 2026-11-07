# Developing BLiNX: the time sweeper

How the port is built and how to work on it. Players want the
[README](../README.md) and [playing.md](playing.md).

## How the port works

The game's x86 code is lifted to C by the
[xboxrecomp](https://github.com/apexJCL/xboxrecomp) toolkit and compiled
together with a replacement Xbox kernel, GPU (NV2A) and audio (APU) runtime,
driven by [xboxrecomp-cli](https://github.com/apexJCL/xboxrecomp-cli). No
emulator, no disc in a drive: one command builds a private, installable
bundle from your disc dump.

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
[env.md](env.md).

## The toolchain and its pins

| Component | Pinned at | Where |
|---|---|---|
| xboxrecomp-cli (`xbr`) | tag `v0.20.0`, commit `d047f6c` | `game.toml` `[cli]` |
| xboxrecomp toolkit (fork) | tag `blinx2-v0.13.0`, commit `40ed51d`, branch `blinx2/portability` | `game.toml` `[toolkit]` |
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

## Documentation

| Document | For |
|---|---|
| [env.md](env.md) | this game's environment keys and the toolkit defaults it changes |
| [playing.md](playing.md) | building by hand, installing, options, troubleshooting |
| [CHANGELOG.md](../CHANGELOG.md) | what changed for players in each release |
