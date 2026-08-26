Dungeons & Dolphins translation hooks

Stable UI keys and English fallbacks are listed in en.txt. The source uses the zero-allocation POCKET_D20_TRANSLATE(key, fallback) hook in pocket_d20_i18n.h for top-level navigation and profile actions.

Localized builds may define the hook without adding a runtime translation table. Optional runtime packs are registered in `custom_index.txt` as `id|display_name|custom_filename` and use `key=value` lines. The runtime loader enforces a 2 KiB heap ceiling, translates navigation and structured-editor labels, and falls back to compiled English for every missing key. User packs belong under `/ext/apps_assets/dungeons_and_dolphins/i18n/`, and their filenames must start with `custom_`. Catalog and campaign text remains externalized independently.
