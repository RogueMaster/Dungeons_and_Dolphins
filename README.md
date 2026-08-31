<h1 align="center"><a href='https://rogue-master.net'><img src="https://raw.githubusercontent.com/RogueMaster/flipperzero-firmware-wPlugins/420/.github/assets/rmlogo.png" width="40%"></a>
<br><a href='https://discord.gg/gF2bBUzAFe' target='_blank'><img src="https://raw.githubusercontent.com/RogueMaster/flipperzero-firmware-wPlugins/420/.github/assets/Discord.png" alt='Discord' title='Discord'></a>
&nbsp;<a href='https://github.com/RogueMaster/flipperzero-firmware-wPlugins/releases/latest' target='_blank'><img src="https://raw.githubusercontent.com/RogueMaster/flipperzero-firmware-wPlugins/420/.github/assets/Github.png"  alt='Firmware GitHub' title='Firmware GitHub'></a>
&nbsp;<a href='https://www.patreon.com/RogueMaster?filters[tag]=Latest%20Release' target='_blank'><img src="https://raw.githubusercontent.com/RogueMaster/flipperzero-firmware-wPlugins/420/.github/assets/Patreon.png"  alt='Latest PATREON Release' title='Latest PATREON Release'></a>
&nbsp;<a href='https://github.com/RogueMaster/awesome-flipperzero-withModules' target='_blank'><img src="https://raw.githubusercontent.com/RogueMaster/flipperzero-firmware-wPlugins/420/.github/assets/Resources.png"  alt='More Research / Assets' title='More Research / Assets'></a></h1>

# DNDolphins

Offline 5E-compatible tools for Flipper Zero, split into seven FAPs so the main character hub stays within loader RAM while collection-heavy workflows own their UI/catalog memory separately.

## Apps

| App | Purpose | Writable root |
|---|---|---|
| DNDolphins | Characters, progression, dice and character combat hub | `/ext/apps_data/dndolphins/` |
| DNDInventory | Per-character Inventory, Item editor/catalog and equipment state | `/ext/apps_data/dndolphins/` (shared character root) |
| DNDSpellbook | Per-character Spellbook, Spell editor/catalog and preparation state | `/ext/apps_data/dndolphins/` (shared character root) |
| DNDAdventure | Campaigns, checks, rewards, milestones and campaign packs | `/ext/apps_data/dndadventure/` |
| DNDJournal | Per-character notes and Adventure milestone history | `/ext/apps_data/dndjournal/` |
| DNDInitiative | Party roster, initiative, rounds, HP/AC and conditions | `/ext/apps_data/dndinitiative/` |
| DNDBestiary | Monsters, custom monsters, packs and encounters | `/ext/apps_data/dndbestiary/` |

Cross-app launches use explicit `/ext/apps/Games/*.fap` paths. The outgoing app tears down its views, callbacks, services, caches and heap state before Loader starts the next FAP. In each of the six companion FAPs, Short Back from the main screen returns to DNDolphins when that FAP is installed; if it is absent, the companion simply exits. Hold Back exits the companion without a parent handoff, so explicit Return/Open-DNDolphins rows are not needed on normal companion menus. Companion selection is driven only by `/ext/apps_data/dndolphins/custom_active_profile.txt` (`Active=<id>`); launch arguments do not override it and companions never discover or fall forward to another character. Adventure and Journal require that exact ID to have a canonical character profile. Inventory and Spellbook read that exact ID through the `dnd_storage.c` implementation they already use for character/collection I/O, then load that same character through the normal character loader. Initiative uses ID `0` only when the active-profile metadata itself is absent or unreadable, then validates that exact ID; an existing but stale `Active=<id>` does not fall back to 0. Bestiary likewise defaults its displayed/transfer character ID to `0` only when the metadata is absent or unreadable and otherwise keeps the persisted ID while remaining usable without a character profile. Adventure, Bestiary, Journal, Initiative, Inventory and Spellbook display the resolved character ID in brackets at the top-right of their main screen only. Inventory and Spellbook keep their character-owned sidecars in `/ext/apps_data/dndolphins/` so Combat, Adventure, Journal and progression always see one source of truth.

## Features

- **Level Choices:** explicit ASI/Feat choices are detected from class level, survive restarts through existing grant records, and never silently choose a feat for the player.
- **Adventure pack controls:** installed campaign packs can be enabled/disabled; installed campaign packs remain registered in the campaign pack list; Hold OK toggles a selected pack active/inactive, short OK does nothing on existing pack rows, and campaign files remain on storage.

### Characters

