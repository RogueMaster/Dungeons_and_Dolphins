# Changelog

## 0.3

- Added Main plus five additional independent character profiles.
- Added character creation, autosave-and-switch, and protected deletion controls.
- Moved each complete character into its own readable `character_0.txt` through `character_5.txt` SD-card save.
- Made party roster presets and active initiative state character-specific.
- Added text-format temporary files, backup rotation, checksums, and backup recovery.
- Started a fresh pre-1.0 save format without binary-save migration.
- Added autosave-on-mutation, catalog selection with long-OK custom names, CP/SP/EP/GP/PP currency, animated multi-die result breakdowns, and character-specific initiative presets.
- Added source-class and class-level filtered spell selection with an All-catalog override.
- Added an independent SD `backgrounds.txt` catalog and assignable core, Ravenloft, and Forgotten Realms background names.
- Expanded the SD feature catalog with core 2024 class-feature names and linked feature resource recharge cadence.
- Grouped all 18 skills by ability, made all six saving throws explicit, and exposed editable passive skill rows.
- Corrected 2024 Short Rest, Long Rest, Hit Point Dice, Exhaustion, and Wizard Arcane Recovery helpers.
- Added and build-validated a 10x10 1-bit d20 `icon.png` through the manifest's `fap_icon` field.

## 0.2

- Added miscellaneous modifiers for all six saving throws and all 18 skills.
- Added Passive Insight and Passive Investigation alongside Passive Perception.
- Exposed the existing spell attack and spell save DC miscellaneous modifiers in the Magic UI.
- Added explicit Proficiency Bonus display to the Magic screen.
- Added per-spell Known, Prepared, Always Prepared, Ritual, free-casts-current, and free-casts-maximum tracking.
- Added one-button spending of free spell casts and Long Rest restoration.
- Added compact `K`, `P`, `A`, and `F` markers to the spell list.
- Added automatic migration of version 1 save files to version 2.
- Updated documentation with an SRD 5.2.1 statistics audit.

## 0.1

- Initial character, multiclass, features, spells, inventory, journal, dice, weapon roll, saving, and initiative tracker.
