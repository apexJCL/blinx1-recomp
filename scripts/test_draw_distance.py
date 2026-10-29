#!/usr/bin/env python3
"""Tests for the draw_distance mod (src/far_cull.c, src/draw_distance.c).

Compiles a small driver against far_cull.c and checks the cutoff values and
the guard: the mod writes only over the stock 400.0f, stops for the run on
a foreign change and puts stock back when switched off. Also checks that
the env keys, docs/env.md and config.default.toml name the same settings.

  python3 scripts/test_draw_distance.py
  uv run pytest scripts/test_draw_distance.py

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

STOCK = 0x43C80000

DRIVER = r"""
#include <stdio.h>
#include <stdlib.h>
#include "far_cull.h"
int main(int argc, char **argv)
{
    struct far_cull_state st;
    volatile uint32_t word = FAR_CULL_STOCK_BITS;
    far_cull_state_init(&st);
    for (int i = 1; i < argc; i++) {
        const char *a = argv[i] + 1;
        switch (argv[i][0]) {
        case 'b':   /* bits for a distance */
            printf("%08X\n", (unsigned)far_cull_bits(strtod(a, NULL)));
            break;
        case 'w':   /* someone else writes the word */
            word = (uint32_t)strtoul(a, NULL, 16);
            break;
        case 's': { /* one mod step towards a value */
            int r = far_cull_step(&st, &word, (uint32_t)strtoul(a, NULL, 16));
            printf("%d %08X %d\n", r, (unsigned)word, st.dead);
            break;
        }
        }
    }
    return 0;
}
"""

SAME, WROTE, DEAD, NOT_STOCK, FOREIGN = range(5)


def _cc():
    return shutil.which("clang") or shutil.which("gcc") or shutil.which("cc")


def _f(x):
    return struct.unpack("<I", struct.pack("<f", x))[0]


def _run(*args):
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
                os.path.join(SRC, "far_cull.c"),
            ],
            check=True,
        )
        out = subprocess.run([exe, *args], check=True, capture_output=True, text=True)
    return out.stdout.split("\n")[:-1]


def _steps(*args):
    """(result, word, dead) per 's' argument."""
    rows = []
    for line in _run(*args):
        r, w, dead = line.split()
        rows.append((int(r), int(w, 16), int(dead)))
    return rows


needs_cc = pytest.mark.skipif(_cc() is None, reason="no C compiler")


@needs_cc
def test_bits():
    out = _run("b1", "b2", "b2.5", "b4", "b9", "b0.5", "b0", "b-3", "bnan")
    assert [int(x, 16) for x in out] == [
        STOCK,
        _f(800.0),
        _f(1000.0),
        _f(1600.0),
        _f(1600.0),
        STOCK,
        STOCK,
        STOCK,
        STOCK,
    ]


@needs_cc
def test_off_never_writes():
    # An off mod asks for stock, which it already "has": no read, no write,
    # even over a value that is not stock.
    assert _steps("w12345678", f"s{STOCK:X}") == [(SAME, 0x12345678, 0)]


@needs_cc
def test_on_then_off():
    v = _f(800.0)
    assert _steps(f"s{v:X}", f"s{v:X}", f"s{STOCK:X}") == [
        (WROTE, v, 0),
        (SAME, v, 0),
        (WROTE, STOCK, 0),
    ]


@needs_cc
def test_change_of_distance():
    a, b = _f(800.0), _f(1600.0)
    assert _steps(f"s{a:X}", f"s{b:X}") == [(WROTE, a, 0), (WROTE, b, 0)]


@needs_cc
def test_not_stock_left_alone():
    v = _f(800.0)
    assert _steps("w44000000", f"s{v:X}", f"s{STOCK:X}") == [
        (NOT_STOCK, 0x44000000, 1),
        (DEAD, 0x44000000, 1),
    ]


@needs_cc
def test_foreign_change_stops_the_mod():
    v = _f(800.0)
    assert _steps(f"s{v:X}", "w44000000", f"s{STOCK:X}", f"s{v:X}") == [
        (WROTE, v, 0),
        (FOREIGN, 0x44000000, 1),
        (DEAD, 0x44000000, 1),
    ]


def _read(*parts):
    with open(os.path.join(ROOT, *parts)) as f:
        return f.read()


KEYS = {
    "RECOMP_DRAW_DISTANCE": "draw_distance.enable",
    "RECOMP_DRAW_DISTANCE_SCALE": "draw_distance.distance",
}


def test_env_rows_and_docs():
    env = _read("src", "env", "recomp_env_game.h")
    doc = _read("docs", "env.md")
    mod = _read("src", "draw_distance.c").split("\n")
    cfg = _read("packaging", "config.default.toml")
    for var, key in KEYS.items():
        assert re.search(rf'X\(\w+,\s+CONFIG, "{var}",', env), var
        assert f"mods.{key}" in env, key
        m = re.search(rf"^\| `{var}` \|.*`src/draw_distance\.c:(\d+)` \|$", doc, re.M)
        assert m, var
        assert f'"{var}"' in mod[int(m.group(1)) - 1], (var, m.group(1))
        assert f'mods."{key}"' in doc, key
        assert f'# "{key}" = ' in cfg, key


def test_wrapper_calls_the_mod_frame():
    src = _read("src", "recomp_manual.c")
    body = re.search(r"void sub_000A2DE0\(void\)\n\{(.*?)\n\}", src, re.S)
    assert body, "no sub_000A2DE0 wrapper"
    lines = [x.strip() for x in body.group(1).strip().split("\n")]
    assert lines == [
        "RECOMP_ABI_CALL(0x000A2DE0u, sub_000A2DE0_gen);",
        "draw_distance_frame();",
    ]


if __name__ == "__main__":
    sys.exit(pytest.main([__file__, "-q"]))
