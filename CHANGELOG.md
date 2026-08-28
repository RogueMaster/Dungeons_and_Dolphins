# Dungeons & Dolphins changelog

Released work only. Each released revision is retained as a concise summary; troubleshooting experiments and changes removed before a release are not expanded here.

## 3.2.18

- Added deterministic level-progression synchronization without changing the save schema: class Hit Dice, shared multiclass slots, Pact Magic, Sorcery Points, cantrip/prepared limits and Wizard spellbook minimums now follow class levels while preserving already-spent resources where practical.
- Made fixed-grant progression metadata action/level-up driven: only the explicit level-1 **Grant Initial Traits** action and actual level increases open it; scans consume bounded eight-line pages, and no progression hash/signature/catalog is retained in memory.
- Added bounded, duplicate-safe fixed class/species feature and spell grants for deterministic level thresholds; choice-based rewards remain manual, and existing fixed spell records are updated rather than duplicated as grant/free-cast thresholds increase.
- New characters now begin with **New Class** and **New Species** placeholders and receive no fixed traits automatically. After naming the character and choosing both values, **Grant Initial Traits** applies deterministic level-1 grants explicitly and duplicate-safely; automatic later level-up progression remains enabled.
- Changed new-character ability defaults from six 10s to the standard array 15/14/13/12/10/8 in STR/DEX/CON/INT/WIS/CHA order without modifying existing saves.
- Removed the remaining whole-character recognized-body-field validation gate so a readable character file stays best-effort loadable even when its body contains only future/unknown fields.
- Made spell School, Ritual and Source filters explicitly opt-in secondary filters: defaults remain Any, Ritual supports Any/Yes/No, and normal Allowed/All Classes eligibility continues to use only character classes and each class's spell level access.
- Restored the form-first Spellbook/Inventory add flow: **+ Add New** creates a blank record and opens its editor; OK on **Name** opens the catalog and Hold OK on **Name** keeps custom text entry.
- Removed the post-append sidecar reread from **+ Add New**: the committed blank item/spell is adopted directly into the editor page, so an unavailable, partially malformed or memory-constrained collection read cannot make Add appear to do nothing.
- Made item/spell page hydration best effort: complete records parsed before malformed trailing data or a later storage error remain usable instead of invalidating the entire page.
- Fixed allowed-spell selection for empty spellbooks and multiclass characters: the default list is the union of spells allowed by every character class at that class's own level, the optional class filter narrows it, and a chosen catalog spell is assigned to an eligible source class.
- Replaced whole-sidecar rewrites for simple spell/item appends with rollback-safe in-place appends that restore the original file length on write/sync failure, reducing temporary memory and SD write work.
- Hardened Inventory first-open defaults so a missing, header-only or otherwise record-empty item sidecar is treated as uninitialized and reseeded from class/species/background equipment plus the hidden d100 trinket; seeding waits for explicit class/species choices and Inventory still opens if seeding itself fails.
- Expanded the Bestiary full-screen monster-stat reader to wrap at 26 characters per line; its line/count buffers now hold the complete requested width instead of truncating the wrapper to 20 visible characters.
- Standardized full-width scrolling/menu row renderers across DNDolphins, Bestiary, Adventure, Journal and Initiative to display up to 26 characters; compact fixed layouts such as Adventure's sprite-adjacent story text remain width-specific.

## 3.2.17

- Added the bundled **Ghost Protocol** fictional authorized-security-audit campaign using existing Adventure checks, branching, rewards, flags, achievement and milestone support.
- Added bundled default custom monsters **Dolphin** and **Capybara**; Bestiary seeds them only when neither user custom file exists and never overwrites existing/partial custom data.
- Kept the five-FAP save structures, paging model and stack reservations unchanged.

## 3.2.16

- Split inventory/equipment/weapon behavior into `dndolphins_items.*` and spellcasting/resource behavior into `dndolphins_spells.*`, keeping shared character/dice mechanics in `dndolphins_rules.*`.
- Reconnected and regression-checked moved rule functions; made Inventory the only normal starting-inventory initializer and added the hidden d100 trinket.
- Removed Adventure's duplicate starting-equipment assets/initialization while preserving sidecar formats and bounded paging.

