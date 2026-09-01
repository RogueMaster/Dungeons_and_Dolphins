# Accessibility and text presentation

- Long values should scroll or open into a readable detail view rather than being silently truncated when practical.
- Menu labels should be short and consistent; detailed explanations belong in secondary/detail screens.
- Dice/check results must remain understandable without relying on animation alone.
- Adventure failure branches should explain what happened and provide a usable continuation path.
- Avoid color-only meaning; Flipper UI must remain legible in monochrome.
- Preserve Back/long-Back navigation conventions consistently across companion FAPs: Short Back moves up one screen and, from the companion main screen, returns to DNDolphins when installed; Hold Back exits to firmware without a handoff.
- Adventure, Bestiary, Journal, Initiative, Inventory and Spellbook show the active character ID as a compact right-aligned `[id]` marker on the main screen only. Inventory/Spellbook normal lists do not add a persistent paging glyph beside it; explicit catalog/name-selection pages show their own page controls. Editors, details and tools keep the header uncluttered.

- Combat → Rituals uses the same five-row bounded list/navigation pattern as Spell Attacks and gives explicit empty states (`No Ritual Adept` / `No known rituals`) instead of a blank screen.
