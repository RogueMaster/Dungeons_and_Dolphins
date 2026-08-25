#include "pocket_d20_rules.h"

#include <furi_hal_random.h>

const char* const pocket_d20_ability_names[POCKET_D20_ABILITY_COUNT] = {
    "STR", "DEX", "CON", "INT", "WIS", "CHA"};

const char* const pocket_d20_skill_names[POCKET_D20_SKILL_COUNT] = {
    "Acrobatics",
    "Animal Handling",
    "Arcana",
    "Athletics",
    "Deception",
    "History",
    "Insight",
    "Intimidation",
    "Investigation",
    "Medicine",
    "Nature",
    "Perception",
    "Performance",
    "Persuasion",
    "Religion",
    "Sleight of Hand",
    "Stealth",
    "Survival",
};

const uint8_t pocket_d20_skill_abilities[POCKET_D20_SKILL_COUNT] = {
    PocketAbilityDexterity,
    PocketAbilityWisdom,
    PocketAbilityIntelligence,
    PocketAbilityStrength,
    PocketAbilityCharisma,
    PocketAbilityIntelligence,
    PocketAbilityWisdom,
    PocketAbilityCharisma,
    PocketAbilityIntelligence,
    PocketAbilityWisdom,
    PocketAbilityIntelligence,
    PocketAbilityWisdom,
    PocketAbilityCharisma,
    PocketAbilityCharisma,
    PocketAbilityIntelligence,
    PocketAbilityDexterity,
    PocketAbilityDexterity,
    PocketAbilityWisdom,
};

const char* const pocket_d20_damage_names[PocketDamageTypeCount] = {
    "Bludgeoning",
    "Piercing",
    "Slashing",
    "Acid",
    "Cold",
    "Fire",
    "Force",
    "Lightning",
    "Necrotic",
    "Poison",
    "Psychic",
    "Radiant",
    "Thunder",
};

const char* const pocket_d20_journal_category_names[PocketJournalCategoryCount] = {
    "Quick", "Adventure", "Item", "Milestone"};

static uint8_t pocket_d20_roll_one(uint8_t sides) {
    if(sides < 2U) return 0U;
    return (uint8_t)((furi_hal_random_get() % sides) + 1U);
}

int8_t pocket_d20_ability_modifier(int8_t score) {
    int16_t delta = (int16_t)score - 10;
    if(delta >= 0) return (int8_t)(delta / 2);
    return (int8_t)-(((-delta) + 1) / 2);
}

uint8_t pocket_d20_total_level(const PocketCharacter* character) {
    uint8_t level = 0U;
    for(uint8_t i = 0U; i < character->class_count && i < POCKET_D20_MAX_CLASSES; ++i) {
        level += character->classes[i].level;
    }
    if(level < 1U) return 1U;
    if(level > 20U) return 20U;
    return level;
}

uint8_t pocket_d20_proficiency_bonus(const PocketCharacter* character) {
    return (uint8_t)(2U + ((pocket_d20_total_level(character) - 1U) / 4U));
}

static int8_t pocket_d20_apply_proficiency(
    int8_t base,
    uint8_t proficiency,
    uint8_t bonus) {
    if(proficiency == PocketProficiencyExpertise) return (int8_t)(base + (2 * bonus));
    if(proficiency == PocketProficiencyProficient) return (int8_t)(base + bonus);
    return base;
}

int8_t pocket_d20_saving_throw_modifier(const PocketCharacter* character, uint8_t ability) {
    if(ability >= POCKET_D20_ABILITY_COUNT) return 0;
    int8_t base = pocket_d20_ability_modifier(character->ability_scores[ability]);
    return pocket_d20_apply_proficiency(
        base,
        character->saving_throw_proficiency[ability],
        pocket_d20_proficiency_bonus(character));
}

int8_t pocket_d20_skill_modifier(const PocketCharacter* character, uint8_t skill) {
    if(skill >= POCKET_D20_SKILL_COUNT) return 0;
    uint8_t ability = pocket_d20_skill_abilities[skill];
    int8_t base = pocket_d20_ability_modifier(character->ability_scores[ability]);
    return pocket_d20_apply_proficiency(
        base,
        character->skill_proficiency[skill],
        pocket_d20_proficiency_bonus(character));
}

int8_t pocket_d20_initiative_modifier(const PocketCharacter* character) {
    return (int8_t)(pocket_d20_ability_modifier(
                        character->ability_scores[PocketAbilityDexterity]) +
                    character->initiative_misc);
}

