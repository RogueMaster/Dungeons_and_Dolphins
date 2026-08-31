#pragma once

#include "dnd_rules.h"
#include "dnd_storage.h"

#include <stdbool.h>
#include <stdint.h>

/* DNDInventory-only initialization. Merely opening DNDolphins never creates or
   seeds an Inventory sidecar. */
bool dndinventory_items_initialize_inventory(
    Storage* storage,
    uint32_t profile,
    PocketSaveData* data,
    bool* initialized);
bool dndinventory_items_regrant_inventory_once(
    Storage* storage,
    uint32_t profile,
    PocketSaveData* data,
    bool* regranted);

int16_t dndinventory_rules_carrying_capacity(const PocketCharacter* character);
void dndinventory_rules_normalize_currency(PocketCharacter* character);
int16_t dndinventory_rules_calculated_armor_class(
    const PocketCharacter* character,
    const PocketD20ItemAggregate* aggregate);
