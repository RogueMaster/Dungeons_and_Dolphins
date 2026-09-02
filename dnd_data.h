#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DND_SAVE_VERSION 5U

#define DND_NAME_LEN           32U
#define DND_CHARACTER_NAME_LEN 25U
#define DND_CLASS_NAME_LEN     16U
#define DND_SUBCLASS_NAME_LEN  31U
#define DND_SPELL_NAME_LEN     31U
#define DND_FEATURE_NAME_LEN   31U
#define DND_ITEM_NAME_LEN      47U
#define DND_CATALOG_NAME_LEN   DND_ITEM_NAME_LEN
#define DND_SHORT_LEN          24U
#define DND_DETAIL_LEN         192U
#define DND_GRANT_VALUE_LEN    64U

#define DND_MAX_CLASSES           4U
#define DND_RESIDENT_RECORD_LIMIT 8U
#define DND_MAX_GRANTS            24U
#define DND_MAX_ATTACK_TEMPLATES  8U

#define DND_SKILL_COUNT   18U
#define DND_ABILITY_COUNT 6U
#define DND_SLOT_COUNT    10U

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
    PocketRechargeTurn,
    PocketRechargeEncounter,
    PocketRechargeDawn,
    PocketRechargeShortOrLong,
    PocketRechargeLong,
    PocketRechargeCount,
} PocketRecharge;

typedef enum {
    PocketSizeTiny,
    PocketSizeSmall,
    PocketSizeMedium,
    PocketSizeLarge,
    PocketSizeCount,
} PocketSize;

typedef enum {
    PocketSpellcastingNone,
    PocketSpellcastingFull,
    PocketSpellcastingHalf,
    PocketSpellcastingThird,
    PocketSpellcastingPact,
    PocketSpellcastingSpellPoints,
    PocketSpellcastingCustom,
    PocketSpellcastingModeCount,
} PocketSpellcastingMode;

typedef enum {
    PocketGrantSpecies,
    PocketGrantBackground,
    PocketGrantFeat,
    PocketGrantClassFeature,
    PocketGrantSubclassFeature,
    PocketGrantItem,
    PocketGrantSourceCount,
} PocketGrantSource;

typedef enum {
    PocketGrantPending,
    PocketGrantApplied,
    PocketGrantSkipped,
} PocketGrantStatus;

typedef enum {
    PocketResourceManual,
    PocketResourceProficiency,
    PocketResourceAbility,
    PocketResourceFormulaCount,
} PocketResourceFormula;

typedef enum {
    PocketAttackTemplateUnarmed,
    PocketAttackTemplateSpellAttack,
    PocketAttackTemplateSavingThrow,
    PocketAttackTemplateCustom,
    PocketAttackTemplateTypeCount,
} PocketAttackTemplateType;

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

enum {
    PocketWeaponFinesse = 1U << 0,
    PocketWeaponRanged = 1U << 1,
    PocketWeaponLight = 1U << 2,
    PocketWeaponHeavy = 1U << 3,
    PocketWeaponThrown = 1U << 4,
    PocketWeaponAmmunition = 1U << 5,
};

typedef struct {
    char name[DND_CLASS_NAME_LEN];
    char subclass[DND_SUBCLASS_NAME_LEN];
    uint8_t level;
    uint8_t hit_die;
    uint8_t hit_dice_current;
    uint8_t hit_dice_max;
    uint8_t spellcasting_mode;
    uint8_t spellcasting_ability;
    uint8_t cantrip_limit;
    uint8_t prepared_limit;
    uint16_t spellbook_size;
    uint8_t pact_slot_level;
    uint8_t pact_slots_current;
    uint8_t pact_slots_max;
    uint16_t mystic_arcanum_mask;
    uint16_t spell_points_current;
    uint16_t spell_points_max;
} PocketClassLevel;

typedef struct {
    char name[DND_SPELL_NAME_LEN];
    char detail[DND_DETAIL_LEN];
    uint8_t level;
    uint8_t class_index;
    uint8_t prepared;
    uint8_t ritual;
    char stable_id[DND_SHORT_LEN];
    char source[DND_SHORT_LEN];
    char school[DND_SHORT_LEN];
    uint8_t grant_source;
    char grant_name[DND_SHORT_LEN];
} PocketSpell;

typedef struct {
    char name[DND_FEATURE_NAME_LEN];
    char detail[DND_DETAIL_LEN];
    int16_t uses_current;
    int16_t uses_max;
    uint8_t class_index;
    uint8_t class_level_gained;
    uint8_t recharge;
    uint8_t resource_formula;
    uint8_t resource_ability;
} PocketFeature;

