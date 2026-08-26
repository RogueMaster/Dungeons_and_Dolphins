# Community translation template

Copy `custom_strings.template.txt` as `custom_strings.txt`, translate values after `=`, and keep keys unchanged. Register it in `custom_index.txt`. Localized builds map the stable keys through `POCKET_D20_TRANSLATE(key, fallback)` in `pocket_d20_i18n.h`. Catalog and campaign packs can be translated separately.

The default build uses compile-time English fallbacks and therefore allocates no translation table on the Flipper heap.
