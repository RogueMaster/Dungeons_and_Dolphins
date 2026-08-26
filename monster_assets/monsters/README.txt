MONSTER PACK FORMAT

The bundled index contains redistributable reference records. Each index row is:
id|name|challenge rating in eighths|XP|AC|HP|type|environment|source|role

Examples: CR 1/4 is 2, CR 1/2 is 4, CR 1 is 8, and CR 10 is 80.
Place optional user records in `/ext/apps_assets/dolphin_bestiary/monsters/custom_index.txt`
and matching key/value blocks in `statblocks/custom_{id}.txt`.

The app scans indexes on demand, pages 50 summaries at a time, and opens only one stat block at a time.
