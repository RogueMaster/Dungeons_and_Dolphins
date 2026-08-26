# Community monster pack template

Copy the custom monster files beneath `/ext/apps_assets/dolphin_bestiary/monsters/` on the SD card. Use `custom_index.txt` for the index and `statblocks/custom_{id}.txt` for blocks.

Add one index row per creature using:

`id|name|challenge rating in eighths|XP|AC|HP|type|environment|source|role`

IDs must be unique and filename-safe. A matching `statblocks/custom_{id}.txt` file can contain `SizeAlignment`, `Speed`, `Abilities`, `Skills`, `Defenses`, `Senses`, `Languages`, `Traits`, `Actions`, and `Extra` key/value lines. Keep each value on one line and within 191 characters.

Use only material you have permission to redistribute. The on-device Pack Diagnostics screen reports missing blocks and duplicate IDs.
