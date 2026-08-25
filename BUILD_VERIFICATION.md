# Build verification

- App version: `0.5`
- Display name: `Dungeons and Dolphins`
- Target: Flipper Zero, RogueMaster firmware
- Firmware branch: `420`
- Firmware commit: `d3ae1f86bd961852af6969e887a8dd3492a19321`
- Firmware commit date: `2026-08-22`
- Build command: `./fbt fap_pocket_d20`
- Build result: passed
- Firmware API reported by build: `88.4`
- FBT checks completed: compile, link, app metadata, FAP generation, FASTFAP, and APPCHK
- Artifact: `dist/pocket_d20.fap`
- Artifact size: `74,692 bytes`
- SHA-256: `e3b4a125710c04abdd624ac6d6b87864bda972f4800cee13c97eac7269eb7f2c`

The build was performed from a clean RogueMaster checkout with the app linked into `applications_user/pocket_d20`. No device hardware was connected, so this verifies compilation and firmware API compatibility, not physical-device UI behavior.

Version 0.5 compiles the six-profile character manager; independent text-save, backup, and recovery paths; complete standard spell metadata; class-filtered subclasses; the 519-record structured item catalog; background catalog; rest helpers; and manifest icon. FBT accepted `icon.png` as the required 10x10 1-bit application icon. Pre-1.0 binary saves are intentionally not migrated.