- Multiple independent character profiles stored as readable, manually editable text files.
- Best-effort named-field loading: unknown, reordered, missing or individually malformed fields do not invalidate otherwise usable character data.
- Automatic saves plus import, export, duplicate, rename, archive, delete, retry and write-only shadow history.
- Multiclass characters with class levels, subclasses, Hit Dice and class-linked feature handling.
- Ability scores, saving throws, skills, proficiency/expertise, passive scores and miscellaneous modifiers. New characters use the standard array; level-1 class/species/background traits are applied only when **Grant Initial Traits** is selected.
- HP, temporary HP, AC, speed, initiative, exhaustion, death saves, inspiration, XP and milestones.
- Species, background, alignment, languages, feats, tools, armor/weapon training, size, senses and proficiencies. Supported species progression is checked against total character level, including deterministic innate spells/features where metadata is available.

### Magic & spells

- Known, Prepared, Always Prepared, Ritual and free-cast spell states.
- Per-class casting ability/mode, prepared limits, spellbook size, Pact Magic, Mystic Arcanum and spell points.
- Shared multiclass slots, automatic empty-slot initialization, Eldritch Knight/Arcane Trickster third-caster handling with Wizard-list filtering, Spell Attack, Spell Save DC, slot spending and rest recovery.
- Spell filtering by class, level, ritual, school, source and prepared state. **Spell Class** defaults to **All Classes**, the union of currently eligible spells across every class on a multiclass character, while individual classes remain selectable. Hold Up from the Spellbook list opens the filter screen directly. `Eligibility: Allowed` is the default; the explicit `All Spells` override restores the older show-all picker without weakening normal eligibility filtering. The bundled 448-spell catalog fully populates these filter fields and filtering occurs during streaming before the bounded result page is built. Hold OK on a Spellbook row toggles Prepared and saves immediately; always-prepared spells remain fixed.
- Streamed spell catalog plus custom spell support; short or hold OK on **+ Add New** creates a blank spell and opens the full editor immediately. The successful `Spell added` notice is one-shot and clears on the next real input. Catalog selection remains available from the Name field inside that editor; catalog headers retain page `<>` hints.
- Combat Spell Attacks use structured spell mappings for supported attacks, saves, scaling, healing and multi-part effects; unmapped custom spells can use the first valid `XdY`/`XDY` expression in Notes as a bounded fallback.
- DNDSpellbook reserves its fixed main UI before variable character/collection loading, then opens directly to the active character Spellbook list with **+ Add New** selected; an absent sidecar is treated as an empty list until the first spell is saved. The first four spells remain visible together with `+ Add New`; the fifth is the first row that requires scrolling. Ordinary Up/Down stays cache-only until an eight-record page boundary; the normal list does not display a persistent `<>` hint. Explicit `P# <>` paging remains in the Name-field spell catalog. Spell rows show `A` (always prepared), `P` (prepared), `K` (known) or `-`, plus `F` when a free cast remains (for example `AF`). DNDSpellbook owns the full Spellbook list/editor/catalog UI. Character-owned spellbooks remain in the shared DNDolphins data root and use eight-record sidecar paging instead of keeping the whole collection resident. DNDolphins retains only the runtime access needed for Combat and progression. It does not hydrate a Spellbook page during startup or ordinary character navigation; Spell Combat builds a bounded logical index on entry and loads an eight-record page only when the combat UI requests a spell. Non-combat progression maintenance streams the sidecar without retaining a Spellbook page.

### Inventory, equipment & items

