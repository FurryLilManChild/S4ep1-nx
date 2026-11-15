# Progress log

Newest first.

## 2026-11-14: kickoff

- Repo made from TFU-nx's tree (`docs/upstream.md`). GPL-3.0 (Mark's OK).
- Extracted Mark's XBLA package (version 1) to `assets\extracted\` on his PC; the "Addon" folder is avatar items, not
  a title update.
- Scanned default.xex (`docs/scan.md`): XDK 2.0.11626 (same as LoS1), CRI File System and Audio, Sega NN assets, and the
  HLSL compiler linked in (theory: run-time shader compiles, to confirm).
- Added `tools/xex_unpack.py` and `tools/xexunpack/` (LZX unpack with the SDK's libmspack).
- robzilla10001/SwitchPorts X360 list checked: no Sonic 4 port exists; ChanseyIsTheBest's XBLA ports and sa2-nx are the
  nearest references.

## 2026-11-14 (evening): first codegen, sweeps, Direct3D match

- Generator (`sdk/out/linux-amd64/Release/rexglue`) built with clang 20 (Ubuntu's clang-20 package) after the SDL3
  X11/XCB development packages were installed.
- Codegen options: the core speed set (x360 CPU step 1) on from the start; non_argument_as_local and its keep list
  (TFU addresses) and TFU's setjmp/longjmp removed (`app/renut_config.toml`). TFU's lists emptied: overrides,
  `config/renut_funcs.toml`, `src/cpu/hot/localized_bulk.txt` (copies in `docs/from-tfu/app-config/`).
- Codegen pass 1: two UnresolvedCalls, both leaves with no .pdata: 0x82186490 (size 0x170, a leaf whose two halves
  branch into each other) and 0x821F42D8 (an 8-byte thunk). Declared in `overrides.toml`. Pass 2: 12,440 functions,
  129 files, 64 MB, 8 s. Code range 0x820C0000-0x824A05DC.
- Pointer sweep: pass 1 accepted 69 of 74, pass 2 none (fixpoint). Gap sweep: 54 of 113, pass 2 none.
  `override_labels_check.py --code 820C0000:824A05DC` flagged 10 (1 SEH scope, 9 epilogue tails): removed.
- `switch_check.py`: 107 switches, 169 indirect; truncated 0, wrong targets 0, missed 0. Four tables with no bound in
  the code (34, 50, 17, 16 cases generated); five odd shapes generated as indirect jumps (0x821545A0, 0x8215B960,
  0x8215C6E0, 0x82417158, 0x82418EC0): watch for "invalid or unregistered function" at those addresses at boot.
- `d3d_match.py` against LoS1's default.xex (Mark's copy, SHA-256 0287bbd8..., the same XDK 11626 but plain D3D9; Sonic 4
  links D3D9LTCG): 22 of 41 named functions at 0.97 to 1.00 (draws, BeginVertices, shader and declaration creation,
  render targets, ring buffer, CBlocker::Check, SynchronizeToPresentationInterval); 19 to check by hand (0.26 to 0.94),
  as LTCG predicts. Full list: to be written into `docs/d3d-map.md` with the Renut work.
- PC build adapted: `tools/win/build_pc.ps1` and `boot_test.ps1` use `assets\extracted` (extracting the XBLA package
  with `tools/stfs_extract.py` if missing); TFU's `tfu_job_idle.cpp` and its `tfu_spin_idle.py` step left out.

## 2026-11-14 (night): Linux build, first cloud boot reaches the title screen

- Mark's first PC configure failed: `app/generated/rexglue.cmake` and three FFmpeg config headers were missing from the
  repo (the tree copy plus `git add -A` skipped files TFU force-tracks). Added (`docs/pitfalls.md`).
- `tools/post_codegen.py` still ran TFU's `tfu_spin_idle.py` (stops at "sub_826B1398 not found"): removed. Mark's PC
  build would have hit it too.
- Linux build (cloud boot tests; Ubuntu 24.04, clang 20, Xvfb, lavapipe): `-march=x86-64-v2` (SSSE3 in the SDK's
  memory.cpp), host glslangValidator (`-DBANJO_GLSLANG_VALIDATOR=/usr/bin/glslangValidator`), `REXGLUE_DUMP_IMAGE`
  set for post_codegen, and the Windows desktop UI (path wizard, keyboard and mouse, PC profiler) gated to `_WIN32` in
  `renut_app.h` (`RENUT_DESKTOP_UI`), with the Switch's no-op keyboard hooks on Linux (`app/CMakeLists.txt`).
- Boot 1 and 2 (emulated GPU, 130-150 s each, no crash): SEGA logo, Sonic Team logo, then the title screen with
  "Press the START button" by about 35 s; the attract loop pans the scene. Audio runs (CRI's render driver client
  registered, SDL frames with data). Screenshots: `out/boot/` (not in git).
- Checked: the inherited hand natives and localized copies (RR6/TFU's) show "ON" in the enhancements line but hook
  nothing: `hand.txt` and both localized lists are empty, `generated/hot` holds no functions.
- Leftover names in the log to rename later: "[tfu] TFU-nx pc build" (build stamp), "rr6_dlc_unlock_keys".

## 2026-11-14 (late): Renut moved to Sonic 4

- TFU's default.xex (Mark's, read-only folder access) translated as the reference; hooks and device layout mapped TFU to
  Sonic 4 (`docs/renut-map.md`): 299 hooks registered, 151 offset lines, 10 guest data constants, the full viewport
  passed by value, GetFunction imports dropped (not linked in Sonic 4), queued.cpp's chunk count 42.
- Tools: `tools/s4_d3d_propagate.py`, `tools/s4_renut_retarget.py`, `tools/s4_device_offsets.py`.
- Compile check with Renut on (Linux): the hook files, d3d_native_state.cpp and queued.cpp clean.
- Renut build check in the cloud (Mark's OK for DXC, 2026-11-14): with the pinned dxc-bin (`12395e36`), the shader dump
  from default.xex (8 containers), XenosRecomp and every generated header built (855 of 855); all 79 app and Renut
  sources pass `-fsyntax-only` with the build's own commands. Windows-only code (D3D12 backend, path wizard) is TFU's,
  unchanged.
- Sweep of other games' code still in the tree: N&B's keyboard/mouse pad mask, N&B's photo hook, banjo's native PM4
  renderer (masseffect_renderer_native) and RR6's DLC unlock are off by default or never registered; widescreen.cpp's
  N&B hooks are compiled out. Inert, to be removed in a cleanup.

## 2026-11-14 (late): first PC play, emulated GPU

- After the path wizard fix: title screen on Mark's PC. New Game crashed: "Call to invalid or unregistered function at
  0x82149FF8": the generator ran 0x82149E30 past its .pdata end and swallowed a function with no .pdata entry (its
  address built by lis/addi at 0x82149C18, 0x82149D30). A scan for every lis/addi address that lands on a .pdata end
  without being a function start found one more (0x8235E920); both declared in overrides.toml (0x82130004 is padding).
- The menu offered "Unlock Full Game": the SDK's license_mask defaulted to 0 (trial). Sonic 4 tests bit 0 of
  XamContentGetLicenseMask's mask (sub_82163E00) and clears its trial flag when set: license_mask now defaults to 1.
  Mark's "Addon" package is avatar items, not a title update; nothing else is needed for the full game.
- Renut run: "Direct3D_CreateDevice: InitializePresentationParameters failed"; 6 shaders captured. Next to look at.

## 2026-11-15: emulated GPU plays; Renut's hooks fitted to Sonic 4's LTCG signatures

- Mark's PC, emulated GPU: title screen, New Game, play. One note: a small save-file prompt at the title screen (an SDK
  dialog, not the 360's look); its scale is to fix later.
- Mark's PC, Renut: black screen. Log: "InitializePresentationParameters: 1920x1080 back 0x00000000 ... depth
  CreateSurface(... 0x00000780) failed": the hook read garbage arguments. Sonic 4's D3D is linked with LTCG, which
  drops arguments that never vary and reorders the rest, so TFU's signatures do not hold even where the code matched.
- Checked each paired hook's argument use in Sonic 4's generated code (which registers it reads, where each goes):
  - CreateDevice is inlined into the NN setup, sub_82494160 takes nothing (the parameters at 0x82505BC8, the device
    holder 0x82586168, behaviour flags 0x0C000001); InitializePresentationParameters (sub_821CF318) takes the device.
  - DrawVertices (device, primitive, count; start 0); DrawIndexedVertices (device, start, count; triangle strip, base 0).
  - Resolve (device, flags, rect, dest, point, colour r8, stencil r10, Z f1; level, slice, parameters 0).
  - SetViewport takes six floats (D3D_SetViewport's form), not a D3DVIEWPORT9 pointer.
  - Present takes nothing: the NN wrapper around the inlined D3D Present; written as a raw hook that keeps the game's
    own parts (a once-only init, its own front-buffer flip mode, the frame-time call after).
  - DrawVerticesUP, BeginVertices, UnlockResource, XGRegisterPixelShader: same as Renut's.
- Adapters in device.cpp, draw.cpp, state.cpp; Linux compile with Renut on: clean.

## 2026-11-15 (morning): Renut boots, runs at 60 fps, black picture

- Mark's PC, Renut, after the LTCG adapters: device created (1920x1080, back and depth surfaces made), title music,
  59.9 fps, no crash. Picture black.
- Log: "game gamma ramp applied ... [0]=0/0/0 [255]=0/0/0": an all-zero ramp, the scan-out multiplies every frame to
  black (TFU's 2026-11-11 symptom, opposite cause). Sonic 4's SetGammaRamp (sub_821B6978) and SetPWLGamma
  (sub_821B6A48) take two arguments, the ramp in r4 (sub_821CF318 calls them with r4 = its local ramp); TFU's
  three-argument hooks read r5. Registrations moved to the two-argument hooks.
- Log: "DRAWS NOT DRAWN: 6367 (no host shader 6367)" in 10 s: Sonic 4 makes its shaders at run time (51 captured to
  out\pc\shader_capture, 45 in this run), none in the build's cache. The next build collects them
  (collect_captured_shaders.cmake, the build folder is out\pc).
