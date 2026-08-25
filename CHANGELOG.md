# Dungeons & Dolphins changelog

## Unreleased documentation update

- Audited every completed roadmap claim against the implemented code, packaged assets, tests, and build records.
- Consolidated completed release history in this changelog and removed it from the roadmap.
- Replaced the roadmap with release-scoped future work containing exactly three testable features per release.

## 2.0

- Declared monster pack schema 1 stable and documented its compatibility contract.
- Made custom stat-block writes interruption-resistant by publishing the block before its index record.
- Added live free-heap and largest-block readings to Pack Diagnostics.
- Expanded the bundled compendium to twenty creatures with ten original, freely redistributable entries.
- Completed the five-release monster and encounter roadmap established after version 1.5.

## 1.9

- Added an on-device custom monster editor.
- Added editable challenge/XP, defenses, identity, environment, movement, abilities, senses, languages, traits, and actions.
- Added direct persistence into the versioned user monster pack with collision-resistant IDs.

## 1.8

- Added an explicit monster-pack format version.
- Added field-level validation for required stat-block sections.
- Added bundled/user version reporting and incompatible-version warnings to Pack Diagnostics.

## 1.7

- Added Balanced, Horde, and Elite encounter templates.
- Added weighted environment selection so themed creatures are preferred without making small packs unusable.
- Tuned target-budget usage and creature-count limits by template.

## 1.6

- Added case-insensitive monster-name search.
- Combined name, maximum-challenge, and creature-type filters in the streaming browser.

## 1.5

- Added on-device monster-pack diagnostics.
- Added checks for missing stat blocks and duplicate stable IDs.
- Added a ready-to-copy community monster-pack template with a complete example record.

## 1.4

- Added environment metadata and themed encounter generation.
- Added Aquatic, Dungeon, Planar, Urban, Wilderness, and unrestricted themes.
- Added a composition toggle for repeated creature types versus mixed-only groups.

## 1.3

- Added generated-encounter transfer into Initiative.
- Prefilled monster names, quantities, HP, Armor Class, and Dexterity-based initiative modifiers.
- Capped transfers safely at the initiative tracker capacity.

## 1.2

- Added challenge-rating and creature-type filters to the monster browser.
- Made filter changes immediately rebuild the disk-backed result count without retaining an in-memory catalog.

## 1.1

- Added a disk-backed monster compendium and on-device stat-block browser.
- Added party-level, party-size, and difficulty controls for random encounters.
- Added per-character XP budgets for Low, Moderate, and High encounters.
- Added safety limits for above-level creatures, oversized groups, and over-budget results.
- Added bundled monster assets and a documented SD-card extension format.
- Kept the monster index streaming and stat blocks lazy-loaded to protect heap memory.
- Updated the feature list, tests, roadmap, and build verification for the new release.

## 1.0

- Declared schema 1 stable for future forward migrations.
- Retained one prior successful save generation for every active profile.
- Added an explicit profile backup-restore action alongside checksum verification.
- Added zero-allocation translation hooks and a community translation template.
- Added compatibility, accessibility, catalog policy, stable schema, and reproducible-build documents.
- Added a release verification script that runs host tests and the RogueMaster FAP target.
- Finalized source-only release packaging with no compiled FAP or `dist` directory.

## 0.9

- Froze character save schema 1 and documented its compatibility contract.
- Replaced compiler-layout checksums with checksums over canonical serialized file bytes.
- Added automatic one-time migration from the 0.8 text layout.
- Added profile actions for rename, duplicate, export, first-valid-export import, archive, delete, and checksum verification.
- Kept duplicate, export, and archive operations chunked so they do not require a second character-sized allocation.
- Added host-side tests for calculations, dice bounds, catalogs, metadata IDs, parsers, checksums, migration acceptance, spell filtering, manifest fields, and release-document wording.
- Added a physical-device test matrix for hardware verification.

## 0.8

- Added a data-driven Adventure mode with compact sprite-and-text scenes.
- Added selectable choices, character-based skill checks, and success/failure branches.
- Added per-character story location, quest flags, achievements, and checkpoints.
- Added adventure inventory rewards and milestone journal rewards with duplicate protection.
- Added a bundled sample campaign and editable SD-card campaign override.
- Kept campaign scene memory lazy and released it when Adventure mode closes.

## 0.7

- Changed the internal application ID and SD namespace to `dungeons_and_dolphins`.
- Added FAP-packaged file assets for catalogs and metadata, with app-data overlays for user changes.
- Reworked catalog memory into lazy heap allocations that grow only while a picker is open and are released on exit.
- Replaced full-name diagnostic storage with compact hashes and removed the second full-character allocation during save.
- Added graceful allocation failure handling and catalog-memory status reporting.
- Added structured grants with a review/apply/skip screen.
- Added species, Origin Feat, tool proficiency, armor training, weapon training, size, and senses fields.
- Added annotated catalog metadata, strict import validation, and on-device catalog diagnostics.
- Added class-specific Hit Point Dice and pools for multiclass characters.
- Added per-class spellcasting modes, abilities, limits, spellbook size, Pact slots, Mystic Arcanum, and spell points.
- Added multiclass shared-slot calculation and spell filters.
- Added spell stable ID, source, school, ritual, grant type, and grant-name tracking.
- Added conditions, concentration, reactions, temporary effects, defenses, movement modes, and attack templates.
- Added initiative HP, Armor Class, conditions, encounter history, and undo for turn, HP, and feature-resource changes.
- Added containers, weights, carrying capacity, Armor Class formulas, attunement warnings, ammunition groups, and charges.
- Added resource formulas and recovery cadences beyond rests.
- Added coin normalization and optional encumbrance tracking.
- Counted container contents in total carried weight while retaining container organization.
- Added a community-maintainable metadata pack and generator.
- Updated the roadmap and full feature documentation.

## 0.6

- Added SD-card-limited profiles with dynamic profile indexing.
- Added profile filenames in `ch_{id}_{characterName}_{characterLevel}.txt` format.
- Expanded class, subclass, spell, item, background, and feature-name catalogs.
- Added complete README and changelog documents.
- Used the manifest version macro in the About screen.

## 0.3

- Added separate readable character save files, profile switching, custom catalogs, dice animation, autosave, and the 10×10 application icon.

## 0.2

- Added all saving throws and skills, miscellaneous modifiers, passive statistics, expanded spell states, and free-cast recovery.

## 0.1

- Added the initial character tracker, multiclass records, inventory, spells, journal, weapon rolls, party roster, and initiative tracker.
