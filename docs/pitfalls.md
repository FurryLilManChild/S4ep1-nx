# Pitfalls to avoid (S4ep1-nx session list)

Mistakes and wrong conclusions from this port, newest first. Reviewed with Mark at wrap-up, then moved into the skills.

- 2026-11-18: release 0.5.0 shipped 97 of the 282 shaders Sonic 4 keeps in data.cpk (NNSTDSHADER/SHADER.AMB,
  WORLDMAP.AMB): the dump scanned only default.xex (8) and what our runs captured. Renut skips every draw whose shader
  is not in the build, so areas nobody had played showed invisible objects (GitHub issue 1). TFU-nx had already hit
  this and written tools/tfu_pack_shaders.py; the step was not carried over. Before the first release of any Renut
  port: find where the game stores its shaders (scan every data file for 0x102A11 containers), dump them all into
  the build, and say plainly to Mark which shaders a release lacks and what that means on screen.
- 2026-11-18: I told Mark the Switch "can't" translate shaders. It can (MSR-nx does); this port just has no
  on-console translation step. Say what a port does or lacks, not what the hardware can't do.
- 2026-11-18: 0x8210C638 (a callback whose address is built by lis/addi with two instructions between them, stored
  in an object at 0x8210C0D8) had no .pdata and was never registered: Mad Gear's "invalid or unregistered function".
  The pointer sweep only takes pointers stored in data; built addresses need a scan that follows the register.
- 2026-11-18: from the reporter's video alone I built four renderer theories (culling, the face flag, early alpha, the
  CPU picking a side) and proposed a test build, before reading the logs the reporter had attached. The logs showed
  the 0.5.0 release skipping every draw whose shader is missing from its shader set ("no host shader", 20 shaders).
  For any bug report, read the attached logs (DRAWS NOT DRAWN, missing shaders, FATAL lines) before forming theories.


- 2026-11-14: copying TFU-nx's tree with tar and committing with `git add -A` silently dropped four files TFU tracks
  past ignore rules (`app/generated/rexglue.cmake`, three FFmpeg config headers), and Mark's first PC configure failed.
  After copying a base tree, compare `git ls-files` of both repos before the first commit and force-add what is missing.
- 2026-11-14: the cloud boot tests ran a Linux build with Renut off; nothing exercised Windows' `build_pc.ps1 -Renut`
  path. Mark's first -Renut configure failed: the script checked and installed xxhash with the `python` on PATH while
  CMake's find_package(Python3) chose another python.exe. Fixed by passing -DPython3_EXECUTABLE (the PATH python) and
  checking the module in that one. Lesson: a cloud test only covers the build path it ran; say which path was tested,
  and when a Windows path has an interpreter or tool lookup, make the script pin it instead of trusting two lookups to agree.
- 2026-11-14: after porting Renut I compile-checked only the files I had changed; a layout-dependent static_assert in
  an unchanged file (command_buffers.cpp's last snapshot chunk: 24 bytes for Sonic 4) and a missing tool
  (dump_shaders.py, never in this tree; TFU always had its dump folder) would have failed Mark's build one at a time.
  Check every source of the target (`ninja -t commands` plus `-fsyntax-only`) and walk the build script's steps for
  tools or folders the base project had on disk but never committed.
- 2026-11-14: the cloud boot tests ran the Linux build, where I had gated the Windows path wizard off; on Mark's PC the
  wizard waited for a click on its first run and both boot tests sat at a black window. And boot_test.ps1's "emulated
  GPU" run was really Renut in a -Renut build (banjo_renderer defaults to "native", which falls back to Renut). A cloud
  test only stands in for Mark's run when both take the same code path: list what differs (OS-gated code, defaults,
  the renderer chosen) before saying a cloud result carries over.
- 2026-11-14: the pointer sweep missed a function whose address is built in code, because the generator had already
  merged it into the previous function (it ran past that function's .pdata end). Check every lis/addi code address that
  lands on a .pdata end without being a function start, at kickoff.
- 2026-11-15: Renut's hooks were moved to Sonic 4 by matching code shape and call sites, without checking each
  function's arguments. Sonic 4's D3D is LTCG-linked (TFU's and LoS1's are not, or less so): CreateDevice and Present
  take no arguments, draw calls drop constant ones, SetViewport takes floats. The first Renut run read garbage. For
  every hooked function, list the argument registers it reads in the generated code and compare with the hook's
  signature before the first Renut build.
- 2026-11-15: banjo_late_input (on by default since banjo-nx) delays the first controller read after each Swap. It
  assumes the game reads input on the thread that swaps. Sonic 4 reads on a separate frame thread: the delay put the
  game in slow motion (a quarter speed) on Renut only. Check where a new game reads its controller before keeping it.
- 2026-11-15: a resolution setting copied from TFU was "fixed" by changing the toml without checking that it took
  effect. The cvar's range rejected the value and the scale only applied with a known composite shader. Any setting
  change must have a log line that proves the new value is in force, and the first log after it must be checked.
- 2026-11-15: a feature carried over from another game's port (sasrt-nx's EDRAM depth restore) broke a picture in this
  one (the title's white sky). When a picture is wrong and shaders, textures and constants check out, A/B the inherited
  renderer features (depth restore, skips, hand-overs) before writing new code.
