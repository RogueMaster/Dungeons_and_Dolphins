# Rules audit

Rule ownership is by feature domain rather than by the word “rule.”

## Shared rules

`dndolphins_rules.*` owns generic dice, ability modifiers, total level, proficiency, saves, skills, initiative, XP/level helpers, exhaustion/speed and other cross-feature character math.

## Progression

`dndolphins_progression.*` owns deterministic class-level resource derivation: per-class Hit Dice, multiclass shared-slot maxima, Pact slots/slot level, Sorcery Point maximums, class cantrip/prepared limits and Wizard spellbook minimums. `dndolphins.c` opens bundled fixed-grant progression metadata only on an actual level gain and consumes it in bounded eight-line pages using one reusable line/read buffer; no progression hash/signature/catalog survives the call. Player-choice outcomes are deliberately not auto-selected.

Progression changes derive from existing class/species/level state and reuse existing feature/spell fields; they do not require a new save schema. Existing free-cast spell records are updated duplicate-safely as deterministic grant maxima increase, and Long Rest restores current free casts to their stored maxima.

## Items

`dndolphins_items.*` owns carrying capacity, currency normalization, calculated equipment AC, weapon ability selection, attack modifier, attack rolls and weapon damage behavior. It also owns starting-inventory policy and Adventure item reward helpers.

## Spells

`dndolphins_spells.*` owns casting ability, spell attack/save DC, class spell-level limits, multiclass slot calculation, Pact/shared-slot initialization, spell-point costs and cast-resource options. `dndolphins_spell_combat.*` remains the structured spell effect/damage mapping layer.

The previous rule split was regression-checked during the refactor so moved functions retained their call sites and behavior. Deterministic progression is intentionally separate from interactive level-up choices, and the Ghost Protocol/default-monster additions do not change character or combat rules.
