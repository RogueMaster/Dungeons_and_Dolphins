# Dungeons & Dolphins changelog

Released work only. Normal releases are retained as concise summaries; closely related recovery spans may be consolidated when the individual troubleshooting chronology would obscure the released outcome.

## 3.3.3 — Combat spell eligibility, one-time inventory regrant and UI consistency

- Corrected Combat → Spell Attacks for Wizard characters. Wizard cantrips remain normally available; level-1+ Wizard spells appear only when Prepared/Always Prepared or when a Free Cast remains. An unprepared Wizard spell exposed solely by a Free Cast offers only that Free Cast and cannot spend ordinary slots, Pact slots or spell points from the combat picker. Other classes retain their existing Known/Prepared rules, and the existing combat damage mappings, scaling, upcasting and resource consumption remain intact.
- Added a deliberate one-time override for **Grant Initial Inventory**. Short OK keeps the normal one-shot protections. Hold OK on an already granted Inventory appends the starting class/species/background package again, adds its starting currency again, preserves existing Items, and changes the Inventory marker from `InitialInventory=1` to `InitialInventory=2`. A successful override cannot be repeated.
- Made routine successful collection/save feedback transient. `Saved`, catalog-save, add, Equip/Prepare and grant/regrant confirmations clear on the next real input, while failures such as `UNSAVED` remain visible. DNDolphins likewise clears ordinary `Saved`/`Already saved`/catalog-save notices on the next real input.
- Kept the companion navigation contract consistent across Adventure, Bestiary, Journal, Initiative, Inventory and Spellbook: Short Back from a companion main screen returns to DNDolphins when present; Hold Back exits to firmware without launching DNDolphins. Sub-screen Short Back remains local navigation, with Initiative combat retaining its Short-Back previous-turn behavior.
- Updated all seven FAP manifests to release version 3.3.3 and refreshed documentation/test coverage for the retained behavior.

## 3.3.2 — Independent Inventory/Spellbook collections and companion profile handling

- Finalized standalone DNDInventory and DNDSpellbook collection UIs with independent source/assets ownership, direct entry to `+ Add New`, full 36-field Item and 17-field Spell editors, immediate persistence, Delete, custom-name editing, Hold-OK Equip/Prepare, A/P/K/F spell state marks and bounded eight-record collection paging.
- Restored collection/catalog parity: Item and Spell Name catalogs retain explicit `P# <>` paging, Item Catalog marks magic entries with `*`, Spell Class defaults to **All Classes** across a multiclass character, and the separate opt-in **All Spells** eligibility override remains available. Normal Inventory/Spellbook lists do not show persistent `<>` hints.
- Standardized active-character lookup on `/ext/apps_data/dndolphins/custom_active_profile.txt` (`Active=<id>`). Companion FAPs do not accept launch arguments as character selectors or discover a substitute character. Adventure/Journal require the exact persisted profile; Inventory/Spellbook load that exact ID through normal character storage; Bestiary/Initiative use ID `0` only when active-profile metadata itself is absent/unreadable.
- Added `[characterId]` at the top-right of the six companion **main screens only**. Detail/editor/tool/combat/result screens retain their own headers.
- Optimized Inventory/Spellbook startup and list interaction for constrained memory: fixed UI state is reserved before variable collection loading, drawing is cache-only, no timer/background worker runs during scrolling, no-op input events avoid redundant redraws, and storage reads occur only at startup, real cache-page boundaries, explicit catalog paging, edits and tool actions.
- Preserved DNDolphins Combat access to character-owned Inventory/Spellbook sidecars. Weapon Combat retains attack/damage/ammunition behavior; Spell Combat retains the mapped spell-damage table, Notes `XdY` fallback, cantrip scaling, upcasting and class-specific casting modifiers.
- Normal DNDolphins profile load no longer displays a redundant persistent `Loaded` notice.
- Standardized companion main-menu navigation so Short Back returns to DNDolphins when present and Hold Back exits without a parent handoff; redundant normal-menu Return/Open-DNDolphins rows were removed while no-character recovery prompts remain available.

## 3.3.1 — Inventory ownership, bounded campaign discovery and Journal continuation

