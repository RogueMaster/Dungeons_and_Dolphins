# Dungeons and Dolphins

Dungeons and Dolphins is an offline, fifth-edition-compatible character, campaign, dice, and initiative tracker for Flipper Zero. Version 0.5 targets RogueMaster firmware and uses only the screen, directional controls, vibration, sound, and SD-card storage.

## Full feature list

### Character profiles and saving

- Create, switch, and delete up to six independent character profiles.
- Protect the Main profile from deletion.
- Store each character in a separate, readable, versioned `.txt` file.
- Keep stats, classes, spells, equipment, notes, party presets, and active combat isolated per character.
- Autosave every meaningful edit, including stat, item, spell, currency, class, feature, journal, and initiative changes.
- Write through a temporary file and retain a backup to reduce save corruption risk.

### Character sheet

- Character name, player name, species, background, alignment, and other proficiencies.
- Strength, Dexterity, Constitution, Intelligence, Wisdom, and Charisma scores and modifiers.
- All six saving throws with proficiency and miscellaneous modifiers.
- All 18 standard skills grouped by governing ability.
- Skill proficiency, expertise, miscellaneous modifiers, and calculated totals.
- Passive Perception, Passive Insight, and Passive Investigation.
- Armor Class, current and maximum Hit Points, temporary Hit Points, Initiative, Speed, and Inspiration.
- Proficiency Bonus calculated from combined character level.
- Hit Point Dice, death saves, Exhaustion, conditions, experience, and milestone leveling.
- Languages and currency in Copper, Silver, Electrum, Gold, and Platinum pieces.

### Classes, subclasses, backgrounds, and features

- Track up to four classes per character with independent class, subclass, and level values.
- Calculate total level and Proficiency Bonus across all class records.
- Default subclass picker filters every bundled subclass to the selected parent class.
- Hold OK in the subclass picker to show all entries for custom campaigns and overrides.
- Assign features and perks to a source class and the class level at which they were gained.
- Track feature notes, current and maximum uses, and Manual, Short/Long Rest, or Long Rest recharge cadence.
- Select backgrounds, classes, subclasses, feats, and abilities from bundled SD-card catalogs.
- Hold OK on supported name fields to enter custom text.

### Spellcasting

- Track spellcasting ability, Spell Attack modifier, Spell Save DC modifier, and calculated totals.
- Track Known, Prepared, Always Prepared, Ritual, and limited free-cast states independently.
- Track current and maximum free casts for each spell and restore them on a Long Rest.
- Track current and maximum spell slots for levels 1 through 9.
- Default spell picker filters annotated spells by the assigned class and that class's available spell level.
- Hold OK in the spell picker to switch between Allowed and All views.
- Include complete standard spell-name, spell-level, and class-list metadata in the SD-card catalog.
- Arcane Recovery helper restores selected spell slots within a level-based budget and tracks its use.
- Long Rest restores spell slots; valid exceptions and granted spells can be entered through the All view or custom text.

### Inventory and equipment

- Track item name, notes, quantity, weight, value, charges, and equipped state.
- Mark items as weapons and store attack ability, proficiency, magic/attack bonus, damage dice, damage bonus, and damage type.
- Track weapon properties including Finesse, Ranged, Thrown, Versatile, and Two-Handed behavior.
- Track ammunition and limited-use resources.
- Create an inventory item directly from a journal entry.
- Select item names from the bundled catalog or hold OK for custom text.
- Browse 519 bundled equipment and magic-item records with category, rarity, and source metadata.
- Item rows show a compact category marker and a magic-item marker on the Flipper display.
- Selecting a cataloged weapon automatically enables its weapon-tracking fields.

### Dice and attacks

- Roll d4, d6, d8, d10, d12, d20, d100, and configurable multi-die combinations.
- Normal, Advantage, and Disadvantage d20 modes.
- Code-drawn multi-frame dice tumble animation with sound and vibration feedback.
- Display every individual die whenever multiple dice are rolled, plus the dice sum and final modified total.
- Roll ability checks and saving throws using the active character's calculated modifiers.
- Roll attacks directly from equipped weapon data.
- Calculate attack rolls from the selected ability, proficiency, weapon bonus, and other modifiers.
- Roll normal, versatile, and critical damage.
- Critical hits double applicable damage dice while adding flat modifiers once.
- Support configurable extra damage dice for character abilities, magic, poison, and similar effects.
- Retain roll history details while viewing the result.

### Journal and milestones

- Quick notes, session notes, adventure notes, item notes, and milestones.
- Editable title and body for each journal entry.
- Pending and completed milestone states.
- Milestone level-up action assigned to a selected class.
- Persistent milestone and journal history per character.

### Initiative tracker

- Saved party roster with names and initiative modifiers.
- Automatically include the active character.
- Manually enter or roll initiative values.
- Add allies, summons, non-player characters, and enemies.
- Sort highest to lowest and manually resolve ties.
- Current-turn indicator, Next Turn, Previous Turn, and round counter.
- Add late arrivals and remove participants during combat.
- Start, resume, save, and end combat.
- Keep separate party presets and active encounters for every character profile.

### SD-card catalogs

Copy the included `sd_card/apps_data/pocket_d20/catalogs/` folder to the same path on the Flipper SD card. Catalogs are provided for:

- Classes
- Subclasses with parent-class metadata
- Backgrounds
- Spells with level and class-list metadata
- Feats
- Abilities and class features
- Items

Catalog entries can be extended with additional one-line names. Spell lines may use `Spell|Level|Class, Class`; subclass lines use `Subclass|Parent Class`; item lines use `Name|Category|Rarity|Source`.

## Controls

- Up/Down: move through menus or lists.
- Left/Right: adjust the selected value.
- Short OK: open, select, toggle, roll, or confirm.
- Long OK on supported name fields: edit custom text.
- Long OK in spell or subclass catalogs: toggle filtered and All views.
- Back: return to the previous screen; from Home, save and exit.

## Installation

1. Copy `dist/pocket_d20.fap` to the desired apps folder on the Flipper SD card.
2. Copy the contents of the included `sd_card/` directory to the root of the SD card.
3. Launch **Dungeons and Dolphins** from the Flipper application browser.

The internal application ID and data directory remain `pocket_d20` so existing deployment paths and firmware integration stay stable during pre-1.0 development.

## Build verification

The included FAP was compiled against RogueMaster branch `420`, commit `d3ae1f86bd961852af6969e887a8dd3492a19321`. See `BUILD_VERIFICATION.md` for the exact validation record.

Physical-device interaction remains to be tested. This application is a tracker and roller, not a complete automated rules engine; unusual campaign rules and granted abilities can be represented with custom entries.
