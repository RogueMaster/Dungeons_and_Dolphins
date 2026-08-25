#include "pocket_d20_storage.h"

#include <furi.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define POCKET_D20_TEXT_VERSION 1U
#define POCKET_D20_LINE_LEN 768U
#define POCKET_D20_DATA_DIR APP_DATA_PATH("")

#define POCKET_D20_PROFILES_PATH APP_DATA_PATH("profiles.txt")
#define POCKET_D20_PROFILES_TEMP_PATH APP_DATA_PATH("profiles.tmp.txt")
#define POCKET_D20_PROFILES_BACKUP_PATH APP_DATA_PATH("profiles.bak.txt")

static uint32_t pocket_d20_checksum(const void* data, size_t size) {
    const uint8_t* bytes = data;
    uint32_t hash = 2166136261UL;
    for(size_t i = 0U; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619UL;
    }
    return hash;
}

static void pocket_d20_copy(char* destination, size_t size, const char* source) {
    if(size == 0U) return;
    strncpy(destination, source, size - 1U);
    destination[size - 1U] = '\0';
}

static void pocket_d20_normalize_unused(PocketSaveData* data) {
    PocketCharacter* c = &data->character;
    for(uint8_t i = c->class_count; i < POCKET_D20_MAX_CLASSES; ++i)
        memset(&c->classes[i], 0, sizeof(c->classes[i]));
    for(uint8_t i = c->spell_count; i < POCKET_D20_MAX_SPELLS; ++i) {
        memset(&c->spells[i], 0, sizeof(c->spells[i]));
        c->spell_known[i] = 0U;
        c->spell_always_prepared[i] = 0U;
        c->spell_free_casts_current[i] = 0U;
        c->spell_free_casts_max[i] = 0U;
    }
    for(uint8_t i = c->feature_count; i < POCKET_D20_MAX_FEATURES; ++i)
        memset(&c->features[i], 0, sizeof(c->features[i]));
    for(uint8_t i = c->item_count; i < POCKET_D20_MAX_ITEMS; ++i)
        memset(&c->items[i], 0, sizeof(c->items[i]));
    for(uint8_t i = c->language_count; i < POCKET_D20_MAX_LANGUAGES; ++i)
        memset(c->languages[i], 0, sizeof(c->languages[i]));
    for(uint8_t i = c->journal_count; i < POCKET_D20_MAX_JOURNAL; ++i)
        memset(&c->journal[i], 0, sizeof(c->journal[i]));
    for(uint8_t i = data->party_count; i < POCKET_D20_MAX_PARTY; ++i)
        memset(&data->party[i], 0, sizeof(data->party[i]));
    for(uint8_t i = data->initiative.count; i < POCKET_D20_MAX_INITIATIVE; ++i)
        memset(&data->initiative.entries[i], 0, sizeof(data->initiative.entries[i]));
}

static void pocket_d20_profile_path(
    char* output,
    size_t size,
    uint8_t profile,
    const char* qualifier) {
    snprintf(
        output,
        size,
        "%scharacter_%u%s.txt",
        POCKET_D20_DATA_DIR,
        (unsigned int)profile,
        qualifier);
}

static bool pocket_d20_writef(File* file, const char* format, ...) {
    char line[POCKET_D20_LINE_LEN];
    va_list arguments;
    va_start(arguments, format);
    int length = vsnprintf(line, sizeof(line), format, arguments);
    va_end(arguments);
    if(length < 0 || (size_t)length >= sizeof(line)) return false;
    return storage_file_write(file, line, (size_t)length) == (size_t)length;
}

static uint8_t pocket_d20_hex_value(char value) {
    if(value >= '0' && value <= '9') return (uint8_t)(value - '0');
    if(value >= 'A' && value <= 'F') return (uint8_t)(value - 'A' + 10);
    if(value >= 'a' && value <= 'f') return (uint8_t)(value - 'a' + 10);
    return 0xFFU;
}

static bool pocket_d20_write_string(File* file, const char* key, const char* value) {
    char encoded[(POCKET_D20_DETAIL_LEN * 3U) + 1U];
    size_t position = 0U;
    static const char digits[] = "0123456789ABCDEF";
    for(size_t i = 0U; value[i] != '\0'; ++i) {
        uint8_t byte = (uint8_t)value[i];
        bool escape = byte == '%' || byte == '\n' || byte == '\r' || byte < 0x20U;
        if(escape) {
            if(position + 3U >= sizeof(encoded)) return false;
            encoded[position++] = '%';
            encoded[position++] = digits[byte >> 4U];
            encoded[position++] = digits[byte & 0x0FU];
        } else {
            if(position + 1U >= sizeof(encoded)) return false;
            encoded[position++] = (char)byte;
        }
    }
    encoded[position] = '\0';
    return pocket_d20_writef(file, "%s=%s\n", key, encoded);
}

