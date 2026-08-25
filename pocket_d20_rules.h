#pragma once

#include "pocket_d20.h"

typedef enum {
    PocketRollNormal,
    PocketRollAdvantage,
    PocketRollDisadvantage,
} PocketRollMode;

typedef struct {
    uint8_t first_die;
    uint8_t second_die;
    int16_t modifier;
    int16_t total;
    uint8_t natural_roll;
    uint8_t critical;
    uint8_t automatic_miss;
} PocketAttackRoll;

typedef struct {
    int16_t weapon_total;
    int16_t extra_total;
    int16_t modifier;
    int16_t total;
    uint8_t critical;
} PocketDamageRoll;

extern const char* const pocket_d20_ability_names[POCKET_D20_ABILITY_COUNT];
extern const char* const pocket_d20_skill_names[POCKET_D20_SKILL_COUNT];
extern const uint8_t pocket_d20_skill_abilities[POCKET_D20_SKILL_COUNT];
extern const char* const pocket_d20_damage_names[PocketDamageTypeCount];
extern const char* const pocket_d20_journal_category_names[PocketJournalCategoryCount];

int8_t pocket_d20_ability_modifier(int8_t score);
uint8_t pocket_d20_total_level(const PocketCharacter* character);
uint8_t pocket_d20_proficiency_bonus(const PocketCharacter* character);
int8_t pocket_d20_saving_throw_modifier(const PocketCharacter* character, uint8_t ability);
int8_t pocket_d20_skill_modifier(const PocketCharacter* character, uint8_t skill);
int8_t pocket_d20_initiative_modifier(const PocketCharacter* character);
int8_t pocket_d20_spell_attack_modifier(const PocketCharacter* character);
int8_t pocket_d20_spell_save_dc(const PocketCharacter* character);
int8_t pocket_d20_weapon_ability(const PocketCharacter* character, const PocketItem* item);
int8_t pocket_d20_weapon_attack_modifier(
    const PocketCharacter* character,
    const PocketItem* item);

uint16_t pocket_d20_roll_dice(uint8_t count, uint8_t sides);
PocketAttackRoll pocket_d20_roll_attack(
    const PocketCharacter* character,
    const PocketItem* item,
    PocketRollMode mode);
PocketDamageRoll pocket_d20_roll_damage(
    const PocketCharacter* character,
    const PocketItem* item,
    bool critical);

void pocket_d20_short_rest(PocketCharacter* character);
void pocket_d20_long_rest(PocketCharacter* character);

