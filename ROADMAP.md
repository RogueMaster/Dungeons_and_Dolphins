# Dungeons & Dolphins roadmap

Future releases only. Released work belongs in `CHANGELOG.md`.

## Persistence rules

Save structures are frozen by default. Do not change a schema merely for write safety, validation, cleanup or implementation convenience. Add persisted fields only when a new feature truly needs information that cannot be derived from current state. Add campaign variables only when a future campaign feature genuinely requires new persisted state.

Preserve explicit full-path FAP handoffs, teardown-before-launch, combat-only lazy eight-record Inventory/Spellbook paging, eight-record Feature paging, storage-backed Journal/profile paging, write-only character shadows, explicit DNDInventory-only starting-equipment grant, explicit Grant Initial Traits gating, lazy/no-hash progression metadata reads, and bounded Bestiary/campaign access. Character-owned Inventory, Spellbook, Feature and applied-grant sidecars remain centralized under `/ext/apps_data/dndolphins/` even though Inventory and Spellbook have their own FAPs.

## 3.3.4 — Hardware validation and bounded profile access

- Hardware-validate Wizard Combat → Spell Attacks eligibility: cantrips available, prepared/always-prepared level-1+ spells available, unprepared Wizard spells excluded unless a Free Cast remains, and free-only unprepared spells exposing no normal slot/Pact/point options.
- Hardware-validate the explicit one-time Inventory regrant (`InitialInventory=1` → `2`), including preservation of existing Items/currency, failed-write non-consumption, second-override rejection and no background seed work outside the explicit action.
- Hardware-validate companion Back behavior on all six FAPs: Short Back from main returns to DNDolphins when present; Hold Back from main exits to firmware and never launches the parent.
- Verify routine successful save/add/catalog/action statuses clear on the next real input while actionable errors remain visible.
- Hardware-test DNDolphins loader headroom plus repeated DNDolphins ↔ DNDInventory / DNDSpellbook handoffs, including low-free-heap conditions.
- Hardware/build-validate direct companion active-profile reads across all FAPs, including repeated Inventory/Spellbook launch cycles, absent metadata, stale `Active=<id>`, nonzero IDs, empty/populated collection sidecars and the main-screen-only top-right `[id]` indicator; keep Journal/Initiative/Bestiary independent of the full storage module solely for this lookup.
- Hardware-validate Item/Spell immediate persistence, 7→8→9 and 15→16→17 page boundaries, delete/restart behavior, catalog page `<>`, Item magic `*`, Spell Class `All Classes`, opt-in `All Spells`, one-shot Add status and Hold-OK `[X]` quick-action acknowledgement after the split.
- Hardware-validate Inventory/Spellbook list responsiveness and paging (`+ Add New` retention through four records, cache-only Up/Down, page-boundary responsiveness without persistent main-list `<>`, page-zero wrap recovery and Spellbook A/P/K/F marks), plus Inventory-owned currency: Currency edits/normalization, starting-equipment coin grants, currency-only Inventory followed by Grant Initial Inventory, ignored character-profile `Currency=` lines, and preservation across Weapon Combat ammunition updates and Adventure/Journal Item rewards.
- Continue expanding verified deterministic class/subclass/species progression metadata while leaving player choices explicit.
- Add a bounded level-up results/review screen that combines numeric rule changes, deterministic traits, spell-choice notices and pending ASI/Feat choices without retaining progression metadata.
- Validate lazy Feature/applied-grant first-write behavior and progression idempotency across larger real character histories; do not reintroduce embedded progression migration.
- Prototype bounded per-FAP profile projections for DNDInventory, DNDSpellbook and DNDAdventure so companion apps stop embedding the full character core. Keep the canonical character file unchanged: projections must be streamed by field name and write back only the fields each FAP legitimately owns.
- Split the broad storage implementation into narrow collection/profile interfaces only where measured RogueMaster `.fap` or heap results justify it; do not duplicate parsers merely for source-file aesthetics.

## 3.4 — Combat and Initiative

- Expand structured spell-combat mappings, including upcasting, multiple attacks and secondary effects.
- Improve Combat presentation without growing resident lists or re-linking the full Spellbook/Inventory UIs into DNDolphins.
- If hardware Loader testing still shows marginal DNDolphins headroom, evaluate a standalone `DNDCombat` FAP owning Weapon Combat, Spell Combat and attack-template execution. It should stream the existing character/Inventory/Spellbook sidecars, return through the existing handoff model and add no new persisted state. Do not split Combat solely for code organization if Loader headroom is already healthy.
- Continue improving Initiative roster-to-combat presentation and larger-encounter navigation without increasing resident state.
- Add completed-combat history only as a new Initiative-owned record if explicitly selected.

## 3.5 — Scale and compatibility

- Stress large character, Journal, campaign and monster indexes on hardware.
- Add narrow compatibility aliases only for demonstrated real files; avoid broad schema migrations.
- Continue hardware-driven stack reductions, especially Bestiary encounter writer paths if needed.

## Later

- More class/spell rules coverage.
- Better inventory/container/ammunition workflows using existing state where possible.
- More declarative campaigns and monster content.
- Accessibility and long-text improvements across all seven FAPs.

## Out of scope

- Executable campaign scripting.
- Journal-created Adventure progress.
- Re-embedding companion-app state into character saves.
- Whole-file list loading where bounded streaming/paging works.
- Save-format changes made only for validation or atomicity.