static void pocket_d20_decode_string(char* destination, size_t size, const char* value) {
    size_t output = 0U;
    for(size_t input = 0U; value[input] != '\0' && output + 1U < size; ++input) {
        if(value[input] == '%' && value[input + 1U] && value[input + 2U]) {
            uint8_t high = pocket_d20_hex_value(value[input + 1U]);
            uint8_t low = pocket_d20_hex_value(value[input + 2U]);
            if(high != 0xFFU && low != 0xFFU) {
                destination[output++] = (char)((high << 4U) | low);
                input += 2U;
                continue;
            }
        }
        destination[output++] = value[input];
    }
    if(size) destination[output] = '\0';
}

static bool pocket_d20_write_i8_array(
    File* file,
    const char* key,
    const int8_t* values,
    size_t count) {
    char line[256];
    size_t position = (size_t)snprintf(line, sizeof(line), "%s=", key);
    for(size_t i = 0U; i < count; ++i) {
        int written = snprintf(
            line + position,
            sizeof(line) - position,
            "%s%d",
            i ? "," : "",
            values[i]);
        if(written < 0 || (size_t)written >= sizeof(line) - position) return false;
        position += (size_t)written;
    }
    line[position++] = '\n';
    return storage_file_write(file, line, position) == position;
}

static bool pocket_d20_write_u8_array(
    File* file,
    const char* key,
    const uint8_t* values,
    size_t count) {
    char line[256];
    size_t position = (size_t)snprintf(line, sizeof(line), "%s=", key);
    for(size_t i = 0U; i < count; ++i) {
        int written = snprintf(
            line + position,
            sizeof(line) - position,
            "%s%u",
            i ? "," : "",
            (unsigned int)values[i]);
        if(written < 0 || (size_t)written >= sizeof(line) - position) return false;
        position += (size_t)written;
    }
    line[position++] = '\n';
    return storage_file_write(file, line, position) == position;
}

static bool pocket_d20_read_line(File* file, char* line, size_t size) {
    size_t position = 0U;
    char character = '\0';
    while(position + 1U < size) {
        if(storage_file_read(file, &character, 1U) != 1U) break;
        if(character == '\n') break;
        if(character != '\r') line[position++] = character;
    }
    line[position] = '\0';
    return position > 0U || character == '\n';
}

static bool pocket_d20_read_value(
    File* file,
    const char* expected_key,
    char* value,
    size_t value_size) {
    char line[POCKET_D20_LINE_LEN];
    if(!pocket_d20_read_line(file, line, sizeof(line))) return false;
    char* separator = strchr(line, '=');
    if(!separator) return false;
    *separator = '\0';
    if(strcmp(line, expected_key) != 0) return false;
    pocket_d20_copy(value, value_size, separator + 1U);
    return true;
}

static bool pocket_d20_read_string(
    File* file,
    const char* key,
    char* destination,
    size_t destination_size) {
    char value[(POCKET_D20_DETAIL_LEN * 3U) + 1U];
    if(!pocket_d20_read_value(file, key, value, sizeof(value))) return false;
    pocket_d20_decode_string(destination, destination_size, value);
    return true;
}

static size_t pocket_d20_parse_numbers(const char* value, int32_t* numbers, size_t maximum) {
    size_t count = 0U;
    const char* cursor = value;
    while(*cursor && count < maximum) {
        char* end = NULL;
        long number = strtol(cursor, &end, 10);
        if(end == cursor) break;
        numbers[count++] = (int32_t)number;
        if(*end == ',')
            cursor = end + 1U;
        else if(*end == '\0')
            break;
        else
            return 0U;
    }
    return count;
}

static bool pocket_d20_read_numbers(
    File* file,
    const char* key,
    int32_t* numbers,
    size_t expected) {
    char value[256];
    return pocket_d20_read_value(file, key, value, sizeof(value)) &&
           pocket_d20_parse_numbers(value, numbers, expected) == expected;
}