- Inventory quantities, weight, containers, equipped/attuned state, charges and ammunition groups. Hold OK on an Inventory row toggles Equipped and saves immediately; successful quick actions briefly mark the affected row with `[X]`.
- Weapon attack modifiers, damage dice, versatile damage, riders, proficiency, magic bonuses and weapon properties.
- Armor/shield values, calculated AC, carrying capacity and currency normalization.
- Copper, Silver, Electrum, Gold and Platinum tracking.
- Starting inventory is created only from **Hold Up → Inventory Tools → Grant Initial Inventory**. Short OK performs the normal one-shot grant while no grant marker exists and the sidecar contains no manual Item records. After a successful normal grant, Hold OK on the same tool is a deliberate **one-time regrant override**: existing Items are preserved, the starting class/species/background package and its currency are appended once more, and the override is then permanently consumed. Merely opening DNDInventory does not create or seed an absent Inventory sidecar; an existing item-only sidecar may be completed with missing Inventory metadata without granting equipment. Class/species/background defaults are streamed from assets. A random d100 trinket is used **only as fallback equipment if the normal starting-equipment seed/regrant produces no equipment/currency**. The Item Catalog supports Hold-OK filters for All, Weapons, Armor, Ammunition, Gear, Tools and Magic.
- **Hold Up** from the Inventory list opens the bounded Inventory Tools menu containing Currency, Inventory Resources and Grant Initial Inventory. Currency and Inventory Resources are owned by DNDInventory rather than occupying DNDolphins home-menu rows.
- Routine successful collection notices such as Saved, Item/Spell added, catalog saved, Equip/Prepare and grant/regrant feedback are one-shot and clear on the next real input; storage failures remain visible until replaced.
- Currency persistence is exclusive to `inventory_{id}.txt`; character-profile `Currency=` lines are ignored and character saves do not serialize currency. Non-Inventory Item appenders never create `Currency=`. If DNDInventory opens an existing item-only sidecar with no Currency record, DNDInventory initializes it to `Currency=0,0,0,0,0`.
- Resources, Weapon Attacks and Adventure do not silently seed starting equipment.
- DNDInventory reserves its fixed main UI before variable character/collection loading, then opens directly to the active character Inventory list with **+ Add New** selected; Hold Up from that list opens Inventory Tools. The first four items remain visible together with `+ Add New`; the fifth is the first row that requires scrolling. Ordinary Up/Down stays cache-only until an eight-record page boundary; the normal list does not display a persistent `<>` hint. Explicit `P# <>` paging remains in the Name-field item catalog. DNDInventory owns the full Inventory list/editor/catalog UI; Item Catalog headers retain page `<>`, and rows show category plus `*` for magic entries. Character-owned inventory remains in the shared DNDolphins data root, uses eight-record sidecar paging and streamed whole-collection calculations, and short or hold OK on **+ Add New** creates a blank item and opens the full editor immediately. A successful `Item added` notice is one-shot and clears on the next real input. Catalog selection remains available from the Name field inside that editor. DNDolphins retains only combat-time item access for attacks, ammunition and combat calculations. Weapon Combat creates a bounded weapon index on entry and loads an eight-record Item page only when a concrete weapon record is requested; the index/page are released on exit.

### Dice & combat

- Animated d4, d6, d8, d10, d12, d20 and d100 rolls.
- Advantage, Disadvantage, modifiers, multiple dice and Guidance mode.
- Individual results, dice sum, modifier and final total display.
- Weapon Attacks and Spell Attacks use the character's owned records and current resources. Weapon attacks retain proficiency/magic/ability/exhaustion modifiers, advantage/disadvantage, ammunition use, versatile damage, extra dice and critical doubling. Spell attacks retain per-class casting ability, attack/save modifiers, cantrip scaling, upcasting and mapped/fallback damage. For Wizard-sourced combat spells, cantrips remain available normally; level-1+ spells appear only when Prepared/Always Prepared or when a Free Cast remains. An unprepared Wizard spell exposed only by a Free Cast offers that Free Cast only rather than normal slot/Pact/spell-point choices. Other classes retain their existing Known/Prepared behavior.
- HP, temporary HP, rests, Hit Dice, conditions, concentration, reactions, exhaustion, resistances, immunities, vulnerabilities, senses and movement support.

### Initiative

- Standalone per-character Party Roster with name, initiative modifier, AC, current HP and maximum HP.
- Initiative setup provides Roll for All, individual automatic rolls, temporary members, short/repeat left/right initiative adjustment, long left/right manual participant reordering, and hold OK full participant editing.
- Initiative automatically refreshes the active character's current name, HP, AC and initiative modifier from the canonical primary character profile when opened; Inventory/Spellbook sidecars cannot be mistaken for the profile, character ID 0 remains valid, and other roster members keep their independently assigned modifiers.
- Initiative Roll mode can be set to Normal, Advantage or Disadvantage. Roll for All and individual automatic rolls use the selected mode; the full editor accepts direct numeric initiative total, modifier, AC, current HP and maximum HP values.
- Round/current-turn tracking; short Back during active combat moves to the previous turn. Hold Back follows the companion-wide force-exit behavior and exits Initiative to firmware. The menu can explicitly end the current combat.
- During active combat, AC is shown beside HP; left/right changes HP, hold Up raises AC, hold Down opens quick condition editing, hold left/right manually reorders participants, and hold OK opens the full participant editor.
- HP/AC edits for the tracked main character synchronize back to the canonical character profile. Turn- and Encounter-recharge features are restored automatically at their corresponding Initiative cadence.
- Participant editing, temporary participants, negative HP, conditions and removal.
- Bestiary Add to Initiative can append selected monsters or encounter members to the active character roster before opening Initiative.

### Journal

- Standalone per-character timestamped text entries stored outside the character save.
- Newest-first storage-backed listing with bounded metadata caching and body-on-open loading.
- Adventure milestone history is written directly to Journal. Milestone entries can select a character class and apply that milestone level exactly once.
- Item-category entries can create an inventory item from the Journal title/body without embedding Journal data in the character save.
- Journal can return to DNDolphins or use **Continue active Adventure** from a milestone entry. That handoff resumes the character's persisted active campaign/scene directly when available; Journal does not create or duplicate Adventure progress.

