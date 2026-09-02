# Dungeons & Dolphins — Expansion Context

This file preserves planning and engineering context that is useful when extending Dungeons & Dolphins but does not belong in the user manual, feature checklist, changelog or forward feature roadmap. It is intended to help future ChatGPT sessions make consistent decisions, avoid reintroducing old regressions, and distinguish a genuinely new feature from behavior that already exists.

## How to use the project documents

Use the current source as the final authority when documentation disagrees with code. For planning, treat `README.md` and `FEATURE_CHECKLIST.md` as the inventory of current user-visible behavior, `RULES_AUDIT.md` as the current game-rule contract, `SAVE_SCHEMA.md` as the persistence contract, `MEMORY_AUDIT.md` as the memory model, `SOURCE_OWNERSHIP.md` as the module-ownership contract, and `ROADMAP.md` as future work only.

`EXTRA_.md` should hold historical decisions, removed planning notes, expansion guardrails, risk areas and rationale that can improve future implementation choices without cluttering user-facing documentation.

When a roadmap feature is implemented, remove it from the roadmap, document the resulting current behavior in README and the capability-level result in FEATURE_CHECKLIST, and summarize only the actual release change in CHANGELOG. Do not keep implementation-defense notes, validation chatter, manifest-refresh bullets, or statements that something is merely "unchanged" or "still working" in CHANGELOG.

README should remain organized per FAP in the real on-device option order. It should explain normal actions, Hold controls, shortcuts, defaults, implicit automation and hidden conveniences that provide user value. FAP-owned behavior may be discovered from source structure and behavior-bearing routines, but low-level allocation/free, parser, cache, drawing and generic storage helpers are not user features by themselves and should not be promoted into README.

FEATURE_CHECKLIST is a capability inventory, not a duplicate control manual. ROADMAP should contain only genuinely missing features or a clearly scoped improvement to an existing feature. The current roadmap convention is eight features per planned build with full explanatory sentences describing what each feature would do and how it would help.

## Core expansion guardrails

- Keep character-owned Inventory, Spellbook, Feature and applied-grant data centralized under `/ext/apps_data/dndolphins/` so every FAP sees the same character-owned records. Inventory and Spellbook being separate FAPs must not fragment those live records into separate app-data stores.
- Preserve bounded streaming and paging. Do not load whole Item, Spell, Feature, Bestiary, Journal, campaign or custom-pack collections into RAM when a bounded reader/window can serve the feature.
- Keep player-choice progression explicit. Automation may calculate deterministic rules and may apply deterministic grants only through the explicit grant actions, but it must not silently choose feats, ASIs, spells, subclasses, Fighting Styles, Invocations, Metamagic or similar player decisions.
- Save structures are frozen by default. Add a persisted field only when a real user-facing feature requires information that cannot be derived safely from current state. Do not change a save schema merely for validation, cleanup, atomicity or implementation convenience.
- Add campaign variables only when a future campaign feature genuinely requires new persisted campaign state. Do not create speculative variables for convenience or future-proofing.
- Prefer direct play utility over diagnostics/control screens. A new screen should help a player or GM during normal use; internal inspection belongs in engineering documentation unless it has a real user workflow.
- Preserve text-editable pack/catalog formats. Do not add checksum or hash rejection as a normal requirement; current pack/catalog policy intentionally favors best-effort editable text and structural validation.
- Prefer best-effort field-name compatibility where the current schemas already use it. Avoid migrations that rewrite working user data solely to normalize or validate it.
- Cross-FAP launches should remain explicit full-path handoffs with outgoing callbacks/timers/project state quiesced before Loader starts the next FAP. Short Back and Hold Back semantics should remain consistent with the current README unless a deliberate UX feature changes them.
- Canvas/draw callbacks should remain presentation-only: no project heap allocation, storage reads, collection hydration, rewrites or polling from draw time.
- Do not introduce firmware `qsort` dependencies merely for convenience. Existing deterministic ordering uses bounded app-owned approaches or asset ordering.
- Keep large transient work action-local and release it on every success/failure exit. If a companion needs a narrower storage API, prefer adding that bounded API over restoring a resident full-character object.

## Current persistence and ownership assumptions worth protecting

