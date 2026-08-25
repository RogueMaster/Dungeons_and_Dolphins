# 2024 5E rules audit

This audit compares Pocket d20 0.3 with the 2024 rules exposed in SRD 5.2.1. Pocket d20 is a tracker and roller, not a substitute for the rules. Expansion files contain option names only.

## Character statistics

| Statistic | Pocket d20 behavior | Audit result |
|---|---|---|
| Ability scores | STR, DEX, CON, INT, WIS, CHA; modifier is floor((score - 10) / 2) | Implemented |
| Proficiency Bonus | `2 + floor((total level - 1) / 4)`, using combined class levels | Implemented |
| Saving throws | All six saves show ability modifier, proficiency, miscellaneous adjustment, Exhaustion, and final modifier | Implemented |
| Skills | All 18 standard skills use their default ability and support none/proficiency/Expertise plus a miscellaneous adjustment | Implemented |
| Passive checks | Passive Perception, Insight, and Investigation are `10 + base skill modifier`; the UI exposes the linked skill adjustment | Implemented |
| Initiative | Dexterity modifier + editable miscellaneous adjustment; 2024 Exhaustion applies | Implemented |
| AC, HP, Temp HP, Speed | Directly tracked; Speed shows the current Exhaustion-adjusted value | Implemented |
| Spell attack | spellcasting ability modifier + Proficiency Bonus + miscellaneous adjustment; Exhaustion applies to the attack roll | Implemented |
| Spell save DC | 8 + spellcasting ability modifier + Proficiency Bonus + miscellaneous adjustment | Implemented |
| Death saves | successes and failures are tracked from 0-3 | Implemented as a tracker |

Saving-throw proficiency deliberately has only two states: none or proficient. Expertise remains available for skills, where a feature can grant it.

## Standard skill mapping

| Ability | Skills |
|---|---|
| Strength | Athletics |
| Dexterity | Acrobatics, Sleight of Hand, Stealth |
| Intelligence | Arcana, History, Investigation, Nature, Religion |
| Wisdom | Animal Handling, Insight, Medicine, Perception, Survival |
| Charisma | Deception, Intimidation, Performance, Persuasion |

Every skill row begins with the governing ability abbreviation. A table can still apply a different ability for a particular check by entering the table's result as a miscellaneous adjustment; selectable alternate ability/skill pairings are on the roadmap.

## Rests, recovery, and Exhaustion

- Short Rest requires at least 1 HP. It refreshes only user-marked Short/Long resources.
- Hit Point Dice are spent separately. One configured die is rolled, Constitution is added, healing is at least 1 and cannot exceed maximum HP, and the pool decreases by one.
- Wizard Arcane Recovery is offered after a Short Rest when available. Its combined slot-level budget is half the Wizard level rounded up, it cannot restore level 6+ slots, and it resets after a Long Rest.
- Long Rest requires at least 1 HP; restores HP, spent Hit Point Dice, slots, free casts, and configured resources; clears temporary HP and death-save marks; resets Arcane Recovery; and reduces Exhaustion by 1.
- Each 2024 Exhaustion level imposes `-2` on D20 Tests and reduces Speed by 5 feet. Pocket d20 applies this to saving throws, skills, initiative, spell attacks, and weapon attacks. Level 6 remains visible as the fatal state for the table to resolve.

The app cannot determine whether an interrupted rest qualifies, whether the once-per-24-hours Long Rest restriction is met, or which campaign exception applies. Those remain table decisions.

## Spell tracking and selection

- Spell records track source class, level, Known, Prepared, Always Prepared, Ritual, free casts current/maximum, and notes.
- Slot levels 1-9 are tracked. Long Rest restores current slots to the user-configured maxima.
- The default spell catalog filters annotated entries by the selected spell's source class and that class's level. Full-caster, Paladin/Ranger/Artificer, and Pact Magic level progressions are recognized.
- Holding OK switches to the full catalog for unannotated expansion names, homebrew, custom classes, granted spells, and subclass exceptions.
- The SD catalog includes the verified public names of all 95 spells in Xanathar's Guide to Everything and the public spell names from Forgotten Realms: Heroes of Faerun. Van Richten's Guide to Ravenloft has no general new-spells chapter; relevant public expanded-list names are included rather than inventing a Ravenloft spell list.

The filter is a guardrail, not a complete entitlement engine. It does not yet automatically model subclass spell lists, species/background/feat grants, spellbook copying, Magical Secrets, prepared-count formulas, multiclass slot pooling, or every expansion erratum. The All view and custom entry remain available for valid exceptions.

## Classes, subclasses, backgrounds, and features

- Up to four class records retain independent class, subclass, and class level values.
- Features retain their source class, class level gained, notes, uses, and recharge cadence.
- `abilities.txt` supplies public names of core 2024 class features and Artificer feature names for assignment. It does not ship proprietary mechanics.
- `backgrounds.txt` independently supplies the 16 2024 core background names, Haunted One and Investigator from Ravenloft, and the 18 publicly listed Forgotten Realms backgrounds. A custom background can be entered by holding OK.
- Subclass catalogs include the SRD starter choices plus public option names from Xanathar's Guide to Everything, Ravenloft, Forgotten Realms, and Eberron.

Pocket d20 does not automatically grant background ability-score choices, Origin feats, skill/tool proficiencies, equipment, subclass features, or level-gated class features. Users assign the matching names and record the mechanics they own.

## Attacks and damage

- Weapon attacks use the selected STR/DEX/best ability, weapon proficiency, Proficiency Bonus, magic adjustment, 2024 Exhaustion, and Normal/Advantage/Disadvantage.
- Natural 20 and natural 1 are identified. Damage supports base and extra dice, ability-to-damage, magic/static adjustments, versatile damage, and forced critical dice.
- When more than one die is rolled, each die result and the sum are displayed; longer pools use pages.
- Weapon properties and ammunition can be tracked, but Mastery properties, nick interactions, two-weapon action economy, Sneak Attack eligibility, resistance/vulnerability, Graze, critical exceptions, and class-specific damage riders are table-managed.

## Persistence audit

Every character has an independent readable `character_N.txt` containing the entire sheet, notes, inventory, party preset, and current initiative state. All mutations autosave. Writes use a temporary file, sync, checksum validation, previous-file backup, and backup recovery. This is a fresh pre-1.0 schema; old binary saves are intentionally not migrated.

## Official references used

- SRD 5.2.1: https://www.dndbeyond.com/srd
- SRD 5.2.1 PDF: https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf
- Xanathar's Guide to Everything contents: https://www.dndbeyond.com/sources/dnd/xgte
- Van Richten's Guide to Ravenloft contents: https://www.dndbeyond.com/sources/dnd/vrgtr
- Forgotten Realms: Heroes of Faerun contents: https://www.dndbeyond.com/sources/dnd/frhof
- Eberron: Forge of the Artificer overview: https://www.dndbeyond.com/posts/2106-whats-new-with-the-artificer-in-eberron-forge-of

