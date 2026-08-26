#include "pocket_d20_monsters.h"

#include <furi.h>
#include <furi_hal_random.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MONSTER_BUNDLED_INDEX APP_ASSETS_PATH("monsters/index.txt")
#define MONSTER_USER_INDEX APP_ASSETS_PATH("monsters/custom_index.txt")
#define MONSTER_BUNDLED_BLOCK APP_ASSETS_PATH("monsters/statblocks/%s.txt")
#define MONSTER_USER_BLOCK APP_ASSETS_PATH("monsters/statblocks/custom_%s.txt")
#define MONSTER_LINE_LEN 768U

static const uint16_t pocket_budget[20][3] = {
    {50,75,100},{100,150,200},{150,225,400},{250,375,500},{500,750,1100},
    {600,1000,1400},{750,1300,1700},{1000,1700,2100},{1300,2000,2600},
    {1600,2300,3100},{1900,2900,4100},{2200,3700,4700},{2600,4200,5400},
    {2900,4900,6200},{3300,5400,7800},{3800,6100,9800},{4500,7200,11700},
    {5000,8700,14200},{5500,10700,17200},{6400,13200,22000},
};

static void monster_copy(char* out, size_t size, const char* value) {
    if(!size) return;
    strncpy(out, value ? value : "", size - 1U);
    out[size - 1U] = '\0';
}

static bool monster_read_line(File* file, char* line, size_t size) {
    size_t position = 0U;
    char value;
    while(position + 1U < size && storage_file_read(file, &value, 1U) == 1U) {
        if(value == '\r') continue;
        if(value == '\n') break;
        line[position++] = value;
    }
    line[position] = '\0';
    return position > 0U;
}

static bool monster_parse_summary(char* line, PocketMonsterSummary* output) {
    if(!line[0] || line[0] == '#') return false;
    char* cursor = line;
    char* extended[10] = {0};
    uint8_t field_count = 0U;
    while(field_count < 10U) {
        extended[field_count++] = cursor;
        char* separator = strchr(cursor, '|');
        if(!separator) break;
        *separator = '\0';
        cursor = separator + 1U;
    }
    if(field_count < 8U) return false;
    memset(output, 0, sizeof(*output));
    monster_copy(output->id, sizeof(output->id), extended[0]);
    monster_copy(output->name, sizeof(output->name), extended[1]);
    output->cr_eighths = (uint8_t)strtoul(extended[2], NULL, 10);
    output->xp = strtoul(extended[3], NULL, 10);
    output->armor_class = (uint8_t)strtoul(extended[4], NULL, 10);
    output->hit_points = (uint16_t)strtoul(extended[5], NULL, 10);
    monster_copy(output->type, sizeof(output->type), extended[6]);
    monster_copy(output->environment, sizeof(output->environment), extended[7]);
    monster_copy(output->source, sizeof(output->source), field_count > 8U ? extended[8] : "Custom");
    monster_copy(output->role, sizeof(output->role), field_count > 9U ? extended[9] : "Any");
    return output->id[0] && output->name[0] && output->xp;
}

static uint16_t monster_count_path(Storage* storage, const char* path) {
    File* file = storage_file_alloc(storage);
    uint16_t count = 0U;
    if(storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        char line[MONSTER_LINE_LEN];
        PocketMonsterSummary summary;
        while(monster_read_line(file, line, sizeof(line)))
            if(monster_parse_summary(line, &summary) && count < UINT16_MAX) ++count;
    }
    storage_file_close(file);
    storage_file_free(file);
    return count;
}

static bool monster_at_path(
    Storage* storage,
    const char* path,
    uint16_t wanted,
    PocketMonsterSummary* output) {
    File* file = storage_file_alloc(storage);
    bool found = false;
    if(storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        char line[MONSTER_LINE_LEN];
        uint16_t index = 0U;
        while(monster_read_line(file, line, sizeof(line))) {
            PocketMonsterSummary summary;
            if(!monster_parse_summary(line, &summary)) continue;
            if(index++ == wanted) {
                *output = summary;
                found = true;
                break;
            }
        }
    }
    storage_file_close(file);
    storage_file_free(file);
    return found;
}

