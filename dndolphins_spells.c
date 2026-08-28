#include "dndolphins_spells.h"

#include "dndolphins_rules.h"
#include "dndolphins_spell_combat.h"

#include <string.h>


static uint16_t pocket_d20_spells_class_mask_from_name(const char* name) {
    if(!name) return 0U;
    if(strcmp(name, "Artificer") == 0) return (uint16_t)(1U << 0);
    if(strcmp(name, "Bard") == 0) return (uint16_t)(1U << 2);
    if(strcmp(name, "Cleric") == 0) return (uint16_t)(1U << 3);
    if(strcmp(name, "Druid") == 0) return (uint16_t)(1U << 4);
    if(strcmp(name, "Paladin") == 0) return (uint16_t)(1U << 7);
    if(strcmp(name, "Ranger") == 0) return (uint16_t)(1U << 8);
    if(strcmp(name, "Sorcerer") == 0) return (uint16_t)(1U << 10);
    if(strcmp(name, "Warlock") == 0) return (uint16_t)(1U << 11);
    if(strcmp(name, "Wizard") == 0) return (uint16_t)(1U << 12);
    return 0U;
}

void pocket_d20_spells_init_editor_record(PocketSpell* spell) {
    if(!spell) return;
    memset(spell, 0, sizeof(*spell));
    strncpy(spell->name, "New Spell", sizeof(spell->name) - 1U);
    spell->name[sizeof(spell->name) - 1U] = '\0';
}

bool pocket_d20_spells_append_editor_record(
    Storage* storage, uint32_t profile, PocketCharacter* owner, PocketSpell* spell) {
    if(!storage || !owner || !spell) return false;
    pocket_d20_spells_init_editor_record(spell);
    return pocket_d20_storage_append_spell(storage, profile, owner, spell, 1U, 0U, 0U, 0U);
}

bool pocket_d20_spells_class_allows(
    const PocketCharacter* character, uint8_t class_index, uint8_t level, uint16_t class_mask) {
    if(!character || class_index >= character->class_count) return false;
    const PocketClassLevel* class_level = &character->classes[class_index];
    uint16_t selected_class = pocket_d20_spells_class_mask_from_name(class_level->name);
    return selected_class && (class_mask & selected_class) &&
           level <= pocket_d20_class_max_spell_level(class_level);
}

bool pocket_d20_spells_allowed_for_character(
    const PocketCharacter* character, uint8_t class_filter, uint8_t level, uint16_t class_mask) {
    if(!character || !character->class_count || !class_mask) return false;
    if(class_filter < character->class_count)
        return pocket_d20_spells_class_allows(character, class_filter, level, class_mask);
    for(uint8_t i = 0U; i < character->class_count; ++i)
        if(pocket_d20_spells_class_allows(character, i, level, class_mask)) return true;
    return false;
}

bool pocket_d20_spells_metadata_matches_filters(
    uint8_t spell_school,
    uint8_t spell_source,
    bool spell_ritual,
    uint8_t school_filter,
    uint8_t source_filter,
    uint8_t ritual_filter) {
    /* Metadata filters are deliberately opt-in. Zero means Any and must never
       narrow the default Allowed/All Classes catalog. */
    if(school_filter && spell_school != school_filter) return false;
    if(source_filter && spell_source != source_filter) return false;
    if(ritual_filter == 1U && !spell_ritual) return false;
    if(ritual_filter == 2U && spell_ritual) return false;
    return true;
}

uint8_t pocket_d20_spells_choose_class(
    const PocketCharacter* character,
    uint8_t class_filter,
    uint8_t level,
    uint16_t class_mask,
    uint8_t preferred) {
    if(!character || !character->class_count) return 0U;
    if(class_filter < character->class_count &&
       pocket_d20_spells_class_allows(character, class_filter, level, class_mask))
        return class_filter;
    if(preferred < character->class_count &&
       pocket_d20_spells_class_allows(character, preferred, level, class_mask))
        return preferred;
    for(uint8_t i = 0U; i < character->class_count; ++i)
        if(pocket_d20_spells_class_allows(character, i, level, class_mask)) return i;
    return preferred < character->class_count ? preferred : 0U;
}