static bool pocket_d20_write_character(File* file, const PocketSaveData* data) {
    const PocketCharacter* c = &data->character;
    char key[48];
    if(!pocket_d20_writef(file, "PocketD20Character=%u\n", POCKET_D20_TEXT_VERSION) ||
       !pocket_d20_write_string(file, "Name", c->name) ||
       !pocket_d20_write_string(file, "Player", c->player) ||
       !pocket_d20_write_string(file, "Species", c->species) ||
       !pocket_d20_write_string(file, "Background", c->background) ||
       !pocket_d20_write_string(file, "Alignment", c->alignment) ||
       !pocket_d20_write_string(file, "OtherProficiencies", c->other_proficiencies) ||
       !pocket_d20_writef(
           file,
           "Progress=%u,%lu,%u,%u\n",
           c->class_count,
           (unsigned long)c->experience,
           c->milestone_leveling,
           c->inspiration))
        return false;

    for(uint8_t i = 0U; i < c->class_count; ++i) {
        snprintf(key, sizeof(key), "Class%uName", i);
        if(!pocket_d20_write_string(file, key, c->classes[i].name)) return false;
        snprintf(key, sizeof(key), "Class%uSubclass", i);
        if(!pocket_d20_write_string(file, key, c->classes[i].subclass) ||
           !pocket_d20_writef(file, "Class%uLevel=%u\n", i, c->classes[i].level))
            return false;
    }

    if(!pocket_d20_write_i8_array(file, "AbilityScores", c->ability_scores, POCKET_D20_ABILITY_COUNT) ||
       !pocket_d20_write_u8_array(
           file, "SaveProficiency", c->saving_throw_proficiency, POCKET_D20_ABILITY_COUNT) ||
       !pocket_d20_write_i8_array(file, "SaveMisc", c->saving_throw_misc, POCKET_D20_ABILITY_COUNT) ||
       !pocket_d20_write_u8_array(
           file, "SkillProficiency", c->skill_proficiency, POCKET_D20_SKILL_COUNT) ||
       !pocket_d20_write_i8_array(file, "SkillMisc", c->skill_misc, POCKET_D20_SKILL_COUNT) ||
       !pocket_d20_writef(
           file,
           "Vitals=%d,%d,%d,%d,%d,%d,%u,%u,%u,%u,%u,%u\n",
           c->hp_current,
           c->hp_max,
           c->hp_temporary,
           c->armor_class,
           c->speed,
           c->initiative_misc,
           c->exhaustion,
           c->death_successes,
           c->death_failures,
           c->hit_die,
           c->hit_dice_current,
           c->hit_dice_max) ||
       !pocket_d20_writef(
           file,
           "Spellcasting=%u,%d,%d,%u\n",
           c->spellcasting_ability,
           c->spell_attack_misc,
           c->spell_save_misc,
           c->arcane_recovery_used) ||
       !pocket_d20_write_u8_array(
           file, "SpellSlotsCurrent", c->spell_slots_current, POCKET_D20_SLOT_COUNT) ||
       !pocket_d20_write_u8_array(
           file, "SpellSlotsMax", c->spell_slots_max, POCKET_D20_SLOT_COUNT) ||
       !pocket_d20_writef(
           file,
           "Currency=%ld,%ld,%ld,%ld,%ld\n",
           (long)c->currency_cp,
           (long)c->currency_sp,
           (long)c->currency_ep,
           (long)c->currency_gp,
           (long)c->currency_pp) ||
       !pocket_d20_writef(file, "SpellCount=%u\n", c->spell_count))
        return false;

    for(uint8_t i = 0U; i < c->spell_count; ++i) {
        snprintf(key, sizeof(key), "Spell%uName", i);
        if(!pocket_d20_write_string(file, key, c->spells[i].name)) return false;
        snprintf(key, sizeof(key), "Spell%uDetail", i);
        if(!pocket_d20_write_string(file, key, c->spells[i].detail) ||
           !pocket_d20_writef(
               file,
               "Spell%uData=%u,%u,%u,%u,%u,%u,%u,%u\n",
               i,
               c->spells[i].level,
               c->spells[i].class_index,
               c->spells[i].prepared,
               c->spells[i].ritual,
               c->spell_known[i],
               c->spell_always_prepared[i],
               c->spell_free_casts_current[i],
               c->spell_free_casts_max[i]))
            return false;
    }

    if(!pocket_d20_writef(file, "FeatureCount=%u\n", c->feature_count)) return false;
    for(uint8_t i = 0U; i < c->feature_count; ++i) {
        snprintf(key, sizeof(key), "Feature%uName", i);
        if(!pocket_d20_write_string(file, key, c->features[i].name)) return false;
        snprintf(key, sizeof(key), "Feature%uDetail", i);
        if(!pocket_d20_write_string(file, key, c->features[i].detail) ||
           !pocket_d20_writef(
               file,
               "Feature%uData=%d,%d,%u,%u,%u\n",
               i,
               c->features[i].uses_current,
               c->features[i].uses_max,
               c->features[i].class_index,
               c->features[i].class_level_gained,
               c->features[i].recharge))
            return false;
    }

    if(!pocket_d20_writef(file, "ItemCount=%u\n", c->item_count)) return false;
    for(uint8_t i = 0U; i < c->item_count; ++i) {
        const PocketItem* item = &c->items[i];
        snprintf(key, sizeof(key), "Item%uName", i);
        if(!pocket_d20_write_string(file, key, item->name)) return false;
        snprintf(key, sizeof(key), "Item%uDetail", i);
        if(!pocket_d20_write_string(file, key, item->detail) ||
           !pocket_d20_writef(
               file,
               "Item%uData=%d,%d,%u,%u,%u,%u,%u,%d,%u,%u,%u,%u,%u,%u,%u,%u,%u,%d,%d\n",
               i,
               item->quantity,
               item->weight_tenths,
               item->equipped,
               item->attuned,
               item->is_weapon,
               item->attack_ability,
               item->proficient,
               item->magic_bonus,
               item->damage_dice,
               item->damage_die,
               item->versatile_die,
               item->use_versatile,
               item->damage_type,
               item->add_ability_damage,
               item->extra_dice,
               item->extra_die,
               item->weapon_properties,
               item->ammo_current,
               item->ammo_max))
            return false;
    }

    if(!pocket_d20_writef(file, "LanguageCount=%u\n", c->language_count)) return false;
    for(uint8_t i = 0U; i < c->language_count; ++i) {
        snprintf(key, sizeof(key), "Language%u", i);
        if(!pocket_d20_write_string(file, key, c->languages[i])) return false;
    }

    if(!pocket_d20_writef(file, "JournalCount=%u\n", c->journal_count)) return false;
    for(uint8_t i = 0U; i < c->journal_count; ++i) {
        snprintf(key, sizeof(key), "Journal%uTitle", i);
        if(!pocket_d20_write_string(file, key, c->journal[i].title)) return false;
        snprintf(key, sizeof(key), "Journal%uBody", i);
        if(!pocket_d20_write_string(file, key, c->journal[i].body) ||
           !pocket_d20_writef(
               file,
               "Journal%uData=%u,%u,%u,%u\n",
               i,
               c->journal[i].category,
               c->journal[i].completed,
               c->journal[i].level_granted,
               c->journal[i].class_index))
            return false;
    }

    if(!pocket_d20_writef(file, "PartyCount=%u\n", data->party_count)) return false;
    for(uint8_t i = 0U; i < data->party_count; ++i) {
        snprintf(key, sizeof(key), "Party%uName", i);
        if(!pocket_d20_write_string(file, key, data->party[i].name) ||
           !pocket_d20_writef(
               file, "Party%uInitiativeModifier=%d\n", i, data->party[i].initiative_modifier))
            return false;
    }

    if(!pocket_d20_writef(
           file,
           "InitiativeState=%u,%u,%u,%u\n",
           data->initiative.active,
           data->initiative.round,
           data->initiative.current_turn,
           data->initiative.count))
        return false;
    for(uint8_t i = 0U; i < data->initiative.count; ++i) {
        snprintf(key, sizeof(key), "Initiative%uName", i);
        if(!pocket_d20_write_string(file, key, data->initiative.entries[i].name) ||
           !pocket_d20_writef(
               file,
               "Initiative%uData=%d,%d,%u\n",
               i,
               data->initiative.entries[i].initiative_modifier,
               data->initiative.entries[i].initiative_total,
               data->initiative.entries[i].is_player_character))
            return false;
    }

    return pocket_d20_writef(
        file, "DataChecksum=%08lX\n", (unsigned long)pocket_d20_checksum(data, sizeof(*data)));
}