- Canonical character profiles belong to DNDolphins and use exact character-profile filenames; sidecars, exports, shadows, Journal files and Initiative files must never satisfy a canonical-profile lookup.
- Character ID `0` is valid.
- Active-profile resolution is shared across all FAPs and should remain one contract rather than diverging into app-specific parsers.
- Inventory currency is authoritative in the character Inventory sidecar, not the canonical character profile.
- Inventory records, Spellbook records, Features and applied deterministic-grant IDs are separate character-owned sidecars and are streamed/paged rather than embedded back into the canonical character save.
- Character `.shd` files are historical snapshots, never live authority. Current level history is a core SHD plus separately owned Inventory, Spellbook, Feature and applied-grant SHDs. Pre-4.19.0b core-only SHDs remain restorable without treating missing sidecars as evidence that those collections were empty.
- Current-level collection snapshots are history; the live `.txt` sidecars remain authoritative.
- Inventory, Spellbook and Adventure intentionally use narrow profile projections for the fields they need instead of retaining a full character in resident app state.
- Journal, Adventure, Initiative and Bestiary own their own app-specific persistence except for explicit bridges to character-owned sidecars.
- Initiative owns completed-encounter history. Saving history is explicit; it is not a Journal or Adventure responsibility.
- Campaign files should not be deleted as a recovery mechanism. Existing campaign-pack copy/install behavior intentionally leaves failed or existing content recoverable rather than destructively removing it. If future campaign/pack management needs a disable control, prefer an active/inactive state over deleting content or silently removing it from the catalog/index. Apply the same non-destructive principle to monster-pack management.

## Starting Inventory and deterministic grant semantics

Starting Inventory and character progression have several deliberate one-shot/explicit boundaries that future features should not accidentally collapse:

- A truly empty, never-granted Inventory may receive normal starting equipment automatically when Inventory opens.
- Once the starting-Inventory marker exists, deleting all Items must not silently reseed starting equipment.
- The explicit Inventory regrant is a deliberate one-time override and should preserve existing Items while adding the starting package again according to the current marker rules.
- **Grant Initial Traits** is for starting/level-1 deterministic grants.
- **Apply Level Grants** is the explicit catch-up action for currently eligible deterministic species/class/subclass grants.
- Increasing a class level updates deterministic numeric advancement such as fixed-average HP, Hit Dice and derived limits, but must not silently apply class/species/subclass Feature or Spell grants.
- Actual character/Feature/Spellbook payload is authoritative for whether a deterministic grant exists. Applied-grant markers are bookkeeping and must not invalidate a successful real write or suppress repair of a missing record.
- Level Choices owns player-selectable ASI/Feat opportunities. The two-stat `+1/+1` choice must not mutate the first score on the first pick and must apply exactly one point to each selected ability when committed.

## Memory and performance decisions removed from the roadmap

These were previously roadmap/testing notes. They remain useful engineering context but are not product features.

- The design optimizes resident memory first. Inventory, Spellbook, Feature lists, Bestiary access, campaigns and Journal/profile access use bounded windows/streams rather than whole-file hydration.
- The established collection page size is eight records for Inventory, Spellbook and Features. Do not casually increase it without a measured reason.
- Catalog offset maps are bounded acceleration hints; storage remains authoritative and the entire catalog is not materialized in heap.
- Projection-based companion FAPs may still use bounded transient compatibility adapters for a few shared operations. If hardware measurements show those peaks are a problem, prefer narrower collection/storage APIs rather than moving a full canonical character back into resident companion state.
- App stack reservations were intentionally kept tight and app-specific. When adding a feature with substantial stack locals or nested calls, review the current `MEMORY_AUDIT.md` before increasing stack reservations.
- Historical hardware-risk areas included grant/catalog overlap, Inventory/Spellbook sidecar rewrite peaks, Adventure reward bridging, Initiative history/profile synchronization, maximum Bestiary encounter generation and repeated cross-FAP launches. These are engineering test targets when touching those systems, not roadmap items.
- Large assets and catalogs should stay with the FAP that owns the user-facing feature so unrelated FAPs do not grow simply because they link a shared file.


## Removed engineering follow-up ideas worth retaining

These were once roadmap items but were removed because they describe implementation strategy or testing rather than user features. They can still guide expansion work when the relevant subsystem changes.

- Evaluate a standalone Combat FAP only if measured Loader/runtime memory pressure justifies it. Do not split Combat merely to make source organization look cleaner.
- If transient full-character compatibility adapters become a measured memory problem, add narrower collection/profile APIs that carry only the fields needed by Inventory, Spellbook or Adventure instead of changing the canonical character schema.
- Improve ammunition and container workflows by reusing existing Item state transactionally before inventing new persisted state.
- Favor declarative campaign and monster content over executable scripting, and keep long-text/accessibility improvements compatible with bounded readers.
- When touching progression, Inventory containers or Initiative history, fault-injection and repeated cold-launch testing are historically high-value because those areas have had persistence/navigation regressions before.
- When touching large campaigns, Bestiary streaming or maximum Initiative rosters, stress the bounded access paths rather than raising resident limits as the first response.

