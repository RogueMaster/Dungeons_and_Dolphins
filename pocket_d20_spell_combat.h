#pragma once

#include "pocket_d20.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    PocketSpellResolutionNone,
    PocketSpellResolutionAttack,
    PocketSpellResolutionSave,
    PocketSpellResolutionAutomatic,
    PocketSpellResolutionTriggered,
} PocketSpellResolution;

typedef struct {
    uint8_t primary_dice;
    uint8_t primary_die;
    uint8_t secondary_dice;
    uint8_t secondary_die;
    int16_t flat_bonus;
    uint8_t resolution;
} PocketSpellDamageSpec;

bool pocket_d20_spell_damage_spec(
    const PocketSpell* spell,
    uint8_t cast_level,
    uint8_t character_level,
    int8_t spellcasting_modifier,
    PocketSpellDamageSpec* output);
