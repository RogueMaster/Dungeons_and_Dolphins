# Request checklist

| Requested capability | Version 1.0 status |
|---|---|
| Flipper FAP d20 icon | `icon.png`, 10x10 1-bit, assigned by `fap_icon` and FBT-validated |
| Buttons and screen only | Implemented |
| Multiple independent characters | No fixed application limit; dynamically indexed from SD |
| Separate readable `.txt` per character | `ch_{x}_{characterName}_{characterLvl}.txt` |
| Autosave every character mutation | Implemented with checksum/backup recovery |
| Multiclass levels and class-linked perks | Up to four classes; features store source class and gained level |
| Character stats and 2024 proficiency formula | Implemented |
| All six saving throws | Explicitly displayed/editable in Abilities & Saves |
| All 18 standard skills grouped by ability | Implemented |
| AC, initiative, passive scores, Spell Attack, Spell Save DC | Implemented and visible |
| Known/Prepared/Always Prepared/Ritual/free-cast spells | Implemented per spell |
| Class-and-level spell catalog default | Implemented for annotated records; hold OK for All |
| Spell slots and Wizard Arcane Recovery | Implemented with Short/Long Rest rules |
| Feature recharge cadence | Manual, turn, encounter, dawn, Short/Long, or Long |
| Items, equipment, weapons, attack/damage rolls | Implemented |
| Multi-die individual results plus sum | Implemented with paging |
| Animated dice sequence | Implemented with original code-drawn frames |
| CP/SP/EP/GP/PP | Implemented |
| Notes, adventure notes, item notes, milestones | Implemented; milestones can level a selected class once |
| Party initiative presets and turn tracking | Per character with round/current turn, participant HP/AC/conditions, history, and undo |
| SD catalogs and custom long-OK text | Implemented for names; Background uses short catalog/long custom |
| Background catalog in its own SD file | `catalogs/backgrounds.txt` included |
| Core and expansion option names | Names-only catalogs include SRD/core, XGtE, Ravenloft, Forgotten Realms, and Eberron selections |
| Structured rules-aware grants | Species, background, feat, class feature, subclass feature, spell, and item grants have review/apply/skip state |
| Core class-feature assignment | Names in `abilities.txt`; assigned feature stores class, level, notes, uses, formulas, and recharge |
| Per-class spellcasting | Shared multiclass slots, casting modes/ability, limits, spellbook, Pact slots, Arcanum, and spell points |
| Inventory resources | Containers, carried/equipped weight, capacity, armor/shield AC, attunement, ammo groups, and charges |
| Catalog diagnostics | Stable-ID validation, duplicate detection, required-field checks, and packaged-asset fallback |
| Fantasy quest / choices / skill checks / achievements | Data-driven Adventure mode with branches, rewards, flags, achievements, sprite tags, and checkpoints |
| Old save backward compatibility | Pre-0.8 layouts remain unsupported; verified 0.8 text saves migrate to schema 1 |
| Stable save schema and migration | Schema 1 frozen; verified 0.8 text saves migrate once and rewrite atomically |
| Profile portability | Rename, chunked duplicate/export/archive, validated import, and checksum diagnostics |
| Recoverable prior generation | One previous successful save retained per profile with explicit restore action |
| Translation and accessibility | Stable zero-allocation UI keys, translation template, and documented display/control conventions |

The expansion catalogs intentionally contain option names and original metadata rather than proprietary descriptions or mechanics. Custom text and notes support owned books, homebrew, errata, and table rulings.
