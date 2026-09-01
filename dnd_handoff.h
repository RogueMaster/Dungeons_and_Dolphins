#pragma once

#include <stdbool.h>
#include <stddef.h>

/* Shared cross-FAP launch contract. Every Loader handoff uses an explicit,
   absolute FAP path; app IDs are manifest identifiers only and are never used
   as launch targets. DNDolphins passes a decimal character ID to companion
   FAPs. DNDBestiary transfers monsters directly to DNDInitiative:
   CharacterId;Name,HP,AC,InitiativeMod;Name,HP,AC,InitiativeMod;... */
#define POCKET_D20_HANDOFF_LAUNCH_ARG    "initiative"
#define POCKET_D20_HANDOFF_LAUNCH_PREFIX "initiative;"
#define POCKET_D20_HANDOFF_ADVENTURE_CONTINUE ";continue"
#define POCKET_D20_RETURN_FOCUS_INVENTORY  "focus=inventory"
#define POCKET_D20_RETURN_FOCUS_SPELLBOOK  "focus=spellbook"
#define POCKET_D20_RETURN_FOCUS_ADVENTURE  "focus=adventure"
#define POCKET_D20_RETURN_FOCUS_JOURNAL    "focus=journal"
#define POCKET_D20_RETURN_FOCUS_INITIATIVE "focus=initiative"
#define POCKET_D20_RETURN_FOCUS_BESTIARY   "focus=bestiary"
#define DNDOLPHINS_FAP_PATH               "/ext/apps/Games/dndolphins.fap"
#define DNDJOURNAL_FAP_PATH               "/ext/apps/Games/dndjournal.fap"
#define DNDADVENTURE_FAP_PATH             "/ext/apps/Games/dndadventure.fap"
#define DNDINITIATIVE_FAP_PATH            "/ext/apps/Games/dndinitiative.fap"
#define DNDINVENTORY_FAP_PATH             "/ext/apps/Games/dndinventory.fap"
#define DNDSPELLBOOK_FAP_PATH             "/ext/apps/Games/dndspellbook.fap"
#define DNDBESTIARY_FAP_PATH              "/ext/apps/Games/dndbestiary.fap"
#define POCKET_D20_LAUNCH_ARGS_MAX       1536U
#define POCKET_D20_TRANSFER_MAX          23U
#define POCKET_D20_CHARACTER_DATA_ROOT   "/ext/apps_data/dndolphins"
#define POCKET_D20_JOURNAL_DATA_ROOT     "/ext/apps_data/dndjournal"

/* Launches an already-known absolute FAP path after the caller has torn down
   its app state. Loader is intentionally isolated to dnd_handoff.c. */
bool dnd_handoff_launch(const char* fap_path, const char* args);
/* Best-effort return target: launch only when the target FAP exists. */
bool dnd_handoff_launch_if_present(const char* fap_path, const char* args);
