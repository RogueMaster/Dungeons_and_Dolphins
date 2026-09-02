# Changelog

## 3.6 — Catalog compliance, grants, performance and documentation

- Strengthened SRD catalog/grant validation across Character, Spellbook, Inventory, proficiencies, starting equipment, abilities/features and trinkets.
- Expanded SRD grant coverage and cross-checked **475 fixed SRD grant payloads** against SRD-valid spell, feat, language, skill, save, proficiency, size, resistance, sense and speed values.
- Kept DNDInventory as the exclusive owner of starting-equipment writes and added explicit **Review inventory grant** confirmation; primary-class saving throws and starting languages use reviewed grants.
- Consolidated Character and Spellbook onto one canonical **355-spell SRD catalog**, retained the Any/Cantrip/Level 1–9 Spellbook filter, and added Known/knowable/free-granted totals.
- Restored the 36 Homebrew/DNDolphins Item rows to the normal Item catalog, renamed user-facing **Extra Items** to **Get Elevated**, and retained the legacy `ExtraItems=` key only for save compatibility. Get Elevated controls randomized bundle granting; Homebrew controls catalog visibility.
- Corrected generic **Unarmed Strike** math: attack = ability modifier + Proficiency Bonus, damage = `1 + ability modifier`, and Grapple/Shove DC = `8 + ability modifier + Proficiency Bonus` before applicable attack-only modifiers.
- Reduced avoidable storage/heap work: Settings reads in 128-byte chunks with best-effort per-line recovery, DNDolphins defers the complete `Catalog: All` availability check until Settings is opened, Spellbook status filtering uses a fixed **128-byte negative prefilter** before exact matches, and the release gate keeps direct heap/storage work out of **84 draw helpers**.
- Replaced the remaining legacy Pocket-prefixed constants with shared or FAP-owned DND namespaces and raised DNDInitiative from a 3 KB to **4 KB stack reservation** for additional call-chain/framework margin. Its ~5.28 KB app state remains heap-owned and is not part of the thread stack.
- Simplified reconstructable settings persistence so shared and Bestiary party settings no longer leave settings-specific `.tmp`/`.bak` companions; transactional recovery files for character/collection data remain intact.
- Host validation passes all seven strict FAP manifest links plus ASan/UBSan Storage, Character, Inventory and Spellbook tests, 320-record collection paths, failure rollback and catalog/grant audits. No confirmed project allocation leak remains in exercised paths; physical-device heap/stack validation remains a target test.
- Updated README, feature checklist, catalog/grant audits, memory audit and roadmap to reflect completed behavior and remaining work.

## 4.19.0a–c — Scalable collections, shared settings and table usability

- Removed former small owned Spell/Item/Feature ceilings while retaining bounded eight-record pages and stable collection ownership.
- Added shared Settings, SHD bundles/restore with rollback support, and scalable catalog-backed Languages and Proficiencies sidecars.
- Improved Combat casting information, Inventory ordering/quantity/filter controls, starting-equipment coverage and Spellbook sorting.
- Preserved bounded storage behavior and compatibility with older snapshots.

## 4.19 — Spell filters

- Added **Character Classes** as the default Spellbook class filter plus **Any Class** and bundled class filters.
- `Allowed` applies character spell-list/level eligibility; `All Spells` browses the selected catalog class directly.

## 3.5.x — Grants, choices, Inventory and Adventure

- Added progression Feat **Allowed/All** filtering, Level Choices, explicit deterministic grant actions and stronger grant persistence/status handling.
- Added completed-encounter history, Stack Qty editing, safer container deletion/remapping and improved Inventory paging.
- Added loose ammunition matching and consumption, Adventure full-scene viewing/checkpoint controls and additional bundled project content.
- Added fixed-average level-up HP/Hit Dice refresh and Journal milestone leveling integration.

## 3.4.x — Profile projections and navigation

- Added bounded Level-Up Review, narrow streamed profile projections and expanded structured spell-combat mappings.
- Improved Initiative large-roster navigation and DNDolphins Home/Combat menu organization.

## 3.3.x — Split FAP ownership and bounded collections

- Consolidated active-profile handoff and companion return-focus behavior.
- Moved Inventory/Currency and Spellbook ownership into their standalone FAPs with bounded paging, editors and immediate persistence.
- Added one-time starting-inventory regrant, generic Spell Scroll rows, Rituals combat support and draw-path stability work.
- Moved app-specific helpers out of shared modules and tightened allocation cleanup.