uint32_t pocket_monster_xp_budget(
    uint8_t party_level,
    uint8_t party_size,
    PocketEncounterDifficulty difficulty) {
    if(party_level < 1U) party_level = 1U;
    if(party_level > 20U) party_level = 20U;
    if(party_size < 1U) party_size = 1U;
    if(party_size > 12U) party_size = 12U;
    if(difficulty >= PocketEncounterDifficultyCount) difficulty = PocketEncounterModerate;
    return (uint32_t)pocket_budget[party_level - 1U][difficulty] * party_size;
}

uint16_t pocket_monster_count(Storage* storage) {
    uint32_t total = monster_count_path(storage, MONSTER_BUNDLED_INDEX) +
                     monster_count_path(storage, MONSTER_USER_INDEX);
    return total > UINT16_MAX ? UINT16_MAX : (uint16_t)total;
}

bool pocket_monster_at(Storage* storage, uint16_t index, PocketMonsterSummary* output) {
    uint16_t bundled = monster_count_path(storage, MONSTER_BUNDLED_INDEX);
    return index < bundled ? monster_at_path(storage, MONSTER_BUNDLED_INDEX, index, output) :
                             monster_at_path(storage, MONSTER_USER_INDEX, index - bundled, output);
}

static void monster_query_path(
    Storage* storage,
    const char* path,
    PocketMonsterFilter filter,
    void* context,
    uint16_t start,
    PocketMonsterSummary* output,
    uint16_t capacity,
    uint16_t* matched,
    uint16_t* loaded) {
    File* file = storage_file_alloc(storage);
    if(storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        char line[MONSTER_LINE_LEN];
        while(monster_read_line(file, line, sizeof(line))) {
            PocketMonsterSummary summary;
            if(!monster_parse_summary(line, &summary) ||
               (filter && !filter(&summary, context)))
                continue;
            if(*matched >= start && *loaded < capacity && output)
                output[(*loaded)++] = summary;
            if(*matched < UINT16_MAX) ++*matched;
        }
    }
    storage_file_close(file);
    storage_file_free(file);
}

uint16_t pocket_monster_query(
    Storage* storage,
    PocketMonsterFilter filter,
    void* context,
    uint16_t start,
    PocketMonsterSummary* output,
    uint16_t capacity,
    uint16_t* total_matches) {
    uint16_t matched = 0U;
    uint16_t loaded = 0U;
    monster_query_path(
        storage,
        MONSTER_BUNDLED_INDEX,
        filter,
        context,
        start,
        output,
        capacity,
        &matched,
        &loaded);
    monster_query_path(
        storage,
        MONSTER_USER_INDEX,
        filter,
        context,
        start,
        output,
        capacity,
        &matched,
        &loaded);
    if(total_matches) *total_matches = matched;
    return loaded;
}

static bool monster_load_path(Storage* storage, const char* path, PocketMonsterDetail* output) {
    File* file = storage_file_alloc(storage);
    bool opened = storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING);
    if(opened) {
        char line[MONSTER_LINE_LEN];
        while(monster_read_line(file, line, sizeof(line))) {
            char* separator = strchr(line, '=');
            if(!separator) continue;
            *separator++ = '\0';
            if(!strcmp(line, "SizeAlignment")) { monster_copy(output->size_alignment, sizeof(output->size_alignment), separator); output->present_fields |= PocketMonsterFieldSize; }
            else if(!strcmp(line, "Speed")) { monster_copy(output->speed, sizeof(output->speed), separator); output->present_fields |= PocketMonsterFieldSpeed; }
            else if(!strcmp(line, "Abilities")) { if(sscanf(separator, "%hhd,%hhd,%hhd,%hhd,%hhd,%hhd", &output->abilities[0], &output->abilities[1], &output->abilities[2], &output->abilities[3], &output->abilities[4], &output->abilities[5]) == 6) output->present_fields |= PocketMonsterFieldAbilities; }
            else if(!strcmp(line, "Skills")) monster_copy(output->skills, sizeof(output->skills), separator);
            else if(!strcmp(line, "Defenses")) monster_copy(output->defenses, sizeof(output->defenses), separator);
            else if(!strcmp(line, "Senses")) { monster_copy(output->senses, sizeof(output->senses), separator); output->present_fields |= PocketMonsterFieldSenses; }
            else if(!strcmp(line, "Languages")) { monster_copy(output->languages, sizeof(output->languages), separator); output->present_fields |= PocketMonsterFieldLanguages; }
            else if(!strcmp(line, "Traits")) monster_copy(output->traits, sizeof(output->traits), separator);
            else if(!strcmp(line, "Actions")) { monster_copy(output->actions, sizeof(output->actions), separator); output->present_fields |= PocketMonsterFieldActions; }
            else if(!strcmp(line, "Extra")) monster_copy(output->extra, sizeof(output->extra), separator);
        }
    }
    storage_file_close(file);
    storage_file_free(file);
    return opened;
}

