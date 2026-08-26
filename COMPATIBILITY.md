# Compatibility matrix

| Environment | Status | Notes |
|---|---|---|
| Flipper Zero | Target | 128x64 screen and five-way buttons; physical verification remains pending |
| RogueMaster branch 420 | Compiler verified | Both explicit FAP targets are built before release packaging |
| Official firmware | Not verified | May require manifest/API adjustments |
| Momentum firmware | Not verified | May require manifest/API adjustments |
| SD card present | Required for persistence | Bundled assets and character files use each application's asset namespace |
| No SD card / removed SD | Unsupported for saves | App reports save/load failure; device behavior remains in the hardware matrix |
| Schema 1 | Supported | Stable from 0.9 through 1.0 |
| Pre-freeze 0.8 text saves | Migration supported | Verified legacy checksum, sanitize, atomic rewrite, retained backup |
| Earlier pre-release saves | Unsupported | Start fresh or use manual text reconstruction |

The source-only release does not bundle a compiled FAP. Build both targets using the command in `README.md`.
