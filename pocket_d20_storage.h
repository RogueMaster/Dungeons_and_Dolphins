#pragma once

#include "pocket_d20.h"

#include <storage/storage.h>

#define POCKET_D20_MAX_PROFILES 6U

typedef struct {
    uint8_t active_profile;
    uint8_t occupied_mask;
    char names[POCKET_D20_MAX_PROFILES][POCKET_D20_NAME_LEN];
} PocketProfileState;

void pocket_d20_profiles_set_defaults(PocketProfileState* profiles);
bool pocket_d20_profiles_load(Storage* storage, PocketProfileState* profiles);
bool pocket_d20_profiles_save(Storage* storage, const PocketProfileState* profiles);

bool pocket_d20_storage_load_profile(
    Storage* storage,
    uint8_t profile,
    PocketSaveData* data,
    bool* recovered_backup);
bool pocket_d20_storage_save_profile(
    Storage* storage,
    uint8_t profile,
    const PocketSaveData* data);
bool pocket_d20_storage_delete_profile(Storage* storage, uint8_t profile);