- Moved Currency, encumbrance/carrying resources and starting-equipment controls into DNDInventory. Inventory is the sole normal owner of `Currency=` metadata; other Item appenders preserve that metadata and never synthesize it.
- Replaced automatic first-open equipment seeding with explicit **Grant Initial Inventory**. An absent Inventory stays absent until a real Inventory write/grant; a currency-only sidecar can still receive the normal grant; existing manual Items block the normal grant. Class/species/background assets remain the primary seed and a random d100 trinket is fallback-only when the normal seed yields no equipment/currency.
- Kept character-owned Inventory, Spellbook, Feature and applied-grant state centralized under `/ext/apps_data/dndolphins/` while allowing the companion FAPs to own their interfaces and asset packs.
- Added bounded campaign discovery/index handling and preserved install/activation controls without loading the full campaign catalog into memory.
- Journal → Adventure continuation now passes only continuation intent; Adventure resolves the persisted active character and resumes that character's stored campaign progress itself.
- Completed source-ownership/API cleanup so app-specific functions stay under their owning module and shared rules/storage interfaces remain neutral.
- Retained 4 KB stacks for Inventory/Spellbook after full character parsing showed the smaller reservation lacked safe margin; Initiative remains 3 KB.

## 3.3 — Inventory/Spellbook FAP split and loader-memory reduction

- Split Inventory and Spellbook out of DNDolphins into standalone FAPs while retaining DNDolphins' lightweight streamed access needed by Combat, progression and cross-feature integration.
- Preserved the recovered collection feature set: eight-record paging, Add New, full Item/Spell editors, immediate Add/Edit/Delete persistence, Equip/Prepare quick actions, custom names, catalogs, free-cast state, multiclass **All Classes**, and spell metadata filtering.
- Moved Item Catalog/starting-equipment assets to DNDInventory and the full Spell Catalog to DNDSpellbook; neither app loads the other collection's assets.
- Kept `inventory_{charID}.txt` and `spellbook_{charID}.txt` in the shared DNDolphins character-data root so all FAPs see the same authoritative records.
- Reduced DNDolphins loader/runtime pressure by removing the collection-heavy UIs from the main FAP without shrinking its 6 KB stack.

## 3.2.32–3.2.35 — Progression, multiclass casting and spell-catalog update

- Moved persistent Features and applied progression history from the core character file into lazy `feats_{charID}.txt` and `appliedgrants_{charID}.txt` sidecars. Features use bounded eight-record paging; applied-grant IDs are streamed during progression checks.
- Added deterministic supported species progression by total character level and normalized one-shot grant IDs. Ordinary player-selected class spell choices remain explicit rather than being auto-selected.
- Added Item Catalog filtering for All, Weapons, Armor, Ammunition, Gear, Tools and Magic while preserving Inventory-list Hold OK as Equip/Unequip.
- Corrected Eldritch Knight and Arcane Trickster casting to use Third-caster progression, Intelligence and Wizard-list eligibility, including correct multiclass slot contribution while Pact Magic remains separate.
- Completed the bundled streamed Spell Catalog metadata contract with Level/Class/School/Ritual/Source/Status filtering and bounded matching pages.
- Reduced progression memory pressure by removing continuously resident Feature/Grant collections and avoiding allocation for missing/empty progression sidecars.

## 3.2.6–3.2.31 — Inventory/Spellbook regression and recovery period

- This range is intentionally summarized as one extended recovery period. Inventory and Spellbook Add/Edit/Delete persistence regressed after 3.2.5/3.2.6-era changes and remained unreliable across many intermediate builds while the app was simultaneously being split into companion FAPs, converted to sidecar storage, and optimized for Flipper memory limits.
- Adventure, Bestiary, Journal and Initiative were progressively separated/hardened during this period, including standalone Journal/Initiative/Adventure FAPs, cross-FAP handoffs, bounded readers/caches, character best-effort loading, write-only shadows, saved encounters, spell-combat support, campaign/monster packs and the restored Initiative/Journal gameplay controls.
- Inventory and Spellbook moved from embedded character arrays to per-character sidecars with bounded eight-record paging. Multiple intermediate append/create/fingerprint/cache approaches were attempted; device testing showed that several builds could enter a blank editor but still failed to create/publish the collection file, expose newly added records in-session, support repeated adds/deletes, or safely cross an eight-record page boundary.
- The recovery ultimately returned Add New to a RAM-first lifecycle: stage/grow the resident collection first, enter the editor independent of storage success, then immediately rewrite the authoritative sidecar and only mark state saved after the write succeeds. Live files became `inventory_{characterId}.txt` and `spellbook_{characterId}.txt`.
- Final recovery work removed render-time SD I/O/cache mutation, kept the newly added record visible after editing, made user edits/catalog selections/deletes commit immediately, released catalog memory before collection rewrites, and fixed repeated-add/page-boundary behavior. Starting equipment again seeds only an empty Inventory, and Spellbook regained the multiclass **All Classes** filter.
- Hold OK quick actions were restored by the end of the period: Spellbook toggles Prepared/Unprepared and Inventory toggles Equipped/Unequipped, with immediate sidecar persistence and protected always-prepared spells.
- The same period also restored/expanded class progression, proficiency/slot/resource updates, item/spell catalog workflows, spell-attack mappings and related low-memory behavior. The detailed troubleshooting chronology for the broken Inventory/Spellbook path is intentionally not repeated per intermediate revision here because it spanned 3.2.6 through 3.2.31.

