#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define POCKET_D20_SAVE_VERSION 3U

#define POCKET_D20_NAME_LEN 32U
#define POCKET_D20_SHORT_LEN 24U
#define POCKET_D20_DETAIL_LEN 192U

#define POCKET_D20_MAX_SPELLS 24U
#define POCKET_D20_MAX_CLASSES 4U
#define POCKET_D20_MAX_FEATURES 20U
#define POCKET_D20_MAX_ITEMS 24U
#define POCKET_D20_MAX_LANGUAGES 12U
#define POCKET_D20_MAX_JOURNAL 24U
#define POCKET_D20_MAX_PARTY 16U
#define POCKET_D20_MAX_INITIATIVE 24U

#define POCKET_D20_SKILL_COUNT 18U
#define POCKET_D20_ABILITY_COUNT 6U
#define POCKET_D20_SLOT_COUNT 10U

typedef enum {
    PocketAbilityStrength,
    PocketAbilityDexterity,
    PocketAbilityConstitution,
    PocketAbilityIntelligence,
    PocketAbilityWisdom,
    PocketAbilityCharisma,
} PocketAbility;

typedef enum {
    PocketProficiencyNone,
    PocketProficiencyProficient,
    PocketProficiencyExpertise,
} PocketProficiency;

typedef enum {
    PocketRechargeManual,
    PocketRechargeShortOrLong,
    PocketRechargeLong,
    PocketRechargeCount,
} PocketRecharge;

typedef enum {
    PocketAttackAbilityAuto,
    PocketAttackAbilityStrength,
    PocketAttackAbilityDexterity,
    PocketAttackAbilityBest,
} PocketAttackAbility;

typedef enum {
    PocketDamageBludgeoning,
    PocketDamagePiercing,
    PocketDamageSlashing,
    PocketDamageAcid,
    PocketDamageCold,
    PocketDamageFire,
    PocketDamageForce,
    PocketDamageLightning,
    PocketDamageNecrotic,
    PocketDamagePoison,
    PocketDamagePsychic,
    PocketDamageRadiant,
    PocketDamageThunder,
    PocketDamageTypeCount,
} PocketDamageType;

typedef enum {
    PocketJournalQuick,
    PocketJournalAdventure,
    PocketJournalItem,
    PocketJournalMilestone,
    PocketJournalCategoryCount,
} PocketJournalCategory;

enum {
    PocketWeaponFinesse = 1U << 0,
    PocketWeaponRanged = 1U << 1,
    PocketWeaponLight = 1U << 2,
    PocketWeaponHeavy = 1U << 3,
    PocketWeaponThrown = 1U << 4,
    PocketWeaponAmmunition = 1U << 5,
};

typedef struct {
    char name[POCKET_D20_NAME_LEN];
    char subclass[POCKET_D20_NAME_LEN];
    uint8_t level;
} PocketClassLevel;

typedef struct {
    char name[POCKET_D20_NAME_LEN];
    char detail[POCKET_D20_DETAIL_LEN];
    uint8_t level;
    uint8_t class_index;
    uint8_t prepared;
    uint8_t ritual;
} PocketSpell;

typedef struct {
    char name[POCKET_D20_NAME_LEN];
    char detail[POCKET_D20_DETAIL_LEN];
    int16_t uses_current;
    int16_t uses_max;
    uint8_t class_index;
    uint8_t class_level_gained;
    uint8_t recharge;
} PocketFeature;

typedef struct {
    char name[POCKET_D20_NAME_LEN];
    char detail[POCKET_D20_DETAIL_LEN];
    int16_t quantity;
    int16_t weight_tenths;
    uint8_t equipped;
    uint8_t attuned;
    uint8_t is_weapon;
    uint8_t attack_ability;
    uint8_t proficient;
    int8_t magic_bonus;
    uint8_t damage_dice;
    uint8_t damage_die;
    uint8_t versatile_die;
    uint8_t use_versatile;
    uint8_t damage_type;
    uint8_t add_ability_damage;
    uint8_t extra_dice;
    uint8_t extra_die;
    uint16_t weapon_properties;
    int16_t ammo_current;
    int16_t ammo_max;
} PocketItem;

