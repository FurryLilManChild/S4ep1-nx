# S4ep1-nx 1.0

The 1.0 release of the native Switch port of Sonic the Hedgehog 4: Episode I (Xbox 360 XBLA), now played start to
finish. The game code is the same as 0.5.1; the NRO now has an icon and reports version 1.0.0.

## What's in it

- The whole game recompiled from the Xbox 360 `default.xex` and drawn by a native deko3d renderer: no emulation.
- A steady 60 fps at stock clocks: drawn at 960x540 in handheld and 1296x729 docked, with Nintendo's own 460.8 MHz
  handheld GPU profile (not an overclock). The game keeps its own speed: no slow motion and no speed-ups in heavy
  moments.
- Every shader the game ships built into the NRO, and a precompiled shader file (`deko3d_shaders.bin`, 581 variants)
  so effects do not stutter the first time they appear.
- A port menu: hold - and + together. FPS counter, handheld and docked resolution, handheld GPU profile, stick dead
  zone; Save and close keeps the choices in `renut.toml`.

## Since 0.5.0

- Objects that were invisible in 0.5.0 (cards in Casino Street Act 2, spheres and ropes in Lost Labyrinth, an enemy
  underwater) are drawn: 0.5.0 lacked 185 of the game's shaders.
- Mad Gear no longer crashes and can be finished.

Thanks to vladimirkasan2-cpu for the 0.5.0 report and logs (GitHub issue 1).

## Installing

1. Copy the zip's `switch` folder to the root of the SD card: `/switch/s4ep1-nx/` gets `s4ep1.nro`, `renut.toml` and
   `deko3d_shaders.bin`. Updating from 0.5.x: replace the old files; your `game_root` folder and saves stay as they
   are.
2. New install: make `/switch/s4ep1-nx/game_root/` and put `default.xex` and `data.cpk` from your own copy of the XBLA
   game in it (extracted from its package; `default.xex` SHA-256
   `0a2525018d23e908cdcefd779e350d49f632f5773baf248108435cc8dac00a7b`).
3. Start the homebrew menu through title takeover (hold R while opening a game), not the Album, and run `s4ep1.nro`.

No game files are included.

## Known issues

- Docked mode has had less testing than handheld.
- An effect the shader file lacks compiles the first time it appears and can stutter briefly.
- Logs: `/switch/s4ep1-nx/logs/`. Send the whole folder with a report.
