Pocket d20 SD-card catalogs

Copy this catalogs folder to:
/ext/apps_data/pocket_d20/catalogs/

Each file normally uses one option per line. Blank lines and lines beginning with # are ignored.
Pocket d20 merges these entries with its built-in SRD starter names and removes duplicates.
Append homebrew or legally obtained add-on names to the appropriate file. The app stores only
the selected name in the character save; rules text and licensed add-on content are not bundled.

Short OK on an item, spell, feature/feat, class, subclass, or Background opens its catalog.
Hold OK on that name to enter fully custom text instead.

Background names are kept separately in backgrounds.txt. The selected background is stored in
the active character's own character_N.txt save.

Spell lines may optionally provide filtering metadata:
Spell Name|level|Class1,Class2

Annotated spells appear by default only when the spell's assigned source class can cast that
spell level and its class is listed. Plain spell names remain supported but appear in the
Spells: All view. Hold OK in the spell catalog to toggle Allowed/All.