### Adventure

- Declarative campaign manifests/scenes with branching choices, skill checks, flags, achievements, item rewards, milestones and checkpoints.
- Per-character campaign progress owned by DNDAdventure rather than embedded in the character file.
- Bundled **Reef Wardens** aquatic adventure.
- Bundled **Ghost Protocol**, a fictional authorized security-audit adventure using a Flipper Zero as an in-world inspection tool; it contains game checks and branching outcomes rather than real exploit instructions.
- Adventure item rewards use shared item storage behavior but never initialize starting equipment.
- Installable campaign-pack support with a concise pre-install details/validation screen. The preview checks manifest/index shape, compatibility, content presence, declared entry-scene presence and ID conflicts before Hold OK can install.
- Campaign discovery uses bounded sparse index hints and streaming lookup rather than a heap offset for every campaign row.

### Bestiary

- Large bundled monster catalog with streamed browsing, filtering, detail views and encounter generation.
- Search/filter support for monster metadata such as name, challenge, type, source, environment and encounter role where present.
- Favorites, recents, saved filters, diagnostics and named saved encounters.
- Saved encounter resume, rename, duplicate, archive, delete and Add to Initiative workflows.
- Custom monster create/edit/delete without rewriting packaged monster assets.
- Installable monster packs remain separate from packaged and direct-custom records.
- **Monster pack controls:** existing installed packs remain registered and preserve their files; Hold OK toggles Active/Inactive, while short OK does nothing on existing pack rows. Short OK remains the install action on the separate inbox row.
- If no custom monster files exist, Bestiary seeds a small bundled custom pack containing **Dolphin** and **Capybara**. Existing or partial user custom files always win and are never overwritten by the seed.
- Full stat-block rows can be opened in the scrolling full-screen reader.

### Storage, memory & resilience

- Character core saves, spellbooks, inventories, Journal entries, Adventure progress and Bestiary state are owned by the FAP that uses them.
- Character shadows are write-only history and are never used as live recovery input.
- Packaged catalogs/campaigns/monsters remain in app assets; mutable user data remains in app data.
- Large lists are streamed, paged or bounded rather than loaded wholesale.
- Cross-FAP handoffs free outgoing runtime state before launching the next app.
- Text data remains manually editable; no checksum match is required for ordinary character/custom-pack loading.

## Controls

- **OK** — select, open, confirm or roll.
- **Hold OK** — alternate action such as custom text, numeric entry or participant editing where supported.
- **Back** — previous screen; on a companion app's main screen, return to DNDolphins when installed. During active Initiative combat, short Back retains its combat-specific previous-turn behavior.
- **Hold Back** — exit the current companion app back to firmware without launching DNDolphins. DNDolphins itself retains its screen-specific controls.
- **Left / Right** — change pages or values where supported.
- **Up / Down** — move through rows and lists.

## Memory model

- DNDolphins: 6 KB stack
- DNDInventory: 4 KB
- DNDSpellbook: 4 KB
- DNDAdventure: 4 KB
- DNDJournal: 4 KB
- DNDInitiative: 3 KB
- DNDBestiary: 6 KB

See `MEMORY_AUDIT.md` for current source-derived stack-use estimates and remaining device stress targets. Persistent Feature state is stored in `feats_{charID}.txt` and loaded in pages of at most eight only when needed. Applied progression history is streamed from `appliedgrants_{charID}.txt` during progression checks rather than kept resident.

## Documentation

- `CHANGELOG.md` — released history from the retained release line onward, summarized per revision.
- `ROADMAP.md` — future work only.
- `SAVE_SCHEMA.md` — persistence ownership and paths.
- `CAMPAIGN_PACK_SCHEMA.md` / `MONSTER_PACK_SCHEMA.md` — declarative pack formats.
- `MEMORY_AUDIT.md` — stack reservations, estimated source-visible usage and memory ownership.
- `DEVICE_TEST_MATRIX.md` — device validation checklist.

### Initiative roll behavior

Initiative refreshes the active character from the current profile when launched. The player modifier includes Dexterity, Initiative Misc, exhaustion, and recognized initiative-related features currently mapped by the app. Each participant can independently use Normal, Advantage, or Disadvantage for generated rolls; a directly edited initiative total remains exactly the entered total until the participant is rolled again.

### Initial traits and level progression

`Grant Initial Traits` stages the initial species/background/class/subclass grants into the Grant Review screen before applying them. Deterministic class and supported species progression updates numeric resources/features automatically. Species progression uses total character level, so multiclassing does not delay species-level unlocks. Deterministic species spells such as High Elf innate progression are granted automatically; ordinary class spell choices remain player-selected and the app prompts when class spell/cantrip allowances expand.
