# Memory audit — 3.6 post-namespace migration

This is the current memory review for Dungeons & Dolphins 3.6 after the active `Pocket*` / `POCKET_*` type-and-symbol migration. It distinguishes exact source/manifest facts, current 32-bit host-layout regression measurements, historical ARM SDK measurements, and values that still require an actual Flipper/RogueMaster run.

## Executive finding

The strongest explanation for an **Out of Memory while launching DNDolphins** is the size of the external FAP itself, not catalog loading. Flipper external-app code and data are copied into RAM by the App Loader before the app starts, reducing the heap left for the app. The last available official-SDK ARM build in this workspace measured DNDolphins at **117,646 B of linked text/read-only data**, far larger than the companion FAPs. That measurement is historical 4.19.0c evidence rather than a fresh 3.6 target build, but the architecture has not become smaller enough to dismiss loader pressure.

Normal 3.6 startup does **not** read the Item/Spell/Feature/language/proficiency catalogs and no longer probes the complete `_All` catalog set. The core character parser also deliberately leaves Spells, Items, Features, and Grants as lazy sidecars, so those collections are not hydrated on cold launch. A delay such as 50 ms between catalog reads therefore cannot fix a cold-launch OOM.

A standalone **DNDCombat FAP is now justified as a memory-reduction candidate if the device error occurs at launch or before the Home screen.** Its main benefit would be removing Combat executable code/read-only data from DNDolphins' always-resident FAP image. It would save relatively little fixed app-state RAM by itself because Combat's runtime record pages are already lazy and bounded.

## Current exact manifest reservations and host stack regression

| FAP | Current stack | Largest strict-host individual frame |
|---|---:|---:|
| DNDolphins | **6,144 B** | **1,776 B** |
| DNDInventory | **4,096 B** | **1,728 B** |
| DNDSpellbook | **4,096 B** | **1,728 B** |
| DNDAdventure | **4,096 B** | **2,256 B** |
| DNDJournal | **4,096 B** | **1,312 B** |
| DNDInitiative | **4,096 B** | **1,536 B** |
| DNDBestiary | **6,144 B** | **2,288 B** |

These host frames come from the strict non-sanitized `-fstack-usage` regression pass. They are individual x86_64 frames, not cumulative ARM stack high-water measurements. The current values do **not** justify increasing DNDolphins' stack. Increasing the stack to address an OOM would reserve still more RAM and reduce available heap.

## Current 32-bit layout regression

`tests/host/layout32.py` currently reports:

| Record/state | Bytes |
|---|---:|
| `DndDolphinsApp` | **4,888** |
| `DndCharacter` / `DndSaveData` | **2,920** |
| `DndGrant` | **180** |
| `DndSpell` | **324** |
| `DndItem` | **300** |
| `DndFeature` | **234** |
| `DndProfileState` | **308** |
| `DndCharacterProficiency` | **63** |
| `DndDolphinsSpellClassCounts` | **24** |
| DNDInventory app state | **1,632** |
| DNDSpellbook app state | **1,720** |
| DNDAdventure app state | **520** |
| DNDJournal app state | **1,352** |
| DNDInitiative app state | **5,284** |
| DNDBestiary app state | **1,528** |
| SHD restore context | **2,029** |

This is a pointer-width/layout proxy, not an official ARM SDK `sizeof` result. It is nevertheless useful for detecting regressions. During this audit DNDolphins was reduced from **5,264 B to 4,888 B** by overlaying its mutually exclusive Language and Proficiency eight-row caches in one union, saving **376 B** of fixed resident project state.

## Cold-launch allocation path

DNDolphins currently does the following before Home is shown:

1. The firmware App Loader has already placed the FAP's executable/data sections in RAM.
2. `malloc(sizeof(DndDolphinsApp))` requests one contiguous block of roughly **4.9 KB** in the current 32-bit proxy.
3. GUI and Storage records are opened.
4. Settings are read with a bounded reader. `_All` availability is **not** probed here.
5. DNDolphins deliberately reserves its core GUI objects while the heap is still relatively clean: ViewDispatcher, autosave timer, main View, and the tiny pointer model.
6. The profile directory is streamed through a fixed eight-entry `DndProfileState` cache.
7. The active core character is read into the app's embedded `DndCharacter`. The current parser explicitly clears/keeps lazy the Spell, Item, Feature, and Grant collections.
8. Input-event subscription is attached and the Home view is entered.

