# Memory audit

This audit is source-derived. It separates **stack reservation**, **project-owned heap**, and **loaded FAP image pressure** because they fail differently on Flipper Zero. Firmware/framework objects (`View`, `ViewDispatcher`, `File`, Loader bookkeeping, RTOS state, interrupts and allocator metadata) are additional and are not assigned invented sizes here. Device high-water and loader testing remain authoritative.

## Stack reservations and estimated project-visible use

| FAP | Reserved stack | Estimated project-visible peak | Approx. reservation used | Approx. headroom |
|---|---:|---:|---:|---:|
| DNDolphins | 6,144 B | ~2,900 B | ~47% | ~3,240 B |
| DNDInventory | 4,096 B | ~2,000 B | ~49% | ~2,090 B |
| DNDSpellbook | 4,096 B | ~2,000 B | ~49% | ~2,090 B |
| DNDAdventure | 4,096 B | ~2,770 B | ~68% | ~1,330 B |
| DNDJournal | 4,096 B | ~2,660 B | ~65% | ~1,440 B |
| DNDInitiative | 3,072 B | ~2,000 B | ~65% | ~1,070 B |
| DNDBestiary | 6,144 B | ~4,370 B | ~71% | ~1,770 B |

The Inventory/Spellbook split does **not** justify shrinking DNDolphins below 6 KB merely to make Loader accept it. Increasing the stack would increase loader RAM demand; decreasing it would trade loader headroom for runtime stack risk. The structural split is therefore the preferred fix.

The source-ownership audit still supports DNDInitiative at 3 KB. DNDInventory and DNDSpellbook remain at 4 KB because each standalone app must parse the full canonical character profile before its own sidecar/catalog UI can apply class/species/background and spellcasting rules; 3 KB did not leave enough margin for that nested parser path. The current catalog optimization adds only a 128-byte stack reader to the catalog-load path, so their conservative project-visible estimate is now ~2.0 KB and still leaves about 2.1 KB of the 4 KB reservation. The Spellbook ordering key table is heap-owned, not stack-owned. DNDAdventure and DNDJournal remain at 4 KB, DNDBestiary at 6 KB, and DNDolphins at 6 KB. Device stack-high-water testing remains authoritative.


DNDInitiative no longer links the general progression store merely for Turn/Encounter recharge. Its Initiative-only Feature rewrite preserves the existing Feature sidecar format and resets only persisted `uses_current` values that match the Turn/Encounter cadence. A same-flags local stack-usage comparison reduced the recharge helper from a ~1,712-byte frame plus the nested ~752-byte general Feature writer to a single ~1,536-byte compiled recharge path. The table keeps a conservative ~2.0 KB project-visible estimate for caller overhead; hardware stack-high-water testing remains authoritative.

## Project-owned heap

The fixed app-state figures below are **source-derived ARM32 struct-layout sizes** for the current code, measured for Cortex-M4/32-bit alignment. They are the number of bytes requested for each app's heap-owned state block; firmware allocator metadata/alignment, `View`/`ViewDispatcher`, `File`, Loader, RTOS and other framework objects are additional. Collection/page figures are likewise record-layout bounds from the current structs.

| FAP | Fixed app-state allocation | Largest ordinary project-owned resident addition | Representative project-owned working set |
|---|---:|---:|---:|
| DNDolphins | **5,248 B** | Spell page 2,624 B + 24 B logical index + 320 B visible-row cache | **8,216 B** during Spell/Ritual Combat; Weapon Combat is 7,976 B |
| DNDInventory | **5,072 B** | Item page 2,384 B | **7,456 B** normal; **8,736 B** during a 1,280 B rewrite |
| DNDSpellbook | **5,108 B** | Spell page 2,624 B | **7,732 B** normal; **9,876 B** during sort-with-page resident; ordinary rewrite is 9,012 B |
| DNDAdventure | **4,760 B** | 1,536 B bounded campaign-diagnostics scene-ID table | **6,296 B** during explicit diagnostics; ordinary scene state is 5,625 B |
| DNDJournal | **1,352 B** | Two bounded 768 B scan buffers during index work | **2,888 B** during that transient scan |
| DNDInitiative | **5,276 B** | Two 768 B character-patch buffers on explicit main-character sync | **6,812 B** during that transactional patch |
| DNDBestiary | **1,528 B** | Encounter 2,088 B + 16-summary generation sample 2,752 B | **6,368 B** during encounter generation; main 15-row window is 4,108 B |

