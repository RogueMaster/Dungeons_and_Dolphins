# Dungeons & Dolphins roadmap

This document contains planned work only. Implemented work belongs in `CHANGELOG.md`. A roadmap item moves to the changelog only after its code, tests, documentation, and RogueMaster build verification are complete.

## 2.1 — monster discovery and encounter roles

- Add source and environment filters to the ordinary monster browser, combinable with name, challenge, and creature-type filters.
- Add optional Leader, Controller, Skirmisher, Artillery, Brute, and Minion role metadata with role-aware encounter weighting.
- Add per-record diagnostic navigation that identifies the exact monster ID, file, and invalid or missing field.

## 2.2 — custom monster lifecycle

- Allow existing user-created monsters to be opened and edited on-device without changing their stable IDs.
- Allow user-created monsters to be deleted on-device while protecting bundled records from modification.
- Add atomic index rewrite, orphan-block recovery, and interrupted-edit rollback for user monster packs.

## 2.3 — campaign pack manager

- Add on-device campaign selection with separate per-character campaign progress and checkpoints.
- Add versioned campaign manifests with compatibility, missing-scene, duplicate-ID, and broken-link diagnostics.
- Add a documented third-party campaign-pack format and an SD-card starter template.

## 2.4 — complete structured editors

- Add full on-device editing for every attack-template field, including save actions, Mastery, damage riders, and recharge behavior.
- Add a structured-grant editor for prerequisites, class association, gained level, source, and grant payloads.
- Add optional runtime language packs for navigation and field labels with measured heap limits and English fallback.

## 2.5 — device resilience and validation

- Add an on-device stress test for repeated catalog, campaign, monster, profile, and encounter allocation/release cycles.
- Add graceful SD-card removal handling, read-only fallback, retry controls, and clear unsaved-state warnings.
- Complete and publish the physical-device test matrix for controls, display truncation, long sessions, power interruption, and low-memory behavior.