Consequently, an OOM **before Home** points primarily to FAP loader residency, inability to obtain the contiguous app/UI blocks, or low/fragmented firmware heap. Catalogs, spell pages, item pages, feature pages, and grants are not the launch culprit.

The Debug setting now emits free-heap checkpoints after the app-state allocation, after core UI reservation, when the app becomes ready, and on entry to Combat-backed screens. Allocation failure logs also include total free heap. If the Loader fails before DNDolphins executes, the firmware's own Loader/Elf logs are required instead.

## DNDolphins named project heap by operation

These figures include only allocations owned by this project. Firmware GUI objects, File objects/stream internals, allocator bookkeeping, the thread stack, Loader allocations, and fragmentation are additional.

| Path | Calculation | Named project bytes |
|---|---|---:|
| Cold app state | app only | **4,888** |
| Character/feat catalog page | app + 24 × (47-byte name + 1 + 2 + 1 metadata) | **6,112** |
| Feature page | app + 8 × 234 | **6,760** |
| Weapon Combat | app + 336-byte index/row block + 8 × 300 Item page | **7,624** |
| Spell/Ritual Combat | app + 336-byte index/row block + 8 × 324 Spells + four 8-byte flag arrays | **7,848** |
| SHD restore | app + 2,029-byte rollback context | **6,917** |
| 24-grant resident batch | app + 24 × 180 | **9,208** |
| Grant review + choice catalog | app + 4,320-byte grant batch + 1,224-byte catalog | **10,432** |
| Possible grant `realloc` move, 16 → 24 | app + old 2,880-byte block + new 4,320-byte block | **12,088** transient |

The **grant `realloc` move** is the largest clear DNDolphins project-owned transient found in this audit. `realloc()` is allowed to allocate a new block before freeing the old one when it cannot grow in place, so a fragmented/low heap can fail here even if the eventual 4.32 KB grant block would fit by itself. This is a credible explanation for an OOM that occurs specifically while applying/reviewing grants, not for a cold-launch error.

Combat itself is bounded: the active weapon or spell path uses one eight-record page plus one eight-index/five-row display block, and the opposite collection is released when it is no longer needed. This means a Combat-only OOM is more likely to expose **low base heap caused by the large DNDolphins FAP** or framework/storage allocations than an unbounded Combat collection.

## Historical ARM loader evidence

The last official-SDK ARM linked-section evidence retained in `tests/sdk/linked_sizes.json` is from the earlier 4.19.0c baseline:

| FAP | Linked text/read-only data | Data + BSS |
|---|---:|---:|
| DNDolphins | **117,646 B** | 1 B |
| DNDInventory | 44,696 B | 0 B |
| DNDSpellbook | 33,154 B | 0 B |
| DNDAdventure | 34,703 B | 112 B |
| DNDJournal | 12,852 B | 0 B |
| DNDInitiative | 17,481 B | 0 B |
| DNDBestiary | 42,343 B | 617 B |

Do not interpret this as a fresh 3.6 binary-size claim. It is retained because it demonstrates the existing architecture: DNDolphins was already roughly 2.6–9× the linked code/read-only footprint of its companion FAPs. The current source still contains a very large monolithic `dndolphins.c` plus dedicated spell/weapon Combat modules, so target rebuilding is the next authoritative measurement.

## Other credible OOM mechanisms