These numbers are **not total device heap consumption**. Framework objects and allocator overhead are deliberately not guessed. Hardware free-heap/fragmentation measurements remain the final authority.

The source-ownership/header pass moves code and type declarations only. The later DNDolphins home-return focus field is a `uint16_t` placed in padding that already existed before the following enum-aligned field on ARM32, so the verified fixed app-state and resident-page heap figures in this table remain unchanged.

### DNDolphins

- Fixed heap-owned `PocketD20App`: **5,248 B**. The five × 64-byte Combat row cache is **not** embedded in this fixed block; it is a 320-byte trailing region allocated together with the active 24-byte Weapon/Spell logical-index array so it exists only while that combat family is open.
- DNDolphins startup/ordinary navigation hydrates **no Inventory or Spellbook page**. Weapon/Spell/Ritual Combat lazily allocates one 24-byte logical-index buffer for the active combat family and at most one bounded collection page when rows are prepared from screen-entry/input code:
  - Item page: **2,384 B** (8 × 298-byte Item records).
  - Spell page: **2,624 B** (8 × 324-byte Spell records plus four 8-byte state arrays).
  - Feature page: **1,872 B** (8 × 234-byte Feature records).
- Spell/Ritual Combat representative project-owned resident state is **8,216 B** (5,248 + 24 logical index + 320 visible-row cache + 2,624 Spell page). Weapon Combat is **7,976 B** (5,248 + 24 + 320 + 2,384 Item page). Screen transitions release the collection page/index/row-cache allocation when leaving that family, so Weapon and Spell pages are not intentionally resident together.
- A collection rewrite can temporarily add the bounded **1,280-byte** line buffer.
- Pending deterministic grants remain transient. At the 24-grant bound the structured Grant batch is about **3,552 B**; the current 24-entry character/feat catalog block is **1,224 B**. A conservative progression-review overlap is therefore **10,024 B** (5,248 + 3,552 + 1,224) project-owned heap before firmware/framework allocations. Applied-grant history itself is streamed and never retained as an array.
- Profile-list windows, class spell counts, Journal-independent character data and Combat display rows are prepared outside draw callbacks. No DNDolphins canvas path reaches project storage or project heap allocation/free in the current call-graph audit.

### DNDInventory

- Fixed heap-owned `CollectionApp`: **5,072 B**. This includes the ten-record catalog result page, edit buffers, Inventory-resource aggregate, transient-status/action-ack state, a bounded rolling 64-entry catalog page-offset map (256 B), and three 32-bit owned-sidecar page offsets.
- Full resident Item page: **2,384 B**.
- Normal fixed + full-page project-owned working set: **7,456 B**.
- A sidecar rewrite can add one **1,280-byte** bounded line buffer, giving a representative project-owned save peak of **8,736 B** before firmware `File`/GUI allocations.
- Catalog parsing uses a **128-byte stack read buffer** and the bounded rolling offset map. Once a filtered catalog page offset has been learned, later nearby/sequential page navigation seeks near that page instead of rescanning from byte zero; Inventory rolls the same 64 offsets forward for catalogs exceeding 64 filtered pages. The character-owned sidecar similarly caches only the three possible aligned 8-record page offsets at the 24-item cap.
- Starting-equipment asset composition uses one **256-byte** heap line buffer and streams assets. The one-time regrant uses the 1,280-byte streamed sidecar-copy buffer, releases it before asset composition, then uses the 256-byte asset buffer; those two temporaries are not intentionally overlapped.
- List/detail drawing is RAM-only. No timer, queue, pub-sub subscription or background worker runs for collection scrolling.