## Source-ownership decision rules

- Shared code is appropriate when multiple FAPs use the same contract or when splitting it would duplicate a canonical parser, scanner, rules primitive or transactional publish path.
- App-specific behavior belongs with the FAP that owns the user workflow. Moving a helper local is appropriate only when no other FAP needs the implementation.
- A shared storage operation may remain shared even with one current caller when moving it would duplicate canonical parsing, scanning or atomic rewrite/publish behavior.
- Implementation-only helpers should stay private to their source. Do not preserve stale wrappers solely to avoid reorganizing callers; remove unused wrappers after confirming the real behavior is still reachable.
- Ownership cleanup must not alter save schemas, active-profile selection, launch/return behavior, paging, draw-time I/O rules, grant semantics or resource-consumption behavior.

## Retired or deliberately removed UI concepts

Do not reintroduce these exact screens merely because older source/history mentions them. If the underlying need becomes useful, redesign it as a clear player/GM feature.

- Adventure **Campaign Diagnostics** was removed because the screen was broken and did not provide useful normal-play value.
- Adventure **Installed Pack Controls** was removed because the control screen was broken. Normal campaign-pack loading remains useful; any future Pack Library/Controls feature should be a redesigned user workflow, not restoration of the old screen.
- Bestiary **Monster Pack Controls** was removed because it did nothing useful. Pack loading remains separate from that retired menu item.
- Bestiary **Pack Diagnostics** remains available and belongs at the end of the Bestiary menu rather than interrupting browse/encounter workflows.

## Roadmap ideas removed because they already exist or overlap current behavior

These names are useful historical context because they can otherwise be mistakenly proposed again as "new" features.

- **Pending Progression** was removed as a broad roadmap feature because Level Choices and Level-Up Review already expose pending ASI/Feat and progression information. Future progression work must add a capability those screens do not provide.
- **Send Encounter to Initiative** was removed because Bestiary already hands generated/saved encounter participants into Initiative. Future encounter handoff work should add something beyond the existing full-encounter transfer.
- **Grant Preview** was removed because the application already has structured grant review/retry/edit behavior and the concept overlapped that workflow. The current roadmap's **Grant Review Center** is specifically about making unresolved grant review directly discoverable from a normal Character menu, not recreating existing grant mechanics.
- A generic **Timed Effects** concept was removed because it overlapped better-scoped Initiative condition-duration and Spell duration/effect tracking features. Keep duration ownership tied to the system that understands the effect.
- Hardware validation, soak testing, fault-injection, stack-high-water measurement and regression confirmation were removed from ROADMAP because they are engineering acceptance work, not user features. Use `DEVICE_TEST_MATRIX.md` and `MEMORY_AUDIT.md` for that work.

## Historically out-of-scope approaches

Treat these as strong defaults unless a future user request explicitly changes direction:

- Do not add executable campaign scripting merely for extensibility; prefer declarative campaign content and bounded metadata.
- Do not make Journal entries implicitly drive Adventure progress.
- Do not re-embed Inventory, Spellbook, Feature or applied-grant sidecars into canonical character saves.
- Do not replace bounded paging with whole-file list loading.
- Do not make save-format changes solely for validation or atomicity.
- Do not split a FAP solely for source organization. A new FAP should solve a real runtime-memory, ownership or user-workflow problem.

## Existing behavior that future features must account for

Before proposing or implementing an expansion, check README and FEATURE_CHECKLIST for these hidden/convenience behaviors because new work can easily duplicate or break them:

- Loose ammunition uses case-insensitive name-token matching, so an Item such as `Fire Arrow` can satisfy a weapon that requires `arrow`; weapon-local ammunition counters take priority when configured.
- Spell catalogs are storage-streamed, sorted by spell level/name, and support Character Classes as the default plus Any Class/individual class browsing. Eligibility and pure catalog-class browsing are separate concepts.
- Progression feat choices default to conservative Allowed filtering; custom/unknown rows remain reachable through All rather than being guessed eligible.
- Companion FAPs share the active character but keep app-owned screens/state separate. Returning to DNDolphins may refocus the originating Home option.
- Inventory catalog presets populate real weapon/armor mechanics; custom records can still be edited.
- Initiative synchronizes relevant main-character combat state and owns automatic Turn/Encounter Feature recharge behavior.
- Adventure uses one-shot guards for rewards/milestones so revisiting a scene cannot silently duplicate guarded rewards.
- Bestiary tracks Recent monsters and persistent encounter settings and can hand encounter participants to Initiative.

