# Physical-device test matrix

These cases require a Flipper Zero and are not represented as completed by the compiler or host tests.

| Area | Cases | Status |
|---|---|---|
| Navigation | Every screen, Back behavior, long/short OK, long directional shortcuts | Pending hardware |
| Text | Maximum-length names, details, passive-stat labels, scrolling, truncation | Pending hardware |
| Storage | SD removal, write failure, interrupted replacement, backup recovery, import/export/archive | Pending hardware |
| Memory | Repeated catalog/adventure/monster open-close, largest catalogs, live free-heap/max-block diagnostics, profile scans | Pending hardware |
| Autosave | Rapid value changes, resource spending, dice flow, combat undo, profile switch | Pending hardware |
| Display | 10x10 launcher icon, dice frames, adventure sprites, all 128x64 row layouts | Pending hardware |

Host tests and RogueMaster FBT verification must pass before each device session. Record firmware commit, test date, and failures in this file when hardware results are available.
