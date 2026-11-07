#!/usr/bin/env python3
"""Tests for the speed_readout mod (src/speed_meter.c, src/speed_readout.c).

Compiles a small driver against speed_meter.c and checks the card's text:
the current speed and the peak of the last 30 polls, "now -" with no
player, a value that is not a speed read as no player, and the pointer
guard. Also checks
that the env row, docs/env.md, config.default.toml and the names the mod
reads agree, that the help fits the menu's tooltip, that the mod never
writes guest memory, and that main.c registers it.

  python3 scripts/test_speed_readout.py
  uv run pytest scripts/test_speed_readout.py

The C tests skip when no C compiler is on PATH.
"""

import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile

import pytest

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, "..")
SRC = os.path.join(ROOT, "src")

DRIVER = r"""
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "speed_meter.h"
int main(int argc, char **argv)
{
    /* argv: float bits in hex, pushed in order, "t" to print the text
     * (its newline as "|"), or "p:P:OFF:RAM" (hex) to print the guard */
    struct speed_meter m;
    char out[SPEED_METER_TEXT];
    speed_meter_reset(&m);
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == 'p') {
            unsigned long p, off, ram;
            sscanf(argv[i], "p:%lx:%lx:%lx", &p, &off, &ram);
            printf("%d\n", speed_meter_ptr_ok((uint32_t)p, (uint32_t)off, (uint32_t)ram));
            continue;
        }
        if (!strcmp(argv[i], "t")) {
            speed_meter_text(&m, out, sizeof out);
            for (char *c = out; *c; c++)
                if (*c == '\n')
                    *c = '|';
            printf("%s\n", out);
            continue;
        }
        uint32_t u = (uint32_t)strtoul(argv[i], NULL, 16);
        float f;
        memcpy(&f, &u, sizeof f);
        speed_meter_push(&m, f);
    }
    return 0;
}
"""


def _cc():
    return shutil.which("clang") or shutil.which("gcc") or shutil.which("cc")


def _b(x):
    return f"{struct.unpack('<I', struct.pack('<f', x))[0]:08X}"


def _run(*steps):
    """The texts printed at each "t" step; other steps are speeds."""
    args = [s if isinstance(s, str) else _b(s) for s in steps]
    with tempfile.TemporaryDirectory() as d:
        c, exe = os.path.join(d, "t.c"), os.path.join(d, "t")
        with open(c, "w") as f:
            f.write(DRIVER)
        subprocess.run(
            [
                _cc(),
                "-std=c11",
                "-Wall",
                "-Wextra",
                "-Werror",
                "-I",
                SRC,
                "-o",
                exe,
                c,
                os.path.join(SRC, "speed_meter.c"),
            ],
            check=True,
        )
        out = subprocess.run([exe, *args], check=True, capture_output=True, text=True)
    return out.stdout.split("\n")[:-1]


needs_cc = pytest.mark.skipif(_cc() is None, reason="no C compiler")

CORNER = (2**0.5 - 0.4) * 5 / 3  # stock W+D: 1.69036


@needs_cc
def test_empty_reads_no_player():
    assert _run("t") == ["Speed|now -"]


@needs_cc
def test_now_and_peak():
    assert _run(1.0, "t", CORNER, "t", 0.0, "t") == [
        "Speed|now 1.00  peak 1.00",
        "Speed|now 1.69  peak 1.69",
        "Speed|now 0.00  peak 1.69",
    ]


@needs_cc
def test_peak_lasts_thirty_frames():
    # The corner, then 29 idle frames: still the peak; one more drops it.
    rows = _run(CORNER, *[0.0] * 29, "t", 0.0, "t")
    assert rows == ["Speed|now 0.00  peak 1.69", "Speed|now 0.00  peak 0.00"]


@needs_cc
def test_not_a_speed_empties_the_meter():
    nan, inf = float("nan"), float("inf")
    for bad in (nan, inf, -0.5, -1e-6, 10.001, 11.0):
        assert _run(CORNER, bad, "t", 1.0, "t") == [
            "Speed|now -",
            "Speed|now 1.00  peak 1.00",
        ], bad


@needs_cc
def test_speed_bounds_kept():
    # 0.0 (standing) and 10.0 (the cap) are speeds; the peak keeps 10.
    assert _run(0.0, "t", 10.0, "t") == [
        "Speed|now 0.00  peak 0.00",
        "Speed|now 10.00  peak 10.00",
    ]


RAM, OFF = 0x04000000, 0x30


def _ok(p, off=OFF, ram=RAM):
    [r] = _run(f"p:{p:x}:{off:x}:{ram:x}")
    return r == "1"


@needs_cc
def test_pointer_guard():
    assert _ok(0x4FB260)
    assert not _ok(0)  # before the first stage
    assert not _ok(0x4FB262) and not _ok(0x4FB261)  # misaligned
    # The float must end inside RAM: the last aligned pointer is ram-4-off.
    last = RAM - 4 - OFF
    assert _ok(last)
    assert not _ok(last + 4)
    assert not _ok(RAM) and not _ok(0xFFFFFFFC)
    # A tiny RAM or an offset past it never wraps into a yes.
    assert not _ok(4, off=0, ram=2)
    assert not _ok(4, off=RAM, ram=RAM)


