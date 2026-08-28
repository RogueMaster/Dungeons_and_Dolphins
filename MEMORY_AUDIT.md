# Memory audit

This is a source-derived estimate, not an RTOS high-water measurement. The estimated project-visible peaks below track the largest known nested local-frame paths in project code; compiler prologue/alignment, firmware/framework calls, interrupts and RTOS bookkeeping can consume additional stack. Hardware high-water testing remains authoritative.

## Stack reservations and estimated use

| FAP | Reserved stack | Estimated project-visible peak | Estimated reservation used | Estimated headroom |
|---|---:|---:|---:|---:|
| DNDolphins | 6,144 B (6 KB) | ~3,352 B | ~55% | ~2,792 B |
| DNDAdventure | 4,096 B (4 KB) | ~2,768 B | ~68% | ~1,328 B |
| DNDJournal | 4,096 B (4 KB) | ~2,656 B | ~65% | ~1,440 B |
| DNDInitiative | 4,096 B (4 KB) | ~1,344 B | ~33% | ~2,752 B |
| DNDBestiary | 6,144 B (6 KB) | ~4,368 B | ~71% | ~1,776 B |

The current highest estimated stack pressure remains **DNDBestiary**, followed by **DNDAdventure** and **DNDJournal**. DNDolphins retains substantially more headroom after the Verify Profile temporary was moved off stack. The estimates are intentionally conservative project-code figures and should not be interpreted as free-stack guarantees.

## Known peak paths

### DNDolphins

- Current source-visible project peak: about **3,352 B**.
- Verify Profile is no longer the peak; its large temporary character state is checked transient heap state and is cleared/freed after parsing.
- Spell/item feature modules do not add persistent heap caches and use the same shared dice/storage primitives after the ownership split.
- Deterministic progression synchronization is action/level-up driven. Fixed-grant metadata is opened only when the player selects **Grant Initial Traits** at level 1 or when a class level actually increases (including completion of a newly added multiclass level), then consumed as logical **eight-line pages** through one reusable **192-byte line buffer plus 256-byte read buffer**. No eight-line array, progression hash/signature, or progression catalog is retained after the scan returns. Character creation, existing-profile loading and ordinary save/navigation operations do not open the progression asset.
- Core class progression tables are static constants rather than resident heap collections; no complete progression catalog is materialized in RAM.
- Character spellbook and inventory workflows retain at most one eight-record page each rather than complete owned collections.

### DNDAdventure

- Current source-visible project peak estimate: about **2,768 B**.
- Campaign manifests/scenes are storage-backed and loaded on demand; bundled campaign content does not increase the scene-object maximum or create a resident campaign database.
- The additional bundled campaign is asset data only and does not add a new runtime cache or save structure.

### DNDJournal

- Current source-visible project peak estimate: about **2,656 B**.
- The list keeps only a bounded metadata cache; entry bodies are loaded when opened rather than retained for the whole Journal.
- Newest-first ordering is implemented without firmware `qsort` and without materializing the complete journal in RAM.

### DNDInitiative

- Current source-visible project peak estimate: about **1,344 B**.
- Party/combat persistence is streamed named-field data rather than a whole-file temporary buffer.
- Initiative remains the largest relative stack reserve margin of the five FAPs.

### DNDBestiary

- Current source-visible project peak estimate: about **4,368 B**.
- The largest known visible path remains encounter named-field writing rather than normal browsing or the default custom-pack seed.
- Monster lookup uses bounded sparse/recent hints rather than complete offset/hash arrays; the bounded lookup cache is static and does not grow with catalog size.
- The browse window remains bounded rather than retaining the full result set.
- The full-screen stat reader uses a 27-byte stack line buffer for 26 visible characters plus NUL; this six-byte correction does not change the documented Bestiary peak path.

## Heap and collection ownership

- Spellbook and inventory UI ownership pages hold at most eight records.
- Whole-collection spell/resource/item calculations stream one record at a time instead of assuming the resident page is the full collection.
- Character picker and Journal lists use storage-backed bounded metadata caches.
- Bestiary lookup/browse state is bounded; full packaged/custom/pack tables remain on storage.
- Companion apps are separate processes. Cross-FAP transitions quiesce callbacks/timers and free outgoing views, caches, services and dynamic state before launch.
- The item/spell ownership split did not add persistent heap caches.
- Spell/item additions now append one encoded record directly instead of allocating the 1,280-byte whole-collection rewrite line buffer; failed appends truncate back to the original sidecar length.
- After a successful append, the new item/spell is copied directly into a one-record editor page when it is not adjacent to the resident page. Opening the editor therefore performs no second 1,280-byte sidecar read/allocation and does not depend on rereading a partially damaged collection.

## Starting inventory path

First Inventory open after Class and Species are confirmed is the only normal starting-inventory initializer. If either still has its new-character placeholder, Inventory opens without seeding. Otherwise, when the live item sidecar is missing or contains no valid item records, it streams matching class/species/background equipment and one d100 trinket through bounded readers, writes the new sidecar and applies starting currency once. The existing-sidecar probe stops after the first valid item record rather than hydrating an inventory page.

The initialization path does not keep the source equipment catalogs resident. Failure restores in-memory currency and removes a newly-created live item sidecar so retry starts from a clean state.

## Default custom monster seed

The Bestiary seed adds no persistent runtime cache or heap allocation of its own. Its file-copy helper uses two `File*` objects plus a **256-byte stack copy buffer**, closes/frees both handles on all paths and publishes through temporary files.

That seed-copy frame is not the Bestiary stack peak. Seeding is attempted only when both live user custom files are absent; existing or partial user custom data is left untouched.

## Leak/ownership audit status

The item/spell refactor preserved the prior dynamic-allocation ownership pattern: moved rule functions themselves allocate no retained heap. Changed storage/copy paths close/free their `File*` objects and temporary buffers on success and failure branches.

Host sanitizer coverage has exercised the moved rule/item/spell paths and default custom seed without AddressSanitizer, UndefinedBehaviorSanitizer or LeakSanitizer findings. This does not replace device-level fragmentation/high-water testing.

## Hardware stress priorities

1. Repeated DNDolphins ↔ companion-FAP handoff loops.
2. DNDBestiary maximum-size encounter write/save/transfer paths.
3. Large character/profile, Journal, campaign and monster indexes.
4. Verify Profile followed by repeated spell/inventory page transitions.
5. First Inventory open with missing sidecar, including failure/retry cases.
6. Repeated Bestiary startup with fresh, existing and partial custom-monster files.
7. Repeated class-level changes across caster/resource/fixed-grant thresholds, including multiclass and species-spell synchronization.

A real RogueMaster build plus device stack high-water/fragmentation stress remains the final authority for reducing any reservation further.
