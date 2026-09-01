# Source ownership

The project uses shared source only when behavior or storage mechanics are genuinely reused. A header is an interface, not an inventory of everything implemented in its `.c` file.

## Header rule

- Keep implementation-only helpers `static` in the owning `.c` file.
- Put a function declaration in a header only when another translation unit calls it.
- Put app-only declarations and result/state types in an app-owned header rather than a broad shared header.
- Keep shared data/rule/storage declarations narrow enough that unrelated FAPs do not need app-specific types merely to use one shared primitive.
- Do not suppress `unused-function` warnings. If a moved call leaves a private wrapper unused, remove the stale wrapper after confirming the real implementation still has callers.

## Shared modules retained intentionally

- `dnd_data.*`: shared character/record allocation, defaults and sanitize/load support. A few convenience reserve functions currently have one UI caller, but they reuse the shared capacity allocator and are not duplicated into app code.
- `dnd_handoff.*`: cross-FAP launch behavior used throughout the suite.
- `dnd_profile_ref.*`: lightweight exact active-profile/path handling shared by companion FAPs that do not link the full character storage implementation.
- `dnd_rules_core.c` / `dnd_rules.h`: compact rule math needed across FAPs. DNDolphins-only roll-mode/value-recording dice and Initiative/effective-speed helpers are outside this interface. The save-modifier routine remains here even though its current UI caller is DNDolphins because it directly shares the core proficiency/exhaustion calculation; moving it would duplicate that primitive for no ownership or size benefit.
- `dnd_spell_eligibility.*`: only the class maximum-spell-level calculation remains because both DNDolphins and DNDSpellbook use it.
- `dnd_weapon_rules.*`: weapon ability/attack modifier math shared by DNDolphins and DNDInventory.
- `dnd_storage.*`: canonical character/sidecar parsing, bounded paging and transactional rewrite/publish mechanics. Derived UI state does not belong here. A storage operation may remain here with one current caller when moving it would duplicate a parser, path scanner or atomic rewrite path.

## App-owned code

- DNDolphins: `dndolphins_rules_character.*`, `dndolphins_dice.*`, `dndolphins_spells.*`, `dndolphins_spell_combat.*`, `dndolphins_weapon_combat.*`, `dndolphins_progression_store.*` and the main hub source.
- DNDInventory: collection, Item policy/rules and derived equipped/weight aggregation remain Inventory-owned.
- DNDSpellbook: collection UI, catalog filtering, Wizard-list alias selection and deterministic level/name sorting remain Spellbook-owned.
- DNDAdventure, DNDJournal, DNDInitiative and DNDBestiary keep their feature-specific support modules rather than linking broader DNDolphins implementations.

## Stability constraint

Ownership cleanup must not change persisted schemas, collection ordering semantics, active-profile selection, FAP handoff paths, lazy paging, draw-time I/O rules or resource-consumption behavior. Parser/rewrite code is split only when it can remain single-source and behaviorally identical.