typedef struct {
    char title[POCKET_D20_NAME_LEN];
    char body[POCKET_D20_DETAIL_LEN];
    uint8_t category;
    uint8_t completed;
    uint8_t level_granted;
    uint8_t class_index;
} PocketJournalEntry;

typedef struct {
    char name[POCKET_D20_SHORT_LEN];
    int8_t initiative_modifier;
} PocketPartyMember;

typedef struct {
    char name[POCKET_D20_SHORT_LEN];
    int8_t initiative_modifier;
    int16_t initiative_total;
    uint8_t is_player_character;
} PocketInitiativeEntry;

typedef struct {
    uint8_t active;
    uint16_t round;
    uint8_t current_turn;
    uint8_t count;
    PocketInitiativeEntry entries[POCKET_D20_MAX_INITIATIVE];
} PocketInitiativeState;

typedef struct {
    char name[POCKET_D20_NAME_LEN];
    char player[POCKET_D20_NAME_LEN];
    char species[POCKET_D20_NAME_LEN];
    char background[POCKET_D20_NAME_LEN];
    char alignment[POCKET_D20_SHORT_LEN];
    char other_proficiencies[POCKET_D20_DETAIL_LEN];

    uint8_t class_count;
    PocketClassLevel classes[POCKET_D20_MAX_CLASSES];
    uint32_t experience;
    uint8_t milestone_leveling;
    uint8_t inspiration;

    int8_t ability_scores[POCKET_D20_ABILITY_COUNT];
    uint8_t saving_throw_proficiency[POCKET_D20_ABILITY_COUNT];
    uint8_t skill_proficiency[POCKET_D20_SKILL_COUNT];

    int16_t hp_current;
    int16_t hp_max;
    int16_t hp_temporary;
    int16_t armor_class;
    int16_t speed;
    int8_t initiative_misc;
    uint8_t exhaustion;
    uint8_t death_successes;
    uint8_t death_failures;
    uint8_t hit_die;
    uint8_t hit_dice_current;
    uint8_t hit_dice_max;

    uint8_t spellcasting_ability;
    int8_t spell_attack_misc;
    int8_t spell_save_misc;
    uint8_t arcane_recovery_used;
    uint8_t spell_slots_current[POCKET_D20_SLOT_COUNT];
    uint8_t spell_slots_max[POCKET_D20_SLOT_COUNT];

    int32_t currency_cp;
    int32_t currency_sp;
    int32_t currency_ep;
    int32_t currency_gp;
    int32_t currency_pp;

    uint8_t spell_count;
    PocketSpell spells[POCKET_D20_MAX_SPELLS];
    uint8_t feature_count;
    PocketFeature features[POCKET_D20_MAX_FEATURES];
    uint8_t item_count;
    PocketItem items[POCKET_D20_MAX_ITEMS];
    uint8_t language_count;
    char languages[POCKET_D20_MAX_LANGUAGES][POCKET_D20_SHORT_LEN];
    uint8_t journal_count;
    PocketJournalEntry journal[POCKET_D20_MAX_JOURNAL];

    int8_t saving_throw_misc[POCKET_D20_ABILITY_COUNT];
    int8_t skill_misc[POCKET_D20_SKILL_COUNT];
    uint8_t spell_known[POCKET_D20_MAX_SPELLS];
    uint8_t spell_always_prepared[POCKET_D20_MAX_SPELLS];
    uint8_t spell_free_casts_current[POCKET_D20_MAX_SPELLS];
    uint8_t spell_free_casts_max[POCKET_D20_MAX_SPELLS];
} PocketCharacter;

typedef struct {
    PocketCharacter character;
    uint8_t party_count;
    PocketPartyMember party[POCKET_D20_MAX_PARTY];
    PocketInitiativeState initiative;
} PocketSaveData;

void pocket_d20_data_set_defaults(PocketSaveData* data);
void pocket_d20_data_sanitize(PocketSaveData* data);