int8_t pocket_d20_spell_attack_modifier(const PocketCharacter* character) {
    uint8_t ability = character->spellcasting_ability;
    if(ability >= POCKET_D20_ABILITY_COUNT) ability = PocketAbilityIntelligence;
    return (int8_t)(pocket_d20_ability_modifier(character->ability_scores[ability]) +
                    pocket_d20_proficiency_bonus(character) + character->spell_attack_misc);
}

int8_t pocket_d20_spell_save_dc(const PocketCharacter* character) {
    uint8_t ability = character->spellcasting_ability;
    if(ability >= POCKET_D20_ABILITY_COUNT) ability = PocketAbilityIntelligence;
    return (int8_t)(8 + pocket_d20_ability_modifier(character->ability_scores[ability]) +
                    pocket_d20_proficiency_bonus(character) + character->spell_save_misc);
}

int8_t pocket_d20_weapon_ability(const PocketCharacter* character, const PocketItem* item) {
    int8_t strength = pocket_d20_ability_modifier(
        character->ability_scores[PocketAbilityStrength]);
    int8_t dexterity = pocket_d20_ability_modifier(
        character->ability_scores[PocketAbilityDexterity]);
    switch(item->attack_ability) {
    case PocketAttackAbilityStrength:
        return strength;
    case PocketAttackAbilityDexterity:
        return dexterity;
    case PocketAttackAbilityBest:
        return strength > dexterity ? strength : dexterity;
    default:
        if(item->weapon_properties & PocketWeaponRanged) return dexterity;
        if(item->weapon_properties & PocketWeaponFinesse)
            return strength > dexterity ? strength : dexterity;
        return strength;
    }
}

int8_t pocket_d20_weapon_attack_modifier(
    const PocketCharacter* character,
    const PocketItem* item) {
    int8_t result = pocket_d20_weapon_ability(character, item) + item->magic_bonus;
    if(item->proficient) result += pocket_d20_proficiency_bonus(character);
    return result;
}

uint16_t pocket_d20_roll_dice(uint8_t count, uint8_t sides) {
    uint16_t total = 0;
    for(uint8_t i = 0; i < count; ++i) total += pocket_d20_roll_one(sides);
    return total;
}

PocketAttackRoll pocket_d20_roll_attack(
    const PocketCharacter* character,
    const PocketItem* item,
    PocketRollMode mode) {
    PocketAttackRoll result = {0};
    result.first_die = pocket_d20_roll_one(20U);
    result.natural_roll = result.first_die;
    if(mode != PocketRollNormal) {
        result.second_die = pocket_d20_roll_one(20U);
        if(mode == PocketRollAdvantage) {
            if(result.second_die > result.natural_roll) result.natural_roll = result.second_die;
        } else if(result.second_die < result.natural_roll) {
            result.natural_roll = result.second_die;
        }
    }
    result.modifier = pocket_d20_weapon_attack_modifier(character, item);
    result.total = (int16_t)result.natural_roll + result.modifier;
    result.critical = result.natural_roll == 20U;
    result.automatic_miss = result.natural_roll == 1U;
    return result;
}

PocketDamageRoll pocket_d20_roll_damage(
    const PocketCharacter* character,
    const PocketItem* item,
    bool critical) {
    PocketDamageRoll result = {0};
    uint8_t multiplier = critical ? 2U : 1U;
    uint8_t die = item->damage_die;
    if(item->use_versatile && item->versatile_die >= 2U) die = item->versatile_die;
    result.weapon_total = (int16_t)pocket_d20_roll_dice(item->damage_dice * multiplier, die);
    result.extra_total =
        (int16_t)pocket_d20_roll_dice(item->extra_dice * multiplier, item->extra_die);
    result.modifier = item->magic_bonus;
    if(item->add_ability_damage) result.modifier += pocket_d20_weapon_ability(character, item);
    result.total = result.weapon_total + result.extra_total + result.modifier;
    if(result.total < 0) result.total = 0;
    result.critical = critical;
    return result;
}

void pocket_d20_short_rest(PocketCharacter* character) {
    character->death_successes = 0;
    character->death_failures = 0;
}

void pocket_d20_long_rest(PocketCharacter* character) {
    character->hp_current = character->hp_max;
    character->hp_temporary = 0;
    character->death_successes = 0;
    character->death_failures = 0;
    character->hit_dice_current = character->hit_dice_max;
    for(uint8_t i = 1U; i < POCKET_D20_SLOT_COUNT; ++i) {
        character->spell_slots_current[i] = character->spell_slots_max[i];
    }
    for(uint8_t i = 0; i < character->feature_count; ++i) {
        character->features[i].uses_current = character->features[i].uses_max;
    }
}