static uint8_t monster_pack_version_path(Storage* storage, const char* path, bool* present) {
    File* file = storage_file_alloc(storage);
    uint8_t version = 0U;
    *present = storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING);
    if(*present) {
        char line[MONSTER_LINE_LEN];
        for(uint8_t i = 0U; i < 4U && monster_read_line(file, line, sizeof(line)); ++i)
            if(sscanf(line, "# MonsterPack=%hhu", &version) == 1) break;
    }
    storage_file_close(file);
    storage_file_free(file);
    return version;
}

void pocket_monster_pack_versions(
    Storage* storage,
    uint8_t* bundled_version,
    uint8_t* user_version,
    bool* user_present) {
    bool bundled_present = false;
    *bundled_version = monster_pack_version_path(storage, MONSTER_BUNDLED_INDEX, &bundled_present);
    *user_version = monster_pack_version_path(storage, MONSTER_USER_INDEX, user_present);
}

bool pocket_monster_load(Storage* storage, const PocketMonsterSummary* summary, PocketMonsterDetail* output) {
    memset(output, 0, sizeof(*output));
    output->summary = *summary;
    char path[192];
    snprintf(path, sizeof(path), MONSTER_USER_BLOCK, summary->id);
    if(monster_load_path(storage, path, output)) return true;
    snprintf(path, sizeof(path), MONSTER_BUNDLED_BLOCK, summary->id);
    return monster_load_path(storage, path, output);
}

static bool monster_write(File* file, const char* text) {
    size_t size = strlen(text);
    return storage_file_write(file, text, size) == size;
}

static void monster_safe_id(char* output, size_t size, const char* name) {
    size_t position = 0U;
    for(size_t i = 0U; name[i] && position + 1U < size; ++i) {
        char value = name[i];
        if(value >= 'A' && value <= 'Z') value = (char)(value + ('a' - 'A'));
        if((value >= 'a' && value <= 'z') || (value >= '0' && value <= '9'))
            output[position++] = value;
        else if(position && output[position - 1U] != '_')
            output[position++] = '_';
    }
    while(position && output[position - 1U] == '_') --position;
    if(!position) monster_copy(output, size, "custom_monster");
    else output[position] = '\0';
}

static bool monster_exists(Storage* storage, const char* path) {
    FileInfo info;
    return storage_common_stat(storage, path, &info) == FSE_OK;
}

static bool monster_publish(
    Storage* storage,
    const char* temp_path,
    const char* final_path,
    const char* backup_path) {
    storage_common_remove(storage, backup_path);
    bool had_final = storage_common_rename(storage, final_path, backup_path) == FSE_OK;
    if(storage_common_rename(storage, temp_path, final_path) == FSE_OK) return true;
    if(had_final) storage_common_rename(storage, backup_path, final_path);
    return false;
}

static bool monster_format_summary(
    const PocketMonsterSummary* summary,
    char* line,
    size_t size) {
    int length = snprintf(line, size, "%s|%s|%u|%lu|%u|%u|%s|%s|%s|%s",
        summary->id, summary->name, summary->cr_eighths, (unsigned long)summary->xp,
        summary->armor_class, summary->hit_points, summary->type, summary->environment,
        summary->source, summary->role);
    return length > 0 && (size_t)length < size;
}

