# Testing sFall scripting features

## Basic info

This folder contains scripts which can be used for sFall opcodes.

## How to build script

1. Get "Fallout sFall Script Editor v4.1.7.RC1", for example from https://nuclear-grot.ucoz.net/forum/12-20-1

2. Run script editor, add "sfall_headers" into "Location folder of headers files" in the options

3. Open any script here, compile it, move compiled .int file into game folder as `mods/tests/SCRIPTS/OBJ_DUDE.INT` (case-sentisitive!)

4. Change `mods/mods_order.txt` file: add a line `tests` there

5. Run game, check that game displays message about tests

6. Comment tests line in `mods/mods_order.txt` in order to enable game back



## TODO

- How to build from command line?
- Use custom global script instead of `obj_dude`
