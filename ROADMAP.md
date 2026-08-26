# Dungeons & Dolphins roadmap

This document contains planned work only. Implemented work belongs in `CHANGELOG.md`. A roadmap item moves to the changelog only after its code, tests, documentation, and RogueMaster build verification are complete.

## 2.7 — encounter workspace

- Add named, reusable encounter files with duplicate, rename, archive, and resume controls.
- Add configurable role targets and composition warnings for leader support, artillery protection, and excessive minion density.
- Add a non-destructive difficulty simulator for changing party size/level and creature quantities before committing an encounter.

## 2.8 — compendium operations

- Add on-device install, enable/disable, and uninstall controls for user monster packs without touching bundled assets.
- Add custom-monster export and pack merge with stable-ID conflict review.
- Add monster favorites, recent records, and saved filter presets using disk-backed indexes.

## 2.9 — campaign state engine

- Add typed campaign variables and conditional choice visibility without embedding executable scripts.
- Add campaign objectives that synchronize selected scene events with journal milestones and inventory rewards.
- Add campaign import/export with manifest preview, compatibility checks, and conflict-safe progress mapping.

## 3.0 — accessibility and controls

- Add compact, standard, and large-text row layouts with per-screen truncation previews.
- Add configurable long-press shortcuts and left/right behavior with a reset-to-default control map.
- Add language-pack completeness diagnostics, fallback counts, and an on-device preview before activation.

## 3.1 — stable pack platform

- Define save schema 2 with forward migration, rollback snapshots, and host-side golden-file compatibility tests.
- Add checksummed catalog, campaign, language, and monster pack manifests with transactional installation.
- Require a release qualification gate combining host tests, RogueMaster validation, stress-test evidence, and a signed physical-device matrix.
