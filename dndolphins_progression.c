#include "dndolphins_progression.h"

#include <string.h>

static uint8_t pocket_progression_cantrip_minimum(const PocketClassLevel* level) {
    if(!level || !level->name[0]) return 0U;
    uint8_t class_level = level->level ? level->level : 1U;
    uint8_t base = 0U;
    if(!strcmp(level->name, "Bard"))
        base = 2U;
    else if(!strcmp(level->name, "Cleric"))
        base = 3U;
    else if(!strcmp(level->name, "Druid"))
        base = 2U;
    else if(!strcmp(level->name, "Sorcerer"))
        base = 4U;
    else if(!strcmp(level->name, "Warlock"))
        base = 2U;
    else if(!strcmp(level->name, "Wizard"))
        base = 3U;
    else
        return 0U;
    if(class_level >= 4U) ++base;
    if(class_level >= 10U) ++base;
    return base;
}


static uint8_t pocket_progression_prepared_limit(const PocketClassLevel* level) {
    if(!level || !level->name[0]) return 0U;
    static const uint8_t full_prepared[20] = {
        4U,5U,6U,7U,9U,10U,11U,12U,14U,15U,16U,16U,17U,17U,18U,18U,19U,20U,21U,22U};
    static const uint8_t sorcerer_prepared[20] = {
        2U,4U,6U,7U,9U,10U,11U,12U,14U,15U,16U,16U,17U,17U,18U,18U,19U,20U,21U,22U};
    static const uint8_t warlock_prepared[20] = {
        2U,3U,4U,5U,6U,7U,8U,9U,10U,10U,11U,11U,12U,12U,13U,13U,14U,14U,15U,15U};
    static const uint8_t wizard_prepared[20] = {
        4U,5U,6U,7U,9U,10U,11U,12U,14U,15U,16U,16U,17U,18U,19U,21U,22U,23U,24U,25U};
    static const uint8_t half_prepared[20] = {
        2U,3U,4U,5U,6U,6U,7U,7U,9U,9U,10U,10U,11U,11U,12U,12U,14U,14U,15U,15U};
    uint8_t class_level = level->level ? level->level : 1U;
    if(class_level > 20U) class_level = 20U;
    uint8_t index = (uint8_t)(class_level - 1U);
    if(!strcmp(level->name, "Bard") || !strcmp(level->name, "Cleric") ||
       !strcmp(level->name, "Druid"))
        return full_prepared[index];
    if(!strcmp(level->name, "Sorcerer")) return sorcerer_prepared[index];
    if(!strcmp(level->name, "Warlock")) return warlock_prepared[index];
    if(!strcmp(level->name, "Wizard")) return wizard_prepared[index];
    if(!strcmp(level->name, "Paladin") || !strcmp(level->name, "Ranger"))
        return half_prepared[index];
    return 0U;
}

static void pocket_progression_preserve_spent_u8(
    uint8_t* current, uint8_t* maximum, uint8_t target) {
    uint8_t old_max = *maximum;
    uint8_t old_current = *current > old_max ? old_max : *current;
    uint8_t spent = old_max >= old_current ? (uint8_t)(old_max - old_current) : 0U;
    *maximum = target;
    *current = target > spent ? (uint8_t)(target - spent) : 0U;
}

static void pocket_progression_preserve_spent_u16(
    uint16_t* current, uint16_t* maximum, uint16_t target) {
    uint16_t old_max = *maximum;
    uint16_t old_current = *current > old_max ? old_max : *current;
    uint16_t spent = old_max >= old_current ? (uint16_t)(old_max - old_current) : 0U;
    *maximum = target;
    *current = target > spent ? (uint16_t)(target - spent) : 0U;
}

