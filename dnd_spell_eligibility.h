#pragma once

#include "dnd_data.h"

#include <stdbool.h>
#include <stdint.h>

/* Tiny rules surface intentionally shared by DNDolphins progression and
   DNDSpellbook catalog eligibility. */
bool dnd_spell_eligibility_class_uses_wizard_spell_list(const PocketClassLevel* class_level);
uint8_t dnd_spell_eligibility_class_max_spell_level(const PocketClassLevel* class_level);