### DNDSpellbook

- Fixed heap-owned `CollectionApp`: **5,108 B**. This includes the ten-record catalog result page, editor/filter buffers, transient-status/action-ack state, a bounded 64-entry catalog page-offset map (256 B), and three 32-bit owned-sidecar page offsets.
- Full resident Spell page: **2,624 B**.
- Normal fixed + full-page project-owned working set: **7,732 B**.
- An ordinary sidecar rewrite can add one **1,280-byte** bounded line buffer, giving **9,012 B** project-owned before firmware `File`/GUI allocations.
- Deterministic level/name ordering allocates at most **864 B** for 24 compact sort keys plus the same **1,280-byte** line buffer. When triggered while the resident 8-spell page is still present, the conservative project-owned sort peak is therefore **9,876 B** (5,108 + 2,624 + 864 + 1,280). Startup sorting happens before the first page is hydrated, so its project-owned peak is lower. The keys contain only file offset, level and name; a second full Spell collection is never retained. The sorter now lives entirely in `dndspellbook_collection.c`, so DNDolphins, Inventory and Adventure do not carry Spellbook-only sorting code through their shared `dnd_storage.c` source.
- The full bundled Spell Catalog remains on storage and is streamed into the fixed ten-result page; it is never materialized as a whole in RAM. Catalog parsing uses a **128-byte stack read buffer** plus the bounded offset map, and the character-owned sidecar caches only its three possible aligned 8-record page offsets.
- List/detail drawing is RAM-only. Filter state introduces no timer, resident catalog copy or background worker.

### Other FAPs

- **DNDAdventure:** fixed app state is **4,760 B**; one active `PocketAdventureScene` is **865 B** (5,625 B total). Explicit Campaign Diagnostics can instead allocate one 64 × 24-byte scene-ID table (**1,536 B**), for **6,296 B** with fixed app state. Duplicate-campaign diagnostics no longer keep a dynamically growing ID array: they rescan the three indexes and trade explicit diagnostic I/O for bounded heap. Campaign indexes remain bounded sparse hints rather than campaign-sized offset arrays.
- **DNDJournal:** fixed app state is **1,352 B**. Journal list drawing uses resident entry metadata only. Bounded scan/index work may allocate two 768-byte temporary buffers (**2,888 B** including fixed app state) outside draw, then frees them on both normal and failure exits.
- **DNDInitiative:** fixed app state is **5,276 B** and already embeds the bounded roster/combat participant arrays. A transactional main-character patch allocates two 768-byte line buffers temporarily (**6,812 B** including fixed app state), then frees both on success/failure. Its narrow Turn/Encounter recharge streamer does not link/retain the full progression store.
- **DNDBestiary:** fixed app state is **1,528 B**. The main 15-summary window is **2,580 B** (4,108 B together); monster detail is **1,544 B** and encounter state **2,088 B**. Encounter generation temporarily adds a 16-summary candidate sample (**2,752 B**) while the encounter exists, producing a bounded project-block peak of **6,368 B**. State readers use a bounded **1,288 B** workspace, pack duplicate validation uses at most **3,072 B** for 96 IDs, and launch arguments are capped at **1,536 B** after large browser caches are released. Handoff and normal teardown release window/detail/encounter/pending-argument allocations before Loader handoff. Startup failure cleanup also frees any partially constructed dynamic blocks.
- Journal/Initiative/Bestiary keep the lightweight active-profile reader with a fixed 96-byte line buffer; Inventory/Spellbook use the storage reader they already link. No companion discovers another character solely to satisfy active selection.

### Draw-time allocation / loop audit