static uint8_t pocket_progression_shared_caster_level(
    const PocketCharacter* character, bool* has_shared, bool* has_custom) {
    uint8_t caster_level = 0U;
    if(has_shared) *has_shared = false;
    if(has_custom) *has_custom = false;
    for(uint8_t i = 0U; i < character->class_count && i < POCKET_D20_MAX_CLASSES; ++i) {
        const PocketClassLevel* level = &character->classes[i];
        if(level->spellcasting_mode == PocketSpellcastingCustom) {
            if(has_custom) *has_custom = true;
            continue;
        }
        if(level->spellcasting_mode == PocketSpellcastingFull) {
            if(has_shared) *has_shared = true;
            caster_level += level->level;
        } else if(level->spellcasting_mode == PocketSpellcastingHalf) {
            if(has_shared) *has_shared = true;
            caster_level += (uint8_t)((level->level + 1U) / 2U);
        } else if(level->spellcasting_mode == PocketSpellcastingThird) {
            if(has_shared) *has_shared = true;
            caster_level += (uint8_t)(level->level / 3U);
        }
    }
    return caster_level > 20U ? 20U : caster_level;
}

static void pocket_progression_sync_shared_slots(PocketCharacter* character) {
    static const uint8_t slots[20][9] = {
        {2,0,0,0,0,0,0,0,0}, {3,0,0,0,0,0,0,0,0}, {4,2,0,0,0,0,0,0,0},
        {4,3,0,0,0,0,0,0,0}, {4,3,2,0,0,0,0,0,0}, {4,3,3,0,0,0,0,0,0},
        {4,3,3,1,0,0,0,0,0}, {4,3,3,2,0,0,0,0,0}, {4,3,3,3,1,0,0,0,0},
        {4,3,3,3,2,0,0,0,0}, {4,3,3,3,2,1,0,0,0}, {4,3,3,3,2,1,0,0,0},
        {4,3,3,3,2,1,1,0,0}, {4,3,3,3,2,1,1,0,0}, {4,3,3,3,2,1,1,1,0},
        {4,3,3,3,2,1,1,1,0}, {4,3,3,3,2,1,1,1,1}, {4,3,3,3,3,1,1,1,1},
        {4,3,3,3,3,2,1,1,1}, {4,3,3,3,3,2,2,1,1},
    };
    bool has_shared = false, has_custom = false;
    uint8_t caster_level =
        pocket_progression_shared_caster_level(character, &has_shared, &has_custom);
    if(!has_shared || has_custom) return;
    character->spell_slots_current[0] = 0U;
    character->spell_slots_max[0] = 0U;
    for(uint8_t slot = 1U; slot < POCKET_D20_SLOT_COUNT; ++slot) {
        uint8_t target = caster_level ? slots[caster_level - 1U][slot - 1U] : 0U;
        pocket_progression_preserve_spent_u8(
            &character->spell_slots_current[slot], &character->spell_slots_max[slot], target);
    }
}

static void pocket_progression_sync_pact(PocketClassLevel* level) {
    static const uint8_t pact_slots[20] = {
        1U,2U,2U,2U,2U,2U,2U,2U,2U,2U,3U,3U,3U,3U,3U,3U,4U,4U,4U,4U};
    static const uint8_t pact_levels[20] = {
        1U,1U,2U,2U,3U,3U,4U,4U,5U,5U,5U,5U,5U,5U,5U,5U,5U,5U,5U,5U};
    uint8_t class_level = level->level ? level->level : 1U;
    if(class_level > 20U) class_level = 20U;
    uint8_t target_slots = pact_slots[class_level - 1U];
    pocket_progression_preserve_spent_u8(
        &level->pact_slots_current, &level->pact_slots_max, target_slots);
    level->pact_slot_level = pact_levels[class_level - 1U];
    uint16_t arcanum = 0U;
    if(class_level >= 11U) arcanum |= (uint16_t)(1U << 6U);
    if(class_level >= 13U) arcanum |= (uint16_t)(1U << 7U);
    if(class_level >= 15U) arcanum |= (uint16_t)(1U << 8U);
    if(class_level >= 17U) arcanum |= (uint16_t)(1U << 9U);
    level->mystic_arcanum_mask |= arcanum;
}

