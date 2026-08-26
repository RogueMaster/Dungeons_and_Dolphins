# Build verification

- Release version: `2.6`
- Character application ID: `dungeons_and_dolphins`
- Bestiary application ID: `dolphin_bestiary`
- Target: Flipper Zero, RogueMaster firmware
- Firmware branch: `420`
- Firmware commit: `dd196a93089970d3bd20b18492d6c70c9a402f60`
- Firmware commit date: `2026-08-25`
- Build command: `./fbt fap_dungeons_and_dolphins fap_dolphin_bestiary`
- Build result: passed for both FAPs
- Firmware API reported by build: `88.4`
- FBT checks completed: compile, link, app metadata, FAP generation, FASTFAP, and APPCHK
- Dungeons & Dolphins FAP size: `274,944 bytes`
- Dungeons & Dolphins FAP SHA-256: `8c0440ef18fc764aa6b2ceec485d5f04e830f2976be4e069eaa8c73a65579ced`
- Dolphin Bestiary FAP size: `238,884 bytes`
- Dolphin Bestiary FAP SHA-256: `e4fa21893d740cbdfb0d1efc3f75baba850e9db7aac01558db55fc495f856345`
- Release ZIP policy: source and assets only; no `dist` directory or compiled FAP

The build used a clean RogueMaster checkout with this source linked at `applications_user/pocket_d20`. Both applications were discovered from the same `application.fam`; their explicit `sources` lists exclude the other application's entry point and feature modules.

Version 2.6 validates separate packaged-asset namespaces, the 50-record catalog and bestiary windows, 340 monster records, profile save paths and recovery, custom-file prefixes, the 10x10 1-bit icon, and firmware API compatibility. No device was connected, so this record does not claim physical-device UI or long-session results.
