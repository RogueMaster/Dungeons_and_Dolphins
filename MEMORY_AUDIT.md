# Memory audit

This audit is source-derived. It separates **stack reservation**, **project-owned heap**, and **loaded FAP image pressure** because they fail differently on Flipper Zero. Firmware/framework objects (`View`, `ViewDispatcher`, `File`, Loader bookkeeping, RTOS state, interrupts and allocator metadata) are additional and are not assigned invented sizes here. Device high-water and loader testing remain authoritative.

## Stack reservations and estimated project-visible use

| FAP | Reserved stack | Estimated project-visible peak | Approx. reservation used | Approx. headroom |
|---|---:|---:|---:|---:|
| DNDolphins | 6,144 B | ~2,900 B | ~47% | ~3,240 B |
| DNDInventory | 4,096 B | ~1,800 B | ~44% | ~2,290 B |
| DNDSpellbook | 4,096 B | ~1,800 B | ~44% | ~2,290 B |
| DNDAdventure | 4,096 B | ~2,770 B | ~68% | ~1,330 B |
| DNDJournal | 4,096 B | ~2,660 B | ~65% | ~1,440 B |
| DNDInitiative | 3,072 B | ~2,000 B | ~65% | ~1,070 B |
| DNDBestiary | 6,144 B | ~4,370 B | ~71% | ~1,770 B |

The Inventory/Spellbook split does **not** justify shrinking DNDolphins below 6 KB merely to make Loader accept it. Increasing the stack would increase loader RAM demand; decreasing it would trade loader headroom for runtime stack risk. The structural split is therefore the preferred fix.

The source-ownership audit still supports DNDInitiative at 3 KB. DNDInventory and DNDSpellbook are restored to 4 KB because each standalone app must parse the full canonical character profile before its own sidecar/catalog UI can apply class/species/background and spellcasting rules; 3 KB did not leave enough margin for that nested parser path. DNDAdventure and DNDJournal remain at 4 KB, DNDBestiary at 6 KB, and DNDolphins at 6 KB. Device stack-high-water testing remains authoritative.


DNDInitiative no longer links the general progression store merely for Turn/Encounter recharge. Its Initiative-only Feature rewrite preserves the existing Feature sidecar format and resets only persisted `uses_current` values that match the Turn/Encounter cadence. A same-flags local stack-usage comparison reduced the recharge helper from a ~1,712-byte frame plus the nested ~752-byte general Feature writer to a single ~1,536-byte compiled recharge path. The table keeps a conservative ~2.0 KB project-visible estimate for caller overhead; hardware stack-high-water testing remains authoritative.

## Project-owned heap

### DNDolphins

- Fixed heap-owned app state: approximately **5.1 KB** after removing obsolete collection-only fields and converting the 24-byte Spell/24-byte weapon combat index arrays to lazy pointers. The exact firmware allocator footprint is device-dependent.
- Inventory and Spellbook list/editor/catalog state is no longer part of this FAP.
- DNDolphins startup/ordinary navigation hydrates **no Inventory or Spellbook page**. Weapon/Spell Combat lazily allocates a 24-byte logical-index buffer for the active combat type, then hydrates at most one bounded collection page when a concrete record is first requested:
  - item page: up to **2,384 B** (8 × 298-byte Item records),
  - spell page: up to **2,624 B** (8 × 324-byte Spell records plus four 8-byte state arrays),
  - Feature page: up to **1,872 B** (8 × 234-byte Feature records).
- Screen transitions release both the collection page and its combat-index buffer when that combat family is no longer required, so weapon and spell combat collection state does not intentionally remain resident together.
- A collection rewrite can temporarily add the bounded **1,280-byte** collection line buffer.
- Pending deterministic grants remain transient. At the 24-grant bound, the old structured Grant batch is about **3,552 B**; a 24-entry character/feat catalog page is about **1,344 B**. A conservative progression-review overlap is therefore around **10 KB project-owned heap** including the fixed app state, before framework allocations.
- Applied-grant history itself is never retained as an array; `appliedgrants_{id}.txt` is streamed only when progression checks need it.

### DNDInventory