bool pocket_d20_progression_sync_resources(PocketCharacter* character) {
    if(!character) return false;
    bool changed = false;
    uint8_t total_hit_dice = 0U;
    for(uint8_t i = 0U; i < character->class_count && i < POCKET_D20_MAX_CLASSES; ++i) {
        PocketClassLevel* level = &character->classes[i];
        uint8_t class_level = level->level ? level->level : 1U;
        if(class_level > 20U) class_level = 20U;
        uint8_t old_hd_max = level->hit_dice_max;
        uint8_t old_hd_current = level->hit_dice_current;
        pocket_progression_preserve_spent_u8(
            &level->hit_dice_current, &level->hit_dice_max, class_level);
        if(old_hd_max != level->hit_dice_max || old_hd_current != level->hit_dice_current)
            changed = true;
        total_hit_dice = (uint8_t)(total_hit_dice + class_level);

        uint8_t cantrip_target = pocket_progression_cantrip_minimum(level);
        if(cantrip_target && level->cantrip_limit != cantrip_target) {
            level->cantrip_limit = cantrip_target;
            changed = true;
        }
        uint8_t prepared_target = pocket_progression_prepared_limit(level);
        if(prepared_target && level->prepared_limit != prepared_target) {
            level->prepared_limit = prepared_target;
            changed = true;
        }
        if(!strcmp(level->name, "Wizard")) {
            uint16_t minimum_book = (uint16_t)(6U + (2U * (class_level - 1U)));
            if(level->spellbook_size < minimum_book) {
                level->spellbook_size = minimum_book;
                changed = true;
            }
        }
        if(!strcmp(level->name, "Sorcerer")) {
            uint16_t target_points = class_level >= 2U ? class_level : 0U;
            uint16_t old_max = level->spell_points_max;
            uint16_t old_current = level->spell_points_current;
            pocket_progression_preserve_spent_u16(
                &level->spell_points_current, &level->spell_points_max, target_points);
            if(old_max != level->spell_points_max || old_current != level->spell_points_current)
                changed = true;
        }
        if(level->spellcasting_mode == PocketSpellcastingPact || !strcmp(level->name, "Warlock")) {
            uint8_t old_slot_level = level->pact_slot_level;
            uint8_t old_slots_max = level->pact_slots_max;
            uint8_t old_slots_current = level->pact_slots_current;
            uint16_t old_arcanum = level->mystic_arcanum_mask;
            pocket_progression_sync_pact(level);
            if(old_slot_level != level->pact_slot_level || old_slots_max != level->pact_slots_max ||
               old_slots_current != level->pact_slots_current ||
               old_arcanum != level->mystic_arcanum_mask)
                changed = true;
        }
    }
    if(total_hit_dice > 20U) total_hit_dice = 20U;
    uint8_t old_total_hd_max = character->hit_dice_max;
    uint8_t old_total_hd_current = character->hit_dice_current;
    pocket_progression_preserve_spent_u8(
        &character->hit_dice_current, &character->hit_dice_max, total_hit_dice);
    if(old_total_hd_max != character->hit_dice_max || old_total_hd_current != character->hit_dice_current)
        changed = true;

    uint8_t old_slot_current[POCKET_D20_SLOT_COUNT];
    uint8_t old_slot_max[POCKET_D20_SLOT_COUNT];
    memcpy(old_slot_current, character->spell_slots_current, sizeof(old_slot_current));
    memcpy(old_slot_max, character->spell_slots_max, sizeof(old_slot_max));
    pocket_progression_sync_shared_slots(character);
    if(memcmp(old_slot_current, character->spell_slots_current, sizeof(old_slot_current)) ||
       memcmp(old_slot_max, character->spell_slots_max, sizeof(old_slot_max)))
        changed = true;
    return changed;
}