static bool monster_write_block_temp(
    Storage* storage,
    const char* path,
    const PocketMonsterDetail* detail) {
    File* block = storage_file_alloc(storage);
    bool ok = storage_file_open(block, path, FSAM_WRITE, FSOM_CREATE_ALWAYS);
    char line[MONSTER_LINE_LEN];
#define MONSTER_WRITE_FIELD(key, value) do { \
    int length = snprintf(line, sizeof(line), key "=%s\n", value); \
    if(length <= 0 || (size_t)length >= sizeof(line) || !monster_write(block, line)) ok = false; \
} while(false)
    if(ok) MONSTER_WRITE_FIELD("Name", detail->summary.name);
    if(ok) {
        int length = snprintf(line, sizeof(line),
            "Summary=%u,%lu,%u,%u,%s,%s,%s,%s\n", detail->summary.cr_eighths,
            (unsigned long)detail->summary.xp, detail->summary.armor_class,
            detail->summary.hit_points, detail->summary.type, detail->summary.environment,
            detail->summary.source, detail->summary.role);
        ok = length > 0 && (size_t)length < sizeof(line) && monster_write(block, line);
    }
    if(ok) MONSTER_WRITE_FIELD("SizeAlignment", detail->size_alignment);
    if(ok) MONSTER_WRITE_FIELD("Speed", detail->speed);
    if(ok) {
        int length = snprintf(line, sizeof(line), "Abilities=%d,%d,%d,%d,%d,%d\n",
            detail->abilities[0], detail->abilities[1], detail->abilities[2],
            detail->abilities[3], detail->abilities[4], detail->abilities[5]);
        ok = length > 0 && (size_t)length < sizeof(line) && monster_write(block, line);
    }
    if(ok) MONSTER_WRITE_FIELD("Skills", detail->skills);
    if(ok) MONSTER_WRITE_FIELD("Defenses", detail->defenses);
    if(ok) MONSTER_WRITE_FIELD("Senses", detail->senses);
    if(ok) MONSTER_WRITE_FIELD("Languages", detail->languages);
    if(ok) MONSTER_WRITE_FIELD("Traits", detail->traits);
    if(ok) MONSTER_WRITE_FIELD("Actions", detail->actions);
    if(ok) MONSTER_WRITE_FIELD("Extra", detail->extra);
#undef MONSTER_WRITE_FIELD
    storage_file_close(block);
    storage_file_free(block);
    if(!ok) storage_common_remove(storage, path);
    return ok;
}

static bool monster_rewrite_index(
    Storage* storage,
    const PocketMonsterSummary* replacement,
    const char* remove_id) {
    const char* temp_path = APP_ASSETS_PATH("monsters/custom_index.tmp");
    const char* backup_path = APP_ASSETS_PATH("monsters/custom_index.bak");
    File* output = storage_file_alloc(storage);
    bool ok = storage_file_open(output, temp_path, FSAM_WRITE, FSOM_CREATE_ALWAYS) &&
        monster_write(output, "# MonsterPack=1\n# id|name|CR eighths|XP|AC|HP|type|environment|source|role\n");
    bool replaced = false;
    File* input = storage_file_alloc(storage);
    if(storage_file_open(input, MONSTER_USER_INDEX, FSAM_READ, FSOM_OPEN_EXISTING)) {
        char line[MONSTER_LINE_LEN];
        while(ok && monster_read_line(input, line, sizeof(line))) {
            PocketMonsterSummary current;
            if(!monster_parse_summary(line, &current)) continue;
            if(remove_id && !strcmp(current.id, remove_id)) continue;
            if(replacement && !strcmp(current.id, replacement->id)) {
                char formatted[384];
                ok = monster_format_summary(replacement, formatted, sizeof(formatted)) &&
                     monster_write(output, formatted) && monster_write(output, "\n");
                replaced = true;
            } else {
                ok = monster_write(output, line) && monster_write(output, "\n");
            }
        }
    }
    storage_file_close(input);
    storage_file_free(input);
    if(ok && replacement && !replaced) {
        char formatted[384];
        ok = monster_format_summary(replacement, formatted, sizeof(formatted)) &&
             monster_write(output, formatted) && monster_write(output, "\n");
    }
    storage_file_close(output);
    storage_file_free(output);
    if(!ok) {
        storage_common_remove(storage, temp_path);
        return false;
    }
    ok = monster_publish(storage, temp_path, MONSTER_USER_INDEX, backup_path);
    if(ok) storage_common_remove(storage, backup_path);
    return ok;
}

static bool monster_write_transaction(Storage* storage, const char* action, const char* value) {
    File* file = storage_file_alloc(storage);
    const char* path = APP_ASSETS_PATH("monsters/custom_transaction.txt");
    bool ok = storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS) &&
        monster_write(file, action) && monster_write(file, "|") &&
        monster_write(file, value) && monster_write(file, "\n");
    storage_file_close(file);
    storage_file_free(file);
    if(!ok) storage_common_remove(storage, path);
    return ok;
}

