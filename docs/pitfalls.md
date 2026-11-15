# Pitfalls to avoid (S4ep1-nx session list)

Mistakes and wrong conclusions from this port, newest first. Reviewed with Mark at wrap-up, then moved into the skills.



- 2026-11-14: copying TFU-nx's tree with tar and committing with `git add -A` silently dropped four files TFU tracks
  past ignore rules (`app/generated/rexglue.cmake`, three FFmpeg config headers), and Mark's first PC configure failed.
  After copying a base tree, compare `git ls-files` of both repos before the first commit and force-add what is missing.
- 2026-11-14: the cloud boot tests ran a Linux build with Renut off; nothing exercised Windows' `build_pc.ps1 -Renut`
  path. Mark's first -Renut configure failed: the script checked and installed xxhash with the `python` on PATH while
  CMake's find_package(Python3) chose another python.exe. Fixed by passing -DPython3_EXECUTABLE (the PATH python) and
  checking the module in that one. Lesson: a cloud test only covers the build path it ran; say which path was tested,
  and when a Windows path has an interpreter or tool lookup, make the script pin it instead of trusting two lookups to agree.