- Fixed heap-owned `CollectionApp`: approximately **4,948 B**. This already includes the ten-record catalog result page, edit buffers and the small Inventory-resource aggregate.
- The restored transient-status/action-ack flags add only a few bytes of fixed control state; the rounded fixed/working-set estimates above are unchanged. No timer, queue or background worker was introduced.
- Full resident Item page: up to **2,384 B**.
- Normal fixed + full-page working set: approximately **7,332 B** project-owned heap.
- A sidecar rewrite can temporarily add one **1,280-byte** collection line buffer, giving a conservative project-owned save peak around **8,612 B** before firmware `File`/GUI allocations.
- Starting-equipment asset composition uses a single **256-byte heap line buffer** and streams assets; it does not retain the equipment catalogs. The explicit one-time regrant performs one normal **1,280-byte** streamed sidecar-copy buffer, frees it before asset composition, then reuses the 256-byte asset buffer. Its stack locals are bounded path/currency/seed descriptors and do not add a resident/background allocation; the existing conservative Inventory stack estimate remains higher than this explicit tool path, pending device high-water measurement.

### DNDSpellbook

- Fixed heap-owned `CollectionApp`: approximately **4,936 B**.
- Restored All-Classes/All-Spells filter state plus transient-status/action-ack control adds only a few bytes of fixed state; the rounded estimate is unchanged. No filter catalog or status timer is retained in the background.
- Full resident Spell page: up to **2,624 B**.
- Normal fixed + full-page working set: approximately **7,560 B** project-owned heap.
- A sidecar rewrite can temporarily add one **1,280-byte** collection line buffer, giving a conservative project-owned save peak around **8,840 B** before firmware `File`/GUI allocations.
- The full bundled Spell Catalog remains on storage and is scanned into the fixed ten-result page; it is never materialized as a whole in RAM.

### Other FAPs

The split does not materially change the project-owned heap models for DNDJournal or DNDBestiary. The shared companion profile-reference helper now parses `/ext/apps_data/dndolphins/custom_active_profile.txt` itself with a fixed 96-byte line buffer and one temporary `File` handle; it performs no character-directory discovery for active selection and does not call or require `dnd_storage.c`. Journal/Adventure may then validate the exact ID through the bounded primary-profile lookup; Initiative validates its selected ID separately; Bestiary keeps only the selected numeric ID. This preserves the lightweight Journal/Initiative/Bestiary link sets and avoids the unresolved-symbol failure caused by making their shared helper call a storage implementation those FAPs do not link. DNDInitiative owns a narrow Turn/Encounter Feature-recharge streamer instead of linking the full progression store. DNDAdventure no longer links that progression store at all because Adventure never called it. Campaign discovery continues to avoid campaign-sized heap offset arrays: bundled/custom/enabled indexes each keep at most eight 32-bit sparse offsets (96 bytes of offsets total), one hint per eight valid rows, and stream forward from the nearest hint. Active-campaign ID lookup streams each index once. Journal, Initiative and Bestiary caches remain storage-backed and independently loaded because only one FAP runs at a time.

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

The largest remaining companion-FAP memory target is now **profile projection**, not another filename split. On a 32-bit layout, the current `PocketCharacter` is about **3,976 B** before collection pages. DNDInventory and DNDSpellbook each embed that full character core in their roughly 4.9 KB fixed `CollectionApp` even though Inventory mainly needs identity/equipment/resource fields and Spellbook mainly needs identity/class casting metadata. DNDAdventure also holds a full character core while primarily consuming skill/check fields. Replacing those full objects with bounded read/write projections could reclaim several kilobytes of project-owned heap per companion FAP, but it requires narrow storage APIs and therefore belongs in a separately hardware-tested refactor rather than this low-risk linkage cleanup.


## Collection ownership and paging

