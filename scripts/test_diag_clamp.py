#!/usr/bin/env python3
"""Tests for the diag_clamp mod (src/stick_clamp.c, src/diag_clamp.c).

Compiles a small driver against stick_clamp.c and checks the clamp: a stick
inside the unit circle is left alone, one past it keeps its angle at length
1, so the player's run speed (length - 0.4) x 5/3 stops at 1. Also checks
that the env row, docs/env.md, config.default.toml and the names the mod
reads agree, that the help fits the menu's tooltip, and that main.c
registers the mod.

  python3 scripts/test_diag_clamp.py
  uv run pytest scripts/test_diag_clamp.py

The C tests skip when no C compiler is on PATH.
"""

import math
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
#include "stick_clamp.h"
static float bits(const char *s)
{
    uint32_t u = (uint32_t)strtoul(s, NULL, 16);
    float f;
    memcpy(&f, &u, sizeof f);
    return f;
}
int main(int argc, char **argv)
{
    /* argv: pairs of x y as float bits in hex; prints r x y as bits */
    for (int i = 1; i + 1 < argc; i += 2) {
        float x = bits(argv[i]), y = bits(argv[i + 1]);
        uint32_t ux, uy;
        int r = stick_clamp(&x, &y);
        memcpy(&ux, &x, sizeof ux);
        memcpy(&uy, &y, sizeof uy);
        printf("%d %08X %08X\n", r, (unsigned)ux, (unsigned)uy);
    }
    return 0;
}
"""


def _cc():
    return shutil.which("clang") or shutil.which("gcc") or shutil.which("cc")


def _b(x):
    return struct.unpack("<I", struct.pack("<f", x))[0]


def _f(u):
    return struct.unpack("<f", struct.pack("<I", u))[0]


def _clamp(*pairs):
    """[(scaled, x, y)] for each (x, y) pair, through the C clamp."""
    args = [f"{_b(v):08X}" for p in pairs for v in p]
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
                os.path.join(SRC, "stick_clamp.c"),
                "-lm",
            ],
            check=True,
        )
        out = subprocess.run([exe, *args], check=True, capture_output=True, text=True)
    rows = []
    for line in out.stdout.split("\n")[:-1]:
        r, x, y = line.split()
        rows.append((int(r), _f(int(x, 16)), _f(int(y, 16))))
    return rows


def _speed(x, y):
    """sub_000D3D50's run speed for a stick (0 inside the 0.4 dead ring)."""
    m = math.sqrt(x * x + y * y)
    return 0.0 if m <= 0.4 else (m - 0.4) * 5 / 3


needs_cc = pytest.mark.skipif(_cc() is None, reason="no C compiler")

# The game's axis: (v + 0.5) / 32767.5, so -32768 and 32767 are -1 and 1.
FULL = (32767 + 0.5) / 32767.5
ROUND = (23170 + 0.5) / 32767.5


@needs_cc
def test_inside_left_alone():
    pairs = [(0.0, 0.0), (FULL, 0.0), (0.0, -1.0), (0.7071, 0.7071), (0.5, -0.5), (-0.3, 0.9)]
    for (r, x, y), (x0, y0) in zip(_clamp(*pairs), pairs):
        assert r == 0
        assert (x, y) == (_f(_b(x0)), _f(_b(y0)))


@needs_cc
def test_corner_runs_as_cardinal():
    (r, x, y), (_, cx, cy) = _clamp((1.0, 1.0), (1.0, 0.0))
    assert r == 1
    assert x == y
    assert abs(x - math.sqrt(0.5)) < 1e-7
    assert abs(_speed(x, y) - _speed(cx, cy)) < 1e-6
    # Stock: the corner's 1.69.
    assert abs(_speed(1.0, 1.0) - 1.69036) < 1e-5


@needs_cc
def test_round_rim_barely_moves():
    # A round pad's full diagonal (23170, 23170) reads 1.000016 long: the
    # spike's 1.00003 speed. The clamp takes it to 1, a change of 0.002%.
    assert 1.0 < math.hypot(ROUND, ROUND) < 1.0001
    [(r, x, y)] = _clamp((ROUND, ROUND))
    assert r == 1
    assert abs(x - ROUND) < 2e-5 and x == y
    assert abs(math.hypot(x, y) - 1.0) < 1e-6


@needs_cc
def test_angle_kept_every_quadrant():
    pairs = [(1.0, 1.0), (-1.0, 1.0), (-1.0, -1.0), (1.0, -1.0), (1.0, 0.6), (-0.8, 0.9)]
    for (r, x, y), (x0, y0) in zip(_clamp(*pairs), pairs):
        assert r == 1
        assert abs(math.hypot(x, y) - 1.0) < 1e-6
        assert abs(math.atan2(y, x) - math.atan2(y0, x0)) < 1e-6
        assert _speed(x, y) <= 1.0 + 1e-6