## Feature-expansion heuristics

When searching the code for expansion ideas, use behavior-bearing FAP-owned logic as clues to user value: automation, rules application, filtering, syncing, hidden defaults, cross-app handoff, recovery, navigation shortcuts, resource consumption and content workflows. Do not turn allocation/free routines, generic parsers, cache plumbing, drawing helpers or incidental implementation details into feature proposals.

When an existing feature is useful but limited, document the current behavior in README and put only the missing improvement into ROADMAP. Examples include a browser for already-saved combat history, choosing among multiple currently matching ammo stacks, making existing grant review directly accessible, or adding richer organization around an existing collection.

Prefer improvements that reduce table friction, repeated navigation, duplicate data entry, manual arithmetic or forgotten per-turn/per-rest state. Keep optional automation explicit when a D&D rule can vary by table or ruleset.

For any feature that adds persistence, first ask whether the state can be derived from existing character, collection, Initiative, Adventure or Journal records. If it can, derive it. If it cannot, add the smallest app-owned persisted state that satisfies the user feature and document ownership in SAVE_SCHEMA.

For any feature that touches multiple FAPs, define one owner for the authoritative state and make the other FAPs consume or bridge that state rather than creating parallel copies.

## Documentation decision rules preserved from earlier cleanup

- README: current user behavior, per FAP, actual menu order, controls and hidden conveniences.
- FEATURE_CHECKLIST: current capability-level inventory; avoid duplicating every button instruction.
- CHANGELOG: concise released changes only.
- ROADMAP: future features/improvements only; no testing chores, implemented features or implementation-defense notes. Keep detailed explanations for each planned feature.
- Technical schema/audit documents: current technical truth, not release-history narration.
- EXTRA_: AI/planning context, historical rationale, removed guardrails, retired ideas and expansion heuristics that improve future implementation decisions.

## Completed scalable-collection work and future guardrails

The former 4.19.0c WIP is historical. Version **3.6** is the active release line and carries forward the audited scalable collections plus the reviewed grant/catalog work. Do not restart from an older ZIP or infer current behavior from pre-3.6 grant notes.

Item, Spell and Feature logical indexes/counts are 16-bit; resident pages remain eight records. Combat retains eight filtered logical indexes and five formatted rows. Keep filtering and storage reads outside Canvas callbacks. A 32-offset seek table accelerates the first 256 owned records; later pages stream safely without allocating a larger index. Language/proficiency windows stream their sidecars using a 96-byte buffer and 128-byte line. Their asset catalogs use 256-byte reads and a 24-name selection page; this is not a catalog-total ceiling.

Spell records and all four parallel flag arrays have one allocation owner, `spell_storage`. Borrowed adapters detach every pointer before cleanup; transferred pages clear the source owner. Never individually free/reallocate any flag-array interior pointer. Ritual status must never force Known during parsing or serialization. Manual/catalog additions initialize Known explicitly.

Normal character creation does not create an Inventory sidecar. Default **Get Elevated=420** is applied after normal starting equipment is generated. Off-to-420 applies a bundle to the active character only when Inventory exists. Catalog visibility is independent: **Homebrew=Yes/No** alone admits/rejects `Homebrew`/`DNDolphins` catalog rows in either Catalog mode. A whole bundle uses one batch append and checked publication. Do not add speculative campaign variables or a Character Sheet FAP for this work.

Language/proficiency legacy core fields are dropped without migration. Live filenames are `languages_{id}.txt` / `proficiencies_{id}.txt`. SHD bundle version 2 declares six companions; version 1 declares only the original four, so a missing new companion in an old bundle must never clear a current collection.

The former roadmap training entry is now implemented as catalog-backed lists. Its remaining rule-integration opportunity is represented by Training Impact Preview in the future roadmap. Original planning text:

> 2. **Structured Training & Proficiency Sheet:** Add dedicated user-facing sections for tool proficiencies, armor training and weapon training instead of relying on one free-form Other Proficiencies field. Structured entries would make granted training easier to inspect, edit and reuse for future rules checks while still allowing custom text for homebrew proficiencies.

