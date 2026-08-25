Dungeons & Dolphins translation hooks

Stable UI keys and English fallbacks are listed in en.txt. The source uses the zero-allocation POCKET_D20_TRANSLATE(key, fallback) hook in pocket_d20_i18n.h for top-level navigation and profile actions.

Localized builds may define the hook without adding a runtime translation table. Catalog and campaign text is already externalized in packaged file assets and can be localized independently.