@needs_cc
def test_just_past_one():
    # One float step past length 1 is clamped, to exactly 1; length 1 is not.
    above = _f(_b(1.0) + 1)
    rows = _clamp((above, 0.0), (0.0, -above), (1.0, 0.0))
    assert rows[0] == (1, 1.0, 0.0)
    assert rows[1][0] == 1 and rows[1][2] == -1.0
    assert rows[2] == (0, 1.0, 0.0)


@needs_cc
def test_negative_zero_kept():
    # -0.0 keeps its sign bit, left alone or scaled (the menu's sign test).
    rows = _clamp((-0.0, -1.0), (-0.0, -1.5), (1.5, -0.0))
    assert rows[0][0] == 0 and _b(rows[0][1]) == _b(-0.0)
    assert rows[1] == (1, -0.0, -1.0) and _b(rows[1][1]) == _b(-0.0)
    assert rows[2][0] == 1 and _b(rows[2][2]) == _b(-0.0)


@needs_cc
def test_not_a_number_left_alone():
    nan, inf = float("nan"), float("inf")
    rows = _clamp((nan, 1.0), (1.0, nan), (inf, 1.0))
    assert [r for r, _, _ in rows] == [0, 0, 0]
    assert math.isnan(rows[0][1]) and rows[0][2] == 1.0
    assert rows[2][1] == inf


def _read(*parts):
    with open(os.path.join(ROOT, *parts)) as f:
        return f.read()


VAR, KEY = "RECOMP_DIAG_CLAMP", "diag_clamp.enable"


def test_env_row_and_docs():
    env = _read("src", "env", "recomp_env_game.h")
    doc = _read("docs", "env.md")
    mod = _read("src", "diag_clamp.c").split("\n")
    cfg = _read("packaging", "config.default.toml")
    assert re.search(rf'X\(\w+,\s+CONFIG, "{VAR}",', env)
    assert f"mods.{KEY}" in env
    m = re.search(rf"^\| `{VAR}` \|.*`src/diag_clamp\.c:(\d+)` \|$", doc, re.M)
    assert m
    assert f'"{VAR}"' in mod[int(m.group(1)) - 1], m.group(1)
    assert f'mods."{KEY}"' in doc
    assert f'# "{KEY}" = "off"' in cfg


def test_help_fits_the_tooltip():
    # The menu copies a row's help into 160 bytes (enhance_menu.c, struct
    # row): anything past 159 is cut.
    src = _read("src", "diag_clamp.c")
    body = re.search(r'"Clamp diagonal speed",\n((?:\s*"[^"]*"\n?)+),', src)
    assert body
    text = "".join(re.findall(r'"([^"]*)"', body.group(1)))
    assert len(text.encode()) <= 159, len(text.encode())
    assert text == (
        "Caps diagonal speed at straight speed. Without it, full diagonals"
        " (keyboard, or a pad with Square left stick) run 1.69x as fast."
    )


def test_summary_says_what_it_covers():
    src = _read("src", "diag_clamp.c")
    body = re.search(
        r'XBOX_MOD_DESC_VERSION, sizeof k_desc, "Clamp diagonal speed",\n((?:\s*"[^"]*"\n?)+),', src
    )
    assert body
    text = "".join(re.findall(r'"([^"]*)"', body.group(1)))
    # "1.69x as fast", not "1.69x faster" (2.69x). A round pad reaches
    # 1.69 only at full tilt with Square left stick (on by default); with
    # it off its diagonal runs at 1.00.
    assert text == (
        "BLiNX 1 runs full diagonals 1.69x as fast as straight, on the keyboard"
        " and on a pad with Square left stick (on by default). This mod caps"
        " them at straight speed, as in BLiNX 2."
        " Side effect: a diagonal tap no longer counts as a stick shake."
    )


def test_names_present_in_every_map():
    names = _read("config", "names.toml")
    pal = _read("config", "releases", "pal-v3.toml")
    for n in ("pad_states", "pad_state_size", "pad_left_x", "pad_left_y"):
        assert f'name = "{n}"' in names, n
        assert f'name = "{n}"' in pal, n


def _entry(text, name):
    m = re.search(rf'name = "{name}"\n(.*?)(?:\n\n|\Z)', text, re.S)
    assert m, name
    return m.group(1)


def test_pal_values():
    # PAL v3 moves the array (pad_poll_frame 0x9EBC0); the layout is NTSC's.
    pal = _read("config", "releases", "pal-v3.toml")
    want = {
        "pad_states": ("ref = 0x003F09C0", "addr = 0x003E08A0"),
        "pad_state_size": ("ref = 0x1E4", "value = 0x1E4"),
        "pad_left_x": ("ref = 0x14", "value = 0x14"),
        "pad_left_y": ("ref = 0x18", "value = 0x18"),
    }
    for n, lines in want.items():
        e = _entry(pal, n)
        for line in lines:
            assert re.search(rf"^{line}$", e, re.M), (n, line)


def test_main_registers_the_mod():
    src = _read("src", "main.c")
    assert "diag_clamp_mod()" in src
    assert re.search(r"mods\[\d+\];", src)


if __name__ == "__main__":
    sys.exit(pytest.main([__file__, "-q"]))