## Superseded memory-audit context

The following figures are preserved only as historical engineering context from the incoming WIP. They are superseded by the current `MEMORY_AUDIT.md`, include old field/layout assumptions, and must not be treated as current measured device RAM or stack usage.

# Memory audit

This audit separates values that are exact from project source from values that still require firmware/device measurement.

- **Stack reservation** is exact from `application.fam`.
- **Fixed project app block** and listed record/projection sizes are compiler-checked ARM32 layouts.
- **Project working set** is arithmetic over project-owned app blocks and bounded transient project allocations. Firmware/framework objects, allocator metadata and fragmentation are additional.
- **Source-estimated stack peak** is a conservative source review, not a measured high-water mark. Device instrumentation remains authoritative.

## Per-FAP summary

| FAP | Stack reservation | Source-estimated stack peak | Fixed project app block | Representative bounded project working set |
|---|---:|---:|---:|---|
| DNDolphins | **6,144 B** | **~2,900 B** | **4,972 B** | **7,940 B** Spell/Ritual Combat; **7,700 B** Weapon Combat; **9,748 B** conservative grant/catalog overlap; **~6.5 KB** during SHD restore rollback context |
| DNDInventory | **4,096 B** | **~2,500 B** | **1,500 B** | **3,884 B** normal 8-Item page; **5,164 B** ordinary sidecar rewrite; **~9,140 B** conservative regrant adapter/rewrite overlap |
| DNDSpellbook | **4,096 B** | **~2,400 B** | **1,456 B** | **4,080 B** normal 8-Spell page; **~8,056 B** page load with transient canonical adapter; **~9,336 B** save/rewrite overlap; **6,224 B** sort-with-page |
| DNDAdventure | **4,096 B** | **~2,300 B** | **516 B** | **1,381 B** with active scene; **~7.5–8.5 KB** only during transient Inventory reward bridging |
| DNDJournal | **4,096 B** | **~2,660 B** | **1,352 B** | **2,888 B** during two-buffer index rewrite |
| DNDInitiative | **4,096 B** | **~2,300 B** | **5,280 B** | **6,816 B** during main-character two-buffer profile sync; history save is stack-bounded and adds no resident state |
| DNDBestiary | **6,144 B** | **~4,370 B** | **1,528 B** | **4,108 B** main monster window; **6,368 B** encounter generation |

Explicit grant processing uses a bounded 256-byte metadata line plus 512-byte read buffer and retains no grant batch in app state. ASI level-choice state uses existing app-struct alignment. Spell catalog class filters reuse the existing one-byte selector, and Inventory page residency/page-label controls add no resident buffers. The temporary grant-status deferral contributes the prior **8 B** alignment growth. 4.19.0b adds the two-byte shared settings state plus a bounded 20-level SHD selector to DNDolphins, increasing its fixed app block by a further **24 B**; Adventure and Initiative each add the two-byte shared settings state with ARM32 alignment for a **4 B** fixed-block increase. Inventory, Spellbook, Journal and Bestiary do not link `dnd_settings.c` because they have no player-facing dice-roll path.

The projection-based companions optimize **resident** state first. A few existing shared storage APIs still accept `PocketCharacter`, so Inventory grants, Spellbook page I/O and Adventure Item rewards create a bounded full-character adapter only for that operation. Those adapters are freed before returning and are not embedded in the app state. If hardware measurements show those transient peaks matter, the next optimization should be narrower collection-storage APIs rather than restoring a resident full character.

## Exact current ARM32 project sizes

| Record/state | Size |
|---|---:|
| `PocketSaveData` / `PocketCharacter` | **3,976 B** |
| `PocketItem` | **298 B** |
| `PocketSpell` | **324 B** |
| `PocketFeature` | **234 B** |
| `PocketGrant` | **148 B** |
| `PocketMonsterSummary` | **172 B** |
| `PocketMonsterDetail` | **1,544 B** |
| `PocketMonsterEncounter` | **2,088 B** |
| `DndAdventureScene` | **865 B** |
| `PocketCampaignSummary` | **120 B** |
| `PocketCampaignProgress` | **88 B** |
| `PocketBestiaryFilterPreset` | **77 B** |
| `PocketSavedEncounter` | **432 B** |
| `PocketProfileState` | **308 B** |
| `DndInventoryProfileProjection` | **374 B** |
| `DndSpellbookProfileProjection` | **298 B** |
| `DndAdventureProfileProjection` | **72 B** |
| Resident Inventory character/page-owner state | **404 B** |
| Resident Spellbook character/page-owner state | **320 B** |