A same-file call-graph audit was run from each of the seven primary canvas callbacks after the stability changes. **No project-owned draw path reaches `malloc/calloc/realloc/free`, `dnd_storage_*`, `storage_file_*`, `storage_dir_*`, collection rewrite/open/close helpers, or storage-backed page advancement.** Profile windows, Journal windows, class spell counts and Combat Item/Spell rows are hydrated from screen-entry/input paths instead.

The only iterative `while` loops still reachable from draw are bounded text-formatting helpers: DNDolphins labeled-text formatting and Bestiary fixed-field line splitting/counting. `dndolphins_spells_build_cast_options()` also contains a `do { ... } while(false)` macro wrapper used only to emit one option; it is not an iterative runtime loop. They iterate over fixed-size in-memory strings and perform no allocation or storage access. There is no intentional polling/background loop tied to redraw.

## Loader/FAP image pressure

The reported **"Not enough RAM to run the app"** condition can occur before the app entry point runs, so runtime heap cleanup or progression migration changes cannot by themselves fix that specific loader failure.

A same-flags local source-object comparison is directionally useful even though it is **not** the final `.fap` Loader footprint (linker garbage collection, relocation tables and firmware loader overhead still matter):

- Current split DNDolphins source-object text+data aggregate: about **168.5 KB** after removing the now-duplicated Currency/Inventory Resources screens and handlers from the main FAP.
- Last known launching pre-split baseline under the same local comparison: about **177.0 KB**.
- Directional reduction in the main source-object aggregate: about **8.5 KB**.
- DNDInventory and DNDSpellbook each compile to roughly **79.7 KB** source-object text+data under the same local comparison, but they are separate FAP images and are never resident alongside DNDolphins.

The main asset pack is also smaller in responsibility: the Item Catalog/default-equipment assets belong to DNDInventory, the full Spell Catalog belongs to DNDSpellbook, and DNDolphins keeps only character/progression assets required by the hub.

A second, exact before/after comparison using one identical local compile command for this ownership refactor gives the most useful delta for this pass:

- DNDolphins: **143,478 B -> 141,494 B** (**-1,984 B**). The main FAP now links only `dnd_weapon_rules.c` + `dndolphins_weapon_combat.c` from the Item feature; starting-inventory, currency/resource, armor-management and reward code are absent.
- DNDInventory: **67,960 B -> 58,427 B** (**-9,533 B**). It compiles an Inventory-specialized collection implementation and no longer links spell-eligibility code.
- DNDSpellbook: **67,963 B -> 51,976 B** (**-15,987 B**). It compiles a Spellbook-specialized collection implementation and links no Item implementation at all.
- DNDAdventure: **78,733 B -> 76,226 B** (**-2,507 B**). It links only the item-reward helper instead of the complete Item implementation.

`dnd_spell_eligibility.c` remains intentionally shared by DNDolphins and DNDSpellbook because it is already the bounded class-eligibility slice (about **583 B** of local object text). `dndolphins_spells.c` is **not** linked into DNDSpellbook; it remains DNDolphins-only because the main FAP owns class spellcasting progression, slot/resource calculation and Spell Combat. Splitting that file by filename alone would not reduce DNDolphins because its exported runtime/progression functions are actively used there.


A later FAP-ownership audit removed additional broad dependencies without changing persisted data:

