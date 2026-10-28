## Context

BLiNX 2 overrides XInput HLE-style: `recomp_manual.c` answers
`XInputGetState` from `cat_pad_get_state` and `XGetDevices` from
`cat_pad_connected_mask`. BLiNX 1 cannot do that: its XAPI is linked into
the XBE and talks to the MCPX OHCI controller, which the toolkit models as
trapped MMIO (`src/usb/ohci.c`). The device behind it (`src/usb/usb_gamepad.c`)
answers the descriptors and builds each report from `xbox_InputGetState(pad)`.
Pad 1 is plugged once the controller is running; `RECOMP_USB_PADS=N` plugs
more, each after the previous is configured. Nothing is ever unplugged.

XAPI maps root ports to controller ports as 3, 4, 1, 2 -> 1, 2, 3, 4. With
four root ports and the default first port (index 2), pad `d` sits on root
port index `(2 + d) % 4`, which is controller port `d + 1`. The model has had
four root ports and four pads since `RECOMP_USB_PADS`; multi-pad needs no new
ports, only plugging and unplugging them at run time.

## Goals / non-goals

- Goals: BLiNX 2's script grammar, pad slots and KBM merge on BLiNX 1; host
  pads that come and go reach the title as controllers inserted and removed;
  `RECOMP_PAD_SCRIPT` keeps working; no change for any title that does not
  opt in.
- Non-goals: sharing one copy of the layer between BLiNX 1 and BLiNX 2 (a
  follow-up, below); memory units or headsets on the emulated hub; a CPU
  rasteriser change (none is needed).

## Decision 1: a game-agnostic hook in the toolkit, the layer in the game

Two options were on the table.

- **(a) A hook.** The toolkit's USB pad asks a game-installed source for a
  port's state, which ports are present, and where rumble goes. The game
  keeps the layer.
- **(b) The layer in the toolkit.** Move pad_input and pad_slots into
  `src/input` and have both the OHCI pad and BLiNX 2's XInput override call
  it.

(a) is taken. Reasons:

- The toolkit diff is small and inert until a source is installed. Every
  other OHCI title (JSRF, Burnout 3, SSX 3) runs the same code it runs
  today, so no golden outside BLiNX 1 can move. (b) changes BLiNX 2's input
  path as well, and BLiNX 2's goldens would gate a BLiNX 1 feature.
- The script engine carries game data: presets name the title's files and
  guest addresses. Those belong in the game repo either way, which is the
  split BLiNX 2 already has.
- The pieces that are game-agnostic stay single-copy: the keyboard/mouse
  merge (`xbox_InputKbmPoll`, `xbox_InputMergeKbm`), host pads
  (`xbox_InputGetPadState`, `xbox_InputIsConnected`), the pad filter
  (`xbox_enhance_pad_filter`) and rumble (`xbox_InputSetState`) are called,
  not copied. What is copied is BLiNX 2's script engine and pad_slots, about
  1,300 lines of game code.
- It avoids the files another branch (`fix/blinx1-pad-rumble`:
  `src/input/pad_map.c`, `xinput_device.c`) is changing.

The copy is the cost. Lifting the engine and pad_slots into a toolkit
library that both games link, with presets passed in, is a follow-up once
both layers have settled (tasks.md, Follow-ups).

## Decision 2: the source interface

```c
typedef struct {
    int      (*get_state)(int pad, XBOX_GAMEPAD *out); /* 1: a source drives it */
    uint32_t (*present)(void);                         /* pads present now */
    void     (*rumble)(int pad, uint16_t left, uint16_t right);
} UsbPadSource;
void     usb_gamepad_set_source(const UsbPadSource *src);
uint32_t usb_gamepad_flips(void);      /* the walker's flip count */
```

- **Report.** With a source, `usb_gamepad_report` packs `get_state`'s pad
  instead of `xbox_InputGetState`'s. `RECOMP_PAD_PRESS`, `RECOMP_PAD_SCRIPT`,
  `RECOMP_PAD_LIVE` and `RECOMP_DEBUG=pad_record` apply after it, as they
  do now, so the toolkit's timed scripts and the recorder keep working on
  top of the layer. `RECOMP_PAD_SCRIPT` is not superseded:
  `RECOMP_INPUT_SCRIPT` waits on the game, `RECOMP_PAD_SCRIPT` replays a
  flip-timed recording, and both can drive one run.
- **Present.** The pads plugged in are `present()` | the pads the toolkit's
  own script, live file or synthetic press names | pads `0..N-1` when
  `RECOMP_USB_PADS=N` is set. A scripted run with only `RECOMP_PAD_SCRIPT`
  therefore still gets pad 1 as it does now.
- **Rumble.** The OHCI model's rumble packet calls `rumble` when the source
  has one, else `xbox_InputSetState` as now. `RECOMP_RUMBLE=0` still stops
  it in the model.
- The callbacks run on the OHCI thread. `get_state`, `present` and `rumble`
  take the layer's lock, and `rumble` releases it before it calls
  `xbox_InputSetState`; the layer never calls back into the USB model.

## Decision 3: hot-plug on the emulated root hub

Only with a source installed. Once per 4 ms controller pass, while the driver
has the root hub interrupt enabled:

