# Rules audit

Rule ownership is by feature domain rather than by the word “rule.”

## Shared rules

`dnd_rules_core.c` and `dnd_rules.h` own generic dice, ability modifiers, total level, proficiency, saves, skills, initiative, XP/level helpers, exhaustion/speed and other shared character math. DNDolphins-only rest and character-mutation behavior lives in `dndolphins_rules_character.*`.

## Items

`dnd_weapon_rules.*` owns shared weapon ability, attack and damage calculations. `dndolphins_weapon_combat.*` owns DNDolphins combat-time item access; `dndinventory_rules.c` and `dndinventory_items.c` own carrying capacity, currency, equipment AC and starting-inventory policy; `dndadventure_item_reward.*` owns Adventure reward appends.

## Spells

`dndolphins_spells.*` owns casting ability, spell attack/save DC, class spell-level limits, native and multiclass slot calculation (including Eldritch Knight/Arcane Trickster third-caster handling), Pact/shared-slot initialization, spell-point costs and cast-resource options. `dndolphins_spell_combat.*` remains the structured spell effect/damage mapping layer.

Wizard combat eligibility distinguishes the spellbook from the prepared list. Wizard cantrips are available as known cantrips. A level-1+ Wizard spell is eligible for the Combat → Spell Attacks list only when `prepared`, `always_prepared`, or `free_casts_current > 0`. If an unprepared Wizard spell is present only because a Free Cast remains, its combat cast-option list contains only the Free Cast; normal slots, Pact slots and spell points are not offered. The general Wizard ritual rule remains represented by spell metadata, but unprepared spellbook-only rituals are not surfaced merely as combat Spell Attacks. Non-Wizard classes keep their existing Known/Prepared model.

The previous rule split was regression-checked during the refactor so moved functions retained their call sites and behavior. The Ghost Protocol/default-monster additions do not change character or combat rules.

## Initiative feature mapping

The active-character Initiative refresh recognizes the base Dexterity modifier, Initiative Misc, exhaustion penalty, Alert, and Jack of All Trades. Alert uses the character proficiency bonus and suppresses Jack of All Trades' half-proficiency contribution so proficiency is not counted twice. Unmapped initiative effects remain representable through Initiative Misc and the per-participant Normal/Advantage/Disadvantage roll setting.

## Player-choice progression

Deterministic numeric progression and fixed metadata grants may apply automatically on level increases. Player-choice spell acquisition is not guessed: when cantrip/prepared allowances increase, the UI tells the player to choose spells. Initial level-one grants are staged for review before application. **Level Choices** now handles explicit ASI/Feat opportunities: the player selects the choice, applied choices are represented by existing grant-history records, and the app never silently chooses a feat or ability increase.
