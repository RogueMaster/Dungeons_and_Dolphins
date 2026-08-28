# Device test matrix

## Launch and handoff

- [ ] Launch each FAP directly.
- [ ] Cycle DNDolphins → Bestiary → Initiative → DNDolphins repeatedly.
- [ ] Cycle DNDolphins → Journal/Adventure → DNDolphins repeatedly.
- [ ] Confirm active-character fallback and teardown-before-launch behavior.

## Character / Inventory / Spells

- [ ] Create a character and confirm STR/DEX/CON/INT/WIS/CHA start at 15/14/13/12/10/8; edit them and confirm existing/save-loaded scores are not overwritten by defaults.
- [ ] On both first launch with no characters and **Create Character**, confirm Class and Species start unconfirmed and no fixed features/spells are granted. Rename the character, choose Species and choose Wizard; confirm no Fighter traits appear.
- [ ] Select **Grant Initial Traits** at level 1 and confirm the chosen class/species deterministic traits are added. Select it again and confirm no duplicates. Verify it refuses default names, unconfirmed Class/Species and characters above level 1.
- [ ] Increase total level across proficiency thresholds and confirm PB changes at 5/9/13/17 while multiclass levels contribute to the same total.
- [ ] Increase/decrease class levels and confirm per-class Hit Dice maxima follow class level while already-spent Hit Dice are preserved.
- [ ] Test full-caster and Paladin/Ranger multiclass combinations: shared slot maxima follow combined caster level while each class keeps its own cantrip/prepared limit.
- [ ] Test Warlock level thresholds for Pact slot count/level and Mystic Arcanum availability; test Sorcerer level 2+ for Sorcery Point maximum.
- [ ] Test Wizard level increases: minimum spellbook capacity rises by two per Wizard level after 1 and never deletes/copied extra spells when the level is reduced.
- [ ] Cross fixed class-feature progression thresholds and confirm grants are added once with no duplicates; reload the same profile repeatedly and confirm ordinary loads do not rescan or mutate progression state.
- [ ] Exercise ordinary non-progression screens, profile switches and repeated saves after a successful level-up; confirm none opens progression metadata. Increase a class level and confirm the bounded eight-line scan reconciles every fixed class/species grant currently due.
- [ ] Verify fixed grant spells such as Divine Smite/Find Steed/Hunter's Mark update the existing spell record rather than duplicating it; Hunter's Mark free-cast maximum scales at its configured Ranger thresholds and Long Rest restores current free casts.
- [ ] For a supported level-gated species spell lineage, cross total-character-level 3 and 5 using both single-class and multiclass characters; confirm fixed spells/features grant once and Long Rest restores configured free casts.
- [ ] Confirm progression never auto-selects a subclass, ASI/feat, learned-spell choice, Fighting Style option, invocation, metamagic option or other player choice.
- [ ] Load a readable character file containing only unknown/future body fields and confirm the profile remains usable with filename/default fallback rather than being rejected wholesale.
- [ ] Create a character and confirm no inventory exists until Inventory is opened.
- [ ] Inventory opened before Class/Species confirmation remains usable but does not seed defaults. After both choices are confirmed, first Inventory open seeds class/species/background equipment plus exactly one hidden d100 trinket; repeat with a header-only/record-empty item sidecar and confirm it is repaired instead of opening empty.
- [ ] Reopen Inventory and confirm no duplicate starting items/currency.
- [ ] Resources and Weapon Attacks do not create missing inventory.
- [ ] In Spellbook, OK on **+ Add New** opens a blank spell editor; OK on **Name** opens allowed spells, selecting a name updates that same record, and the spell remains after reopening the app.
- [ ] For a multiclass character, the default allowed-spell catalog is the union of eligible spells for every class at each class's own level; the Class filter narrows to one class and selected spells get an eligible Source class.
- [ ] In Inventory, OK on **+ Add New** opens a blank item editor; OK on **Name** opens the item catalog, selecting a name updates that same record, and the item remains after reopening the app.
- [ ] Repeat both Add tests with a sidecar containing valid records followed by a malformed/truncated trailing line; valid records remain visible and the newly appended record opens directly in its editor without a post-append reread.
- [ ] Hold OK on the **Name** row in Spellbook/Inventory opens custom text entry.
- [ ] Simulate/retry after an SD write failure and confirm an interrupted append does not leave a partial spell/item record.
- [ ] Exercise >8 items and >8 spells across page boundaries.
- [ ] Verify spell attack/DC, slots/Pact/points and weapon attack/damage behavior.

## Adventure

- [ ] Reef Wardens loads and completes.
- [ ] Ghost Protocol appears as bundled content and loads `audit_brief`.
- [ ] Exercise successful and failed checks through several branches.
- [ ] Confirm rewards are granted once when guarded and milestone journaling does not intentionally duplicate.
- [ ] Confirm Ghost Protocol contains no external/device-operation dependency.

## Bestiary

- [ ] Fresh app data: Dolphin and Capybara appear as custom monsters after first launch.
- [ ] Existing custom pack: launch does not replace or merge the default seed.
- [ ] Partial custom files: launch preserves them for existing recovery/manual repair behavior.
- [ ] View/edit/delete custom monsters, install/enable packs and generate encounters.
- [ ] Open every monster-stat field with OK and confirm the full-screen reader wraps at up to 26 characters without truncating text or miscounting scroll lines.
- [ ] Send individual and generated monsters to Initiative.

## Stress

- [ ] Inspect long selected/unselected rows in all five FAPs and confirm full-width rows display or marquee exactly 26 characters without drawing beyond the screen; verify compact sprite/numeric layouts remain intact.
- [ ] Repeated launch/back cycles without heap growth or crash.
- [ ] Large profile and Journal counts.
- [ ] Large monster/campaign indexes.
- [ ] Maximum-size encounter save/rename/delete paths.
