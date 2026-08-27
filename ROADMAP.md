# Dungeons & Dolphins roadmap

This document contains planned work only. Implemented work belongs in `CHANGELOG.md`. Each release is ordered by play value and memory/SD impact.

## 3.3 — pack lifecycle

- Add transactional uninstall and export for installed campaign and monster packs with an explicit stable-ID conflict preview.
- Add an on-device manifest preview showing record count, compatibility range, source files, and destination before installation.
- Add installed-pack rename and notes fields while keeping every text file manually editable and fully offline.

## 3.4 — low-latency state

- Persist compact summary-ID indexes so favorites, recents, and saved encounters can reopen without rebuilding unchanged caches after app launch.
- Batch adjacent Bestiary state updates into one transactional write while flushing immediately on app switch or exit.
- Add a low-memory diagnostics page for current heap headroom, peak transient allocation, cache sizes, and SD transaction recovery.

## 3.5 — campaign state engine

- Add typed campaign variables and conditional choice visibility without embedding executable scripts.
- Add campaign objectives synchronized with journal milestones and inventory rewards.
- Add conflict-safe campaign progress import/export with manifest compatibility preview.

## 3.6 — accessibility and controls

- Add compact, standard, and large-text row layouts with per-screen previews.
- Add configurable long-press shortcuts and left/right behavior with a reset-to-default control map.
- Add a reduced-motion option that stops marquee and dice animation timers on static screens.

## 3.7 — encounter history

- Add a compact completed-encounter log with date, rounds, party state, and surviving opponents.
- Allow a completed encounter to be cloned into a new named encounter without retaining its old initiative state.
- Add streamed encounter-history export and pruning controls with no full-history RAM allocation.