Small rule/index records include `DndDolphinsSpellClassCounts` 8 B, `PocketAttackRoll` 10 B, `PocketDamageRoll` 92 B, `DndInventoryItemAggregate` 8 B, `DndInventoryCatalogEntry` 52 B and `DndSpellbookCatalogEntry` 56 B.

## Working-set derivation

### DNDolphins

- Fixed app block: **4,972 B**. The bounded level-up review retains only derived before/after and pending-choice flags; deterministic grant counts are no longer stored because grants are explicit actions.
- Combat logical index: **24 B** maximum.
- Visible Combat row cache: 5 × 64 B = **320 B**.
- Item page: 8 × 298 B = **2,384 B**.
- Spell page: 8 × 324 B plus four 8-byte spell-state arrays = **2,624 B**.
- Feature page: 8 × 234 B = **1,872 B**.
- Spell/Ritual Combat: 4,972 + 24 + 320 + 2,624 = **7,940 B**.
- Weapon Combat: 4,972 + 24 + 320 + 2,384 = **7,700 B**.
- Maximum 24 pending Grants: 24 × 148 B = **3,552 B**.
- Current 24-entry character/feat catalog block: **1,224 B**.
- Conservative grant/catalog overlap: 4,972 + 3,552 + 1,224 = **9,748 B**.
- SHD restore allocates one heap-owned `DndShdRestoreContext` containing 15 bounded path buffers plus presence flags (about **1.5 KB**) and uses streaming 256-byte file-copy buffers. The context is freed on every success/failure path and was deliberately moved off stack; it does not coexist with Combat page caches during normal restore UI use. Bundle-completeness detection uses one transient 96-byte path buffer and adds no resident app-state allocation.

Weapon and Spell pages are not resident together. Combat section headings and level-up review presentation add no dynamic resident list.

### DNDInventory

- Fixed app block: **1,500 B**, reduced from the earlier full-character design by the resident Inventory projection/state.
- Eight-Item page: **2,384 B**.
- Normal resident project blocks: 1,500 + 2,384 = **3,884 B**.
- Ordinary transactional sidecar rewrite line buffer: **1,280 B**, yielding **5,164 B** while the page remains resident.
- Canonical-profile projection scan/rewrite uses one bounded **640 B** heap line, not a full resident character.
- Starting-inventory/regrant compatibility with the existing shared composition API uses one transient **3,976 B** `PocketCharacter`. A conservative regrant overlap with the resident page and 1,280 B collection line is 1,500 + 2,384 + 3,976 + 1,280 = **9,140 B**. The adapter is freed on every success/failure exit.

### DNDSpellbook

- Fixed app block: **1,456 B**.
- Eight-Spell page including four state arrays: **2,624 B**.
- Normal resident project blocks: **4,080 B**.
- The current shared spell-window API still receives a transient **3,976 B** canonical adapter while loading/saving a page. Load overlap: 1,456 + 2,624 + 3,976 = **8,056 B**.
- An ordinary save/rewrite can additionally own the **1,280 B** bounded collection line: **9,336 B** conservative overlap.
- Sorting uses at most 24 compact 36-byte keys = **864 B** plus the 1,280-byte line buffer; sort-with-page: 1,456 + 2,624 + 864 + 1,280 = **6,224 B**.
- Projection scanning itself uses one **640 B** bounded heap line and does not retain the full character.

### DNDAdventure

- Fixed app block: **516 B** with the 72-byte profile projection and shared settings resident.
- Active scene: **865 B**, for **1,381 B**.
- Campaign-pack loading is bounded and storage-backed.
- Item reward bridging creates a transient 3,976-byte canonical adapter because the shared Inventory append/window API still uses that owner shape. When an Item page/rewrite buffer overlaps, the project peak is conservatively **~7.5–8.5 KB**; that state is action-local and freed before returning to Adventure.

### DNDJournal

- Fixed app block: **1,352 B**.
- Index rewrite can own two **768 B** heap buffers simultaneously.
- Working set: 1,352 + 768 + 768 = **2,888 B**.

### DNDInitiative

