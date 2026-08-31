# Catalog policy

Bundled catalogs are read-only app assets; user-owned/custom data lives in app data.

- Character option catalogs provide compact names/metadata suitable for offline selection.
- Owned spells/items are copied into character sidecars and then treated as character state.
- Bestiary bundled monsters, user custom monsters and installed monster packs remain separate sources.
- Default custom Dolphin/Capybara assets are seed material only. They are copied only for a fresh custom-monster state and are never merged over user data.
- Campaigns remain declarative scene/choice files. Ghost Protocol is fictional defensive-audit content and intentionally avoids real-world exploit instructions.
- Text pack formats stay editable; do not add checksum rejection.
- Do not duplicate the same owned catalog in multiple FAP asset trees unless runtime ownership requires it.
## Spell catalog metadata

Bundled spell rows use `Spell|Level|Class, Class|School|Ritual|Source`. School, Ritual and Source are optional when parsing user/custom catalogs for compatibility, but bundled rows should populate all six fields so Add Spell filtering remains complete. Level/Class govern eligibility; School/Ritual/Source are copied into the owned spell and are also filter keys.

Spell filtering is performed while streaming the source catalog, before the bounded catalog page is selected. This keeps filtered pages dense without loading the whole spell catalog into memory.