static bool pocket_d20_read_character(File* file, PocketSaveData* data) {
    PocketCharacter* c = &data->character;
    char key[48];
    char value[64];
    int32_t n[24];
    memset(data, 0, sizeof(*data));

    if(!pocket_d20_read_value(file, "PocketD20Character", value, sizeof(value)) ||
       strtoul(value, NULL, 10) != POCKET_D20_TEXT_VERSION ||
       !pocket_d20_read_string(file, "Name", c->name, sizeof(c->name)) ||
       !pocket_d20_read_string(file, "Player", c->player, sizeof(c->player)) ||
       !pocket_d20_read_string(file, "Species", c->species, sizeof(c->species)) ||
       !pocket_d20_read_string(file, "Background", c->background, sizeof(c->background)) ||
       !pocket_d20_read_string(file, "Alignment", c->alignment, sizeof(c->alignment)) ||
       !pocket_d20_read_string(
           file, "OtherProficiencies", c->other_proficiencies, sizeof(c->other_proficiencies)) ||
       !pocket_d20_read_numbers(file, "Progress", n, 4U))
        return false;
    c->class_count = (uint8_t)n[0];
    c->experience = (uint32_t)n[1];
    c->milestone_leveling = (uint8_t)n[2];
    c->inspiration = (uint8_t)n[3];
    if(c->class_count == 0U || c->class_count > POCKET_D20_MAX_CLASSES) return false;

    for(uint8_t i = 0U; i < c->class_count; ++i) {
        snprintf(key, sizeof(key), "Class%uName", i);
        if(!pocket_d20_read_string(file, key, c->classes[i].name, sizeof(c->classes[i].name)))
            return false;
        snprintf(key, sizeof(key), "Class%uSubclass", i);
        if(!pocket_d20_read_string(
               file, key, c->classes[i].subclass, sizeof(c->classes[i].subclass)))
            return false;
        snprintf(key, sizeof(key), "Class%uLevel", i);
        if(!pocket_d20_read_value(file, key, value, sizeof(value))) return false;
        c->classes[i].level = (uint8_t)strtoul(value, NULL, 10);
    }

    if(!pocket_d20_read_numbers(file, "AbilityScores", n, POCKET_D20_ABILITY_COUNT))
        return false;
    for(uint8_t i = 0U; i < POCKET_D20_ABILITY_COUNT; ++i) c->ability_scores[i] = (int8_t)n[i];
    if(!pocket_d20_read_numbers(file, "SaveProficiency", n, POCKET_D20_ABILITY_COUNT))
        return false;
    for(uint8_t i = 0U; i < POCKET_D20_ABILITY_COUNT; ++i)
        c->saving_throw_proficiency[i] = (uint8_t)n[i];
    if(!pocket_d20_read_numbers(file, "SaveMisc", n, POCKET_D20_ABILITY_COUNT)) return false;
    for(uint8_t i = 0U; i < POCKET_D20_ABILITY_COUNT; ++i) c->saving_throw_misc[i] = (int8_t)n[i];
    if(!pocket_d20_read_numbers(file, "SkillProficiency", n, POCKET_D20_SKILL_COUNT))
        return false;
    for(uint8_t i = 0U; i < POCKET_D20_SKILL_COUNT; ++i)
        c->skill_proficiency[i] = (uint8_t)n[i];
    if(!pocket_d20_read_numbers(file, "SkillMisc", n, POCKET_D20_SKILL_COUNT)) return false;
    for(uint8_t i = 0U; i < POCKET_D20_SKILL_COUNT; ++i) c->skill_misc[i] = (int8_t)n[i];

    if(!pocket_d20_read_numbers(file, "Vitals", n, 12U)) return false;
    c->hp_current = (int16_t)n[0];
    c->hp_max = (int16_t)n[1];
    c->hp_temporary = (int16_t)n[2];
    c->armor_class = (int16_t)n[3];
    c->speed = (int16_t)n[4];
    c->initiative_misc = (int8_t)n[5];
    c->exhaustion = (uint8_t)n[6];
    c->death_successes = (uint8_t)n[7];
    c->death_failures = (uint8_t)n[8];
    c->hit_die = (uint8_t)n[9];
    c->hit_dice_current = (uint8_t)n[10];
    c->hit_dice_max = (uint8_t)n[11];
    if(!pocket_d20_read_numbers(file, "Spellcasting", n, 4U)) return false;
    c->spellcasting_ability = (uint8_t)n[0];
    c->spell_attack_misc = (int8_t)n[1];
    c->spell_save_misc = (int8_t)n[2];
    c->arcane_recovery_used = (uint8_t)n[3];
    if(!pocket_d20_read_numbers(file, "SpellSlotsCurrent", n, POCKET_D20_SLOT_COUNT))
        return false;
    for(uint8_t i = 0U; i < POCKET_D20_SLOT_COUNT; ++i)
        c->spell_slots_current[i] = (uint8_t)n[i];
    if(!pocket_d20_read_numbers(file, "SpellSlotsMax", n, POCKET_D20_SLOT_COUNT))
        return false;
    for(uint8_t i = 0U; i < POCKET_D20_SLOT_COUNT; ++i) c->spell_slots_max[i] = (uint8_t)n[i];
    if(!pocket_d20_read_numbers(file, "Currency", n, 5U)) return false;
    c->currency_cp = n[0];
    c->currency_sp = n[1];
    c->currency_ep = n[2];
    c->currency_gp = n[3];
    c->currency_pp = n[4];

    if(!pocket_d20_read_value(file, "SpellCount", value, sizeof(value))) return false;
    c->spell_count = (uint8_t)strtoul(value, NULL, 10);
    if(c->spell_count > POCKET_D20_MAX_SPELLS) return false;
    for(uint8_t i = 0U; i < c->spell_count; ++i) {
        snprintf(key, sizeof(key), "Spell%uName", i);
        if(!pocket_d20_read_string(file, key, c->spells[i].name, sizeof(c->spells[i].name)))
            return false;
        snprintf(key, sizeof(key), "Spell%uDetail", i);
        if(!pocket_d20_read_string(file, key, c->spells[i].detail, sizeof(c->spells[i].detail)))
            return false;
        snprintf(key, sizeof(key), "Spell%uData", i);
        if(!pocket_d20_read_numbers(file, key, n, 8U)) return false;
        c->spells[i].level = (uint8_t)n[0];
        c->spells[i].class_index = (uint8_t)n[1];
        c->spells[i].prepared = (uint8_t)n[2];
        c->spells[i].ritual = (uint8_t)n[3];
        c->spell_known[i] = (uint8_t)n[4];
        c->spell_always_prepared[i] = (uint8_t)n[5];
        c->spell_free_casts_current[i] = (uint8_t)n[6];
        c->spell_free_casts_max[i] = (uint8_t)n[7];
    }

    if(!pocket_d20_read_value(file, "FeatureCount", value, sizeof(value))) return false;
    c->feature_count = (uint8_t)strtoul(value, NULL, 10);
    if(c->feature_count > POCKET_D20_MAX_FEATURES) return false;
    for(uint8_t i = 0U; i < c->feature_count; ++i) {
        snprintf(key, sizeof(key), "Feature%uName", i);
        if(!pocket_d20_read_string(file, key, c->features[i].name, sizeof(c->features[i].name)))
            return false;
        snprintf(key, sizeof(key), "Feature%uDetail", i);
        if(!pocket_d20_read_string(
               file, key, c->features[i].detail, sizeof(c->features[i].detail)))
            return false;
        snprintf(key, sizeof(key), "Feature%uData", i);
        if(!pocket_d20_read_numbers(file, key, n, 5U)) return false;
        c->features[i].uses_current = (int16_t)n[0];
        c->features[i].uses_max = (int16_t)n[1];
        c->features[i].class_index = (uint8_t)n[2];
        c->features[i].class_level_gained = (uint8_t)n[3];
        c->features[i].recharge = (uint8_t)n[4];
    }

    if(!pocket_d20_read_value(file, "ItemCount", value, sizeof(value))) return false;
    c->item_count = (uint8_t)strtoul(value, NULL, 10);
    if(c->item_count > POCKET_D20_MAX_ITEMS) return false;
    for(uint8_t i = 0U; i < c->item_count; ++i) {
        PocketItem* item = &c->items[i];
        snprintf(key, sizeof(key), "Item%uName", i);
        if(!pocket_d20_read_string(file, key, item->name, sizeof(item->name))) return false;
        snprintf(key, sizeof(key), "Item%uDetail", i);
        if(!pocket_d20_read_string(file, key, item->detail, sizeof(item->detail))) return false;
        snprintf(key, sizeof(key), "Item%uData", i);
        if(!pocket_d20_read_numbers(file, key, n, 19U)) return false;
        item->quantity = (int16_t)n[0];
        item->weight_tenths = (int16_t)n[1];
        item->equipped = (uint8_t)n[2];
        item->attuned = (uint8_t)n[3];
        item->is_weapon = (uint8_t)n[4];
        item->attack_ability = (uint8_t)n[5];
        item->proficient = (uint8_t)n[6];
        item->magic_bonus = (int8_t)n[7];
        item->damage_dice = (uint8_t)n[8];
        item->damage_die = (uint8_t)n[9];
        item->versatile_die = (uint8_t)n[10];
        item->use_versatile = (uint8_t)n[11];
        item->damage_type = (uint8_t)n[12];
        item->add_ability_damage = (uint8_t)n[13];
        item->extra_dice = (uint8_t)n[14];
        item->extra_die = (uint8_t)n[15];
        item->weapon_properties = (uint16_t)n[16];
        item->ammo_current = (int16_t)n[17];
        item->ammo_max = (int16_t)n[18];
    }

    if(!pocket_d20_read_value(file, "LanguageCount", value, sizeof(value))) return false;
    c->language_count = (uint8_t)strtoul(value, NULL, 10);
    if(c->language_count > POCKET_D20_MAX_LANGUAGES) return false;
    for(uint8_t i = 0U; i < c->language_count; ++i) {
        snprintf(key, sizeof(key), "Language%u", i);
        if(!pocket_d20_read_string(file, key, c->languages[i], sizeof(c->languages[i])))
            return false;
    }

    if(!pocket_d20_read_value(file, "JournalCount", value, sizeof(value))) return false;
    c->journal_count = (uint8_t)strtoul(value, NULL, 10);
    if(c->journal_count > POCKET_D20_MAX_JOURNAL) return false;
    for(uint8_t i = 0U; i < c->journal_count; ++i) {
        snprintf(key, sizeof(key), "Journal%uTitle", i);
        if(!pocket_d20_read_string(file, key, c->journal[i].title, sizeof(c->journal[i].title)))
            return false;
        snprintf(key, sizeof(key), "Journal%uBody", i);
        if(!pocket_d20_read_string(file, key, c->journal[i].body, sizeof(c->journal[i].body)))
            return false;
        snprintf(key, sizeof(key), "Journal%uData", i);
        if(!pocket_d20_read_numbers(file, key, n, 4U)) return false;
        c->journal[i].category = (uint8_t)n[0];
        c->journal[i].completed = (uint8_t)n[1];
        c->journal[i].level_granted = (uint8_t)n[2];
        c->journal[i].class_index = (uint8_t)n[3];
    }

    if(!pocket_d20_read_value(file, "PartyCount", value, sizeof(value))) return false;
    data->party_count = (uint8_t)strtoul(value, NULL, 10);
    if(data->party_count > POCKET_D20_MAX_PARTY) return false;
    for(uint8_t i = 0U; i < data->party_count; ++i) {
        snprintf(key, sizeof(key), "Party%uName", i);
        if(!pocket_d20_read_string(file, key, data->party[i].name, sizeof(data->party[i].name)))
            return false;
        snprintf(key, sizeof(key), "Party%uInitiativeModifier", i);
        if(!pocket_d20_read_value(file, key, value, sizeof(value))) return false;
        data->party[i].initiative_modifier = (int8_t)strtol(value, NULL, 10);
    }

    if(!pocket_d20_read_numbers(file, "InitiativeState", n, 4U)) return false;
    data->initiative.active = (uint8_t)n[0];
    data->initiative.round = (uint16_t)n[1];
    data->initiative.current_turn = (uint8_t)n[2];
    data->initiative.count = (uint8_t)n[3];
    if(data->initiative.count > POCKET_D20_MAX_INITIATIVE) return false;
    for(uint8_t i = 0U; i < data->initiative.count; ++i) {
        snprintf(key, sizeof(key), "Initiative%uName", i);
        if(!pocket_d20_read_string(
               file,
               key,
               data->initiative.entries[i].name,
               sizeof(data->initiative.entries[i].name)))
            return false;
        snprintf(key, sizeof(key), "Initiative%uData", i);
        if(!pocket_d20_read_numbers(file, key, n, 3U)) return false;
        data->initiative.entries[i].initiative_modifier = (int8_t)n[0];
        data->initiative.entries[i].initiative_total = (int16_t)n[1];
        data->initiative.entries[i].is_player_character = (uint8_t)n[2];
    }

    if(!pocket_d20_read_value(file, "DataChecksum", value, sizeof(value))) return false;
    uint32_t expected_checksum = (uint32_t)strtoul(value, NULL, 16);
    pocket_d20_data_sanitize(data);
    return expected_checksum == pocket_d20_checksum(data, sizeof(*data));
}

