# Build verification

- App version: `0.1`
- Target: Flipper Zero, RogueMaster firmware
- Firmware branch: `420`
- Firmware commit: `d3ae1f86bd961852af6969e887a8dd3492a19321`
- Firmware commit date: `2026-08-22`
- Build command: `./fbt fap_pocket_d20`
- Build result: passed
- Firmware API reported by build: `88.4`
- FBT checks completed: compile, link, app metadata, FAP generation, FASTFAP, and APPCHK
- Artifact: `dist/pocket_d20.fap`
- Artifact size: `38,656 bytes`
- SHA-256: `4ad32cb858a37eb99a331d7bc15efff4cd00097361ad8e5b2c52ca8fb2de7c17`

The build was performed from a clean RogueMaster checkout with the app linked into `applications_user/pocket_d20`. No device hardware was connected, so this verifies compilation and firmware API compatibility, not physical-device UI behavior.

