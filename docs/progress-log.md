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