void pocket_d20_spells_apply_catalog_record(
    PocketSpell* spell,
    const char* name,
    uint8_t level,
    uint8_t class_index,
    const char* stable_id,
    const char* source,
    const char* school,
    bool ritual) {
    if(!spell || !name) return;
    strncpy(spell->name, name, sizeof(spell->name) - 1U);
    spell->name[sizeof(spell->name) - 1U] = '\0';
    spell->level = level;
    spell->class_index = class_index;
    if(stable_id) {
        strncpy(spell->stable_id, stable_id, sizeof(spell->stable_id) - 1U);
        spell->stable_id[sizeof(spell->stable_id) - 1U] = '\0';
    }
    if(source) {
        strncpy(spell->source, source, sizeof(spell->source) - 1U);
        spell->source[sizeof(spell->source) - 1U] = '\0';
    }
    if(school) {
        strncpy(spell->school, school, sizeof(spell->school) - 1U);
        spell->school[sizeof(spell->school) - 1U] = '\0';
    } else {
        spell->school[0] = '\0';
    }
    spell->ritual = ritual ? 1U : 0U;
}

uint8_t pocket_d20_spell_casting_ability_for(
    const PocketCharacter* character, const PocketSpell* spell) {
    if(spell && spell->grant_source == PocketGrantSpecies)
        return character->spellcasting_ability < POCKET_D20_ABILITY_COUNT ?
                   character->spellcasting_ability :
                   PocketAbilityIntelligence;
    if(spell) {
        uint8_t class_index = spell->class_index;
        if(class_index < character->class_count &&
           character->classes[class_index].spellcasting_mode != PocketSpellcastingNone &&
           character->classes[class_index].spellcasting_ability < POCKET_D20_ABILITY_COUNT)
            return character->classes[class_index].spellcasting_ability;
    }
    return character->spellcasting_ability < POCKET_D20_ABILITY_COUNT ?
               character->spellcasting_ability :
               PocketAbilityIntelligence;
}

int8_t pocket_d20_spell_attack_modifier_for(
    const PocketCharacter* character, const PocketSpell* spell) {
    uint8_t ability = pocket_d20_spell_casting_ability_for(character, spell);
    return (int8_t)(pocket_d20_ability_modifier(character->ability_scores[ability]) +
                    pocket_d20_proficiency_bonus(character) + character->spell_attack_misc +
                    pocket_d20_exhaustion_penalty(character));
}

int8_t pocket_d20_spell_save_dc_for(
    const PocketCharacter* character, const PocketSpell* spell) {
    uint8_t ability = pocket_d20_spell_casting_ability_for(character, spell);
    return (int8_t)(8 + pocket_d20_ability_modifier(character->ability_scores[ability]) +
                    pocket_d20_proficiency_bonus(character) + character->spell_save_misc);
}

int8_t pocket_d20_spell_attack_modifier(const PocketCharacter* character) {
    return pocket_d20_spell_attack_modifier_for(character, NULL);
}

int8_t pocket_d20_spell_save_dc(const PocketCharacter* character) {
    return pocket_d20_spell_save_dc_for(character, NULL);
}

void pocket_d20_recalculate_multiclass_slots(PocketCharacter* character) {
    static const uint8_t slots[20][9] = {
        {2, 0, 0, 0, 0, 0, 0, 0, 0}, {3, 0, 0, 0, 0, 0, 0, 0, 0}, {4, 2, 0, 0, 0, 0, 0, 0, 0},
        {4, 3, 0, 0, 0, 0, 0, 0, 0}, {4, 3, 2, 0, 0, 0, 0, 0, 0}, {4, 3, 3, 0, 0, 0, 0, 0, 0},
        {4, 3, 3, 1, 0, 0, 0, 0, 0}, {4, 3, 3, 2, 0, 0, 0, 0, 0}, {4, 3, 3, 3, 1, 0, 0, 0, 0},
        {4, 3, 3, 3, 2, 0, 0, 0, 0}, {4, 3, 3, 3, 2, 1, 0, 0, 0}, {4, 3, 3, 3, 2, 1, 0, 0, 0},
        {4, 3, 3, 3, 2, 1, 1, 0, 0}, {4, 3, 3, 3, 2, 1, 1, 0, 0}, {4, 3, 3, 3, 2, 1, 1, 1, 0},
        {4, 3, 3, 3, 2, 1, 1, 1, 0}, {4, 3, 3, 3, 2, 1, 1, 1, 1}, {4, 3, 3, 3, 3, 1, 1, 1, 1},
        {4, 3, 3, 3, 3, 2, 1, 1, 1}, {4, 3, 3, 3, 3, 2, 2, 1, 1},
    };
    uint8_t caster_level = 0U;
    for(uint8_t i = 0U; i < character->class_count; ++i) {
        const PocketClassLevel* level = &character->classes[i];
        if(level->spellcasting_mode == PocketSpellcastingFull)
            caster_level += level->level;
        else if(level->spellcasting_mode == PocketSpellcastingHalf)
            caster_level += (level->level + 1U) / 2U;
        else if(level->spellcasting_mode == PocketSpellcastingThird)
            caster_level += level->level / 3U;
    }
    if(caster_level > 20U) caster_level = 20U;
    character->spell_slots_max[0] = 0U;
    character->spell_slots_current[0] = 0U;
    for(uint8_t level = 1U; level <= 9U; ++level) {
        uint8_t maximum = caster_level ? slots[caster_level - 1U][level - 1U] : 0U;
        character->spell_slots_max[level] = maximum;
        if(character->spell_slots_current[level] > maximum)
            character->spell_slots_current[level] = maximum;
    }
}

