#pragma once

#include "dnd_weapon_rules.h"
#include "dnd_storage.h"

#include <stdbool.h>
#include <stdint.h>

PocketAttackRoll dndolphins_weapon_combat_roll_attack(
    const PocketCharacter* character,
    const PocketItem* item,
    PocketRollMode mode);
PocketDamageRoll dndolphins_weapon_combat_roll_damage(
    const PocketCharacter* character,
    const PocketItem* item,
    bool critical);
bool dndolphins_weapon_combat_items_collect_weapon_indices(
    Storage* storage,
    uint32_t profile,
    uint8_t* indices,
    uint8_t capacity,
    uint8_t* count,
    uint8_t* total_count);