## 3.2.5

- Restored Bestiary Save Encounter/Add to Initiative for individual monsters, generated encounters and saved encounters; transferred monsters are appended to the active character roster before Initiative opens.
- Protected existing characters from accidental blank initialization, removed checksum-based rejection and expanded the saved Party Roster to 23 members.
- Added the low-memory Initiative launch path and current Roll for All/manual-roll/Back navigation behavior, plus wider scrolling labels.

## 3.2.4

- Added named saved encounters with resume, rename, duplicate, archive, delete and Add to Initiative actions.
- Added the encounter Difficulty simulator/composition warnings and tightened streaming/allocation release around large Bestiary encounter workflows.
- Added the transactional Bestiary-to-Initiative encounter handoff foundation later refined by the following releases.

## 3.2.3

- Changed Bestiary transfer into a one-button automatic launch after validation and release of large Bestiary buffers.
- Full encounters opened Initiative setup; single-creature transfers opened setup or active combat as appropriate while preserving name, quantity, initiative modifier, AC and HP data.
- Removed the extra prompt/Home step and retained transfer state safely when launch failed.

## 3.2.2

- Ensured required parent directories exist before writes and isolated/hardened the RogueMaster deferred-loader launch path.
- Added single-monster Add to Initiative and transferred AC/current/max HP; active combats could receive appended creatures without resetting round/turn state.
- Added stricter persistence, handoff and allocation-failure coverage.

## 3.2.1

- Persisted Bestiary party level/size immediately and hardened saved-encounter/Send-to-Initiative paths against allocation failures and NULL dereferences.
- Streamed initiative-modifier lookup and made unresolved saved-encounter transfers fail atomically instead of partially applying.
- Added sanitizer/fault-injection coverage for the hardened paths.

## 3.2

- Fixed the prior startup MPU/stack-overflow path by moving large save/migration temporaries off stack and restored larger Bestiary result pages.
- Added the initial saved-encounter management/encounter-to-Initiative workflow with composition warnings and validation.
- Added persistence/handoff validation and memory-release work that subsequent 3.2.x revisions refined.

## 3.1

- Advanced character saves to schema 3 with verified schema-2 rollback snapshots and an on-device rollback action.
- Added transactional campaign/monster pack installation with enable/disable controls and stable-ID conflict validation.
- Added Bestiary favorites, recents, named filter presets, saved encounters and faster stable-ID lookup with host regression coverage.

## 3.0.3

- Replaced byte-at-a-time profile/campaign/Bestiary reads with buffered readers and reusable record-offset caches.
- Added faster campaign/monster lookup while keeping packaged/custom data streamed from disk.
- Coalesced rapid edits into delayed autosaves and added heap-fragmentation/repeated-app-switch stress coverage.

## 3.0.2

- Restricted asset-to-data migration to mutable profiles, custom campaigns/progress and custom monsters while keeping packaged catalogs/campaigns/monster tables in app assets.
- Preserved the combined packaged-plus-custom Bestiary view and migrated legacy mutable files only after their app-data destinations were safely present.
- Kept existing app-data records authoritative and never overwrote matching destinations/profile IDs during migration.

## 3.0.1

- Moved mutable profiles/exports/archives/campaign progress/custom monster data into persistent app data with first-launch migration that never overwrote existing destinations.
- Prevented failed/corrupt/unsupported profile loads from autosaving a blank replacement and established schema 2 as the compatibility baseline.
- Reduced Bestiary result pages and hardened custom-monster legacy/open/write/recovery behavior while keeping packaged tables read-only.

## 3.0

- Added generated-encounter drill-down and full-screen stat-row reading while preserving browser/encounter selection state.
- Isolated custom monsters in atomic custom index/statblock files and streamed packaged/custom monster layers together without loading whole tables.
- Added confirmed custom-monster deletion, kept custom character text profile-local, reduced spell-picker memory and sorted packaged spells by level then name.
