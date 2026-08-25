#include "pocket_d20.h"

#include <string.h>

static void pocket_d20_copy(char* destination, size_t size, const char* source) {
    if(size == 0U) return;
    strncpy(destination, source, size - 1U);
    destination[size - 1U] = '\0';
}

static uint8_t pocket_d20_clamp_u8(uint8_t value, uint8_t maximum) {
    return value > maximum ? maximum : value;
}

static int16_t pocket_d20_clamp_i16(int16_t value, int16_t minimum, int16_t maximum) {
    if(value < minimum) return minimum;
    if(value > maximum) return maximum;
    return value;
}

void pocket_d20_data_set_defaults(PocketSaveData* data) {
    memset(data, 0, sizeof(*data));
    PocketCharacter* character = &data->character;

    pocket_d20_copy(character->name, sizeof(character->name), "New Hero");
    pocket_d20_copy(character->player, sizeof(character->player), "Player");
    pocket_d20_copy(character->species, sizeof(character->species), "Human");
    pocket_d20_copy(character->background, sizeof(character->background), "Adventurer");
    pocket_d20_copy(character->alignment, sizeof(character->alignment), "Neutral");

    character->class_count = 1U;
    pocket_d20_copy(character->classes[0].name, sizeof(character->classes[0].name), "Fighter");
    pocket_d20_copy(
        character->classes[0].subclass,
        sizeof(character->classes[0].subclass),
        "None");
    character->classes[0].level = 1U;
    character->milestone_leveling = 1U;

    for(uint8_t i = 0U; i < POCKET_D20_ABILITY_COUNT; ++i) {
        character->ability_scores[i] = 10;
    }
    character->ability_scores[PocketAbilityConstitution] = 12;
    character->hp_current = 10;
    character->hp_max = 10;
    character->armor_class = 10;
    character->speed = 30;
    character->hit_die = 10U;
    character->hit_dice_current = 1U;
    character->hit_dice_max = 1U;
    character->spellcasting_ability = PocketAbilityIntelligence;

    character->language_count = 1U;
    pocket_d20_copy(
        character->languages[0], sizeof(character->languages[0]), "Common");

    character->item_count = 1U;
    PocketItem* sword = &character->items[0];
    pocket_d20_copy(sword->name, sizeof(sword->name), "Longsword");
    pocket_d20_copy(
        sword->detail, sizeof(sword->detail), "Versatile martial melee weapon.");
    sword->quantity = 1;
    sword->weight_tenths = 30;
    sword->equipped = 1U;
    sword->is_weapon = 1U;
    sword->attack_ability = PocketAttackAbilityAuto;
    sword->proficient = 1U;
    sword->damage_dice = 1U;
    sword->damage_die = 8U;
    sword->versatile_die = 10U;
    sword->damage_type = PocketDamageSlashing;
    sword->add_ability_damage = 1U;

    character->journal_count = 1U;
    PocketJournalEntry* note = &character->journal[0];
    pocket_d20_copy(note->title, sizeof(note->title), "Welcome");
    pocket_d20_copy(
        note->body,
        sizeof(note->body),
        "Use Journal for adventure notes, item ideas, and milestones.");
    note->category = PocketJournalQuick;

    data->initiative.round = 1U;
}

