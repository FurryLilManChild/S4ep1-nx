# Pitfalls to avoid (S4ep1-nx session list)

Mistakes and wrong conclusions from this port, newest first. Reviewed with Mark at wrap-up, then moved into the skills.



- 2026-11-14: copying TFU-nx's tree with tar and committing with `git add -A` silently dropped four files TFU tracks
  past ignore rules (`app/generated/rexglue.cmake`, three FFmpeg config headers), and Mark's first PC configure failed.
  After copying a base tree, compare `git ls-files` of both repos before the first commit and force-add what is missing.
