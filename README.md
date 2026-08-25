# Pocket d20 for Flipper Zero

Pocket d20 is an offline, buttons-and-screen character companion for 2024 5E play. It is built as an external Flipper Zero application and targets RogueMaster firmware.

The included `dist/pocket_d20.fap` was compiled and validated against RogueMaster branch `420`, commit `d3ae1f86bd961852af6969e887a8dd3492a19321` (2026-08-22).

## Install

1. Connect the Flipper Zero or place its microSD card in your computer.
2. Copy `dist/pocket_d20.fap` to `/ext/apps/Games/pocket_d20.fap`.
3. On the Flipper, open **Apps > Games > Pocket d20**.

The app creates its data directory on first save. Progress is stored under `/ext/apps_data/pocket_d20/`.

## What version 0.1 tracks

- Character name, player, species/race, background, alignment, inspiration, XP, and milestone mode.
- Up to four classes, each with its own class name, subclass, and level. Total character level and proficiency bonus are derived from the combined levels.
- Up to 20 perks/features. Each feature records its source class, class level gained, notes, current uses, and maximum uses.
- All six ability scores, ability modifiers, saving-throw proficiencies, all 18 skills, proficiency, and expertise.
- HP, temporary HP, AC, speed, initiative modifier, exhaustion, death saves, hit die, and hit dice.
- Spellcasting ability, spell attack bonus, spell save DC, cantrips and leveled spells, prepared/ritual flags, and spell slots 1-9.
- Up to 24 inventory entries, quantities, weight, equipped/attuned state, currency, and configurable weapon statistics.
- Up to 12 languages and free-form additional proficiencies.
- Up to 24 quick notes, adventure notes, item notes, and milestones. Item notes can create inventory entries. A milestone can award a level to a chosen class once.
- Saved party roster and a combat initiative order with rounds and current turn.
- Generic dice from d2 through d100, modifiers, and d20 advantage/disadvantage.
- Weapon attack and damage rolls using the character's ability modifier, total-level proficiency bonus, weapon proficiency, magic bonus, damage dice, extra dice, critical hits, versatile mode, and ammunition.
- Short- and long-rest helpers for feature uses, hit dice, HP, and spell slots.

## Controls

- **Up / Down:** move through rows.
- **Left / Right:** decrease/increase or cycle the selected field. Holding repeats where appropriate.
- **OK:** open a row, toggle a flag, roll, or confirm.
- **Back:** return to the previous screen. Back from Home saves and exits.
- Text fields use the firmware's on-screen keyboard.

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

## Saving and recovery

Pocket d20 uses a versioned binary save with a magic value, payload size, and FNV-1a checksum. A save is first written and synced as `character.tmp`; the previous valid save is rotated to `character.bak`; then the temporary file is promoted to `character.save`. If the main file is missing or invalid at startup, the app attempts to load the backup before creating a fresh sheet.

The binary format is for app-managed data and is not intended for manual editing.

## Build from source

Install the RogueMaster build environment, then place this folder at `applications_user/pocket_d20` in the firmware tree and run:

```sh
./fbt fap_pocket_d20
```

The result is normally written to:

```text
build/f7-firmware-C/.extapps/pocket_d20.fap
```

See `BUILD_VERIFICATION.md` for the exact tested revision and artifact checksum.

## Scope and limitations

- Version 0.1 stores one character profile. That character can multiclass into four classes; it does not yet manage multiple separate character files.
- The app is a tracker and roller, not a complete rules engine. It does not automatically apply every class table, subclass benefit, prerequisite, spell rule, condition, inventory interaction, or multiclass spell-slot calculation.
- Class perks/features are deliberately user-entered and class-linked. This avoids shipping non-SRD rulebook content and keeps custom campaigns and homebrew usable.
- Long rest restores HP, spell slots, and tracked feature uses and restores hit dice using the app's simplified helper. The table can override any value immediately.
- Compilation and firmware API validation were performed; physical-device interaction was not available in this environment.

Pocket d20 is an independent fan-made utility and is not affiliated with or endorsed by Wizards of the Coast.

