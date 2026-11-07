#!/usr/bin/env python3
"""Tests for the port's names (config/names.toml, the CLI's docs/names.md;
The tracker item): every XBR_FN/GEN/VA/VAL form in src/ and every seed by name is a
name of config/names.toml of a kind that fits, the hand-written code keeps
no NTSC address the names replaced, and each release map names only those
names. The CLI checks the same at analyze and recomp, against the release
it builds; this catches a typo before any dump is there.

  python3 scripts/test_names.py
  uv run pytest scripts/test_names.py

Standard library only (Python 3.9: no tomllib): names.toml is read with
a pattern for its [[name]] rows, which keep one key per line.
"""

import json
import os
import re
import sys

import pytest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GEN = os.path.join(ROOT, "src", "recomp", "gen")

FORM = re.compile(r"\bXBR_(FN|GEN|VA|VAL|HAVE)\(\s*(\w+)\s*\)")
COMMENT = re.compile(r"/\*.*?\*/|//[^\n]*", re.S)
FITS = {"FN": ("function",), "GEN": ("function",), "VA": ("function", "global", "site")}
FITS["VAL"] = ("value",)


def _rows(path):
    """[{key: text}] of a TOML file's [[name]] tables."""
    with open(path, encoding="utf-8") as f:
        text = f.read()
    rows = []
    for block in re.split(r"^\[\[name\]\]\s*$", text, flags=re.M)[1:]:
        block = re.split(r"^\[", block, flags=re.M)[0]
        row = {}
        for m in re.finditer(r'^(\w+)\s*=\s*("[^"]*"|\S+)', block, re.M):
            row[m.group(1)] = m.group(2).strip('"')
        rows.append(row)
    return rows


def _names():
    return {r["name"]: r for r in _rows(os.path.join(ROOT, "config", "names.toml"))}


def _sources():
    for d, dirs, files in os.walk(os.path.join(ROOT, "src")):
        dirs[:] = sorted(x for x in dirs if os.path.join(d, x) != GEN)
        for f in sorted(files):
            if f.endswith((".c", ".h")):
                yield os.path.join(d, f)


def test_game_toml_names_the_file():
    with open(os.path.join(ROOT, "game.toml"), encoding="utf-8") as f:
        assert re.search(r'^names = "config/names.toml"$', f.read(), re.M)


def test_names_are_unique_and_whole():
    rows = _rows(os.path.join(ROOT, "config", "names.toml"))
    seen = [r["name"] for r in rows]
    assert len(seen) == len(set(seen))
    for r in rows:
        assert r["kind"] in ("function", "global", "site", "value"), r
        assert ("value" if r["kind"] == "value" else "addr") in r, r


def test_every_form_names_a_name_of_its_kind():
    names = _names()
    used = set()
    for p in _sources():
        with open(p, encoding="utf-8") as f:
            code = COMMENT.sub(" ", f.read())
        for m in FORM.finditer(code):
            form, n = m.groups()
            assert n in names, "%s: %s(%s) is not in config/names.toml" % (p, form, n)
            if form in FITS:
                assert names[n]["kind"] in FITS[form], (p, form, n, names[n]["kind"])
            used.add(n)
    seeds = _seed_names()
    # a name nothing uses is a stale row
    assert set(names) - used - seeds == set()


def _seed_names():
    with open(os.path.join(ROOT, "config", "seed_functions.json"), encoding="utf-8") as f:
        return {r["name"] for r in json.load(f) if "name" in r}


def test_seeds_by_name_are_functions():
    names = _names()
    for n in _seed_names():
        assert names[n]["kind"] == "function", n


def test_no_named_address_left_in_the_code():
    """The NTSC addresses the names replaced appear in comments only."""
    names = _names()
    addrs = {int(r["addr"], 0) for r in names.values() if "addr" in r}
    for p in _sources():
        with open(p, encoding="utf-8") as f:
            code = COMMENT.sub(" ", f.read())
        for m in re.finditer(r"\b(?:sub_([0-9A-Fa-f]{8})|0x([0-9A-Fa-f]{5,8})u?)\b", code):
            va = int(m.group(1) or m.group(2), 16)
            assert va not in addrs, "%s: %s is a name in config/names.toml" % (p, m.group(0))


def test_release_maps_name_only_names():
    names = _names()
    d = os.path.join(ROOT, "config", "releases")
    for f in sorted(os.listdir(d)) if os.path.isdir(d) else []:
        if f.endswith(".toml"):
            for r in _rows(os.path.join(d, f)):
                assert r["name"] in names, (f, r["name"])


def test_release_rows_were_matched_from_the_current_names():
    """Each row's ref is names.toml's address (or value) now: a moved name
    leaves its reviewed rows stale, and the CLI would stop on them."""
    names = _names()
    d = os.path.join(ROOT, "config", "releases")
    for f in sorted(os.listdir(d)) if os.path.isdir(d) else []:
        if f.endswith(".toml"):
            for r in _rows(os.path.join(d, f)):
                n = names[r["name"]]
                assert "ref" in r, (f, r["name"])
                ref = n["value"] if n["kind"] == "value" else n["addr"]
                assert int(r["ref"], 0) == int(ref, 0), (f, r["name"], r["ref"], ref)


def test_have_guards_are_plain_and_named():
    """Only a plain "#if XBR_HAVE(name)" line is understood by the CLI and
    recomp; its name must be a row (a typo would drop the block)."""
    names = _names()
    for p in _sources():
        with open(p, encoding="utf-8") as f:
            code = COMMENT.sub(" ", f.read())
        for line in code.splitlines():
            if "XBR_HAVE" in line:
                m = re.fullmatch(r"\s*#\s*if\s+XBR_HAVE\(\s*(\w+)\s*\)\s*", line)
                assert m, "%s: %r is not a plain #if XBR_HAVE(name)" % (p, line.strip())
                assert m.group(1) in names, (p, m.group(1))


if __name__ == "__main__":
    sys.exit(pytest.main([__file__, "-q"]))
