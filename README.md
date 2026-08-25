# Dungeons & Dolphins

Dungeons & Dolphins is an offline, 5E-compatible character, journal, dice, inventory, spell, initiative, adventure, monster, and encounter tracker for Flipper Zero. Version 2.0 targets RogueMaster and stores every character in a separate readable text file.

## Full feature list

### Profiles and persistence

- SD-card-limited character profiles with dynamic profile indexing.
- Save names use `ch_{id}_{characterName}_{characterLevel}.txt`.
- Each profile owns its complete character, party presets, initiative state, journal, inventory, spells, grants, and encounter history.
- Immediate autosave after edits and resource changes.
- Temporary-file replacement and backup recovery for interrupted writes.
- One prior successful save generation retained per profile, with explicit checksum verification and restore controls.
- Human-readable, versioned, checksummed save records.
- Frozen schema 1 with serialized-file checksums and automatic migration from the v0.8 text layout.
- Hold OK on a profile for switch, rename, duplicate, export, import, archive, delete, and checksum verification actions.
- Exports are written under `exports/`; archived profiles move under `archive/` and leave the active profile list.

### Character sheet

- Name, player, species, background, alignment, experience, milestone leveling, and inspiration.
- Any combination of up to four classes, with total level and Proficiency Bonus calculated across classes.
- Subclass assignment filtered by parent class, with an All view for custom choices.
- All six ability scores and saving throws, including proficiency and miscellaneous modifiers.
- All 18 standard skills grouped by governing ability, with proficiency, expertise, and miscellaneous modifiers.
- HP, temporary HP, Armor Class, speed, initiative, exhaustion, death saves, and passive Perception, Insight, and Investigation.
- Languages, Origin Feat, tool proficiencies, armor training, weapon training, size, senses, and general proficiencies.

### Rules-aware builder and catalogs

- Structured species, background, feat, class-feature, subclass-feature, and item grant records.
- Review screen before pending grants are applied or skipped.
- Stable catalog IDs, source label, option type, prerequisites, gained level, class associations, and grant value.
- On-device diagnostics for missing catalogs, invalid metadata, and duplicate IDs.
- Strict import validation for annotated metadata records.
- Built-in FAP file assets plus user-editable app-data overlays.
- Separate community metadata pack and generator script.
- Catalog picker for species, backgrounds, classes, subclasses, feats/features, spells, and items.
- Hold OK on supported name fields to retain custom text entry.
- Stable zero-allocation translation hooks for top-level navigation and profile actions; catalog and campaign text remains externalized.

### Multiclass and feature resources

- Per-class level, subclass, Hit Point Die, and current/maximum Hit Dice pools.
- Features retain source class and class level gained.
- Feature resources can use manual, Proficiency Bonus, or ability-modifier maximums.
- Recovery choices include manual, turn, encounter, dawn, Short Rest, and Long Rest cadences.

### Spellcasting

- Known, Prepared, Always Prepared, Ritual, and free-casts-per-rest states.
- Grant source and grant name for spells received from species, background, feat, subclass, or item choices.
- Per-class casting ability, casting mode, cantrip limit, prepared limit, spellbook size, Pact slots, Mystic Arcanum mask, and spell points.
- Multiclass shared-slot calculation kept separate from each class’s known and prepared spells.
- Separate Pact Magic and spell-point/custom tracking.
- Spell attack, spell save DC, editable miscellaneous modifiers, and slot spending.
- Long Rest slot recovery and a dedicated Arcane Recovery helper.
- Filters for class, spell level/cantrips, ritual, school, source, and tracked preparation state.

### Combat and dice

- Generic animated roller for common dice, modifiers, Advantage, and Disadvantage.
- Every multi-die result shows the individual rolls, dice sum, modifier, and final total.
- Weapon attack and damage rolls use current character and item data.
- Critical damage, finesse/ranged ability selection, magic bonuses, ammunition, versatile damage, and configurable extra damage riders.
- Attack templates for unarmed strikes, spell attacks, saving-throw actions, and custom actions.
- Template fields include attack/save ability, save DC, damage dice, rider dice, damage types, and Mastery name.
- Conditions, concentration, reactions, temporary effects, resistances, immunities, vulnerabilities, senses, and movement modes.
- Short Rest, class-specific Hit Dice spending, Long Rest, death saves, exhaustion, and renewable resources.

### Initiative

- Saved lightweight party roster with names and initiative modifiers.
- Active character insertion, temporary participants, manual rolls, automatic rolls, sorting, tie reordering, round tracking, and current-turn selection.
- Per-participant current/maximum HP, Armor Class, and conditions.
- Encounter history with undo for accidental turn, HP, and feature-resource changes.
- Resume combat after leaving the app or changing screens.

### Inventory and currency

