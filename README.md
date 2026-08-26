# Dungeons & Dolphins

Dungeons & Dolphins 2.6 is an offline, 5E-compatible Flipper Zero toolkit for RogueMaster. One source tree and one `application.fam` build two independent FAPs with explicit source lists:

- **Dungeons & Dolphins** — character profiles, rules tracking, spells, equipment, notes, dice, combat, initiative, and campaigns.
- **Dolphin Bestiary** — the monster stat-block browser, custom monsters, diagnostics, and party-level encounter generation.

Both applications use only the screen, buttons, and SD card. No network or external hardware is required.

## Full feature list

### Character profiles and saves

- Dynamically indexed profiles with no fixed profile-count limit.
- One readable, versioned, checksummed text save per character under `profiles/`.
- Save names use `ch_{id}_{characterName}_{characterLevel}.txt`.
- Main and newly created heroes remain visible in Characters even when the first write fails.
- Transactional new-character creation restores the previous active character after a failed write.
- Immediate autosave after character, stat, spell, feature, item, inventory, currency, journal, party, initiative, or resource changes.
- Atomic temporary-file publication, retained backup generation, checksum verification, and manual backup restore.
- Rename, switch, duplicate, export, import, archive, delete, verify, and restore profile actions.
- Save failures show a persistent UNSAVED warning; Save Now retries SD access.

### Character sheet

- Name, player, species, background, alignment, experience, milestone leveling, and inspiration.
- Up to four classes with independent class level, subclass, Hit Point Die, Hit Dice pool, and feature ownership.
- Total level and Proficiency Bonus calculated across all classes.
- All six ability scores and saving throws with proficiency and miscellaneous modifiers.
- All 18 skills grouped by their governing STR, DEX, INT, WIS, or CHA ability, including proficiency, expertise, and miscellaneous modifiers.
- HP, temporary HP, Armor Class, speed, initiative, exhaustion, death saves, and class-specific Hit Dice spending.
- Pass. Perception, Pass. Insight, and Pass. Investigation remain fully visible and editable without abbreviating the governing attribute.
- Languages, Origin Feat, tools, armor training, weapon training, size, senses, and general proficiencies.

### Classes, subclasses, features, and grants

- Every bundled class has associated subclass choices, including all four core 2024 choices for each of the twelve core classes; class filtering is the default and an All view remains available.
- Class-linked features retain the granting class and level gained.
- Feature uses support manual, Proficiency Bonus, or ability-modifier maximums.
- Recharge choices include turn, encounter, dawn, Short Rest, Long Rest, or manual recovery.
- Structured grants retain stable ID, source, option type, prerequisites, class association, level gained, payload, and apply/skip state.
- Species, background, feat, subclass, class-feature, item, and custom grants can be reviewed before application.
- Short OK opens an SD-backed picker; hold OK retains custom text entry where supported.

### Catalogs and low-memory browsing

- Packaged catalogs cover classes, subclasses, species, backgrounds, feats/features, spells, and items.
- Custom reference files use the `custom_` filename prefix and are merged after packaged records.
- Item and spell pickers stream from SD and retain at most 50 matching records in RAM.
- Left/Right changes the current 50-record catalog page; buffers are released immediately when the picker closes.
- Catalog source files, long descriptions, campaigns, and language data are not retained in steady-state RAM.
- Annotated spell rows can include level, class associations, school, ritual state, and source category.
- Optional structured metadata and runtime language packs use bounded allocations with English fallback.

### Spellcasting

- Known, Prepared, Always Prepared, Ritual, and free-casts-per-rest state per spell.
- Spell grant source and grant name for species, background, feat, subclass, item, and custom grants.
- Per-class spellcasting ability, casting mode, cantrip limit, prepared limit, spellbook size, Pact slots, Mystic Arcanum mask, and spell points.
- Shared multiclass slots remain separate from each class's known and prepared spells.
- Spell attack modifier, Spell Save DC, editable miscellaneous modifiers, and slot spending.
- Long Rest recovery and a dedicated Arcane Recovery helper.
- Filters for class, level/cantrip, ritual, school, source category, and prepared status.
- Magic includes an explicit Back to Main Menu row in addition to the Back button.

### Inventory and currency

- Item name, notes, quantity, weight, container, equipped/attuned state, charges, and ammunition group.
- Weapon properties, attack ability, proficiency, magic bonus, damage dice/type, versatile damage, and riders.
- Armor base, Dexterity cap, shield bonus, and calculated Armor Class.
- Carried/equipped weight, Strength-based capacity, optional override, containers, and optional encumbrance.
- Attunement warning above three items.
- Copper, Silver, Electrum, Gold, and Platinum tracking with standard normalization.

### Combat, attacks, and dice