def test_card_fits_its_buffer():
    # The longest text the meter writes must fit the buffer the mod and
    # the driver use (SPEED_METER_TEXT, NUL included), and the card's
    # lines (mod.h: 6 lines of 120 bytes).
    hdr = _read("src", "speed_meter.h")
    m = re.search(r"#define SPEED_METER_TEXT (\d+)", hdr)
    assert m
    longest = "Speed\nnow 10.00  peak 10.00"
    assert len(longest) + 1 <= int(m.group(1))
    assert all(len(line) <= 120 for line in longest.split("\n"))
    assert "s_shown[SPEED_METER_TEXT]" in _read("src", "speed_readout.c")


def _read(*parts):
    with open(os.path.join(ROOT, *parts)) as f:
        return f.read()


VAR, KEY = "RECOMP_SPEED_READOUT", "speed_readout.enable"


def test_env_row_and_docs():
    env = _read("src", "env", "recomp_env_game.h")
    doc = _read("docs", "env.md")
    mod = _read("src", "speed_readout.c").split("\n")
    cfg = _read("packaging", "config.default.toml")
    assert re.search(rf'X\(\w+,\s+CONFIG, "{VAR}",', env)
    assert f"mods.{KEY}" in env
    m = re.search(rf"^\| `{VAR}` \|.*`src/speed_readout\.c:(\d+)` \|$", doc, re.M)
    assert m
    assert f'"{VAR}"' in mod[int(m.group(1)) - 1], m.group(1)
    assert f'mods."{KEY}"' in doc
    assert f'# "{KEY}" = "off"' in cfg


def test_help_fits_the_tooltip():
    # The menu copies a row's help into 160 bytes (enhance_menu.c, struct
    # row): anything past 159 is cut.
    src = _read("src", "speed_readout.c")
    body = re.search(r'"Speed readout",\n((?:\s*"[^"]*"\n?)+),', src)
    assert body
    text = "".join(re.findall(r'"([^"]*)"', body.group(1)))
    assert len(text.encode()) <= 159, len(text.encode())
    assert text == (
        "Shows Blinx's run speed on screen: 1.00 straight, up to 1.69 on a"
        " full diagonal (keyboard, or a pad with Square left stick)."
    )


def test_about_says_what_it_shows():
    src = _read("src", "speed_readout.c")
    body = re.search(
        r'XBOX_MOD_DESC_VERSION, sizeof k_desc, "Speed readout",\n((?:\s*"[^"]*"\n?)+),', src
    )
    assert body
    text = "".join(re.findall(r'"([^"]*)"', body.group(1)))
    assert text == (
        "Debug readout of Blinx's run speed. 1.00 is a full straight push; a"
        " diagonal at full tilt reaches 1.69 on the keyboard or a pad with"
        " Square left stick. Peak is the top of the last 30 polls (about a"
        " second). Outside a stage it shows the last value."
    )


def test_never_writes_guest_memory():
    # Off or on, the readout only reads: no MEM32 store, no write through
    # api->mem, no guest call or hook.
    src = _read("src", "speed_readout.c")
    code = re.sub(r"/\*.*?\*/", "", src, flags=re.S)
    assert not re.search(r"XBOX_MOD_MEM(32|F)\([^)]*\)\s*=[^=]", code)
    assert not re.search(r"memcpy\(\s*\(?[^,]*mem\s*\+", code)
    for call in ("guest_call", "guest_alloc", "hook", "patch"):
        assert f"->{call}" not in code, call


def test_names_present_in_every_map():
    names = _read("config", "names.toml")
    pal = _read("config", "releases", "pal-v3.toml")
    for n in ("player_ptr", "player_run_speed"):
        assert f'name = "{n}"' in names, n
        assert f'name = "{n}"' in pal, n


def _entry(text, name):
    m = re.search(rf'name = "{name}"\n(.*?)(?:\n\n|\Z)', text, re.S)
    assert m, name
    return m.group(1)


def test_pal_values():
    # PAL v3 moves the pointer (player update 0xD4C10); the offset is NTSC's.
    pal = _read("config", "releases", "pal-v3.toml")
    want = {
        "player_ptr": ("ref = 0x004FB244", "addr = 0x004EBC24"),
        "player_run_speed": ("ref = 0x30", "value = 0x30"),
    }
    for n, lines in want.items():
        e = _entry(pal, n)
        for line in lines:
            assert re.search(rf"^{line}$", e, re.M), (n, line)


def test_main_registers_the_mod():
    src = _read("src", "main.c")
    assert "speed_readout_mod()" in src
    m = re.search(r"mods\[(\d+)\];", src)
    assert m
    # One slot per mod and the NULL that ends the table.
    assert int(m.group(1)) >= src.count("_mod()) != NULL") + 1


if __name__ == "__main__":
    sys.exit(pytest.main([__file__, "-q"]))