uint8_t pocket_d20_class_max_spell_level(const PocketClassLevel* class_level) {
    if(!class_level) return 0U;
    uint8_t level = class_level->level;
    const char* name = class_level->name;
    if(!strcmp(name, "Bard") || !strcmp(name, "Cleric") || !strcmp(name, "Druid") ||
       !strcmp(name, "Sorcerer") || !strcmp(name, "Wizard")) {
        uint8_t maximum = (level + 1U) / 2U;
        return maximum > 9U ? 9U : maximum;
    }
    if(!strcmp(name, "Artificer") || !strcmp(name, "Paladin") || !strcmp(name, "Ranger")) {
        uint8_t maximum = (level + 3U) / 4U;
        return maximum > 5U ? 5U : maximum;
    }
    if(!strcmp(name, "Warlock")) {
        uint8_t maximum = (level + 1U) / 2U;
        return maximum > 5U ? 5U : maximum;
    }
    return 0U;
}

uint8_t pocket_d20_spell_point_cost(uint8_t level) {
    static const uint8_t cost[10] = {0U, 2U, 3U, 5U, 6U, 7U, 9U, 10U, 11U, 13U};
    return level < 10U ? cost[level] : 0U;
}

bool pocket_d20_spell_is_tracked(
    const PocketSpell* spell, uint8_t known, uint8_t always_prepared) {
    return spell && (known || spell->prepared || always_prepared);
}

bool pocket_d20_spell_can_ritual(
    const PocketSpell* spell, uint8_t known, uint8_t always_prepared) {
    return spell && spell->ritual && pocket_d20_spell_is_tracked(spell, known, always_prepared);
}

bool pocket_d20_spell_record_has_cast_resource(
    const PocketCharacter* character,
    const PocketSpell* spell,
    uint8_t known,
    uint8_t always_prepared,
    uint8_t free_casts_current) {
    if(!character || !spell || !pocket_d20_spell_is_tracked(spell, known, always_prepared))
        return false;
    if(spell->level == 0U || free_casts_current) return true;
    for(uint8_t level = spell->level; level < POCKET_D20_SLOT_COUNT; ++level)
        if(character->spell_slots_current[level]) return true;
    for(uint8_t class_index = 0U; class_index < character->class_count; ++class_index) {
        const PocketClassLevel* class_level = &character->classes[class_index];
        if(class_level->spellcasting_mode == PocketSpellcastingPact &&
           class_level->pact_slots_current && class_level->pact_slot_level >= spell->level)
            return true;
    }
    if(spell->class_index < character->class_count) {
        const PocketClassLevel* class_level = &character->classes[spell->class_index];
        if(class_level->spellcasting_mode == PocketSpellcastingSpellPoints) {
            uint8_t maximum = pocket_d20_class_max_spell_level(class_level);
            if(maximum > 5U) maximum = 5U;
            for(uint8_t level = spell->level; level <= maximum; ++level) {
                uint8_t cost = pocket_d20_spell_point_cost(level);
                if(cost && class_level->spell_points_current >= cost) return true;
            }
        }
    }
    return spell->ritual != 0U;
}

