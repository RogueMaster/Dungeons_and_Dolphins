# Community monster pack template

Copy `monsters/` beneath `/ext/apps_data/dungeons_and_dolphins/` on the SD card.

Add one index row per creature using:

`id|name|challenge rating in eighths|XP|AC|HP|type|environment`

IDs must be unique and filename-safe. A matching `statblocks/{id}.txt` file can contain `SizeAlignment`, `Speed`, `Abilities`, `Skills`, `Defenses`, `Senses`, `Languages`, `Traits`, `Actions`, and `Extra` key/value lines. Keep each value on one line and within 191 characters.

Use only material you have permission to redistribute. The on-device Pack Diagnostics screen reports missing blocks and duplicate IDs.
