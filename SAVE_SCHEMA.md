# Save and storage schema

## Ownership

Each FAP writes only its own app-data root:

- DNDolphins: characters, active profile, spell/item sidecars, exports/archive.
- DNDAdventure: active campaign, progress, direct custom campaigns and installed campaign registry/index.
- DNDJournal: per-character journal entries.
- DNDInitiative: party/initiative state.
- DNDBestiary: favorites, recents, filters, encounters, custom monsters and installed monster packs.

## DNDolphins

Primary character files are named `ch_{id}_{safeName}_{level}.txt`. Recognized fields load independently and unknown fields are ignored; a readable file is not rejected solely because no body field is recognized by the current build. Character `.shd` files are write-only history and are never used as live input.

Level progression adds no new persisted schema. Derived resource maxima are synchronized into existing fields, and deterministic fixed feature/spell grants use the existing feature records and spell sidecar fields. Progression metadata is read only during a level-gain operation and is never stored as a hash/signature/checksum. Choice-based level-up outcomes remain explicit user selections rather than new implicit save state.

Owned spells and items live in character-specific sidecars. Only one aligned page of up to eight records is resident at a time. Collection-wide operations stream the sidecar rather than allocating all records. Current-level `.swd` snapshots are write history; live `.txt` sidecars remain authoritative.

Starting inventory is not a character-creation side effect. On first Inventory open, if the live item sidecar is missing or contains no valid item records, the Items module requests class/species/background rows plus one hidden d100 trinket and applies any starting currency once.

## Adventure

Campaign progress is DNDAdventure-owned and stores campaign ID, scene/checkpoint, quest flags and achievements in its own text files. Milestones are journaled after the corresponding guard is saved so replaying a guarded reward does not intentionally duplicate it.

## Bestiary custom monsters

Custom monsters use:

- `/ext/apps_data/dndbestiary/monsters/custom_index.txt`
- `/ext/apps_data/dndbestiary/monsters/custom_statblocks.txt`

If neither exists, Bestiary may seed both from bundled default-custom assets. If either user file already exists, the seed does nothing. Existing recovery/transaction logic remains authoritative for user custom edits.

No campaign, Bestiary, Journal or Initiative state is serialized into the core character file.
