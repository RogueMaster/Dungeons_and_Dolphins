# Memory audit

This audit distinguishes current 3.6 host regression evidence, historical ARM compiler measurements and actual device measurements. Stack reservations are exact current manifest values. The ARM struct/layout tables retained below come from GDB `sizeof` expressions against seven ELF files built from the earlier 4.19.0c baseline with the official Flipper 1.4.3 SDK and GNU Arm Embedded 12.3. Evidence is in `tests/sdk/layout_arm.json`; those are real layouts for that historical SDK build, **not current 3.6 layouts and not measured free heap or cumulative stack high-water usage**. Firmware/framework objects, allocator metadata, fragmentation and allocator resize overlap are additional.

## 3.6 grant/performance stack review

The current 3.6 reservations are DNDolphins 6 KB, DNDInventory 4 KB, DNDSpellbook 4 KB, DNDAdventure 4 KB, DNDJournal 4 KB, DNDInitiative 4 KB and DNDBestiary 6 KB. DNDInitiative was raised from 3 KB to 4 KB after reviewing its ~2.3 KB source-estimated peak: the 5.28 KB `InitiativeApp` block is heap-allocated and is not a stack requirement, but 3 KB left too little margin for nested SDK/framework calls. The changed grant paths remain bounded and use heap-backed resident records plus fixed stream buffers rather than recursion or whole-catalog stack arrays.

A current strict host `-fstack-usage` pass across every manifest source set reports the following largest individual project frames. These x86_64 frames are not ARM call-chain peaks, but they are useful regression evidence and do not indicate that the current manifest reservations need to be increased:

| FAP | Reserved stack | Largest strict-host frame | Function/source |
|---|---:|---:|---|
| DNDolphins | 6,144 B | 1,776 B | `dndolphins_stage_character_grants` |
| DNDInventory | 4,096 B | 1,728 B | `dnd_storage_archive_profile` |
| DNDSpellbook | 4,096 B | 1,728 B | `dnd_storage_archive_profile` |
| DNDAdventure | 4,096 B | 2,256 B | `dndadventure_campaign_packs_validate_index` |
| DNDJournal | 4,096 B | 1,312 B | `dndjournal_input` |
| DNDInitiative | 4,096 B | 1,536 B | `dndinitiative_feature_recharge_fast_recharge` |
| DNDBestiary | 6,144 B | 2,288 B | `dndbestiary_state_load_filters` |

Within the changed DNDolphins grant path, `dndolphins_apply_grant` is 1,040 B and `dndolphins_stage_grants_up_to_owned` is 1,008 B; the Magic draw function is 1,072 B and performs no storage scan. The previous official ARM SDK measurements below are retained as a historical architecture baseline; because the current environment has no uFBT/ARM toolchain, they are **not** relabeled as a fresh 3.6 ARM measurement.

Grant processing retains at most 24 pending `PocketGrant` records, scans metadata with a 256-byte line and 512-byte read buffer, resumes the next review batch from a saved byte offset, consolidates dependent Feature/Feat discovery into forward passes, and stops a malformed dependency chain after eight generations. The periodic UI tick only handles deferred-event countdown/dice animation/marquee state. Grant scans and Spellbook-count scans are prohibited from Canvas draw paths by the host release audit; Magic refreshes its Known/knowable/free aggregate on screen entry and then draws the cached values.

Current 3.6 maintenance also removes avoidable repeated I/O without creating unbounded caches. Shared Settings are parsed best-effort from a **128-byte stack read buffer** instead of issuing a storage read for every byte; complete valid lines survive partial/corrupt reads while affected fields retain defaults. DNDolphins reuses its already-computed catalog availability during browsing, while Inventory and Spellbook cache one availability byte in their app state. Spellbook adds a fixed **128-byte status prefilter** to the app block for Prepared/Known/Always catalog filtering; it only rejects definite misses and still performs the exact storage lookup for possible matches. This adds fixed state rather than a collection-sized index.

The host release audit now scans **84 static draw helpers** for direct heap/storage calls, and the sanitizer-backed tests use wrapped `malloc/calloc/realloc/free` accounting that requires zero outstanding project allocations at process exit for exercised paths. Manual failure-path review found no confirmed lost allocation in the changed catalog/status/settings paths. This is evidence for the tested code paths, not proof that every firmware/framework or hardware error path is leak-free.

## Historical 4.19.0c ARM fixed layout baseline