static void monster_sanitize_summary(PocketMonsterSummary* summary) {
    char* fields[] = {summary->name, summary->type, summary->environment,
                      summary->source, summary->role};
    for(size_t field = 0U; field < sizeof(fields) / sizeof(fields[0]); ++field)
        for(char* p = fields[field]; *p; ++p)
            if(*p == '|' || *p == '\n' || *p == '\r') *p = '-';
}

static bool monster_save_custom_common(
    Storage* storage,
    PocketMonsterDetail* detail,
    bool preserve_id) {
    storage_common_mkdir(storage, APP_ASSETS_PATH("monsters"));
    storage_common_mkdir(storage, APP_ASSETS_PATH("monsters/statblocks"));
    if(!preserve_id || !detail->summary.id[0]) {
        char base[20];
        monster_safe_id(base, sizeof(base), detail->summary.name);
        snprintf(detail->summary.id, sizeof(detail->summary.id), "%s_%04lx", base,
                 (unsigned long)(furi_hal_random_get() & 0xFFFFU));
    }
    monster_copy(detail->summary.source, sizeof(detail->summary.source), "Custom");
    if(!detail->summary.role[0])
        monster_copy(detail->summary.role, sizeof(detail->summary.role), "Skirmisher");
    monster_sanitize_summary(&detail->summary);
    char index_line[384];
    if(!monster_format_summary(&detail->summary, index_line, sizeof(index_line)) ||
       !monster_write_transaction(storage, "UPSERT", index_line)) return false;
    char path[192], temp_path[192], backup_path[192];
    snprintf(path, sizeof(path), MONSTER_USER_BLOCK, detail->summary.id);
    snprintf(temp_path, sizeof(temp_path), APP_ASSETS_PATH("monsters/statblocks/custom_%s.tmp"), detail->summary.id);
    snprintf(backup_path, sizeof(backup_path), APP_ASSETS_PATH("monsters/statblocks/custom_%s.bak"), detail->summary.id);
    bool ok = monster_write_block_temp(storage, temp_path, detail) &&
              monster_publish(storage, temp_path, path, backup_path) &&
              monster_rewrite_index(storage, &detail->summary, NULL);
    if(ok) {
        storage_common_remove(storage, backup_path);
        storage_common_remove(storage, APP_ASSETS_PATH("monsters/custom_transaction.txt"));
    } else {
        storage_common_remove(storage, temp_path);
        if(monster_exists(storage, backup_path)) {
            storage_common_remove(storage, path);
            storage_common_rename(storage, backup_path, path);
        }
    }
    return ok;
}

bool pocket_monster_save_custom(Storage* storage, PocketMonsterDetail* detail) {
    return monster_save_custom_common(storage, detail, false);
}

bool pocket_monster_update_custom(Storage* storage, PocketMonsterDetail* detail) {
    return detail->summary.id[0] && !strcmp(detail->summary.source, "Custom") &&
           monster_save_custom_common(storage, detail, true);
}

bool pocket_monster_delete_custom(Storage* storage, const PocketMonsterSummary* summary) {
    if(!summary || strcmp(summary->source, "Custom")) return false;
    char path[192], deleted_path[192];
    snprintf(path, sizeof(path), MONSTER_USER_BLOCK, summary->id);
    snprintf(deleted_path, sizeof(deleted_path),
             APP_ASSETS_PATH("monsters/statblocks/custom_%s.deleted"), summary->id);
    storage_common_remove(storage, deleted_path);
    if(!monster_write_transaction(storage, "DELETE", summary->id) ||
       storage_common_rename(storage, path, deleted_path) != FSE_OK) return false;
    bool ok = monster_rewrite_index(storage, NULL, summary->id);
    if(ok) {
        storage_common_remove(storage, deleted_path);
        storage_common_remove(storage, APP_ASSETS_PATH("monsters/custom_transaction.txt"));
    } else {
        storage_common_rename(storage, deleted_path, path);
    }
    return ok;
}