- Fixed app block: **5,280 B**; the shared settings state adds only aligned fixed bytes and encounter history still adds no resident index.
- Explicit main-character profile synchronization can own two **768 B** heap buffers.
- Working set: 5,280 + 768 + 768 = **6,816 B**.
- Completed-history publication uses bounded path/header/member buffers on stack and one storage `File*`; source review raises the conservative peak to **~2.3 KB**, with the current 4 KB reservation leaving additional SDK/framework call-chain margin. Each record is published atomically and no history index is retained.
- DNDInitiative reserves a **4 KB** stack. Its ~5.28 KB fixed `InitiativeApp` block is heap-owned and therefore is not constrained by the thread stack; the increase from 3 KB adds call-chain/framework safety margin above the ~2.3 KB source-estimated stack peak. Five-row roster/setup/combat/editor windows reuse existing selection/scroll fields.

### DNDBestiary

- Fixed app block: **1,528 B**.
- Main monster window: 15 × 172 B = **2,580 B**, for **4,108 B**.
- Encounter generation can own one 2,088-byte encounter plus a 16 × 172 B = **2,752 B** candidate sample.
- Encounter-generation project blocks: 1,528 + 2,088 + 2,752 = **6,368 B**.

## Projection and shared profile/handoff behavior

All seven FAPs link `dnd_profile_handoff.c`. Its active-profile reader uses a fixed 96-byte stack line and does not hydrate a character or collection.

Only Inventory, Spellbook and Adventure link `dnd_profile_projection.c`. It scans the canonical character by field name and reads the canonical format by field name. The parser/rewrite line is bounded to **640 B**, matching the canonical encoded-character line bound, and is heap-owned only during the stream operation. Inventory's projection writer patches only Vitals AC plus CombatFlags encumbrance/carry-capacity fields and transactionally publishes the rewritten canonical file. Spellbook and Adventure have no canonical-profile write API.

A failed projection load may invoke the existing backup-restoration path with one transient **3,976 B** `PocketSaveData`; this is recovery-only, not normal resident state.

## Draw-time, allocation and ownership audit

- Project canvas callbacks do **not** allocate project heap, perform storage I/O, hydrate pages, rewrite collections or poll storage.
- Storage/catalog/page/projection work occurs on app/screen entry, explicit input, cache boundaries or writes.
- Combat headings and Initiative visibility calculations use existing scalar state only.
- Collection pages/indexes/scenes/Bestiary blocks, projection lines, compatibility adapters and rewrite buffers have explicit normal/failure release paths.
- Cross-FAP launches quiesce callbacks/timers and release outgoing project-owned state before Loader starts the next FAP.
- No firmware `qsort` dependency is used.

## Hardware measurement checklist

1. Measure stack high-water for all seven FAPs under the exact **6/4/4/4/4/4/6 KB** reservations, especially projection save/load paths on the three 4 KB companions.
2. Compare steady-state free heap before/after the projection change; Inventory, Spellbook and Adventure should show the reduced resident blocks above.
3. Stress Inventory reviewed initial-grant/regrant transactions, page-boundary repair, Spellbook page load/save/sort, and Adventure Item rewards while watching transient free-heap lows and fragmentation.
4. Repeatedly enter/exit Weapon, Spell and Ritual Combat while checking that indexes/pages are released.
5. Stress Initiative with the maximum roster through wraparound scrolling, edit, reorder, Resume, next-turn and Hold-Up previous-turn navigation.
6. Exercise projection/allocation/write failure paths while checking canonical-profile integrity and steady-state project heap recovery.

A RogueMaster firmware build plus device free-heap, allocator-fragmentation and stack-high-water instrumentation remains the final authority for total runtime memory.

## Historical 4.19.0c SDK follow-up

The previous 4.19.0c baseline was validated with uFBT 0.2.6, the unmodified official 1.4.3 SDK and ARM GCC 12.3. That historical build exposed two host-shim blind spots: an alphanumeric `fap_version` was rejected by both official and RogueMaster parsers, and `strpbrk` was not exported. At that time all seven manifests used numeric `(4, 19)` metadata plus the shared 4.19.0c release define. The compatible fixes were retained in 3.6: manifests still use numeric version tuples and collection replacement still uses exported delimiter checks.

All seven ARM FAP builds and API checks passed for that 4.19.0c snapshot. The files under `tests/sdk/` are therefore historical target-toolchain evidence, **not a fresh 3.6 ARM build**. Current 3.6 validation is the strict/sanitized host suite plus the catalog and stack-regression audits. Rebuild 3.6 against the intended RogueMaster/Flipper SDK and perform device free-heap/cumulative-stack testing before treating target validation as current.
## 3.6 grant/catalog implementation notes