static bool pocket_d20_load_text_path(Storage* storage, const char* path, PocketSaveData* data) {
    File* file = storage_file_alloc(storage);
    bool success = storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING) &&
                   pocket_d20_read_character(file, data);
    storage_file_close(file);
    storage_file_free(file);
    return success;
}

static bool pocket_d20_replace_file(
    Storage* storage,
    const char* path,
    const char* temp_path,
    const char* backup_path) {
    storage_common_remove(storage, backup_path);
    bool had_save = storage_file_exists(storage, path);
    if(had_save && storage_common_rename(storage, path, backup_path) != FSE_OK) return false;
    if(storage_common_rename(storage, temp_path, path) != FSE_OK) {
        if(had_save) storage_common_rename(storage, backup_path, path);
        return false;
    }
    return true;
}

bool pocket_d20_storage_save_profile(
    Storage* storage,
    uint8_t profile,
    const PocketSaveData* data) {
    furi_assert(storage);
    furi_assert(data);
    if(profile >= POCKET_D20_MAX_PROFILES) return false;
    storage_common_mkdir(storage, POCKET_D20_DATA_DIR);
    char path[96];
    char temp_path[96];
    char backup_path[96];
    pocket_d20_profile_path(path, sizeof(path), profile, "");
    pocket_d20_profile_path(temp_path, sizeof(temp_path), profile, ".tmp");
    pocket_d20_profile_path(backup_path, sizeof(backup_path), profile, ".bak");
    PocketSaveData* normalized = malloc(sizeof(PocketSaveData));
    if(!normalized) return false;
    memcpy(normalized, data, sizeof(PocketSaveData));
    pocket_d20_data_sanitize(normalized);
    pocket_d20_normalize_unused(normalized);
    File* file = storage_file_alloc(storage);
    bool written = storage_file_open(file, temp_path, FSAM_WRITE, FSOM_CREATE_ALWAYS) &&
                   pocket_d20_write_character(file, normalized) && storage_file_sync(file);
    storage_file_close(file);
    storage_file_free(file);
    free(normalized);
    if(!written) {
        storage_common_remove(storage, temp_path);
        return false;
    }
    return pocket_d20_replace_file(storage, path, temp_path, backup_path);
}

