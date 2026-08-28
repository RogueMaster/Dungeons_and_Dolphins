<h1 align="center"><a href='https://rogue-master.net'><img src="https://raw.githubusercontent.com/RogueMaster/flipperzero-firmware-wPlugins/420/.github/assets/rmlogo.png" width="40%"></a>
<br><a href='https://discord.gg/gF2bBUzAFe' target='_blank'><img src="https://raw.githubusercontent.com/RogueMaster/flipperzero-firmware-wPlugins/420/.github/assets/Discord.png" alt='Discord' title='Discord'></a>
&nbsp;<a href='https://github.com/RogueMaster/flipperzero-firmware-wPlugins/releases/latest' target='_blank'><img src="https://raw.githubusercontent.com/RogueMaster/flipperzero-firmware-wPlugins/420/.github/assets/Github.png"  alt='Firmware GitHub' title='Firmware GitHub'></a>
&nbsp;<a href='https://www.patreon.com/RogueMaster?filters[tag]=Latest%20Release' target='_blank'><img src="https://raw.githubusercontent.com/RogueMaster/flipperzero-firmware-wPlugins/420/.github/assets/Patreon.png"  alt='Latest PATREON Release' title='Latest PATREON Release'></a>
&nbsp;<a href='https://github.com/RogueMaster/awesome-flipperzero-withModules' target='_blank'><img src="https://raw.githubusercontent.com/RogueMaster/flipperzero-firmware-wPlugins/420/.github/assets/Resources.png"  alt='More Research / Assets' title='More Research / Assets'></a></h1>

# DNDolphins

Offline 5E-compatible tools for Flipper Zero, split into five FAPs so each workflow owns its data and runtime memory.

## Apps

| App | Purpose | Writable root |
|---|---|---|
| DNDolphins | Characters, spells, inventory, dice and character combat | `/ext/apps_data/dndolphins/` |
| DNDAdventure | Campaigns, checks, rewards, milestones and campaign packs | `/ext/apps_data/dndadventure/` |
| DNDJournal | Per-character notes and Adventure milestone history | `/ext/apps_data/dndjournal/` |
| DNDInitiative | Party roster, initiative, rounds, HP/AC and conditions | `/ext/apps_data/dndinitiative/` |
| DNDBestiary | Monsters, custom monsters, packs and encounters | `/ext/apps_data/dndbestiary/` |

Cross-app launches use explicit `/ext/apps/Games/*.fap` paths. The outgoing app tears down its views, callbacks, services, caches and heap state before Loader starts the next FAP. Journal, Adventure and Initiative use the active character when launched directly; Bestiary remains usable without a character.

## Features

### Characters

- Multiple independent character profiles stored as readable, manually editable text files.
- Best-effort named-field loading: unknown, reordered, missing or individually malformed fields do not invalidate otherwise usable character data.
- Automatic saves plus import, export, duplicate, rename, archive, delete, retry and write-only shadow history.
- Multiclass characters with class levels, subclasses, Hit Dice and class-linked feature handling.
- New characters start from the standard array **15, 14, 13, 12, 10, 8** in STR/DEX/CON/INT/WIS/CHA order; every score remains editable and existing saves are unchanged.
- Deterministic level progression synchronizes proficiency, per-class Hit Dice, caster limits/resources and fixed class/species grants without silently choosing player-choice rewards. New characters start with unconfirmed Class/Species placeholders; after naming and choosing both, **Grant Initial Traits** explicitly applies duplicate-safe level-1 grants. Later scans run only when a level is gained, and progression metadata is not retained in memory.
- Ability scores, saving throws, skills, proficiency/expertise, passive scores and miscellaneous modifiers.
- HP, temporary HP, AC, speed, initiative, exhaustion, death saves, inspiration, XP and milestones.
- Species, background, alignment, languages, feats, tools, armor/weapon training, size, senses and proficiencies.

### Magic & spells

- Known, Prepared, Always Prepared, Ritual and free-cast spell states.
- Per-class casting ability/mode, prepared limits, spellbook size, Pact Magic, Mystic Arcanum and spell points.
- Shared multiclass slots, level-synchronized slot maxima, Pact Magic, Sorcery Point maxima, class cantrip/prepared limits, Wizard spellbook minimums, Spell Attack, Spell Save DC, slot spending and rest recovery.
- Fixed level-gated class/species spells can be granted through bundled progression metadata; the explicit level-1 action and later level-up scans process eight lines at a time, duplicate-safe grants preserve existing spell records, and free-cast uses reset on Long Rest.
- Spell filtering by class and level for normal Allowed eligibility, with optional School, Ritual, Source and prepared-state filters that default to Any and never narrow the default picker unless selected.
- Streamed spell catalog plus custom spell support; **+ Add New** opens a blank spell editor, OK on **Name** selects from spells allowed by all of the character's classes at their individual levels, and Hold OK on **Name** enters a custom spell name.
- Combat Spell Attacks use structured spell mappings for supported attacks, saves, scaling, healing and multi-part effects; unmapped custom spells can use the first valid `XdY`/`XDY` expression in Notes as a bounded fallback.
- Character-owned spellbooks use eight-record sidecar paging instead of keeping the whole collection resident.

### Inventory, equipment & items