| FAP | Manifest stack | Project app block (ARM SDK layout) |
|---|---:|---:|
| DNDolphins | 6,144 B | 5,192 B |
| DNDInventory | 4,096 B | 1,620 B |
| DNDSpellbook | 4,096 B | 1,572 B |
| DNDAdventure | 4,096 B | 512 B |
| DNDJournal | 4,096 B | 1,348 B |
| DNDInitiative | 3,072 B | 5,272 B |
| DNDBestiary | 6,144 B | 1,516 B |

DNDolphins contains the Language and Proficiency page buffers in its fixed block: 8 × 47 = 376 B and 8 × 63 = 504 B. These are bounded even when the sidecars grow. Removal of legacy inline language/training fields reduces the canonical character adapter to 2,920 B. The shared `DndSettings` structure is now five bytes (Skip Dice Loading, Debug, Get Elevated (persisted as legacy `ExtraItems`), Catalog scope and Homebrew). DNDInventory and DNDSpellbook both link/read it because their streamed loaders enforce Catalog/Homebrew at source; this does not materialize either catalog in RAM.

## Historical 4.19.0c linked code and ARM compiler stack frames

Dynamic project buffers are only part of the RAM requirement. The SDK's linked ELF section totals are below; text includes executable code and read-only data. File assets/relocations can make the on-disk FAP much larger and are not represented by these section totals.

| FAP | Text/read-only data | Data + BSS | Largest compiled project frame |
|---|---:|---:|---:|
| DNDolphins | 117,646 B | 1 B | 1,656 B |
| DNDInventory | 44,696 B | 0 B | 1,656 B |
| DNDSpellbook | 33,154 B | 0 B | 1,656 B |
| DNDAdventure | 34,703 B | 112 B | 2,168 B |
| DNDJournal | 12,852 B | 0 B | 1,152 B |
| DNDInitiative | 17,481 B | 0 B | 1,440 B |
| DNDBestiary | 42,343 B | 617 B | 2,224 B |

Largest frames are not call-chain peaks and can include functions later removed by the linker. For DNDolphins, linked sections + reserved stack + the named grant/catalog overlap already total **133,759 B** before framework/loader allocations. This is budget arithmetic, not a measured device minimum or proof of successful launch. Launch/free-heap and stack high-water tests remain necessary on the intended RogueMaster firmware.

## Record and operation layouts

| Record / allocation | Bytes |
|---|---:|
| `PocketCharacter` / `PocketSaveData` | 2,920 |
| `PocketSpell` | 324 |
| Eight spells plus four eight-byte flag arrays | 2,624 |
| `PocketItem` | 300 |
| Eight items | 2,400 |
| `PocketFeature` | 234 |
| Eight features | 1,872 |
| `PocketGrant` | 148 |
| `PocketProfileState` | 308 |
| `DndDolphinsSpellClassCounts` | 16 |
| Inventory projection/page-owner state | 404 |
| Spellbook projection/page-owner state | 324 |
| Spell sort key / 24-key batch | 36 / 864 |
| SHD restore context (21 paths and flags) | 2,029 |
| Three-item 420 bundle | 900 |

The signed Item container reference is now 32-bit; the existing text record field remains in the same position. Logical collection indexes/counts are 16-bit; resident page counts remain eight. The representation and storage/time still impose practical limits. No gameplay ceiling is implemented by allocating a 255-record replacement array.

## Representative project heap arithmetic

These figures describe named project allocations, not whole-device peaks. File objects and GUI/Loader/dispatcher/text-entry objects are excluded.

| Path | Included project allocations | Bytes |
|---|---|---:|
| DNDolphins Spell/Ritual Combat | app + 8 indexes + 5 × 64-byte rows + spell page | 8,152 |
| DNDolphins Weapon Combat | app + index/row block + item page | 7,928 |
| DNDolphins Feature list | app + feature page | 7,064 |
| DNDolphins Language/Proficiency catalog | app + 24-name/metadata catalog | 6,416 |
| DNDolphins grant/catalog upper overlap | app + 24 pending grants + catalog | 9,968 |
| DNDolphins SHD restore | app + rollback context | 7,221 |
| Inventory normal list | app + item page | 4,020 |
| Inventory page transfer | app + old/new item pages + canonical adapter + 1,280-byte line | 10,620 |
| Inventory ordinary save | app + item page + adapter + line | 8,220 |
| Inventory 420 generation | app + adapter + three-item bundle | 5,440 |
| Spellbook normal list | app + spell page | 4,196 |
| Spellbook page transfer | app + old/new spell pages + adapter + line | 11,020 |
| Spellbook ordinary save | app + spell page + adapter + line | 8,396 |
| Spellbook sort with resident page | app + spell page + 24 keys + line | 6,340 |
| Adventure active scene | app + 865-byte scene | 1,377 |
| Adventure reward update | app + scene + adapter + item page + line | 7,977 |
| Journal index rewrite | app + two 768-byte buffers | 2,884 |
| Initiative character sync | app + two 768-byte buffers | 6,808 |
| Bestiary main monster window | app + 15 × 172-byte summaries | 4,096 |
| Bestiary encounter generation | app + 2,088-byte encounter + 16 summaries | 6,356 |