bool pocket_d20_storage_load_profile(
    Storage* storage,
    uint8_t profile,
    PocketSaveData* data,
    bool* recovered_backup) {
    furi_assert(storage);
    furi_assert(data);
    if(recovered_backup) *recovered_backup = false;
    if(profile >= POCKET_D20_MAX_PROFILES) return false;
    char path[96];
    char backup_path[96];
    pocket_d20_profile_path(path, sizeof(path), profile, "");
    pocket_d20_profile_path(backup_path, sizeof(backup_path), profile, ".bak");
    if(pocket_d20_load_text_path(storage, path, data)) return true;
    if(pocket_d20_load_text_path(storage, backup_path, data)) {
        if(recovered_backup) *recovered_backup = true;
        return true;
    }
    pocket_d20_data_set_defaults(data);
    return false;
}

static bool pocket_d20_remove_if_present(Storage* storage, const char* path) {
    return !storage_file_exists(storage, path) || storage_common_remove(storage, path) == FSE_OK;
}

bool pocket_d20_storage_delete_profile(Storage* storage, uint8_t profile) {
    if(profile >= POCKET_D20_MAX_PROFILES) return false;
    char path[96];
    char temp_path[96];
    char backup_path[96];
    pocket_d20_profile_path(path, sizeof(path), profile, "");
    pocket_d20_profile_path(temp_path, sizeof(temp_path), profile, ".tmp");
    pocket_d20_profile_path(backup_path, sizeof(backup_path), profile, ".bak");
    return pocket_d20_remove_if_present(storage, path) &&
           pocket_d20_remove_if_present(storage, temp_path) &&
           pocket_d20_remove_if_present(storage, backup_path);
}