- Inventory quantities, weight, containers, equipped/attuned state, charges and ammunition groups.
- Weapon attack modifiers, damage dice, versatile damage, riders, proficiency, magic bonuses and weapon properties.
- Armor/shield values, calculated AC, carrying capacity and currency normalization.
- Copper, Silver, Electrum, Gold and Platinum tracking.
- Starting inventory is created only when Inventory is first opened after Class and Species are chosen and the live inventory has no valid item records. Class/species/background defaults are streamed from assets and one hidden d100 trinket is added.
- Resources, Weapon Attacks and Adventure do not silently seed starting equipment.
- Character-owned inventory uses eight-record sidecar paging and streamed whole-collection calculations; **+ Add New** opens a blank item editor, OK on **Name** selects from the item catalog, and Hold OK on **Name** enters a custom item name.

### Dice & combat

- Animated d4, d6, d8, d10, d12, d20 and d100 rolls.
- Advantage, Disadvantage, modifiers, multiple dice and Guidance mode.
- Individual results, dice sum, modifier and final total display.
- Weapon Attacks and Spell Attacks use the character's owned records and current resources.
- HP, temporary HP, rests, Hit Dice, conditions, concentration, reactions, exhaustion, resistances, immunities, vulnerabilities, senses and movement support.

### Initiative

- Standalone per-character Party Roster with name, initiative modifier, AC, current HP and maximum HP.
- Roll for All, individual automatic rolls and hold-OK manual d20 entry.
- Round/current-turn tracking; short Back moves to the previous turn and hold Back returns to the Initiative screen.
- Participant editing, temporary participants, negative HP, conditions and removal.
- Bestiary Add to Initiative can append selected monsters or encounter members to the active character roster before opening Initiative.

### Journal

- Standalone per-character timestamped text entries stored outside the character save.
- Newest-first storage-backed listing with bounded metadata caching and body-on-open loading.
- Adventure milestone history is written directly to Journal.
- Journal can return to DNDolphins or launch Adventure to continue after a milestone; Journal does not create Adventure progress.

### Adventure

- Declarative campaign manifests/scenes with branching choices, skill checks, flags, achievements, item rewards, milestones and checkpoints.
- Per-character campaign progress owned by DNDAdventure rather than embedded in the character file.
- Bundled **Reef Wardens** aquatic adventure.
- Bundled **Ghost Protocol**, a fictional authorized security-audit adventure using a Flipper Zero as an in-world inspection tool; it contains game checks and branching outcomes rather than real exploit instructions.
- Adventure item rewards use shared item storage behavior but never initialize starting equipment.
- Installable campaign-pack support with bounded storage-backed loading.

### Bestiary

- Large bundled monster catalog with streamed browsing, filtering, detail views and encounter generation.
- Search/filter support for monster metadata such as name, challenge, type, source, environment and encounter role where present.
- Favorites, recents, saved filters, diagnostics and named saved encounters.
- Saved encounter resume, rename, duplicate, archive, delete and Add to Initiative workflows.
- Custom monster create/edit/delete without rewriting packaged monster assets.
- Installable monster packs remain separate from packaged and direct-custom records.
- If no custom monster files exist, Bestiary seeds a small bundled custom pack containing **Dolphin** and **Capybara**. Existing or partial user custom files always win and are never overwritten by the seed.
- Full stat-block rows can be opened in the scrolling full-screen reader, wrapped at 26 characters per line.

### Storage, memory & resilience

- Character core saves, spellbooks, inventories, Journal entries, Adventure progress and Bestiary state are owned by the FAP that uses them.
- Character shadows are write-only history and are never used as live recovery input.
- Packaged catalogs/campaigns/monsters remain in app assets; mutable user data remains in app data.
- Large lists are streamed, paged or bounded rather than loaded wholesale.
- Cross-FAP handoffs free outgoing runtime state before launching the next app.
- Full-width scrolling/menu rows use a consistent 26-character visible window across all five FAPs; specialized compact layouts retain their own bounded widths.
- Text data remains manually editable; no checksum match is required for ordinary character/custom-pack loading.

## Controls

- **OK** — select, open, confirm or roll.
- **Hold OK** — alternate action such as custom text, numeric entry or participant editing where supported.
- **Back** — previous screen; during active combat, previous turn.
- **Hold Back** — return toward the parent/main workflow; during active combat, return to Initiative.
- **Left / Right** — change pages or values where supported.
- **Up / Down** — move through rows and lists.

## Memory model

- DNDolphins: 6 KB stack
- DNDAdventure: 4 KB
- DNDJournal: 4 KB
- DNDInitiative: 4 KB
- DNDBestiary: 6 KB

See `MEMORY_AUDIT.md` for current source-derived stack-use estimates and remaining device stress targets.

## Documentation

- `CHANGELOG.md` — released history from the retained release line onward, summarized per revision.
- `ROADMAP.md` — future work only.
- `SAVE_SCHEMA.md` — persistence ownership and paths.
- `CAMPAIGN_PACK_SCHEMA.md` / `MONSTER_PACK_SCHEMA.md` — declarative pack formats.
- `MEMORY_AUDIT.md` — stack reservations, estimated source-visible usage and memory ownership.
- `DEVICE_TEST_MATRIX.md` — device validation checklist.