- The former general rules implementation is split into `dnd_rules_core.c` and `dndolphins_rules_character.c`. DNDolphins links both; DNDInventory and DNDAdventure link only the compact core; DNDSpellbook links neither.
- DNDAdventure no longer links `dndolphins_progression_store.c`; it had no progression-store call sites.
- DNDAdventure also drops its dead character-core save helper/dirty byte. Item rewards already persist directly through the Inventory sidecar, and no Adventure path marked the character core dirty.
- DNDInitiative links `dndinitiative_feature_recharge.c` instead of the complete progression/grant store. In a local garbage-collected comparison that exercises only Turn recharge, linked text fell from **4,524 B to 3,505 B** (about **1.0 KB**). This is directional host output, not a final Flipper `.fap` size.
- DNDSpellbook still shares `dnd_spell_eligibility.c` with DNDolphins because that helper is both small and genuinely required by Spellbook catalog eligibility.
- Under the same host `-Os` source-object comparison, splitting general rules changes the broad rules object from **1,793 B** to **1,120 B core + 691 B character/rest**. DNDInventory and DNDAdventure therefore omit about **673 B** of character/rest rule object content each, DNDSpellbook omits the former **1,793 B** rules object entirely, and DNDolphins' combined rule objects are effectively unchanged (+18 B in this host comparison).
- The complete progression-store object is about **8,973 B** in that host comparison versus about **1,920 B** for the Initiative-only recharge object. Because firmware link-time section garbage collection may already discard unreachable sections, these raw object deltas must not be treated as final Loader savings. The exercised Turn-recharge link comparison above is the better evidence of a real code-path reduction.
- A follow-up caller audit tightened the remaining shared headers without changing any app-state struct or heap allocation. `dnd_rules_core` no longer owns DNDolphins-only Initiative/effective-speed or advantage/disadvantage/value-recording dice helpers; `dnd_spell_eligibility` now contains only the maximum-spell-level rule genuinely shared by DNDolphins and Spellbook; shared storage no longer owns DNDolphins spell-count or Inventory Item-aggregate derived state; and sidecar-ensure helpers used only inside `dnd_storage.c` are private.
- Under the same host `-Os -ffunction-sections -fdata-sections` object check, `dnd_rules_core.o` changes from **1,601 B to 1,288 B** text+data and `dnd_spell_eligibility.o` from **583 B to 402 B**. DNDolphins receives the moved code in its own `dndolphins_rules_character.o` and **293 B** `dndolphins_dice.o`; the combined raw DNDolphins rule-object total rises only **115 B** while Inventory/Adventure no longer compile the removed core slices and Spellbook no longer compiles the removed Wizard-list alias slice. These are source-object measurements, not final `.fap` Loader sizes.
- The caller audit intentionally leaves transactional collection/profile parsing and rewrite primitives in `dnd_storage.c` even where only one current UI calls a specific operation. Moving those routines would require duplicating record parsing, path discovery or atomic snapshot/publish behavior; source aesthetics alone are not worth creating divergent persistence code.

The largest remaining companion-FAP memory target is now **profile projection**, not another filename split. The verified ARM32 `PocketCharacter` layout is **3,976 B** before collection pages. That character core is embedded in the **5,072 B** DNDInventory app state, **5,108 B** DNDSpellbook app state and **4,760 B** DNDAdventure app state even though each companion needs only a subset of profile fields. Bounded read/write projections could reclaim several kilobytes per companion, but that requires narrow storage APIs and hardware validation and is intentionally deferred rather than mixed into this stability release.


## Collection ownership and paging

