#!/usr/bin/env python3
"""Tests for the Hor+ lens flare wrapper (src/recomp_manual.c).

Pins sub_00096BB0's wrapper: stock (no wide display.aspect) runs the
generated body alone, and the wide path runs it once and then moves only
the queued vertex records' z.

  python3 scripts/test_hor_flare.py
  uv run pytest scripts/test_hor_flare.py
"""

import os
import re
import sys

import pytest

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, "..")


def _body():
    with open(os.path.join(ROOT, "src", "recomp_manual.c")) as f:
        src = f.read()
    m = re.search(r"void sub_00096BB0\(void\)\n\{(.*?)\n\}", src, re.S)
    assert m, "no sub_00096BB0 wrapper"
    assert "extern void sub_00096BB0_gen(void);" in src
    return m.group(1)


def _code(body):
    """The body's statements, comments dropped."""
    body = re.sub(r"/\*.*?\*/", "", body, flags=re.S)
    return [x.strip() for x in body.split("\n") if x.strip()]


def test_stock_path_is_the_generated_body():
    lines = _code(_body())
    gate = lines.index("if (!s_cull_scale) {")
    # the wide test is the cull wrapper's, initialised before the gate
    assert lines[gate - 2 : gate] == ["if (!s_cull_init)", "cull_init();"]
    assert lines[gate + 1 : gate + 4] == [
        "RECOMP_ABI_CALL(0x00096BB0u, sub_00096BB0_gen);",
        "return;",
        "}",
    ]


def test_wide_path_calls_once_and_moves_vertex_z():
    lines = _code(_body())
    calls = [x for x in lines if "sub_00096BB0_gen" in x]
    assert calls == ["RECOMP_ABI_CALL(0x00096BB0u, sub_00096BB0_gen);"] * 2
    tail = lines[lines.index("p = MEM32(0x8EC3E4);") :]
    assert tail[1] == "RECOMP_ABI_CALL(0x00096BB0u, sub_00096BB0_gen);"
    assert tail[2] == "end = MEM32(0x8EC3E4);"
    # vertex records (type 2) only, z at +0x18, +0.0 and non-finite skipped
    assert "if (MEM32(p) == 2u && MEM32(p + 0x18u) - 1u < 0x7F7FFFFFu)" in tail
    assert "MEM32(p + 0x18u) += 1u;" in tail
    assert "for (; p < end; p += 0x34u)" in tail


if __name__ == "__main__":
    sys.exit(pytest.main([__file__, "-q"]))