void pocket_d20_data_sanitize(PocketSaveData* data) {
    PocketCharacter* character = &data->character;

    character->name[sizeof(character->name) - 1U] = '\0';
    character->player[sizeof(character->player) - 1U] = '\0';
    character->species[sizeof(character->species) - 1U] = '\0';
    character->background[sizeof(character->background) - 1U] = '\0';
    character->alignment[sizeof(character->alignment) - 1U] = '\0';
    character->other_proficiencies[sizeof(character->other_proficiencies) - 1U] = '\0';

    character->class_count =
        pocket_d20_clamp_u8(character->class_count, POCKET_D20_MAX_CLASSES);
    if(character->class_count == 0U) character->class_count = 1U;
    for(uint8_t i = 0U; i < POCKET_D20_MAX_CLASSES; ++i) {
        PocketClassLevel* class_level = &character->classes[i];
        class_level->name[sizeof(class_level->name) - 1U] = '\0';
        class_level->subclass[sizeof(class_level->subclass) - 1U] = '\0';
        class_level->level = pocket_d20_clamp_u8(class_level->level, 20U);
    }
    if(character->classes[0].level == 0U) character->classes[0].level = 1U;

    for(uint8_t i = 0U; i < POCKET_D20_ABILITY_COUNT; ++i) {
        character->ability_scores[i] = (int8_t)pocket_d20_clamp_i16(
            character->ability_scores[i], 1, 30);
        character->saving_throw_proficiency[i] = pocket_d20_clamp_u8(
            character->saving_throw_proficiency[i], PocketProficiencyProficient);
        character->saving_throw_misc[i] = (int8_t)pocket_d20_clamp_i16(
            character->saving_throw_misc[i], -20, 20);
    }
    for(uint8_t i = 0U; i < POCKET_D20_SKILL_COUNT; ++i) {
        character->skill_proficiency[i] =
            pocket_d20_clamp_u8(character->skill_proficiency[i], PocketProficiencyExpertise);
        character->skill_misc[i] =
            (int8_t)pocket_d20_clamp_i16(character->skill_misc[i], -20, 20);
    }

    character->hp_max = pocket_d20_clamp_i16(character->hp_max, 1, 999);
    character->hp_current = pocket_d20_clamp_i16(character->hp_current, 0, 999);
    character->hp_temporary = pocket_d20_clamp_i16(character->hp_temporary, 0, 999);
    character->armor_class = pocket_d20_clamp_i16(character->armor_class, 0, 99);
    character->speed = pocket_d20_clamp_i16(character->speed, 0, 255);
    character->initiative_misc =
        (int8_t)pocket_d20_clamp_i16(character->initiative_misc, -20, 20);
    character->exhaustion = pocket_d20_clamp_u8(character->exhaustion, 6U);
    character->death_successes = pocket_d20_clamp_u8(character->death_successes, 3U);
    character->death_failures = pocket_d20_clamp_u8(character->death_failures, 3U);
    if(character->hit_die != 6U && character->hit_die != 8U && character->hit_die != 10U &&
       character->hit_die != 12U)
        character->hit_die = 8U;
    character->spellcasting_ability =
        pocket_d20_clamp_u8(character->spellcasting_ability, PocketAbilityCharisma);
    character->spell_attack_misc =
        (int8_t)pocket_d20_clamp_i16(character->spell_attack_misc, -20, 20);
    character->spell_save_misc =
        (int8_t)pocket_d20_clamp_i16(character->spell_save_misc, -20, 20);
    character->arcane_recovery_used = character->arcane_recovery_used ? 1U : 0U;

    character->spell_count =
        pocket_d20_clamp_u8(character->spell_count, POCKET_D20_MAX_SPELLS);
    character->feature_count =
        pocket_d20_clamp_u8(character->feature_count, POCKET_D20_MAX_FEATURES);
    character->item_count =
        pocket_d20_clamp_u8(character->item_count, POCKET_D20_MAX_ITEMS);
    character->language_count =
        pocket_d20_clamp_u8(character->language_count, POCKET_D20_MAX_LANGUAGES);
    character->journal_count =
        pocket_d20_clamp_u8(character->journal_count, POCKET_D20_MAX_JOURNAL);

    for(uint8_t i = 0U; i < POCKET_D20_MAX_SPELLS; ++i) {
        character->spells[i].name[POCKET_D20_NAME_LEN - 1U] = '\0';
        character->spells[i].detail[POCKET_D20_DETAIL_LEN - 1U] = '\0';
        character->spells[i].level = pocket_d20_clamp_u8(character->spells[i].level, 9U);
        character->spells[i].class_index =
            pocket_d20_clamp_u8(character->spells[i].class_index, POCKET_D20_MAX_CLASSES - 1U);
        character->spells[i].prepared = character->spells[i].prepared ? 1U : 0U;
        character->spells[i].ritual = character->spells[i].ritual ? 1U : 0U;
        character->spell_known[i] = character->spell_known[i] ? 1U : 0U;
        character->spell_always_prepared[i] =
            character->spell_always_prepared[i] ? 1U : 0U;
        character->spell_free_casts_max[i] =
            pocket_d20_clamp_u8(character->spell_free_casts_max[i], 20U);
        character->spell_free_casts_current[i] = pocket_d20_clamp_u8(
            character->spell_free_casts_current[i],
            character->spell_free_casts_max[i]);
    }
    for(uint8_t i = 0U; i < POCKET_D20_MAX_FEATURES; ++i) {
        character->features[i].name[POCKET_D20_NAME_LEN - 1U] = '\0';
        character->features[i].detail[POCKET_D20_DETAIL_LEN - 1U] = '\0';
        character->features[i].class_index = pocket_d20_clamp_u8(
            character->features[i].class_index,
            POCKET_D20_MAX_CLASSES - 1U);
        character->features[i].class_level_gained =
            pocket_d20_clamp_u8(character->features[i].class_level_gained, 20U);
        character->features[i].recharge =
            pocket_d20_clamp_u8(character->features[i].recharge, PocketRechargeCount - 1U);
    }
    for(uint8_t i = 0U; i < POCKET_D20_MAX_ITEMS; ++i) {
        PocketItem* item = &character->items[i];
        item->name[POCKET_D20_NAME_LEN - 1U] = '\0';
        item->detail[POCKET_D20_DETAIL_LEN - 1U] = '\0';
        item->attack_ability =
            pocket_d20_clamp_u8(item->attack_ability, PocketAttackAbilityBest);
        item->damage_type = pocket_d20_clamp_u8(item->damage_type, PocketDamageTypeCount - 1U);
        item->damage_dice = pocket_d20_clamp_u8(item->damage_dice, 20U);
        item->extra_dice = pocket_d20_clamp_u8(item->extra_dice, 20U);
    }
    for(uint8_t i = 0U; i < POCKET_D20_MAX_LANGUAGES; ++i) {
        character->languages[i][POCKET_D20_SHORT_LEN - 1U] = '\0';
    }
    for(uint8_t i = 0U; i < POCKET_D20_MAX_JOURNAL; ++i) {
        PocketJournalEntry* entry = &character->journal[i];
        entry->title[POCKET_D20_NAME_LEN - 1U] = '\0';
        entry->body[POCKET_D20_DETAIL_LEN - 1U] = '\0';
        entry->category =
            pocket_d20_clamp_u8(entry->category, PocketJournalCategoryCount - 1U);
        entry->class_index =
            pocket_d20_clamp_u8(entry->class_index, POCKET_D20_MAX_CLASSES - 1U);
    }

    data->party_count = pocket_d20_clamp_u8(data->party_count, POCKET_D20_MAX_PARTY);
    for(uint8_t i = 0U; i < POCKET_D20_MAX_PARTY; ++i) {
        data->party[i].name[POCKET_D20_SHORT_LEN - 1U] = '\0';
    }
    data->initiative.count =
        pocket_d20_clamp_u8(data->initiative.count, POCKET_D20_MAX_INITIATIVE);
    if(data->initiative.round == 0U) data->initiative.round = 1U;
    if(data->initiative.current_turn >= data->initiative.count)
        data->initiative.current_turn = 0U;
    for(uint8_t i = 0U; i < POCKET_D20_MAX_INITIATIVE; ++i) {
        data->initiative.entries[i].name[POCKET_D20_SHORT_LEN - 1U] = '\0';
    }
}
