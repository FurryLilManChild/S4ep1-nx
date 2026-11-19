# S4ep1-nx 0.5.1

A fix release for GitHub issue 1. Thanks to vladimirkasan2-cpu for the report and the logs.

## Fixes

- **Invisible objects.** 0.5.0 lacked 185 of the game's shaders, so anything drawn with them was skipped: the cards in
  Casino Street Act 2, the falling spheres and burning ropes in Lost Labyrinth, an enemy underwater, and possibly more
  in places not yet reported. Every shader the game ships (282 in `data.cpk`, plus the ones it builds itself) is now
  in the NRO.
- **Mad Gear crash.** A game function reached only through a stored pointer was never translated, so Mad Gear crashed
  and could not be finished. It is translated now; a sweep of the whole game found no others like it.
- **Fewer first-time hitches.** `deko3d_shaders.bin` now holds 581 precompiled shader variants (434 before).

## Installing

Same as 0.5.0: copy the zip's `switch` folder to the root of the SD card, replacing the old files. Your
`/switch/s4ep1-nx/game_root/` and your saves stay as they are.

## Known issues

- Not yet played start to finish: please report anything that looks or plays wrong.
- Docked mode has had less testing than handheld.
- The homebrew menu still shows version 0.1.0.
- Logs: `/switch/s4ep1-nx/logs/`. Send the whole folder with a report.
