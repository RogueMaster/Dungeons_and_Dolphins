#pragma once

#include "dndolphins.h"
#include "dndolphins_storage.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    PocketSpellCastCantrip,
    PocketSpellCastFree,
    PocketSpellCastSlot,
    PocketSpellCastPact,
    PocketSpellCastPoints,
    PocketSpellCastRitual,
} PocketSpellCastResource;

typedef struct {
    uint8_t level;
    uint8_t resource;
    uint8_t class_index;
} PocketSpellCastOption;

#define POCKET_D20_MAX_SPELL_CAST_OPTIONS 24U


/* Spellbook editor/catalog helpers are feature-owned. All-classes eligibility
   is the union of the character's spell lists, using each class's own level. */
void pocket_d20_spells_init_editor_record(PocketSpell* spell);
bool pocket_d20_spells_append_editor_record(
    Storage* storage, uint32_t profile, PocketCharacter* owner, PocketSpell* spell);
bool pocket_d20_spells_class_allows(
    const PocketCharacter* character, uint8_t class_index, uint8_t level, uint16_t class_mask);
bool pocket_d20_spells_allowed_for_character(
    const PocketCharacter* character, uint8_t class_filter, uint8_t level, uint16_t class_mask);
bool pocket_d20_spells_metadata_matches_filters(
    uint8_t spell_school,
    uint8_t spell_source,
    bool spell_ritual,
    uint8_t school_filter,
    uint8_t source_filter,
    uint8_t ritual_filter);
uint8_t pocket_d20_spells_choose_class(
    const PocketCharacter* character,
    uint8_t class_filter,
    uint8_t level,
    uint16_t class_mask,
    uint8_t preferred);
void pocket_d20_spells_apply_catalog_record(
    PocketSpell* spell,
    const char* name,
    uint8_t level,
    uint8_t class_index,
    const char* stable_id,
    const char* source,
    const char* school,
    bool ritual);

uint8_t pocket_d20_spell_casting_ability_for(
    const PocketCharacter* character, const PocketSpell* spell);
int8_t pocket_d20_spell_attack_modifier(const PocketCharacter* character);
int8_t pocket_d20_spell_save_dc(const PocketCharacter* character);
int8_t pocket_d20_spell_attack_modifier_for(
    const PocketCharacter* character, const PocketSpell* spell);
int8_t pocket_d20_spell_save_dc_for(
    const PocketCharacter* character, const PocketSpell* spell);

void pocket_d20_recalculate_multiclass_slots(PocketCharacter* character);
uint8_t pocket_d20_class_max_spell_level(const PocketClassLevel* class_level);
uint8_t pocket_d20_spell_point_cost(uint8_t level);

bool pocket_d20_spell_is_tracked(
    const PocketSpell* spell, uint8_t known, uint8_t always_prepared);
bool pocket_d20_spell_can_ritual(
    const PocketSpell* spell, uint8_t known, uint8_t always_prepared);
bool pocket_d20_spell_record_has_cast_resource(
    const PocketCharacter* character,
    const PocketSpell* spell,
    uint8_t known,
    uint8_t always_prepared,
    uint8_t free_casts_current);
uint8_t pocket_d20_spells_build_cast_options(
    const PocketCharacter* character,
    const PocketSpell* spell,
    uint8_t known,
    uint8_t always_prepared,
    uint8_t free_casts_current,
    PocketSpellCastOption* options,
    uint8_t capacity);

bool pocket_d20_spells_collect_combat_indices(
    Storage* storage,
    uint32_t profile,
    const PocketCharacter* character,
    uint8_t* indices,
    uint8_t capacity,
    uint8_t* count,
    uint8_t* total_count);
