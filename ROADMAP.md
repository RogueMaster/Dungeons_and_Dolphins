# Dungeons & Dolphins roadmap

Future releases only. Released work belongs in `CHANGELOG.md`.

## Persistence rules

Save structures are frozen by default. Do not change a schema merely for write safety, validation, cleanup or implementation convenience. Add persisted fields only when a new feature truly needs information that cannot be derived from current state. Keep ownership local to the app that uses the data and keep tolerant named-field loading.

Preserve explicit full-path FAP handoffs, teardown-before-launch, eight-record spell/item paging, storage-backed Journal/profile paging, write-only character shadows, Inventory-only starting-inventory initialization, and bounded Bestiary/campaign access.

## 3.2.19 — Adventure, progression and pack polish

- Add an explicit level-up choice queue for rewards that cannot be selected deterministically: subclass, ASI/feat, learned spells, Fighting Style, invocations, metamagic and similar choices. Keep deterministic resource/fixed-grant synchronization separate, preserve explicit-initial-action/level-up-only eight-line metadata scans, and never retain a progression catalog/signature after those operations.
- Add campaign-pack removal and a concise pre-install validation/details screen.
- Improve campaign discovery for larger installed libraries with bounded index hints.
- Improve milestone/achievement presentation and Journal continuation handoff.
- Add campaign variables only if a selected feature requires genuinely new persisted state.

## 3.3 — Combat and Initiative

- Expand structured spell-combat mappings, including upcasting, multiple attacks and secondary effects.
- Improve Combat presentation without growing resident lists.
- Improve roster-to-combat presentation and combat-state review without regressing current editable names, HP/AC/conditions or manual initiative entry.
- Add completed-combat history only as a new Initiative-owned record if explicitly selected.

## 3.4 — Scale and compatibility

- Stress large character, Journal, campaign and monster indexes on hardware.
- Add narrow compatibility aliases only for demonstrated real files; avoid broad schema migrations.
- Continue hardware-driven stack reductions, especially Bestiary encounter writer paths if needed.

## Later

- More class/spell rules coverage.
- Better inventory/container/ammunition workflows using existing state where possible.
- More declarative campaigns and monster content.
- Accessibility and long-text improvements across all five FAPs.

## Out of scope

- Executable campaign scripting.
- Journal-created Adventure progress.
- Re-embedding companion-app state into character saves.
- Whole-file list loading where bounded streaming/paging works.
- Save-format changes made only for validation or atomicity.