- Item name, notes, quantity, weight, equipped/attuned state, container assignment, charges, and ammunition groups.
- Weapon properties, attack ability, proficiency, damage dice/type, extra riders, and ammunition use.
- Armor base, Dexterity cap, and shield bonus fields with calculated Armor Class.
- Carried and equipped weight totals, Strength-based carrying capacity, and an override.
- Standard carrying-capacity and optional encumbrance tracking.
- Attunement warning above three items.
- Copper, Silver, Electrum, Gold, and Platinum tracking with standard conversion/normalization.

### Journal and milestones

- Quick, adventure, item, and milestone notes.
- Completed status, milestone level assignment, and class-specific advancement.
- Create an inventory item directly from a journal entry.

### Adventure mode

- Data-driven scene and choice engine loaded from a campaign asset only while in use.
- Skill checks use the active character's current skill modifier and show the natural roll, modifier, total, and outcome.
- Success/failure branching, per-character quest flags, achievements, inventory rewards, and milestone rewards.
- Compact sprite-and-text scene layouts with multiple selectable choices.
- Per-character scene location and manual checkpoint save/restore.
- Bundled Reef Wardens sample adventure plus an editable SD-card override.
- Long OK saves a checkpoint, long Left restores it, and long Right restarts the sample adventure.

### Monsters and encounters

- Disk-backed monster index with compact challenge, XP, Armor Class, Hit Point, and type records.
- On-device stat-block browser for abilities, movement, skills, defenses, senses, languages, traits, actions, and special actions.
- Maximum-challenge and creature-type filters for the monster browser.
- Case-insensitive monster-name search that combines with challenge and type filters without retaining a second index in RAM.
- Low, Moderate, and High random encounter generation from party level and party size.
- Environment themes for Aquatic, Dungeon, Planar, Urban, and Wilderness encounters, plus an Any setting.
- Toggle between repeated creature types and mixed-only encounter composition.
- Balanced, Horde, and Elite encounter templates alter challenge selection and target budget usage.
- Environment selection is weighted: matching creatures are preferred while occasional off-theme creatures keep sparse packs usable.
- Encounter XP is budgeted per character and generated groups do not exceed the selected budget.
- Default safety limits avoid above-level solo threats and unwieldy groups; rerolling remains available.
- Hold Right on a generated encounter to copy its creatures into Initiative with names, HP, Armor Class, and Dexterity-based initiative modifiers.
- Bundled records and optional user monster packs use separate index and stat-block files.
- On-device pack diagnostics count valid blocks, missing blocks, and duplicate IDs.
- Versioned monster-pack headers and field-level checks for identity, movement, abilities, senses, languages, and actions.
- A ready-to-copy community monster-pack template documents every supported field.
- On-device custom monster creation for identity, challenge/XP, AC, HP, type, environment, speed, six abilities, senses, languages, traits, and actions.
- Custom monsters receive stable filename-safe IDs and are appended to the user SD-card pack.
- Custom stat blocks are written through a temporary file before the index is appended, preventing incomplete records after interrupted writes.
- Pack Diagnostics reports current free heap and largest allocatable block for physical-device troubleshooting.
- Twenty bundled creatures provide a wider challenge and environment spread; half are original, freely redistributable Dungeons & Dolphins creatures.
- Monster pack schema 1 is stable and documented in `MONSTER_PACK_SCHEMA.md`.
- Index scanning and one-stat-block-at-a-time loading keep monster data out of steady-state RAM.

## Controls

- Up/Down: select a row.
- Left/Right: adjust the selected value.
- Short OK: open, toggle, apply, or roll.
- Long OK: custom text, alternate picker view, skip a grant, or a screen-specific action.
- Back: return; leaving the Home screen exits after autosave.

Screen headers and status messages describe special controls where they differ.

## Asset locations

Bundled catalogs are installed with the FAP under the application asset directory. User overlays and character saves live under:

`/ext/apps_data/dungeons_and_dolphins/`

Copy the provided `sd_card/apps_data/dungeons_and_dolphins/` tree to the SD card only when you want editable overrides. Files in app-data are merged after bundled catalogs. Optional monster records use `monsters/index.txt` and `monsters/statblocks/{id}.txt` beneath the same app-data directory.

## Build

Place this source directory in RogueMaster’s `applications_user` tree and run:

```sh
./fbt fap_dungeons_and_dolphins
```

The release ZIP intentionally excludes `dist` and compiled FAP files. Build output is created by the firmware toolchain.

Run host-side rules, catalog, parser, checksum, migration, filter, and packaging checks with:

```sh
./tests/run_host_tests.sh
```

## Hardware status

The source is compiler-verified against RogueMaster. Physical-device navigation, SD-removal behavior, and long-session heap behavior still require device testing.

See `COMPATIBILITY.md`, `ACCESSIBILITY.md`, `CATALOG_POLICY.md`, `SAVE_SCHEMA.md`, and `DEVICE_TEST_MATRIX.md` for the stable-release contracts and remaining hardware checks.