## 3.2.15

- Finalized declarative class/species/background starting equipment, direct live sidecars plus level/name `.swd` history snapshots, and streamed whole-collection spell/resource calculations.
- Fixed low-count eight-record page growth and Combat discovery from sidecars; renamed Bestiary `Difficulty Simulator` consistently to **Difficulty**.
- Reduced DNDolphins stack pressure by moving Verify Profile temporary state off stack and replaced Bestiary full-file lookup/browse caches with bounded sparse/recent caches and a smaller working page.

## 3.2.14

- Split owned spells/items into per-character sidecars with one escaped record per line and eight-record workflow paging; embedded spell/item fields stopped being read or written.
- Updated class/spell/item/combat/rest/Adventure workflows to stream or page the sidecars and separated core-character dirty tracking from sidecar-only changes.
- Preserved self-contained owned records while reducing resident collection memory and tightening companion-app stack reservations.

## 3.2.13

- Made character loading best-effort by recognized field name, removed whole-character consistency/heap vetoes and restored non-overwriting relocation of legacy character files.
- Enforced `.shd` as write-only history and tied shadow updates to actual changed-character saves.
- Hardened direct-launch active-character fallback, renamed source/entry-point ownership around the five FAPs, and standardized explicit absolute `.fap` handoff paths.

## 3.2.12

- Reworked the character picker and Journal into storage-backed bounded metadata caches; Journal bodies load only when opened and are no longer limited by a full-entry resident array.
- Removed the large resident Combat row block and formatted only visible rows on demand.
- Kept structured spell-combat mappings authoritative with guarded Notes dice fallback and removed dead helpers for strict firmware builds.

## 3.2.11

- Added same-stem write-only character shadows, shared Combat Normal/Advantage/Disadvantage attack mode and minimum-XP floors on level increases.
- Split DNDAdventure into its own FAP and made it sole owner of campaign state/progress while keeping one-way milestone output to Journal.
- Kept character files directly under the DNDolphins app-data root and preserved teardown-before-launch handoffs.

## 3.2.10

- Split Journal and Initiative into standalone FAPs with independent app-data namespaces and removed their resident state from DNDolphins.
- Moved Journal entries to timestamped per-character files, restored newest-first ordering without firmware `qsort`, and removed broad old-schema conversion code.
- Returned DNDolphins to an 8 KB stack after the source-based OOM cleanup.

## 3.2.9

- Reduced DNDolphins to an 8 KB stack through bounded paths/readers, lazy Adventure/Journal ownership and tighter shutdown/startup memory handling.
- Added automatic spell-slot/Pact initialization, direct slot editing and completed a large structured Combat Spell Attacks mapping pass with multi-part/healing/special dice handling.
- Kept Notes `XdY` fallback bounded while hardening malformed spell/dice input and reducing startup/transition heap fragmentation.

## 3.2.8

- Reduced dynamic character-array and spell-storage growth pressure, reduced catalog working pages and released workflow allocations when leaving those workflows.
- Removed full-character duplicate allocations from verification/migration paths.
- Expanded structured Spell Attacks mappings and added guarded `XdY`/`XDY` Notes fallback for otherwise unmapped spells.

## 3.2.7

- Moved Bestiary into the fixed Home menu and added Combat > Spell Attacks with slot/Pact/point/free/ritual resource handling and mapped spell rolls.
- Hardened low-memory startup/shutdown, publication rollback and manually editable numeric parsing; set the main app stack to 9 KB.
- Extended Bestiary initiative handoff metadata and fixed low-memory monster-cache/migration recovery paths.

## 3.2.6

- Added the dedicated Adventure skill-check result screen showing natural d20, modifier, total, DC and pass/fail until OK is pressed.
- Added explicit **Start Adventure**, expanded the scene-text layout and made Retry Save visible only after a save failure.

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