- `inventory_{id}.txt`, `spellbook_{id}.txt`, `feats_{id}.txt` and `appliedgrants_{id}.txt` all remain under `/ext/apps_data/dndolphins/` regardless of which FAP edits them.
- Currency is exclusively Inventory-owned. The sidecar stores `Currency=cp,sp,ep,gp,pp`; Item rewrites and appenders preserve an existing record but do not synthesize one. Adventure/Journal Item creation may therefore create an item-only sidecar. When DNDInventory opens an existing item-only sidecar, it writes a zero Currency record. Character-profile `Currency=` lines are ignored and never adopted into Inventory.
- DNDInventory and DNDSpellbook each keep at most one aligned eight-record live page.
- Add New prepares the actual tail page, grows the resident page in RAM, increments the logical total and immediately rewrites the authoritative sidecar.
- List drawing is cache-only. SD reads/writes, page changes and allocations occur from input/screen-transition code rather than canvas rendering. Up/Down reuses the resident page until an eight-record boundary; short Left/Right performs one explicit aligned page read. No-op Press/Release events are ignored without committing a redraw.
- Catalog selection, text edits, numeric edits, left/right changes, Hold-OK quick actions and Delete commit to the canonical sidecar immediately.
- DNDolphins uses streamed whole-collection visitors for small counts/combat indexes and hydrates Item/Spell pages only for concrete Weapon/Spell Combat records. Level-up spell grants, class deletion remaps and Long-Rest free-cast resets use streaming sidecar writes and do not hydrate the resident Spellbook page.

## Starting inventory path

DNDInventory is the only normal starting-inventory initializer. Opening Inventory alone does not initialize it. Short OK on **Hold Up → Inventory Tools → Grant Initial Inventory** performs the normal grant when eligible:

1. It attempts class/species/background starting-equipment composition first.
2. Successful normal composition writes those records/currency into `inventory_{id}.txt` **without adding a trinket**, then records `InitialInventory=1`.
3. If normal composition yields neither items nor currency, it rolls d100 and attempts to write one random trinket as fallback equipment.
4. A manual Inventory containing Item records but no grant marker remains protected from the normal grant; a currency-only sidecar may receive it.

After state `1`, Hold OK on the grant row is an explicit one-time override. It performs one streamed sidecar rewrite: existing non-grant Item records are copied forward, matching class/species/background starting records are appended (with the same d100 fallback if the selected assets yield nothing), Inventory currency is increased by the starting amount, and a successful publish writes `InitialInventory=2`. State `2` means the override is consumed. A failed publish leaves the prior live sidecar/marker authoritative so the override is not consumed. No background task or resident seed catalog is introduced; asset reads occur only during the explicit grant/regrant action.

Starting equipment is never seeded merely by character creation, opening Inventory, Combat, Adventure or resource-summary screens.

## Progression metadata

Level progression does not keep its metadata catalog resident. Grant Initial Traits, class progression and supported species progression open metadata only for the active progression operation, stream rows sequentially and close the file afterward. Species gates use total character level.

Persistent Features live in `feats_{id}.txt`, paged at eight records when a Feature screen needs them. Applied deterministic grant IDs are streamed from `appliedgrants_{id}.txt`. Legacy embedded Feature/Grant fields are ignored during character parsing without allocation or migration. Missing progression sidecars are treated as empty and are created only by the first real write.

## Ownership/leak audit

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
3. Exercise companion active-profile resolution with `Active=0`, a nonzero valid ID, missing/unreadable metadata, and a present-but-missing character ID; confirm no cross-character discovery and verify `[id]` appears only on each companion's main screen.
4. In DNDInventory, confirm opening alone creates no sidecar; test normal Grant Initial Inventory, the one-time Hold-OK regrant (`InitialInventory=1` → `2`), second-override rejection, missing-match fallback trinket, failed-write preservation, and repeated Add/Edit/Delete across 7→8→9 and 15→16→17.
5. In DNDSpellbook, repeat Add/Edit/Delete, all filters, Prepare toggles and page-boundary transitions.
6. Enter/exit weapon and spell Combat repeatedly to confirm the logical index is allocated only inside that combat flow, the first Item/Spell page is loaded on demand, and both index/page are released on exit.
7. Exercise Initiative Turn/Encounter recharge with a multi-page Feature sidecar.
8. Stress DNDBestiary maximum-size encounter writes and DNDAdventure large pack indexes, including rows beyond the sparse-hint window and validated campaign installs.

A real RogueMaster build plus device Loader/stack-high-water/fragmentation testing remains the final authority.
