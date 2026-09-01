# Dungeons & Dolphins roadmap

This roadmap contains user-facing features and improvements that are not present in the current application.

## 3.6 — Faster play at the table

- **Quick Rolls:** Add a character-driven roll screen for ability checks, saving throws and skills using the active character's current modifiers. Normal, Advantage, Disadvantage and Guidance controls would make common rolls available without manually translating a sheet value into the general Dice Roller.
- **Grant Review Center:** Add a normal Character-menu entry for viewing pending, skipped and custom progression grants without first reaching the review screen through a grant problem. This would make the existing retry/skip/edit workflow discoverable and give players one place to inspect unresolved progression work.
- **Concentration Helper:** Let the player enter damage taken and immediately roll the required Constitution save against the calculated concentration DC using the character's current save modifier. This would remove repeated DC arithmetic during combat while leaving concentration state under the player's control.
- **Combat Quick Bar:** Add a compact screen containing favorite weapon attacks, favorite spell attacks, HP/temp HP, reaction state and the most important combat resources. This would reduce repeated navigation through the full Combat menu during every turn.
- **Ammo Stack Choice:** When several Inventory stacks satisfy a weapon's ammunition token, show the matching stacks and let the player choose which one to consume. This would make specialty ammunition such as Fire Arrows, Silvered Arrows or named bolt types intentionally selectable instead of relying on the first eligible stack.
- **Rest Preview:** Show the resources a Short Rest or Long Rest is about to recover before applying the rest. This would help players understand Hit Dice, spell-resource and Feature recharge effects without changing the existing explicit rest actions.
- **Reaction Turn Sync:** When the active character reaches the start of its turn in Initiative, optionally set Reaction back to Ready while retaining the manual Reaction toggle. This would reduce combat bookkeeping and better connect DNDolphins combat state to the turn tracker.

## 3.7 — Better character, Inventory and Spellbook organization

- **Inventory search and sorting:** Add owned-Item search plus sorting by name, category, equipped state, quantity, container or weight while retaining bounded paging. Larger inventories would become much faster to navigate without requiring the whole sidecar to remain resident.
- **Container browsing:** Let a container open as a focused Inventory view containing only the Items assigned to it, with quick move-in and move-out actions. Bags, pouches and chests would become practical navigation tools instead of only a stored container reference.
- **Equipment loadouts:** Allow a small number of named equipped-item sets such as Travel, Dungeon, Ranged or Social. Switching a loadout would change equipped state in one action while preserving Item records, quantities and containers.
- **Spellbook search:** Add name search to the owned Spellbook and Add Spell catalog alongside the existing level, class, ritual, school, source, status and eligibility filters. Finding one spell in a large known/prepared list would require far less paging.
- **Spell favorites / quick-cast list:** Let players mark frequently used spells as favorites and open them from a compact combat-oriented list. Favorites would be a navigation layer only and would not alter Known, Prepared, Always Prepared or free-cast state.
- **Feature favorites:** Let frequently used Features and limited-use resources appear in a compact favorites list. This would make abilities such as Action Surge, Second Wind, Channel Divinity or other commonly referenced resources reachable without paging through every Feature.
- **Character summary card:** Add one condensed screen for AC, HP, speed, proficiency bonus, initiative, passive scores, Spell Attack/DC, Hit Dice and important remaining resources. It would provide a fast at-a-glance sheet when full editing is unnecessary.

## 3.8 — Encounter and GM workflow improvements

