# Monster pack schema 1

Monster pack schema 1 is stable as of application version 2.0. Compatible releases accept an `index.txt` beginning with `# MonsterPack=1`, followed by pipe-delimited records:

`id|name|challenge_eighths|xp|armor_class|hit_points|type|environment|source|role`

Each ID maps to `statblocks/{id}.txt`. Required keys are `SizeAlignment`, `Speed`, `Abilities`, `Senses`, `Languages`, and `Actions`. Optional keys are `Skills`, `Defenses`, `Traits`, and `Extra`. Values are single-line UTF-8 text and should not exceed 191 bytes. `Abilities` contains six comma-separated integers in STR, DEX, CON, INT, WIS, CHA order.

IDs must remain stable and unique. New optional fields may be added without changing the schema. Any incompatible index or required-field change needs a new pack version and an explicit importer.

`source` is a short display/filter label. `role` is optional encounter metadata and accepts `Leader`, `Controller`, `Skirmisher`, `Artillery`, `Brute`, `Minion`, or `Any`. Older schema-1 records with eight fields remain valid and receive safe defaults.

On-device custom creation writes a complete schema-1 block to a temporary file, renames it into place, and appends the index record only after the block is durable.

Packaged records are installed under `/ext/apps_assets/dolphin_bestiary/monsters/`. User records use `custom_index.txt`, `statblocks/custom_{id}.txt`, and other `custom_`-prefixed transaction files in that namespace.