void pocket_d20_profiles_set_defaults(PocketProfileState* profiles) {
    memset(profiles, 0, sizeof(*profiles));
    profiles->occupied_mask = 1U;
    pocket_d20_copy(profiles->names[0], sizeof(profiles->names[0]), "Main");
}

static bool pocket_d20_write_profiles(File* file, const PocketProfileState* profiles) {
    if(!pocket_d20_writef(
           file,
           "PocketD20Profiles=1\nActive=%u\nOccupiedMask=%u\n",
           profiles->active_profile,
           profiles->occupied_mask))
        return false;
    char key[24];
    for(uint8_t i = 0U; i < POCKET_D20_MAX_PROFILES; ++i) {
        snprintf(key, sizeof(key), "Name%u", i);
        if(!pocket_d20_write_string(file, key, profiles->names[i])) return false;
    }
    return pocket_d20_writef(
        file,
        "ProfilesChecksum=%08lX\n",
        (unsigned long)pocket_d20_checksum(profiles, sizeof(*profiles)));
}

static bool pocket_d20_load_profiles_path(
    Storage* storage,
    const char* path,
    PocketProfileState* profiles) {
    File* file = storage_file_alloc(storage);
    if(!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_free(file);
        return false;
    }
    pocket_d20_profiles_set_defaults(profiles);
    char value[128];
    bool success = pocket_d20_read_value(file, "PocketD20Profiles", value, sizeof(value)) &&
                   strtoul(value, NULL, 10) == 1U &&
                   pocket_d20_read_value(file, "Active", value, sizeof(value));
    if(success) profiles->active_profile = (uint8_t)strtoul(value, NULL, 10);
    success = success && pocket_d20_read_value(file, "OccupiedMask", value, sizeof(value));
    if(success) profiles->occupied_mask = (uint8_t)strtoul(value, NULL, 10);
    char key[24];
    for(uint8_t i = 0U; success && i < POCKET_D20_MAX_PROFILES; ++i) {
        snprintf(key, sizeof(key), "Name%u", i);
        success = pocket_d20_read_string(
            file, key, profiles->names[i], sizeof(profiles->names[i]));
    }
    success = success && pocket_d20_read_value(file, "ProfilesChecksum", value, sizeof(value));
    if(success) {
        uint32_t expected = (uint32_t)strtoul(value, NULL, 16);
        success = expected == pocket_d20_checksum(profiles, sizeof(*profiles));
    }
    storage_file_close(file);
    storage_file_free(file);
    return success;
}