Page transfer deliberately retains the old companion page until the new read succeeds. That means two bounded pages can overlap temporarily; normal residency is one page. Reallocation may temporarily require old and new blocks inside the allocator. Combat uses one bounded index/row block for the active weapon/spell path and never an index sized to the entire collection. Streamed Combat filtering may perform a full file pass when its index window changes.

## Streaming, ownership and stack review

- Spell pages have exactly one allocation owner: `spell_storage`. Spells and all four flag arrays are interior pointers. Growth moves flags before clearing the enlarged record region. Transfer clears the source owner; borrowed adapters detach every page pointer before cleanup. No interior pointer is freed separately.
- Spellbook resize uses a short-lived heap adapter instead of placing the complete character on its 4 KB stack. Other Inventory/Spellbook/Adventure compatibility adapters are likewise heap-owned.
- Item/Spell catalog readers stream one selected catalog file per browse session. Cached seek offsets keep paging resumable without materializing the catalog; Inventory's rolling 64-page offset window remains bounded.
- Item/Spell readers use a 256-byte stream buffer and a 1,280-byte heap line. Feature readers use a 256-byte stream buffer and 768-byte stack line. Language/proficiency sidecars use 96-byte reads and a 128-byte line. Their catalogs use 256-byte reads, a 192-byte line and at most 24 resident entries.
- Item/Spell/Feature owned-page indexes each retain at most 32 offsets (128 B), accelerating the first 256 records. Pages outside that map stream from the file; they remain accessible. Rewrites invalidate offsets. This bounded acceleration does not impose an ownership limit.
- Spell sorting retains 24 keys and rescans to emit each ordered batch, with original file offsets as a stable tie-breaker. Already-sorted files need only one scan and no rewrite. Runtime grows with collection size, but sorting heap does not.
- Canvas callbacks render cached rows only. Character list hydration and Combat row preparation happen in event/update handlers. Container names outside the resident page use the logical Item number instead of loading another page during drawing.
- Bundle creation uses a 900-byte heap block, then a single batch append. Item/Spell and new collection publication sync a temporary output before guarded renames; failed writes/renames leave the previous live collection intact in the tested failure paths. A rollback failure can leave recovery data in its backup file.
- SHD restore keeps its 2,029-byte path/presence context on the heap. Archive paths remain bounded stack arrays. No whole `PocketCharacter` local remains in Spellbook resize.
- Settings, SHD and grants do not add speculative campaign variables. Language/proficiency changes set a transient dirty flag so normal save also refreshes their history companions.

## Verification limits

`tests/host/run_tests.py` strictly compiles and links all seven manifest source sets against the host shim and runs the real storage/UI helpers with AddressSanitizer and UndefinedBehaviorSanitizer. Scenarios cover 320 Items/Spells/Features, 300 Language/Proficiency records, late indexes, flags, bounded sorting/Combat, grants, failure rollback and history lifecycle. Linker allocation wrappers require zero outstanding project allocations at each test exit. This verifies exercised paths, not every possible hardware/error path.

LeakSanitizer itself cannot inspect `/proc` in this environment and is disabled; address/undefined-behavior checks and explicit project allocation accounting remain enabled. The seven official-SDK ARM builds and strict API checks stored under `tests/sdk/` passed for the earlier 4.19.0c baseline; they are historical compatibility evidence, **not a fresh 3.6 ARM build**. No physical Flipper, firmware heap measurement or target cumulative stack high-water measurement was available for 3.6. The separate RogueMaster exported-symbol comparison is not a substitute for rebuilding against the intended device firmware. Run the unchecked device gates in `DEVICE_TEST_MATRIX.md` before treating 3.6 as hardware-validated.
