# Compatibility

- Character files load best-effort by field name. A readable character file is not rejected merely because every body field is unknown to the current build; filename metadata and sanitized defaults remain usable while unknown fields are ignored.
- Standard-array defaults apply only when a new character object is initialized; existing saved ability scores are never replaced by the new defaults.
- Level-progression synchronization derives from existing class/species/level state and writes only existing character/feature/spell fields, so it requires no save migration. Fixed-grant metadata is best-effort and used only by the explicit level-1 **Grant Initial Traits** action or an actual level increase; it is not consulted to accept/reject an existing save.
- Current spell/item sidecars remain authoritative; historical SWD/SHD files are not used as live collection/character state.
- Journal, Adventure, Initiative and Bestiary keep independent storage ownership.
- Pack text is intentionally editable and has no checksum requirement.
- The default Dolphin/Capybara custom seed runs only when no user custom monster files exist, so upgrades do not replace an existing custom pack.
- Ghost Protocol uses the existing campaign schema and therefore requires no progress-file conversion.
- Save structure changes are avoided unless new information truly must be persisted.
