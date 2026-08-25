# Release checksums

## RogueMaster compiler artifact

- Version: `1.0`
- Firmware branch: `420`
- Firmware commit: `d3ae1f86bd961852af6969e887a8dd3492a19321`
- Firmware API: `88.4`
- FAP size: `255,664 bytes`
- FAP SHA-256: `ab6185a006b1475f4ac72081e0a722d895300ff59da3540e5c398900a614d763`
- ARM ELF text: `72,526 bytes`

The compiled FAP is intentionally excluded from the source release. Build it with `tools/verify_release.sh /path/to/rogue-master-firmware`. The downloadable source ZIP checksum is reported beside the release artifact because embedding an archive's own checksum inside itself is self-referential.
