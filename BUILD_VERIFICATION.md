# Build verification

- App version: `2.0`
- Application ID: `dungeons_and_dolphins`
- Display name: `Dungeons & Dolphins`
- Target: Flipper Zero, RogueMaster firmware
- Firmware branch: `420`
- Firmware commit: `d3ae1f86bd961852af6969e887a8dd3492a19321`
- Firmware commit date: `2026-08-22`
- Build command: `./fbt fap_dungeons_and_dolphins`
- Build result: passed
- Firmware API reported by build: `88.4`
- FBT checks completed: compile, link, app metadata, FAP generation, FASTFAP, and APPCHK
- Toolchain FAP size: `282,380 bytes` including packaged catalogs, campaign, monster records, and translation-key assets
- Toolchain FAP SHA-256: `be37f0793cbe8cc54f1bba523985695ec0ea8b03e5b112514daf4e5a0bd386e2`
- ARM ELF text: `82,970 bytes`
- Release ZIP policy: source and assets only; no `dist` directory or compiled FAP

The build was performed from a clean RogueMaster checkout with the app linked into `applications_user/pocket_d20`. No device hardware was connected, so this verifies compilation and firmware API compatibility, not physical-device UI behavior.

Version 2.0 compiles the dynamically indexed character manager, lazy catalog/adventure/monster allocations, packaged assets, stable schema-1 `ch_{x}_{characterName}_{characterLvl}.txt` saves, migration, retained backups, profile transfer/archive/restore actions, translation hooks, structured grants, spellcasting models, combat sheet, inventory resources, catalog diagnostics, adventure engine, searchable monster compendium, template-aware encounter generator, initiative transfer, versioned monster packs, on-device custom creation, live heap diagnostics, and manifest icon. FBT accepted `icon.png` as the required 10x10 1-bit application icon.
