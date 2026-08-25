# Pocket d20 for Flipper Zero

Pocket d20 is an offline, buttons-and-screen character companion for 2024 5E play. It is built as an external Flipper Zero application and targets RogueMaster firmware.

The manifest uses the included `icon.png`, a 10x10, 1-bit monochrome d20 icon validated by RogueMaster's FAP build tooling.

The included `dist/pocket_d20.fap` version 0.3 was compiled and validated against RogueMaster branch `420`, commit `d3ae1f86bd961852af6969e887a8dd3492a19321` (2026-08-22).

## Install

1. Connect the Flipper Zero or place its microSD card in your computer.
2. Copy `dist/pocket_d20.fap` to `/ext/apps/Games/pocket_d20.fap`.
3. On the Flipper, open **Apps > Games > Pocket d20**.

The app creates its data directory on first save. Progress is stored under `/ext/apps_data/pocket_d20/`.

## What version 0.3 tracks

- Up to six independent character profiles. Main is always present, and every profile has its own complete sheet, equipment, spell list, notes, party roster, and active initiative state.
- Character name, player, species/race, assignable catalog/custom background, alignment, inspiration, XP, and milestone mode.
- Up to four classes, each with its own class name, subclass, and level. Total character level and proficiency bonus are derived from the combined levels.
- Up to 20 perks/features. Each feature records its source class, class level gained, notes, current/maximum uses, and Manual, Short/Long Rest, or Long Rest recharge cadence. The SD catalog includes the names of core 2024 class features plus an Artificer starter set.
- All six ability scores and saving throws, including ability modifiers, save proficiency, final modifiers, and per-save miscellaneous bonuses or penalties.
- All 18 standard skills grouped by governing ability (STR, DEX, INT, WIS, and CHA), with proficiency/Expertise, final modifiers, and per-skill miscellaneous bonuses or penalties.
- HP, temporary HP, AC, speed, initiative and its miscellaneous modifier, exhaustion, death saves, hit die, hit dice, Passive Perception, Passive Insight, and Passive Investigation.
- Automatically derived total character level and Proficiency Bonus.
- Spellcasting ability, editable spell-attack and spell-save miscellaneous modifiers, final spell attack bonus, spell save DC, and spell slots 1-9.
- Per-spell Known, Prepared, Always Prepared, Ritual, and free-casts-per-Long-Rest tracking. Free casts have current and maximum values and can be spent directly from the spell record.
- Up to 24 inventory entries, quantities, weight, equipped/attuned state, and configurable weapon statistics.
- Standard currency in Copper, Silver, Electrum, Gold, and Platinum pieces (CP/SP/EP/GP/PP).
- Up to 12 languages and free-form additional proficiencies.
- Up to 24 quick notes, adventure notes, item notes, and milestones. Item notes can create inventory entries. A milestone can award a level to a chosen class once.
- Saved party roster and a combat initiative order with rounds and current turn.
- Generic dice from d2 through d100, modifiers, and d20 advantage/disadvantage, with a timed roll animation.
- Every roll involving more than one die shows the individual results and their sum; long damage pools are paged.
- Weapon attack and damage rolls using the character's ability modifier, total-level proficiency bonus, weapon proficiency, magic bonus, damage dice, extra dice, critical hits, versatile mode, and ammunition.
- Rules-aware Short Rest, Hit Point Die spending, Arcane Recovery, and Long Rest helpers.
- Immediate autosave after every character-data mutation, including stats, spells, items, inventory, currency, notes, and initiative changes.

## Controls

- **Up / Down:** move through rows.
- **Left / Right:** decrease/increase or cycle the selected field. Holding repeats where appropriate.
- **OK:** open a row, toggle a flag, roll, or confirm.
- **Back:** return to the previous screen. Back from Home saves and exits.
- Text fields use the firmware's on-screen keyboard.

### Characters

- Open **Characters** from Home to see Main and every additional profile.
- Select **+ New Character** to create and immediately activate a fresh sheet.
- Press OK on a saved character to autosave the current sheet and switch profiles.
- Hold OK to delete a non-active secondary character. Main cannot be deleted, and an active character must be switched away from before deletion.

### Saving throws and skills

- Under **Abilities & Saves**, Left/Right normally changes the ability score and OK toggles the save between none and proficient. Hold OK to switch Left/Right into miscellaneous save-modifier editing.
- That screen explicitly lists all six saves—STR, DEX, CON, INT, WIS, and CHA—with score modifier, proficiency marker, final saving-throw modifier, and editable miscellaneous modifier.
- Under **Skills**, Left/Right and OK cycle none, proficient, and expertise. Hold OK to switch Left/Right into miscellaneous skill-modifier editing.
- The Skills list contains all 18 standard skills in STR, DEX, INT, WIS, and CHA groups. Every row starts with its governing ability abbreviation.
- Every displayed D20 Test total immediately combines its ability modifier, applicable Proficiency Bonus or Expertise, miscellaneous modifier, and the current 2024 Exhaustion penalty.
- Passive Perception, Insight, and Investigation use shortened labels so their values remain visible. Left/Right on those Vitals rows edits the associated skill's miscellaneous modifier, keeping the passive and active skill calculations in sync.

### Catalog choices and custom names

