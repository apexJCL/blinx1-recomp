# Windows build diagnostics

For a wizard failure, use **Copy error** and keep `src/build-logs/` from
the selected build folder. Include the wizard version, Windows version and
failed step. The wizard log is `%APPDATA%\blinx1-wizard\wizard.log`.
Review logs for personal paths before posting; do not attach game data or saves.

## Missing generated functions at link time

[Issue #1](https://github.com/apexJCL/blinx1-recomp/issues/1) reports:

```text
ld.lld: error: undefined symbol: sub_0009AD80_gen
ld.lld: error: undefined symbol: sub_00096BB0_gen
ld.lld: error: undefined symbol: sub_000A2DE0_gen
```

`src/recomp_manual.c` calls these generated bodies. This error means the
linker cannot resolve them; it does not establish why. It is separate from
a Python encoding exception or a CRLF check during packaging.

First report the SHA-256 of the **extracted `default.xbe`**, not the ISO.
Open PowerShell in the source folder (the wizard's build folder contains `src`):

```powershell
(Get-FileHash -LiteralPath '.\game_files\default.xbe' -Algorithm SHA256).Hash.ToLowerInvariant()
```

Also attach the full failing build log. A matching title ID or section list
does not establish an identical executable layout. `game.toml` currently has
an empty `[xbe].sha256` list, so acceptance of the input is not proof that its
addresses match the manual wrappers. Ask the maintainer to confirm the hash.

For a manual Git checkout, also report the source revision and local changes:

```powershell
git rev-parse HEAD
```

```powershell
git status --short
```

Once the XBE and source revision are established, check whether each missing
`_gen` function is defined in `src/recomp/gen/` and whether the corresponding
object is included in the link. A call or declaration alone is not a definition.
Missing generated bodies and bodies omitted from the link need different fixes.

Do not remove wrappers or add empty function bodies just to make linking pass.
If retrying with a clean build folder, keep the failing folder and its logs;
there is no need to delete it or reset local changes to collect this evidence.
