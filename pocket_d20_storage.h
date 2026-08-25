#pragma once

#include "pocket_d20.h"

#include <storage/storage.h>

bool pocket_d20_storage_load(Storage* storage, PocketSaveData* data, bool* recovered_backup);
bool pocket_d20_storage_save(Storage* storage, const PocketSaveData* data);