uint8_t pocket_d20_spells_build_cast_options(
    const PocketCharacter* character,
    const PocketSpell* spell,
    uint8_t known,
    uint8_t always_prepared,
    uint8_t free_casts_current,
    PocketSpellCastOption* options,
    uint8_t capacity) {
    if(!character || !spell || !pocket_d20_spell_is_tracked(spell, known, always_prepared))
        return 0U;
    uint8_t count = 0U;
#define POCKET_ADD_CAST_OPTION(lvl, kind, cls)                                                    \
    do {                                                                                          \
        if(options && count < capacity) {                                                         \
            options[count].level = (lvl);                                                         \
            options[count].resource = (kind);                                                     \
            options[count].class_index = (cls);                                                   \
        }                                                                                         \
        if(count < 255U) ++count;                                                                 \
    } while(false)

    if(spell->level == 0U) {
        POCKET_ADD_CAST_OPTION(0U, PocketSpellCastCantrip, spell->class_index);
        return count;
    }
    if(free_casts_current)
        POCKET_ADD_CAST_OPTION(spell->level, PocketSpellCastFree, spell->class_index);
    for(uint8_t level = spell->level; level < POCKET_D20_SLOT_COUNT; ++level)
        if(character->spell_slots_current[level])
            POCKET_ADD_CAST_OPTION(level, PocketSpellCastSlot, spell->class_index);
    for(uint8_t class_index = 0U; class_index < character->class_count; ++class_index) {
        const PocketClassLevel* class_level = &character->classes[class_index];
        if(class_level->spellcasting_mode == PocketSpellcastingPact &&
           class_level->pact_slots_current && class_level->pact_slot_level >= spell->level)
            POCKET_ADD_CAST_OPTION(class_level->pact_slot_level, PocketSpellCastPact, class_index);
    }
    if(spell->class_index < character->class_count) {
        const PocketClassLevel* class_level = &character->classes[spell->class_index];
        if(class_level->spellcasting_mode == PocketSpellcastingSpellPoints) {
            uint8_t maximum = pocket_d20_class_max_spell_level(class_level);
            if(maximum > 5U) maximum = 5U;
            for(uint8_t level = spell->level; level <= maximum; ++level) {
                uint8_t cost = pocket_d20_spell_point_cost(level);
                if(cost && class_level->spell_points_current >= cost)
                    POCKET_ADD_CAST_OPTION(level, PocketSpellCastPoints, spell->class_index);
            }
        }
    }
    if(pocket_d20_spell_can_ritual(spell, known, always_prepared))
        POCKET_ADD_CAST_OPTION(spell->level, PocketSpellCastRitual, spell->class_index);
#undef POCKET_ADD_CAST_OPTION
    return count;
}

typedef struct {
    const PocketCharacter* character;
    uint8_t* indices;
    uint8_t capacity;
    uint8_t count;
} PocketD20CombatSpellIndexContext;

static bool pocket_d20_combat_spell_index_visitor(
    uint8_t logical_index,
    const PocketSpell* spell,
    uint8_t known,
    uint8_t always_prepared,
    uint8_t free_casts_current,
    uint8_t free_casts_max,
    void* context) {
    (void)free_casts_max;
    PocketD20CombatSpellIndexContext* scan = context;
    if(!pocket_d20_spell_record_has_cast_resource(
           scan->character, spell, known, always_prepared, free_casts_current))
        return true;
    uint8_t ability = pocket_d20_spell_casting_ability_for(scan->character, spell);
    int8_t ability_modifier =
        pocket_d20_ability_modifier(scan->character->ability_scores[ability]);
    PocketSpellDamageSpec damage;
    if(pocket_d20_spell_damage_spec(
           spell,
           spell->level,
           pocket_d20_total_level(scan->character),
           ability_modifier,
           &damage) &&
       scan->count < scan->capacity)
        scan->indices[scan->count++] = logical_index;
    return true;
}

bool pocket_d20_spells_collect_combat_indices(
    Storage* storage,
    uint32_t profile,
    const PocketCharacter* character,
    uint8_t* indices,
    uint8_t capacity,
    uint8_t* count,
    uint8_t* total_count) {
    if(!storage || !character || !indices || !count) return false;
    PocketD20CombatSpellIndexContext context = {
        .character = character,
        .indices = indices,
        .capacity = capacity,
        .count = 0U,
    };
    uint8_t total = 0U;
    bool success = pocket_d20_storage_visit_spells(
        storage, profile, pocket_d20_combat_spell_index_visitor, &context, &total);
    *count = context.count;
    if(total_count) *total_count = total;
    return success;
}
