# Feature checklist

## DNDolphins

- [x] Multiple tolerant named-field character profiles.
- [x] Active profile fallback and legacy character-file relocation.
- [x] Write-only character shadows.
- [x] Eight-record spellbook and inventory paging.
- [x] Inventory-only first-time starting equipment plus hidden d100 trinket.
- [x] Form-first Spellbook/Inventory add editors with Name-field catalog selection.
- [x] Multiclass allowed-spell union with per-class level gating and optional class narrowing.
- [x] Standard-array defaults for new characters: 15/14/13/12/10/8 in STR/DEX/CON/INT/WIS/CHA order.
- [x] Explicit initial-trait/automatic later-level progression for proficiency-derived state, Hit Dice, shared/Pact slots, Sorcery Points, cantrip/prepared limits, Wizard spellbook minimums and duplicate-safe fixed class/species grants.
- [x] Duplicate-safe fixed class/species feature and spell grants from bundled progression metadata read only on level-up in bounded eight-line pages, with no retained progression hash/signature/catalog.
- [x] Choice-based level rewards remain manual by design; progression never silently selects subclasses, ASI/feats, learned spells, styles, invocations or similar choices.
- [x] Readable character saves remain best-effort loadable even when all body fields are future/unknown to this build.
- [x] Item-owned weapon/armor/currency/carrying rules.
- [x] Spell-owned casting ability, DC/attack, slots/Pact/points and spell-level rules.
- [x] Dice, skills/saves and Combat spell/weapon attacks.

## Companion apps

- [x] DNDAdventure declarative campaigns, checks, rewards, flags/achievements and Journal milestones.
- [x] Bundled Reef Wardens and Ghost Protocol campaigns.
- [x] DNDJournal standalone per-character entries, newest first.
- [x] DNDInitiative standalone roster/combat state and Bestiary handoff.
- [x] DNDBestiary bundled catalog, filters, encounters, custom monsters and installable packs.
- [x] Default custom Dolphin/Capybara seed only when no user custom pack exists.

## Runtime

- [x] Explicit full-path FAP launches.
- [x] Outgoing-app teardown before handoff.
- [x] Stack reservations: 6 KB / 4 KB / 4 KB / 4 KB / 6 KB.
- [x] No checksum requirement for editable text packs.
