# Save schema 1

Schema 1 is the stable text-save contract introduced in Dungeons & Dolphins 0.9. Character filenames remain `ch_{id}_{characterName}_{characterLevel}.txt` and each file contains the entire independent profile.

## Envelope

- The first record is `PocketD20Character=1`.
- Records use ordered `key=value` lines.
- Text uses percent escaping for control bytes, line breaks, carriage returns, and `%`.
- Repeated collections use a count record followed by indexed records.
- `End=OK` closes the canonical payload.
- `FileChecksum` is FNV-1a over every byte from the first record through the newline after `End=OK`; the checksum line itself is excluded.

## Compatibility

- Readers reject unknown schema numbers rather than guessing field meanings.
- The 0.9 reader accepts the pre-freeze v0.8 text number, verifies its legacy checksum, sanitizes all fields, and immediately rewrites it as schema 1.
- Schema 1 files are validated before their ordered records are parsed.
- A failed primary load attempts the temporary backup before a fresh character is created.
- Future schemas must add an explicit migration path and retain a recoverable backup.

## Data groups

The ordered payload covers identity and builder fields; adventure state; multiclass and spellcasting data; abilities, saves, skills, and vitals; currency; spells; features and resources; inventory and weapons; languages; journal and milestones; party presets; active initiative; combat-sheet state; structured grants; attack templates; and encounter history.

## Transfer folders

- `exports/` contains user-transferable complete character files.
- `archive/` contains profiles removed from the active list without deletion.
- Import reads the first valid exported text file, assigns a new profile ID, and writes it through the normal atomic-save path.