- `inventory_{id}.txt`, `spellbook_{id}.txt`, `feats_{id}.txt` and `appliedgrants_{id}.txt` all remain under `/ext/apps_data/dndolphins/` regardless of which FAP edits them.
- Currency is exclusively Inventory-owned. The sidecar stores `Currency=cp,sp,ep,gp,pp`; Item rewrites and appenders preserve an existing record but do not synthesize one. Adventure/Journal Item creation may therefore create an item-only sidecar. When DNDInventory opens an existing item-only sidecar, it writes a zero Currency record. Character-profile `Currency=` lines are ignored and never adopted into Inventory.
- DNDInventory and DNDSpellbook each keep at most one aligned eight-record live page. Each standalone collection app retains only three 32-bit sidecar page offsets (records 0/8/16 at the 24-record cap). The initial scan learns the total and offsets; subsequent aligned page changes can seek directly rather than scanning from the beginning.
- Their Item/Spell Name catalogs use a bounded 64-entry (256-byte) filtered-page offset map and a 128-byte buffered reader. Inventory rolls that same 64-entry window forward when a filter exceeds 64 pages; Spellbook retains the fixed map because its bundled filtered catalog remains within that bound. This materially reduces repeated single-byte SD reads and avoids rescanning the complete filtered catalog on every next/previous page without loading the catalog into heap.
- Expanding the Item Catalog category choices does not enlarge the Inventory app state or catalog page cache: the selected filter remains one `uint8_t`, filter labels live in read-only program data, and the existing streamed 10-entry catalog page remains unchanged.
- Add New prepares the actual tail page, grows the resident page in RAM, increments the logical total and immediately rewrites the authoritative sidecar.
- List drawing is cache-only. SD reads/writes, page changes and allocations occur from input/screen-transition code rather than canvas rendering. Up/Down reuses the resident page until an eight-record boundary; short Left/Right performs one explicit aligned page read. No-op Press/Release events are ignored without committing a redraw.
- Catalog selection, text edits, numeric edits, left/right changes, Hold-OK quick actions and Delete commit to the canonical sidecar immediately.
- DNDolphins uses streamed whole-collection visitors for small counts/combat indexes and hydrates Item/Spell pages only for concrete Weapon/Spell Combat records. Level-up spell grants, class deletion remaps and Long-Rest free-cast resets use streaming sidecar writes and do not hydrate the resident Spellbook page.

## Starting inventory path

DNDInventory is the only normal starting-inventory initializer. Opening Inventory alone does not initialize it. Short OK on **Hold Up → Inventory Tools → Grant Initial Inventory** performs the normal grant when eligible:

1. It attempts class/species/background starting-equipment composition first.
2. Successful normal composition writes the existing balance plus granted currency, Item records and `InitialInventory=1` into `inventory_{id}.txt` in the same synced creation; there is no second currency-only rewrite and no extra trinket.
3. If normal composition yields neither items nor currency, it rolls d100 and attempts to write one random trinket as fallback equipment.
4. A manual Inventory containing Item records but no grant marker remains protected from the normal grant; a currency-only sidecar may receive it.

After state `1`, Hold OK on the grant row is an explicit one-time override. It performs one streamed sidecar rewrite: existing non-grant Item records are copied forward, matching class/species/background starting records are appended (with the same d100 fallback if the selected assets yield nothing), Inventory currency is increased by the starting amount, and a successful publish writes `InitialInventory=2`. The storage helper returns the exact combined balance that was published so Inventory mirrors persisted currency rather than recomputing it from potentially stale app state. State `2` means the override is consumed. A failed publish leaves the prior live sidecar/marker authoritative so the override is not consumed. No background task or resident seed catalog is introduced; asset reads occur only during the explicit grant/regrant action.

Starting equipment is never seeded merely by character creation, opening Inventory, Combat, Adventure or resource-summary screens.

## Progression metadata

Level progression does not keep its metadata catalog resident. Grant Initial Traits, class progression and supported species progression open metadata only for the active progression operation, stream rows sequentially and close the file afterward. Species gates use total character level.

Persistent Features live in `feats_{id}.txt`, paged at eight records when a Feature screen needs them. Applied deterministic grant IDs are streamed from `appliedgrants_{id}.txt`. Legacy embedded Feature/Grant fields are ignored during character parsing without allocation or migration. Missing progression sidecars are treated as empty and are created only by the first real write.

## Ownership/leak audit