bool pocket_d20_profiles_load(Storage* storage, PocketProfileState* profiles) {
    furi_assert(storage);
    furi_assert(profiles);
    if(!pocket_d20_load_profiles_path(storage, POCKET_D20_PROFILES_PATH, profiles) &&
       !pocket_d20_load_profiles_path(storage, POCKET_D20_PROFILES_BACKUP_PATH, profiles)) {
        pocket_d20_profiles_set_defaults(profiles);
        return false;
    }
    profiles->occupied_mask &= (uint8_t)((1U << POCKET_D20_MAX_PROFILES) - 1U);
    profiles->occupied_mask |= 1U;
    if(profiles->active_profile >= POCKET_D20_MAX_PROFILES ||
       !(profiles->occupied_mask & (1U << profiles->active_profile)))
        profiles->active_profile = 0U;
    for(uint8_t i = 0U; i < POCKET_D20_MAX_PROFILES; ++i)
        profiles->names[i][POCKET_D20_NAME_LEN - 1U] = '\0';
    return true;
}

bool pocket_d20_profiles_save(Storage* storage, const PocketProfileState* profiles) {
    furi_assert(storage);
    furi_assert(profiles);
    storage_common_mkdir(storage, POCKET_D20_DATA_DIR);
    File* file = storage_file_alloc(storage);
    bool written = storage_file_open(
                       file,
                       POCKET_D20_PROFILES_TEMP_PATH,
                       FSAM_WRITE,
                       FSOM_CREATE_ALWAYS) &&
                   pocket_d20_write_profiles(file, profiles) && storage_file_sync(file);
    storage_file_close(file);
    storage_file_free(file);
    if(!written) {
        storage_common_remove(storage, POCKET_D20_PROFILES_TEMP_PATH);
        return false;
    }
    return pocket_d20_replace_file(
        storage,
        POCKET_D20_PROFILES_PATH,
        POCKET_D20_PROFILES_TEMP_PATH,
        POCKET_D20_PROFILES_BACKUP_PATH);
}
