# Community metadata pack

This directory is a separately maintainable metadata index for Dungeons & Dolphins. It contains names and structured annotations, not descriptive rules text.

Each non-comment line in `custom_options.txt` uses:

`stable_id|source|option_type|option_name|prerequisites|level_gained|class_associations|grant_value`

Supported grant values include `origin_feat=`, `tool=`, `armor=`, `weapon=`, `size=`, `senses=`, `feature=`, and `spell=`. Leave the final field empty for a catalog-only annotation.

Keep IDs stable once published. Run `tools/generate_metadata_pack.sh` after changing the packaged catalogs, then validate the generated file with the host release tests before copying it to `metadata/custom_options.txt`.

Users may populate additional records from campaign books they own. Do not distribute copied descriptive text through this pack.
