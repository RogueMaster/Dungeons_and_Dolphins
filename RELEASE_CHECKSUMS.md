# Release checksums

## RogueMaster compiler artifacts

- Version: `2.7`
- Firmware branch: `420`
- Firmware commit: `38c885032f436e839423e84609d79d352a54b083`
- Firmware API: `88.4`
- Dungeons & Dolphins FAP: `289,128 bytes`, SHA-256 `3496844deb27ead1e547c611bd8d4ed5947a28820f15ce5688ae5020e3f2cd45`
- Dolphin Bestiary FAP: `231,364 bytes`, SHA-256 `9f134d373aeb82b9365be4f4822c66a99c814aa99c9625daab1c81ea501b779c`

Compiled FAPs are intentionally excluded from the source release. Build both with `./fbt fap_dungeons_and_dolphins fap_dolphin_bestiary`. The downloadable source ZIP checksum is reported beside the release artifact because embedding an archive's own checksum inside itself is self-referential.