typedef struct {
    char name[DND_ITEM_NAME_LEN];
    char detail[DND_DETAIL_LEN];
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
    int32_t container_index;
    int16_t charges_current;
    int16_t charges_max;
    uint8_t armor_base;
    int8_t armor_dex_cap;
    uint8_t shield_bonus;
    char ammunition_group[DND_SHORT_LEN];
} PocketItem;

typedef struct {
    char stable_id[DND_SHORT_LEN];
    char source[DND_SHORT_LEN];
    char option_name[DND_NAME_LEN];
    char prerequisites[DND_NAME_LEN];
    char grant_value[DND_GRANT_VALUE_LEN];
    uint8_t source_type;
    uint8_t class_index;
    uint8_t level_gained;
    uint8_t status;
} PocketGrant;

typedef struct {
    char name[DND_NAME_LEN];
    char mastery[DND_SHORT_LEN];
    char damage_type[DND_SHORT_LEN];
    char rider_type[DND_SHORT_LEN];
    uint8_t type;
    uint8_t ability;
    uint8_t save_ability;
    int8_t attack_misc;
    uint8_t save_dc;
    uint8_t damage_dice;
    uint8_t damage_die;
    uint8_t rider_dice;
    uint8_t rider_die;
    uint8_t recharge;
} PocketAttackTemplate;

typedef struct {
    char name[DND_CHARACTER_NAME_LEN];
    char player[DND_NAME_LEN];
    char species[DND_NAME_LEN];
    char background[DND_NAME_LEN];
    char alignment[DND_SHORT_LEN];
    char origin_feat[DND_NAME_LEN];
    uint8_t size;
    char senses[DND_DETAIL_LEN];

    uint8_t class_count;
    PocketClassLevel classes[DND_MAX_CLASSES];
    uint32_t experience;
    uint8_t milestone_leveling;
    uint8_t inspiration;

    int8_t ability_scores[DND_ABILITY_COUNT];
    uint8_t saving_throw_proficiency[DND_ABILITY_COUNT];
    uint8_t skill_proficiency[DND_SKILL_COUNT];

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
    uint8_t spell_slots_current[DND_SLOT_COUNT];
    uint8_t spell_slots_max[DND_SLOT_COUNT];

    int32_t currency_cp;
    int32_t currency_sp;
    int32_t currency_ep;
    int32_t currency_gp;
    int32_t currency_pp;

    uint8_t spell_count;
    uint8_t spell_capacity;
    void* spell_storage;
    PocketSpell* spells;
    uint8_t* spell_known;
    uint8_t* spell_always_prepared;
    uint8_t* spell_free_casts_current;
    uint8_t* spell_free_casts_max;
    uint8_t feature_count;
    uint8_t feature_capacity;
    PocketFeature* features;
    uint8_t item_count;
    uint8_t item_capacity;
    PocketItem* items;
    int8_t saving_throw_misc[DND_ABILITY_COUNT];
    int8_t skill_misc[DND_SKILL_COUNT];

    char conditions[DND_DETAIL_LEN];
    char concentration[DND_NAME_LEN];
    uint8_t reaction_available;
    char temporary_effects[DND_DETAIL_LEN];
    char resistances[DND_DETAIL_LEN];
    char immunities[DND_DETAIL_LEN];
    char vulnerabilities[DND_DETAIL_LEN];
    char movement_modes[DND_DETAIL_LEN];

    uint8_t grant_count;
    uint8_t grant_capacity;
    PocketGrant* grants;
    uint8_t attack_template_count;
    PocketAttackTemplate attack_templates[DND_MAX_ATTACK_TEMPLATES];
    uint8_t encumbrance_mode;
    int16_t carrying_capacity_override;

} PocketCharacter;

typedef struct {
    PocketCharacter character;
} PocketSaveData;

void dnd_data_set_defaults(PocketSaveData* data);
void dnd_data_clear(PocketSaveData* data);
void dnd_data_sanitize(PocketSaveData* data);
bool dnd_data_reserve_spells(PocketCharacter* character, uint8_t required);
void dnd_data_clear_spells(PocketCharacter* character);
bool dnd_data_reserve_features(PocketCharacter* character, uint8_t required);
bool dnd_data_reserve_features_exact(PocketCharacter* character, uint8_t required);
bool dnd_data_reserve_items(PocketCharacter* character, uint8_t required);
void dnd_data_clear_items(PocketCharacter* character);
bool dnd_data_reserve_grants(PocketCharacter* character, uint8_t required);
bool dnd_data_reserve_grants_exact(PocketCharacter* character, uint8_t required);