- Short OK on an item, spell, feature/feat, class, subclass, or Background opens a scrollable catalog.
- Hold OK on the same field to use the on-screen keyboard for a custom name.
- Built-in starter names work without extra files. The ZIP also contains a ready-to-copy `sd_card/apps_data/pocket_d20/catalogs/` tree with larger SRD lists and public option names from Forgotten Realms, Ravenloft, Eberron, and Xanathar's Guide to Everything.
- Copy the ZIP's `sd_card/apps_data/pocket_d20/catalogs` folder to `/ext/apps_data/pocket_d20/catalogs/` on the SD card.
- Catalog files are plain text, one option per line. You can append homebrew or names from add-ons you legally own. Expansion catalogs contain names only; proprietary mechanics, descriptions, and stat blocks are not bundled.
- Background choices live in their own SD file, `catalogs/backgrounds.txt`; the chosen background is also autosaved in the active character's separate character file.

### Spell tracking

- Open **Magic & Spells** to choose the sheet's spellcasting ability, edit spell attack/save miscellaneous modifiers, review the calculated attack bonus and save DC, and track spell slots.
- Each spell independently records whether it is Known, Prepared, Always Prepared, and a Ritual.
- Each spell also records its source class. Opening the spell-name catalog defaults to **Spells: Allowed**, filtering annotated entries by that class's level and spell progression. Hold OK in the catalog to toggle **Spells: All** for unannotated expansion entries, unusual subclass lists, custom classes, or table overrides.
- Optional spell metadata uses `Spell Name|level|Class1,Class2` in `spells.txt`. A plain name remains valid but appears only in the All view because Pocket d20 will not guess its class eligibility.
- `K`, `P`, and `A` in the spell list mean Known, Prepared, and Always Prepared. `F` means at least one free cast remains.
- Set **Free casts max** for spells granted without a spell slot, set the current value, and select **Use one free cast** when cast. A Long Rest restores current free casts to maximum.
- A Wizard's **Short Rest** opens the Arcane Recovery chooser when the feature is ready and eligible slots have been expended. It enforces a recovery budget of half the Wizard level rounded up, accepts only level 1-5 slots, and becomes available again after a Long Rest.

### Weapon rolls

Open **Combat > Weapon Attacks**. Left/Right cycles Normal, Advantage, and Disadvantage. OK rolls the selected weapon attack. On the result screen:

- **OK:** roll damage; on the damage view, roll it again.
- **Right:** force critical damage when the table calls a hit critical.
- **Up:** reroll the attack.

An ammunition weapon consumes one round per attack and will refuse to roll at zero ammunition.

### Initiative

1. Add reusable names and initiative modifiers under **Initiative > Party Roster**.
2. Choose **Start New Combat**. The character and saved party are copied into the setup list.
3. Select a participant and press OK to roll `d20 + modifier`, or use Left/Right to enter a total manually.
4. Add temporary participants if needed, then select **Begin Combat**. The order sorts from highest to lowest.
5. During combat, OK advances the turn and increments the round after the last participant. Hold OK to make the selected participant current. Hold Left/Right to reorder the selected participant.

Active combat is saved and can be resumed after leaving the app.

### Rests and Exhaustion

- **Short Rest** requires at least 1 HP and refreshes only feature resources configured as **Short/Long**.
- **Spend Hit Die** rolls the configured Hit Die, adds the Constitution modifier, heals at least 1 HP up to the maximum, decrements the pool, animates the die, and reports the roll.
- **Long Rest** requires at least 1 HP, restores HP and spent Hit Dice, clears temporary HP and death-save marks, restores spell slots and free casts, refreshes configured rest-based features, resets Arcane Recovery, and reduces Exhaustion by 1.
- Each 2024 Exhaustion level applies `-2` to D20 Tests and reduces Speed by 5 feet. Pocket d20 applies those effects to calculated D20 Test modifiers, attack rolls, spell attacks, and initiative.

## Saving and recovery

Pocket d20 stores each character in a separate, readable, versioned text file under `/ext/apps_data/pocket_d20/`:

- `character_0.txt` is Main.
- `character_1.txt` through `character_5.txt` are additional saved characters.
- `profiles.txt` records which slots exist and which one is active.

Every character file contains the complete sheet plus that character's party roster and initiative state. Saves use a synced temporary `.txt` file and rotate the previous valid file to a `.bak.txt` backup before replacement. If the main character file is missing or invalid, the app attempts its backup before creating a fresh character.

Version 0.3 intentionally does not migrate the pre-1.0 binary save format. Older `character.save` data is ignored and v0.3 starts with the new per-character text format.

## Build from source

Install the RogueMaster build environment, then place this folder at `applications_user/pocket_d20` in the firmware tree and run:

```sh
./fbt fap_pocket_d20
```

The result is normally written to:

```text
build/f7-firmware-C/.extapps/pocket_d20.fap
```

See `RULES_AUDIT.md` for the official-statistics audit, `FEATURE_CHECKLIST.md` for the request-by-request implementation check, `ROADMAP.md` for the proposed path to 1.0, and `BUILD_VERIFICATION.md` for the exact tested revision and artifact checksum.

## Scope and limitations

- Version 0.3 supports six character profiles and up to four classes per character.
- The app is a tracker and roller, not a complete rules engine. It does not automatically apply every class table, subclass benefit, prerequisite, spell rule, condition, inventory interaction, or multiclass spell-slot calculation.
- Class perks/features are selected from names or entered manually, then class-linked. Pocket d20 does not bundle proprietary feature rules text; record mechanics you own in the feature notes and resource fields.
- A multiclass character currently has one configurable pooled Hit Die type. Mixed Hit Die pools are planned for a later release and are called out in the roadmap.
- Expansion catalogs contain option names for convenient selection, not the proprietary rules attached to those options. Enter mechanics you own in the notes and editable statistics.
- Compilation and firmware API validation were performed; physical-device interaction was not available in this environment.

Pocket d20 is an independent fan-made utility and is not affiliated with or endorsed by Wizards of the Coast.