bool pocket_monster_recover_user_pack(
    Storage* storage,
    uint16_t* recovered,
    uint16_t* rolled_back) {
    *recovered = 0U;
    *rolled_back = 0U;
    const char* transaction_path = APP_ASSETS_PATH("monsters/custom_transaction.txt");
    if(!monster_exists(storage, MONSTER_USER_INDEX) &&
       monster_exists(storage, APP_ASSETS_PATH("monsters/custom_index.bak"))) {
        if(storage_common_rename(storage, APP_ASSETS_PATH("monsters/custom_index.bak"),
                                 MONSTER_USER_INDEX) == FSE_OK) ++*rolled_back;
    }
    File* transaction = storage_file_alloc(storage);
    char line[MONSTER_LINE_LEN] = "";
    bool has_transaction = storage_file_open(
        transaction, transaction_path, FSAM_READ, FSOM_OPEN_EXISTING) &&
        monster_read_line(transaction, line, sizeof(line));
    storage_file_close(transaction);
    storage_file_free(transaction);
    if(!has_transaction) return true;
    char* value = strchr(line, '|');
    if(!value) return false;
    *value++ = '\0';
    bool ok = false;
    if(!strcmp(line, "UPSERT")) {
        PocketMonsterSummary summary;
        if(monster_parse_summary(value, &summary)) {
            char path[192], backup[192];
            snprintf(path, sizeof(path), MONSTER_USER_BLOCK, summary.id);
            snprintf(backup, sizeof(backup),
                     APP_ASSETS_PATH("monsters/statblocks/custom_%s.bak"), summary.id);
            if(monster_exists(storage, path)) {
                ok = monster_rewrite_index(storage, &summary, NULL);
                if(ok) ++*recovered;
            } else if(monster_exists(storage, backup)) {
                ok = storage_common_rename(storage, backup, path) == FSE_OK;
                if(ok) ++*rolled_back;
            }
        }
    } else if(!strcmp(line, "DELETE")) {
        char deleted[192];
        snprintf(deleted, sizeof(deleted),
                 APP_ASSETS_PATH("monsters/statblocks/custom_%s.deleted"), value);
        ok = monster_rewrite_index(storage, NULL, value);
        if(ok) {
            storage_common_remove(storage, deleted);
            ++*recovered;
        }
    }
    if(ok) {
        storage_common_remove(storage, transaction_path);
        storage_common_remove(storage, APP_ASSETS_PATH("monsters/custom_index.bak"));
    }
    return ok;
}

bool pocket_monster_generate(
    Storage* storage,
    uint8_t party_level,
    uint8_t party_size,
    PocketEncounterDifficulty difficulty,
    const char* environment,
    bool allow_repeats,
    PocketEncounterTemplate template_kind,
    const char* preferred_role,
    PocketMonsterEncounter* output) {
    memset(output, 0, sizeof(*output));
    output->budget = pocket_monster_xp_budget(party_level, party_size, difficulty);
    uint16_t total = pocket_monster_count(storage);
    if(!total) return false;
    uint8_t attempts = 0U;
    while(attempts++ < 80U && output->count < POCKET_MONSTER_ENCOUNTER_MAX) {
        PocketMonsterSummary candidate;
        if(!pocket_monster_at(storage, (uint16_t)(furi_hal_random_get() % total), &candidate)) continue;
        if(candidate.xp > output->budget - output->spent) continue;
        if(environment && strcmp(environment, "Any") &&
           strcmp(candidate.environment, environment) && (furi_hal_random_get() % 4U)) continue;
        if(preferred_role && strcmp(preferred_role, "Any") &&
           strcmp(candidate.role, preferred_role) && (furi_hal_random_get() % 4U)) continue;
        /* Avoid above-level solo threats and unwieldy hordes by default. */
        if(candidate.cr_eighths > (uint8_t)(party_level * 8U)) continue;
        if(template_kind == PocketEncounterHorde &&
           candidate.cr_eighths > (uint8_t)(party_level * 4U)) continue;
        if(template_kind == PocketEncounterElite &&
           candidate.cr_eighths < (uint8_t)(party_level * 4U)) continue;
        uint8_t existing = UINT8_MAX;
        for(uint8_t i = 0U; i < output->count; ++i)
            if(!strcmp(output->monsters[i].id, candidate.id)) existing = i;
        if(existing != UINT8_MAX) {
            if(!allow_repeats) continue;
            if(output->quantities[existing] >= party_size * 2U) continue;
            ++output->quantities[existing];
        } else {
            if(template_kind == PocketEncounterElite && output->count >= 2U) continue;
            output->monsters[output->count] = candidate;
            output->quantities[output->count++] = 1U;
        }
        output->spent += candidate.xp;
        uint32_t target = template_kind == PocketEncounterHorde ?
            output->budget * 9U / 10U : output->budget * 3U / 4U;
        if(output->spent >= target && (furi_hal_random_get() & 1U)) break;
    }
    return output->count > 0U;
}
