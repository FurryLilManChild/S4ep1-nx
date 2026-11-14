# S4ep1-nx

Native Nintendo Switch (and PC) port of Sonic the Hedgehog 4: Episode I, Xbox 360 XBLA version, by static
recompilation (ReXGlue, PowerPC to C++) and Renut's Direct3D-level renderer on deko3d.

## Who does what

- Claude writes all code, scripts and docs, reads logs, makes cloud Switch builds and cloud boot tests.
- Mark (FurryLilManChild) runs PC builds in PowerShell, tests on his V1 Switch and PC, and sends logs.
- Mark's folder: `C:\S4ep1-nx`. The XBLA package is in `assets\`, extracted to `assets\extracted\` (`default.xex`,
  `data.cpk`). Logs go to `C:\S4ep1-nx\out\sd\switch\switchlogs`.

## Hard rules

- No game data in git, ever (see `.gitignore`). Run `git status` before every commit.
- No em dashes anywhere. No decorative emoji.
- Commit as `FurryLilManChild <346934328+FurryLilManChild@users.noreply.github.com>`.
- Running an upstream project's scripts or building third-party code needs Mark's OK first.
- Never ship a Switch build without the renderer, sound, input and game code all in it.
- Present test findings and the next build's plan to Mark and wait for his go-ahead (cloud boot work before the title
  screen is the exception: build and fix without asking, with an update at least every 10 minutes).
- Never state a theory as a diagnosis: say what a test checks and what result confirms it.
- Anything to get rid of goes to `C:\Games\04 MY PORTS\FOR DELETION` (Mark empties it in bulk). Temporary transfer
  files are written there directly and joined from there.

## Layout

- `docs/scan.md`: what the package and default.xex hold.
- `docs/design.md`: the plan, reviewed with Mark.
- `docs/progress-log.md`: newest first, one entry per session, with evidence.
- `docs/pitfalls.md`: the running list of mistakes to avoid (reviewed with Mark at wrap-up, then moved into the skills).
- `docs/upstream.md`: each upstream, its pinned commit, license and what we take.
- `docs/from-tfu/`, `docs/from-rr6/`: the base trees' notes, for reference.
- `sdk/`, `app/`, `shaders/`, `tools/`: copied from TFU-nx (itself from rr6-nx 1.0.1).