- Combat contains weapon attacks, attack templates, initiative, HP, temporary HP, rests, class Hit Dice, death saves, and exhaustion.
- Attack templates were moved into Combat; the redundant Combat Sheet screen was removed.
- Template fields cover weapon/unarmed/spell/save/custom actions, ability, attack modifier, save DC, damage, Mastery, riders, and recharge.
- Conditions, concentration, reaction state, temporary effects, resistances, immunities, vulnerabilities, senses, and movement remain character fields.
- Weapon attack and damage rolls use current character and item data.
- Critical damage, Advantage/Disadvantage, finesse/ranged selection, magic bonuses, versatile damage, ammunition, and extra riders.
- Generic animated rolls support d4, d6, d8, d10, d12, d20, d100, modifiers, Advantage, and Disadvantage.
- Roll Now remains unchanged while configuring a modifier and changes only after a roll is made.
- Multi-die results show every individual die, the dice sum, modifier, and final total.

### Initiative, journal, and campaigns

- Per-character party presets with names and initiative modifiers.
- Current-character insertion, temporary participants, manual/automatic rolls, sorting, tie reordering, rounds, and current turn.
- Per-participant HP, Armor Class, and conditions, plus history/undo for turn, HP, and resource mistakes.
- Quick, adventure, item, and milestone notes with completion and class-level advancement.
- Inventory items can be created from journal entries.
- Disk-backed campaign manifests, per-profile/per-campaign progress, checkpoints, quest flags, achievements, branches, skill checks, and rewards.
- Campaign diagnostics detect incompatible manifests, missing scenes, duplicate IDs, and broken links.

### Dolphin Bestiary

- Separate FAP and asset namespace, so the character tracker never loads monster tables or encounter state.
- 340 unique packaged monster records, including 120 records added in 2.6.
- Monster browser filters combine name, maximum challenge, creature type, source category, environment, and encounter role.
- Browser pages hold at most 50 summaries; full stat blocks are allocated only while one is open and released on exit.
- Stat blocks show challenge, XP, Armor Class, HP, type, source, role, size, movement, abilities, skills, defenses, senses, languages, traits, actions, and extra actions.
- Low, Moderate, and High encounter budgets use party level and party size.
- Aquatic, Dungeon, Planar, Urban, Wilderness, or unrestricted encounter environments.
- Balanced, Horde, and Elite templates, repeated-creature control, and optional Leader, Controller, Skirmisher, Artillery, Brute, or Minion weighting.
- Custom monster creation and editing preserve stable IDs.
- Two-step deletion applies only to custom records; packaged records are read-only.
- Atomic custom index replacement, transaction recovery, block backup, and rollback.
- Pack diagnostics report valid/invalid records, pack versions, recovery results, and free heap.

### Interface changes in 2.6

- Removed the About section.
- Removed the separate Character Builder entry; character, class, catalog, and grant screens already provide the relevant editing flows.
- Removed the redundant Combat Sheet entry.
- Removed monster and encounter screens from the character FAP.
- Added a 10×10, 1-bit d20 icon showing a roll of 20.

## Controls

- Up/Down: move through rows.
- Left/Right: adjust a value or change a 50-record page.
- Short OK: open, toggle, apply, save, or roll.
- Long OK: custom text, alternate spell/subclass view, or screen-specific action.
- Back: return to the prior screen; Back from the main screen exits.

## SD-card asset locations

All runtime files are accessed through `APP_ASSETS_PATH`; the code does not use `APP_DATA_PATH`.

- Character assets and profiles: `/ext/apps_assets/dungeons_and_dolphins/`
- Bestiary assets and custom monsters: `/ext/apps_assets/dolphin_bestiary/`

Packaged records use their normal filenames. User-maintained reference indexes and overlays use names such as `custom_spells.txt`, `custom_items.txt`, `custom_options.txt`, and `custom_index.txt`. Character saves remain in the requested `ch_{id}_{name}_{level}.txt` format beneath `profiles/`.

The optional `sd_card/apps_assets/` tree contains ready-to-copy custom-file templates.

## Build

Place this directory in RogueMaster's `applications_user` tree, then build both targets from the same manifest:

```sh
./fbt fap_dungeons_and_dolphins fap_dolphin_bestiary
```

The manifest uses explicit `sources=[...]` lists so each FAP excludes the other application's entry point and feature modules. Run host checks with:

```sh
./tests/run_host_tests.sh
```

Release ZIPs contain source and assets only. They intentionally exclude `dist`, compiled FAPs, ELF files, and object files.

## Hardware status

Firmware compilation and automated validation are recorded in `BUILD_VERIFICATION.md`. Physical-device results remain unclaimed until the cases in `DEVICE_TEST_MATRIX.md` are completed on hardware.
