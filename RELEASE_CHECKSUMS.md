# Release checksums

## RogueMaster compiler artifacts

- Version: `2.6`
- Firmware branch: `420`
- Firmware commit: `dd196a93089970d3bd20b18492d6c70c9a402f60`
- Firmware API: `88.4`
- Dungeons & Dolphins FAP: `274,944 bytes`, SHA-256 `8c0440ef18fc764aa6b2ceec485d5f04e830f2976be4e069eaa8c73a65579ced`
- Dolphin Bestiary FAP: `238,884 bytes`, SHA-256 `e4fa21893d740cbdfb0d1efc3f75baba850e9db7aac01558db55fc495f856345`

Compiled FAPs are intentionally excluded from the source release. Build both with `tools/verify_release.sh /path/to/rogue-master-firmware`. The downloadable source ZIP checksum is reported beside the release artifact because embedding an archive's own checksum inside itself is self-referential.