- **Monster turn tools:** From Initiative, allow a Bestiary participant to open its stat block and roll common attacks/damage without leaving the encounter workflow. Initiative could then serve as a lightweight GM combat console rather than only turn-order and HP tracking.
- **Encounter round notes:** Allow a short note or marker on the current encounter/round for lair actions, reinforcements, environmental hazards or scripted events. This would keep time-sensitive encounter reminders beside the turn order instead of requiring a separate Journal entry.
- **Encounter group inserts:** Allow a saved group such as guards, a patrol or a boss/minion package to be inserted into another encounter or an active Initiative setup. Saved Encounters already preserve complete encounters; group inserts would specifically support reusable components inside a different encounter.
- **Initiative condition duration:** Optionally attach a round/turn duration to a participant condition and decrement it as turns advance. Short-lived effects could expire automatically while ordinary free-form conditions remain manually controlled.
- **Encounter loot packages:** Let a Bestiary/Initiative encounter carry an optional reward package of Items and currency that can be awarded to the active character after combat. This would connect GM encounter preparation to Inventory bookkeeping without changing Adventure's existing direct Item-reward behavior.
- **Combat history browser:** Add an Initiative screen for browsing the completed-encounter history records that are already saved on explicit End + Save History. Players and GMs could review encounter date, rounds, party state and surviving opponents directly on the Flipper instead of treating history as storage-only data.

## 3.9 — Adventure and Journal quality-of-life

- **Adventure bookmarks and favorites:** Allow campaigns or specific scenes to be marked for quick access, with a short recent-campaign list. This would make returning to frequently played campaigns or useful scenes faster than browsing the full campaign list each time.
- **Adventure recap:** Add a compact summary of the active campaign, current scene, recent major choices and unresolved objectives. Players returning after a long break could regain context without rereading every previous scene.
- **Campaign progress viewer:** Expose the active campaign's recorded quest flags, achievements, checkpoints and completed milestone/reward markers in a readable progress screen. Adventure already uses that state to control branching and one-shot rewards; making it visible would help players understand what their character has actually accomplished.
- **Journal search and filters:** Add text search and filters for category, completion state, date range and milestone/item entries. The Journal would remain useful as a long-term campaign record even after it contains many sessions of notes.
- **Journal reference links:** Allow a Journal entry to store optional links to an owned Item, Spell, Feature, Bestiary monster or saved encounter and open that referenced record directly. This would extend the current specific Milestone/Inventory actions into a general cross-reference system.
- **Session log helper:** Add a structured Journal template with date, party state, milestones, important NPCs/monsters, loot and free-form notes. It would make consistent session summaries quick to create while ordinary free-form entries remain available.
- **Campaign reward preview:** Show upcoming explicit campaign rewards and selectable reward choices before they are committed to character state. This would make Item/spell/other reward application easier to review when a campaign scene offers several outcomes.

## 4.0 — Broader rules and customization

- **Ruleset profiles:** Allow an optional per-character ruleset selection when materially different compatible rule families cannot be derived safely from existing character data. This would support multiple D&D-era/rules variants without mixing incompatible calculations inside one character.
- **Custom progression packs:** Support documented external metadata for homebrew classes, subclasses, species, backgrounds, level choices and deterministic grants without recompiling the FAP. The parser would retain bounded streaming so custom progression content does not require loading an entire pack into memory.
- **External Spell and Item catalogs:** Allow optional user-supplied Spell and Item catalog files to be merged with the bundled catalogs using documented row formats. Homebrew content could then appear in the same filtered pickers without replacing built-in SRD entries or manually typing every record.
- **Pack library and controls:** Add a reliable user-facing screen for discovering installed campaign/monster packs and enabling, disabling or inspecting them without editing files manually. It should build on the existing pack formats while keeping normal Adventure and Bestiary menus focused on play rather than diagnostics.
- **User-defined quick actions:** Let each character choose a small set of Home shortcuts tailored to how that character is played. A martial character could prioritize weapon attacks and Features while a caster could prioritize Spellbook, Concentration and spell resources.
- **Character-to-character transfer:** Allow Inventory Items or currency to be safely moved or copied between local characters. Party loot sharing and equipment handoffs would no longer require manually recreating the same record in another profile.
- **Exportable session summary:** Generate a compact text summary containing selected character state, recent Journal/session information and encounter results. The summary could be archived or shared outside the Flipper without exposing unrelated save internals.