1. **FAP loader/code residency — highest priority for launch OOM.** External FAP executable/data sections consume RAM before `dndolphins_app()` begins.
2. **Contiguous allocation failure.** The app state requires one ~4.9 KB block, and framework objects require their own blocks. Total free heap can be larger than the failed request while fragmentation prevents a suitably large contiguous allocation.
3. **Framework allocations excluded from project arithmetic.** ViewDispatcher, View, timer, TextInput/NumberInput, pubsub subscription, Storage `File` objects, and allocator metadata all consume additional RAM.
4. **Grant growth/reallocation.** A 16→24 grant capacity move can transiently make both old and new blocks live, producing the largest identified project heap transient.
5. **Text/Number input lifetime.** These UI modules are lazy and are reclaimed after their callback returns to the main view or before another list/catalog opens. They are bounded transient pressure, not a confirmed leak.
6. **Storage activity under low heap.** Catalog and sidecar readers are streamed and bounded, but File objects and firmware storage buffers still have to allocate successfully. A low baseline can make a harmless file operation be the first visible failure.
7. **Stack reservation.** The 6 KB DNDolphins thread stack is real reserved RAM. It is not currently proven excessive enough to reduce safely; lowering it should wait for target stack high-water data. Raising it would make OOM pressure worse.
8. **No confirmed project leak in tested paths.** Host sanitizer/explicit allocation-accounting tests currently finish with zero project-owned outstanding allocations for exercised Storage, Character, Inventory, and Spellbook paths. This does not prove every firmware/framework path is leak-free.

## Should Combat become `DNDCombat`?

**Recommendation: yes, if the device error is at launch/before Home or the Loader reports an unusually large DNDolphins loaded-section total.** This is now an architecture optimization with a clear memory purpose, not merely source organization.

A proper split should:

- create a standalone `dndcombat` FAP launched through the same active-profile handoff used by the existing companions;
- move Combat menu, attack templates, weapon attacks, spell attacks, rituals, cast resolution, resource consumption and related drawing/input code out of DNDolphins;
- use a narrow Combat profile projection instead of embedding the full `DndDolphinsApp`;
- lazily page the same eight Item/Spell records from their authoritative sidecars;
- transactionally write only mutable combat-owned character fields/resources that actually change;
- return to DNDolphins through the established short-Back handoff behavior;
- retain DNDInitiative as a separate encounter/turn-order owner rather than merging it into Combat.

The principal saving would be **loaded executable/read-only sections**. The fixed DNDolphins app-state saving would be modest because Combat-specific scalar fields are small and the large Item/Spell working pages are already lazy allocations.

I would not split Combat blindly before measuring the current target FAP. If current RogueMaster logs confirm that DNDolphins' loaded-section size is close to the available app RAM envelope, the split becomes the highest-value next change. If DNDolphins launches reliably and OOM occurs only during Grant Review, optimizing grant batching/reallocation is higher priority.

## Device diagnosis by failure point

| Where the user sees OOM | First suspect | What to capture |
|---|---|---|
| Selecting DNDolphins, before Home appears | Loader/code/data RAM | Loader/Elf `Total size of loaded sections`; whether `dndolphins_app` logs appear at all |
| Immediately after launch while Home is appearing | app/core GUI contiguous allocations | Debug heap checkpoints and allocation-failure log |
| Opening Combat/Spell Attacks/Rituals/Weapons | low base heap + bounded Combat page/framework I/O | Debug heap before/after Combat entry |
| Apply Level Grants / Grant Review / grant choice | grant batch + `realloc`/catalog overlap | Debug heap and exact screen/action |
| Opening editor after repeated navigation | transient TextInput/NumberInput/framework pressure or leak outside exercised host paths | repeat count + debug heap trend |
| Only after many cross-FAP launches | firmware/framework leak or incomplete teardown | Loader free-heap trend after each app exits |

On-device debug logging should be enabled before reproducing the failure. The firmware Loader logs are especially important because an OOM that occurs while copying/relocating the FAP happens **before application code can diagnose itself**.

## Validation status

- Active C/H project symbols are DND-only. The only permitted `Pocket...` source tokens are three read-only legacy-format aliases: `PocketD20Character` in the character parser and `PocketPack` in Bestiary/Adventure pack parsers.
- New character saves write `DNDolphinsCharacter`; legacy `PocketD20Character` saves remain readable.
- `tests/host/run_tests.py` rejects reintroduction of active Pocket namespaces.
- Strict host links and ASan/UBSan regression tests pass for all seven current FAP source sets and the exercised Storage/Character/Inventory/Spellbook paths.
- `ufbt`, `clang-format`, an ARM compiler, and a physical Flipper are not present in this environment, so there is no fresh 3.6 RogueMaster/ARM loaded-section measurement or target stack high-water measurement in this audit.