- **Unplug.** A plugged pad missing from the present mask for 64 passes
  (about 250 ms, so a backend's brief reconnect does not reach the title)
  is unplugged: CCS and PES cleared, CSC set, PESC set only when the port
  was enabled (as real OHCI and QEMU do), RHSC raised. The pad's address,
  configuration and control-pipe state are reset, so its next arrival
  enumerates from address 0. The unplug restarts the 50-pass settle, so a
  pad the source reports again at once is not replugged before the driver
  has serviced the disconnect.
- **Two threads on the registers.** The controller thread latches status
  bits while the guest's MMIO writes clear them (write-1-to-clear), so every
  change to HcInterruptStatus and the port-status words is an atomic or/and;
  a plain read-modify-write on either side could lose an RHSC or a CSC.
- **Plug.** Pads are plugged one at a time, as now: only when every plugged
  pad is configured (none is at the default address), and 50 passes after
  the last plug. The lowest present pad that is not plugged goes next, on
  its own root port, so pad `d` is always controller port `d + 1`.
- **A removed device.** A TD addressed to no plugged pad completes with
  DEVICE NOT RESPONDING, which halts its endpoint, as a real controller does
  when nothing answers. Without a source the old reply (no error) stays.
- `ohci_pad_for` keeps a mask of plugged pads instead of a count, which
  without a source holds the same pads `0..N-1` as before.

## Decision 4: the BLiNX 1 layer

- Ported from BLiNX 2's `src/pad_input.c` (cat main 6b3bb54) and
  `src/pad_slots.{c,h}` (unchanged but for names). Dropped: BLiNX 2's
  presets, addresses and `trace_handles`, the free camera, `pad_pack`, the
  `memdump` step (it needs BLiNX 2's `mem_dump`), and the 18-byte XInput
  packet. Kept: every other step, the absolute form, `RECOMP_INPUT_STRICT`,
  the sources/ports/pads log lines, `RECOMP_TRACE=input`, `pad_peek`.
- **Polls.** BLiNX 2 counts the title's XInputGetState calls, one a frame.
  The OHCI pad is read every interrupt transfer (4 ms), so a port-0 report
  advances the poll count only when the walker's flip count has moved since
  the last poll, which makes `wait polls`, `every Np` and `BUTTON/Np` mean
  frames on both games. A run whose flips never move (an HLE present path)
  falls back to one poll per 16 ms.
- **Host pad default.** Unset `RECOMP_HOST_PAD`: off when
  `RECOMP_INPUT_SCRIPT` is set (scripted and golden runs never see a pad on
  the machine), else on on every host. BLiNX 2 keeps macOS off until its SDL
  path is verified; on BLiNX 1 the host pad already drives the title on
  macOS through `xbox_InputGetState`, and turning it off would be a
  regression.
- **Script isolation.** With `RECOMP_INPUT_SCRIPT` set, the keyboard and
  mouse are on only when the environment names them, as on BLiNX 2.
- **Present mask.** Port 0 while a script runs or the keyboard or mouse is
  on, and the ports pad_slots gives the host pads. No source, no pad.
- **Presets.** `@skip-intro` and `@attract`, written from the file opens a
  traced boot logs (`RECOMP_TRACE=input` prints `[INPUT] open <path>`):
  the Microsoft logo (`microsoft.sfd`), the Artoon logo, the opening, then
  the title (`h_title02a.sfd`). One START in the logos skips straight to the
  title, and START on the title opens its menu, so `@skip-intro` is `wait
  open microsoft.sfd; tap START until open h_title02a.sfd`. `@attract` adds
  `wait open demoplay`: the title starts the demo (`r01s01demo.dat`) on its
  own about 25 s later.
- `main.c` calls `blinx1_pad_init()` before `xbox_OhciInit()`: it reads the
  script, installs `xbox_FileOpenHook` before the title opens anything, and
  installs the source.

## Risks

- **Unplug and replug in XAPI.** XAPI's remove path is exercised for the
  first time. A real pad pulled on the Mac and the Deck is the test, and is
  a hands-on follow-up, since a headless run has no pad to pull.
- **Callbacks on the OHCI thread, and one guest-thread path.** The layer's
  lock is taken on the OHCI thread (the callbacks), in the init path, and
  on the guest thread by `on_file_open`, the `xbox_FileOpenHook` that
  `wait open` and `tap ... until open` read. That hook runs inside the
  title's NtCreateFile, so nothing that can block is done under the lock:
  `pad_rumble` calls `xbox_InputSetState` after releasing it. The pad-swap
  probe in `watch_activity` still reads host pads and stops motors under
  the lock, as in BLiNX 2; a host backend that blocks there would delay a
  file open by as long. Recorded as a follow-up.
- **Script steps on the OHCI thread.** On BLiNX 2 the script runs on the
  title's poll thread; here it runs inside `get_state`, on the controller
  thread. A `poke` writes guest memory there, and may race the title's own
  writes to the same dword, and a `key` step writes the host key table that
  the window thread also writes. Both are debug steps for scripted
  runs, and a poke target is a value the title reads, not one it writes
  every frame; a script that pokes a dword the title keeps rewriting may
  see its write lost.
- **`xbox_InputLastState`** is fed by `xbox_InputGetState`, which the
  source path does not call for the report, so the layer passes port 0's
  state on with `xbox_InputKeepLastState` (a no-op without the enhancements
  layer).
