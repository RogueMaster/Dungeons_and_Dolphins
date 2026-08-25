# Pocket d20 roadmap to 1.0

Version 0.3 establishes the editable tracker, independent text saves, catalogs, rolls, rests, and initiative. The following sequence targets the largest remaining character-sheet gaps without turning the Flipper into an unmaintainable rules database.

## 0.4 — rules-aware character builder

- Add structured species, background, feat, class-feature, and subclass-feature grants with a review screen before applying changes.
- Add Origin Feat, tool proficiency, armor/weapon training, size, and senses fields.
- Add class-specific Hit Point Die pools for multiclass characters.
- Add annotated catalog records with stable IDs, source book, option type, prerequisites, level gained, and class associations.
- Add import validation and an on-device catalog diagnostics screen.

## 0.5 — spellcasting model

- Calculate multiclass spell slots separately from each class's spells known/prepared.
- Add per-class spellcasting ability, prepared-count limits, spellbook size, Pact Magic slots, Mystic Arcanum, and spell-point/custom modes.
- Model always-prepared and free-cast sources from species, background, feat, subclass, and item grants.
- Add spell filters for cantrip count, class list, level, ritual, school, source, and prepared status.
- Publish a separate community-maintainable metadata pack that users populate from legally owned sources.

## 0.6 — combat sheet

- Add conditions, concentration, reactions, temporary effects, resistances, immunities, vulnerabilities, senses, and movement modes.
- Add attack templates for unarmed strikes, spell attacks, saving-throw actions, Mastery properties, and configurable damage riders.
- Add per-participant HP/AC/conditions to initiative while retaining the lightweight name-and-roll preset.
- Add encounter history and undo for accidental turn/HP/resource changes.

## 0.7 — inventory and resources

- Add containers, carried/equipped weight, carrying capacity, armor/shield AC formulas, attunement limit warnings, ammunition groups, and item charges.
- Add resource formulas tied to Proficiency Bonus or ability modifiers and recovery conditions beyond Short/Long Rest.
- Add standard coin conversion and optional encumbrance rules.

## 0.8 — adventure mode

- Connect choices, skill checks, quest flags, achievements, story location, inventory rewards, and milestones to a small data-driven fantasy adventure engine.
- Add sprite/text scene layouts and save checkpoints per character.
- Keep campaign content in separate SD packs so the tracker remains useful by itself.

## 0.9 — reliability and migration

- Freeze a documented version-1 save schema and add forward migrations, export/import, duplicate/rename character, archive, and corruption diagnostics.
- Add host-side unit tests for rules, parsers, checksums, migration, spell filtering, multiclass calculations, and dice bounds.
- Run physical-device UX tests for every screen, long-name truncation, SD removal, low-memory behavior, interrupted writes, and rapid autosave.

## 1.0 — stable release

- Publish reproducible RogueMaster build instructions and release checksums.
- Establish catalog licensing/source policy, translation hooks, accessibility conventions, and a compatibility matrix.
- Support migration for all subsequent schema changes and retain recoverable backups.
