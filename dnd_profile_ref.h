#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <storage/storage.h>

/* Read only DNDolphins' persisted Active= ID from custom_active_profile.txt.
   This tiny shared companion reader does not depend on dnd_storage.c and does
   not scan, infer, validate, or switch characters. */
bool dnd_profile_ref_active_id(Storage* storage, uint32_t* profile);

/* Resolve only the persisted active character. No cross-character fallback is performed. */
bool dnd_profile_ref_active_exact(Storage* storage, uint32_t* profile);

/* Resolve the canonical primary character file for a profile. Collection files
   such as inventory_<id>.txt and spellbook_<id>.txt are deliberately excluded. */
bool dnd_profile_ref_path(Storage* storage, uint32_t profile, char* output, size_t size);

/* True only when this exact profile ID has a canonical primary character file. */
bool dnd_profile_ref_exists(Storage* storage, uint32_t profile);