- Complete catalog/support files use the `*_All.txt` suffix. These `_All` files are the catch-all `Catalog: All` assets and are the intended destination for future non-SRD findings; the matching unsuffixed files remain the SRD-facing assets. Keep this provenance rule internal to development/compliance documentation.
- Grant review is mandatory for newly eligible character grants. `?` means pending, `A` applied and `S` skipped. Apply All skips choice-bearing rows so player decisions are never guessed.
- Choice dispatch covers Language, Spell, Feat/Perk, Skill, Skill/Tool, Proficiency and Size. Dependent feat/feature grants are staged in later bounded batches.
- Review metadata uses a 24-row resident queue with byte-cursor resume and an eight-generation dependency ceiling. No grant scanner is called from the periodic tick or Canvas draw paths.
- Magic Known/knowable/free-granted totals are refreshed on screen entry and cached for drawing. Normal class-capacity selections are excluded from the free-granted count.
- DNDInventory exclusively owns starting equipment and requires a Review inventory grant confirmation; DNDolphins metadata is audited to reject `item=` grants.
- The Item catalog has 682 rows with non-empty Source fields. Ravenloft sources are split between CoS and VRGtR; structured catalog rows display compact Source tags.
- Release-gate coverage is 103 species, 36 backgrounds, 13 classes, 139 subclasses and 33 feats. Both independent spell catalogs contain 482 names and every fixed spell grant must exist in both.
- Grant metadata is physically scope-separated: `metadata/options.txt` is SRD-only (1,575 total / 616 grant-bearing rows) and complete `options_All.txt` retains all 3,637 / 2,380 rows. `Catalog: All` directly selects the complete metadata file and is unavailable if it is missing. The complete file must preserve every SRD metadata row byte-for-byte at the field level.
- The built-in Class/Subclass/Species/Background/Feat name arrays are missing-asset SRD picker fallbacks only. Recognized class Hit Die/spellcasting mappings are separate runtime rules and should not be removed just because catalog names are externalized.
- Current host stack regression data does not justify changing the manifest reservations. DNDolphins remains 6 KB; the changed grant scanner's strict host frame is 1,776 B. A fresh ARM/device high-water measurement is still required before claiming target stack usage.


## 3.6 formatting release gate

- Flipper app releases should run `ufbt format` before packaging. uFBT/FBT formatting is ClangFormat using the firmware `.clang-format` specification.
- The 3.6 runtime environment did not include the `ufbt` executable or standalone `clang-format`; the installed Clang formatting engine exposed through `clangd` was therefore run against the official Flipper `.clang-format` specification for every one of the 61 application C/H files.
- First pass changed 44 files (8,068 formatter edits); the immediate second pass changed 0 files / 0 edits, establishing formatter idempotence.
- After formatting, the full strict/ASan/UBSan host suite passed, followed by 20/20 additional Character regression runs.
- `.clang-format` remains intentionally ignored and is not a project-owned source file; use the SDK/toolchain version selected for the intended firmware when performing a target-side `ufbt format`/`ufbt lint` gate.

## Documentation visibility policy

Internal non-SRD catalog/provenance names, source-specific examples, compatibility details and split-file implementation notes may be retained in **EXTRA_.md** for future development and compliance work. Do not surface those details in README.md, CHANGELOG.md, ROADMAP.md, normal audit/policy documentation, device-test documentation or public validation summaries; those documents should describe the SRD/Homebrew-facing behavior only.

### 3.6 internal spell-catalog ownership / Artificer notes

- `character_assets/catalogs/spells.txt` and `spells_All.txt` are the canonical runtime spell catalogs. DNDSpellbook prefers `/ext/apps_assets/dndolphins/catalogs/` and keeps only a code-level fallback to the old DNDSpellbook asset root for older SD installations. New packages no longer bundle `spellbook_assets`.
- Spellbook already supports `Any`, `Cantrip`, and level `1` through `9` filtering; host regression coverage now protects this behavior.
- Artificer uses known cantrips and prepared level-1+ spells. Tinker's Magic grants Mending independently of the normal cantrip allowance. The complete spell catalog extends Artificer associations only in `spells_All.txt`, and the Mending/Tinker's Magic progression grants exist only in `options_All.txt`.
- Shared SRD spell rows remain authoritative in `spells.txt`; the complete file may extend only their class-association field. The release audit rejects changes to level, school, ritual flag, source, name, or loss of any base class association.