- Static ownership review found no currently reachable project-owned allocation without a corresponding normal/failure release path. Dynamic collection pages/indexes, Adventure scenes, Journal scan buffers, Bestiary window/detail/encounter/pending-launch blocks, catalog storage and shared storage line buffers are all bounded and released by their owning screen/app path. This is a source audit, not a substitute for device allocator/fragmentation testing.
- Character-ID badge formatting is guarded against `UINT32_MAX` in all six companion main screens. The sentinel remains valid internally for lookup/not-found logic but cannot be formatted as `[4294967295]`; valid ID `0` is unaffected.
- The seven draw call graphs are project-heap/storage clean as described above; no canvas callback is used as a polling loop or hidden collection loader.
- Cross-FAP launches quiesce callbacks/timers and destroy outgoing views, collection pages, catalog state, services and heap-owned app state before Loader starts the next FAP. No 200 ms pre-launch delay is added: sleeping inside the outgoing app keeps that FAP alive longer and postpones, rather than accelerates, Loader memory reclamation.
- DNDInventory/DNDSpellbook do not keep a copy of DNDolphins resident; each FAP is independently loaded. They currently load a full active character core plus their current collection page; replacing that full core with per-FAP projections is the largest remaining companion-app heap opportunity.
- DNDInventory/DNDSpellbook now reserve their fixed dispatcher/main-view objects before loading the active character and first collection page. This prevents variable parser/page allocations from consuming heap needed for the main drawable UI and directly addresses intermittent blank-page startup behavior. Their collection UIs create no timer, pub-sub subscription or worker thread; scrolling is driven only by input events and cache-backed redraws.
- Their active-ID lookup uses `dnd_storage_load_active_profile_id()` before character loading. The helper allocates only the storage `File` object and uses a fixed stack reader buffer; it performs no profile scan and no collection hydration. Inventory/Spellbook no longer link the duplicate lightweight `dnd_profile_ref.c` reader.
- Storage rewrite helpers close/free their `File*` objects and temporary heap buffers on both success and failure paths.
- All seven FAP source sets have no unresolved project-internal DND symbols in the current source-list dependency audit; Journal/Initiative/Bestiary no longer depend on the storage-only active-profile symbol that caused the failed link.
- No firmware `qsort` dependency is used.

## Hardware stress priorities

1. Launch DNDolphins from a cold boot/profile repeatedly and confirm Loader no longer reports insufficient RAM.
2. Repeat DNDolphins ↔ DNDInventory and DNDolphins ↔ DNDSpellbook handoffs many times and watch for fragmentation or delayed Loader failures.
3. Exercise companion active-profile resolution with `Active=0`, a nonzero valid ID, missing/unreadable metadata, and a present-but-missing character ID; confirm no cross-character discovery, verify `[id]` appears only on each companion's main screen, and confirm `[4294967295]` is impossible even under repeated error/relaunch paths.
4. In DNDInventory, confirm opening alone creates no sidecar; test normal Grant Initial Inventory, the one-time Hold-OK regrant (`InitialInventory=1` → `2`), second-override rejection, missing-match fallback trinket, failed-write preservation, and repeated Add/Edit/Delete across 7→8→9 and 15→16→17.
5. In DNDSpellbook, repeat Add/Edit/Delete, all filters, Prepare toggles and page-boundary transitions; verify records remain ordered by level then case-insensitive name after add/rename/level edits and after restart.
6. Enter/exit Weapon Attacks, Spell Attacks and Rituals repeatedly to confirm the logical index is allocated only inside that combat family, the first Item/Spell page is loaded on demand outside draw, and both index/page are released on exit. Verify Wizard unprepared known Ritual spells appear in Rituals without consuming a normal spell resource.
7. Exercise Initiative Turn/Encounter recharge with a multi-page Feature sidecar.
8. Stress DNDBestiary maximum-size encounter writes and DNDAdventure large pack indexes, including rows beyond the sparse-hint window and validated campaign installs.

A real RogueMaster build plus device Loader/free-heap/stack-high-water/fragmentation testing remains the final authority. Source-derived heap sizes above are exact for the project structs/record pages under ARM32 layout, but they intentionally exclude firmware/framework allocator overhead.
