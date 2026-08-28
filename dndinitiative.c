#include "dnd_fs.h"
#include "dnd_handoff.h"
#include "dnd_profile_ref.h"

#include <furi.h>
#include <furi_hal_random.h>
#include <gui/gui.h>
#include <gui/modules/number_input.h>
#include <gui/modules/text_input.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <input/input.h>
#include <storage/storage.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INIT_MAX 24U
#define INIT_NAME_LEN 32U
#define INIT_CONDITION_LEN 64U
#define INIT_PATH_LEN 128U
#define INIT_FILE_PATH APP_DATA_PATH("ch_%lu.%s")

typedef struct {
    char name[INIT_NAME_LEN];
    char conditions[INIT_CONDITION_LEN];
    int16_t hp_current;
    int16_t hp_max;
    int16_t armor_class;
    int16_t total;
    int8_t modifier;
} InitiativeMember;

typedef enum {
    InitiativeScreenMenu,
    InitiativeScreenRoster,
    InitiativeScreenSetup,
    InitiativeScreenCombat,
    InitiativeScreenEdit,
} InitiativeScreen;

typedef enum {
    InitiativeViewMain,
    InitiativeViewText,
    InitiativeViewNumber,
} InitiativeView;

typedef struct {
    Gui* gui;
    Storage* storage;
    ViewDispatcher* dispatcher;
    View* view;
    TextInput* text_input;
    NumberInput* number_input;
    InitiativeMember roster[INIT_MAX];
    InitiativeMember combat[INIT_MAX];
    uint8_t roster_count;
    uint8_t combat_count;
    uint8_t active;
    uint8_t current_turn;
    uint16_t round;
    uint8_t selection;
    uint8_t scroll;
    uint8_t edit_field;
    uint8_t edit_combat;
    uint8_t text_input_active;
    uint8_t number_input_active;
    uint8_t text_target;
    uint8_t setup_number_index;
    char edit_buffer[INIT_CONDITION_LEN];
    InitiativeScreen edit_return_screen;
    uint8_t return_to_dnd;
    uint32_t character_id;
    InitiativeScreen screen;
    char status[32];
} InitiativeApp;

static void initiative_copy(char* out, size_t size, const char* in) {
    if(!size) return;
    strncpy(out, in ? in : "", size - 1U);
    out[size - 1U] = '\0';
}

static void initiative_redraw(InitiativeApp* app) {
    if(!app || !app->view) return;
    InitiativeApp** model = view_get_model(app->view);
    if(!model) return;
    *model = app;
    view_commit_model(app->view, true);
}

static bool initiative_save(InitiativeApp* app);
static int16_t initiative_clamp(int32_t value, int16_t low, int16_t high);

static InitiativeMember* initiative_edit_member(InitiativeApp* app) {
    if(!app) return NULL;
    if(app->edit_combat) {
        if(app->selection >= app->combat_count) return NULL;
        return &app->combat[app->selection];
    }
    if(app->selection >= app->roster_count) return NULL;
    return &app->roster[app->selection];
}

static void initiative_text_done(void* context) {
    InitiativeApp* app = context;
    InitiativeMember* member = initiative_edit_member(app);
    if(member) {
        if(app->text_target == 0U)
            initiative_copy(member->name, sizeof(member->name), app->edit_buffer);
        else
            initiative_copy(member->conditions, sizeof(member->conditions), app->edit_buffer);
        initiative_save(app);
    }
    app->text_input_active = 0U;
    view_dispatcher_switch_to_view(app->dispatcher, InitiativeViewMain);
    initiative_redraw(app);
}

static void initiative_begin_text(
    InitiativeApp* app, uint8_t target, const char* title, const char* initial) {
    if(!app->text_input) {
        app->text_input = text_input_alloc();
        if(!app->text_input) {
            initiative_copy(app->status, sizeof(app->status), "Text input memory low");
            return;
        }
        view_dispatcher_add_view(
            app->dispatcher, InitiativeViewText, text_input_get_view(app->text_input));
    }
    app->text_target = target;
    app->text_input_active = 1U;
    initiative_copy(app->edit_buffer, sizeof(app->edit_buffer), initial);
    text_input_reset(app->text_input);
    text_input_set_header_text(app->text_input, title);
    text_input_set_result_callback(
        app->text_input,
        initiative_text_done,
        app,
        app->edit_buffer,
        target == 0U ? INIT_NAME_LEN : INIT_CONDITION_LEN,
        false);
    view_dispatcher_switch_to_view(app->dispatcher, InitiativeViewText);
}

static void initiative_number_done(void* context, int32_t number) {
    InitiativeApp* app = context;
    if(app->setup_number_index < app->combat_count)
        app->combat[app->setup_number_index].total =
            initiative_clamp(number + app->combat[app->setup_number_index].modifier, -99, 199);
    app->number_input_active = 0U;
    view_dispatcher_switch_to_view(app->dispatcher, InitiativeViewMain);
    initiative_redraw(app);
}

static void initiative_begin_setup_number(InitiativeApp* app, uint8_t index) {
    if(index >= app->combat_count) return;
    if(!app->number_input) {
        app->number_input = number_input_alloc();
        if(!app->number_input) {
            initiative_copy(app->status, sizeof(app->status), "Number input memory low");
            return;
        }
        view_dispatcher_add_view(
            app->dispatcher, InitiativeViewNumber, number_input_get_view(app->number_input));
    }
    app->setup_number_index = index;
    app->number_input_active = 1U;
    number_input_set_header_text(app->number_input, "Initiative d20 roll");
    number_input_set_result_callback(
        app->number_input, initiative_number_done, app, 10, 1, 20);
    view_dispatcher_switch_to_view(app->dispatcher, InitiativeViewNumber);
}

static void initiative_roll_member(InitiativeMember* member) {
    if(!member) return;
    member->total = (int16_t)(1U + (furi_hal_random_get() % 20U) + member->modifier);
}

static void initiative_sort_combat(InitiativeApp* app) {
    for(uint8_t i = 1U; i < app->combat_count; ++i) {
        InitiativeMember value = app->combat[i];
        uint8_t position = i;
        while(position && app->combat[position - 1U].total < value.total) {
            app->combat[position] = app->combat[position - 1U];
            --position;
        }
        app->combat[position] = value;
    }
}

static bool initiative_parse_u32(const char* text, const char** end, uint32_t* output) {
    if(!text || *text < '0' || *text > '9') return false;
    uint32_t value = 0U;
    const char* p = text;
    while(*p >= '0' && *p <= '9') {
        uint32_t digit = (uint32_t)(*p - '0');
        if(value > (UINT32_MAX - digit) / 10U) return false;
        value = value * 10U + digit;
        ++p;
    }
    *output = value;
    if(end) *end = p;
    return true;
}

static int16_t initiative_clamp(int32_t value, int16_t low, int16_t high) {
    if(value < low) return low;
    if(value > high) return high;
    return (int16_t)value;
}

static int32_t initiative_parse_i32(const char* text) {
    if(!text) return 0;
    bool negative = *text == '-';
    if(negative || *text == '+') ++text;
    int32_t value = 0;
    while(*text >= '0' && *text <= '9') {
        int32_t digit = *text++ - '0';
        if(value > (INT32_MAX - digit) / 10) return negative ? INT32_MIN : INT32_MAX;
        value = value * 10 + digit;
    }
    return negative ? -value : value;
}

static bool initiative_path(char* out, size_t size, uint32_t id, const char* suffix) {
    int n = snprintf(out, size, INIT_FILE_PATH, (unsigned long)id, suffix);
    return n > 0 && (size_t)n < size;
}


typedef struct {
    File* file;
    uint8_t buffer[256];
    uint16_t position;
    uint16_t count;
} InitiativeReader;

static bool initiative_read_line(InitiativeReader* reader, char* line, size_t size) {
    if(!reader || !line || size < 2U) return false;
    size_t used = 0U;
    bool consumed = false;
    while(true) {
        if(reader->position >= reader->count) {
            reader->count =
                (uint16_t)storage_file_read(reader->file, reader->buffer, sizeof(reader->buffer));
            reader->position = 0U;
            if(!reader->count) break;
        }
        char ch = (char)reader->buffer[reader->position++];
        consumed = true;
        if(ch == '\r') continue;
        if(ch == '\n') break;
        if(used + 1U < size) line[used++] = ch;
    }
    line[used] = '\0';
    return consumed;
}

static uint8_t initiative_hex_value(char value) {
    if(value >= '0' && value <= '9') return (uint8_t)(value - '0');
    if(value >= 'A' && value <= 'F') return (uint8_t)(value - 'A' + 10U);
    if(value >= 'a' && value <= 'f') return (uint8_t)(value - 'a' + 10U);
    return 0xFFU;
}

static void initiative_decode_string(char* destination, size_t size, const char* value) {
    if(!destination || !size) return;
    size_t output = 0U;
    for(size_t input = 0U; value && value[input] && output + 1U < size; ++input) {
        if(value[input] == '%' && value[input + 1U] && value[input + 2U]) {
            uint8_t high = initiative_hex_value(value[input + 1U]);
            uint8_t low = initiative_hex_value(value[input + 2U]);
            if(high != 0xFFU && low != 0xFFU) {
                destination[output++] = (char)((high << 4U) | low);
                input += 2U;
                continue;
            }
        }
        destination[output++] = value[input];
    }
    destination[output] = '\0';
}

static bool initiative_character_path(
    Storage* storage, uint32_t profile, char* output, size_t size) {
    if(!storage || !output || !size) return false;
    File* directory = storage_file_alloc(storage);
    if(!directory) return false;
    if(!storage_dir_open(directory, POCKET_D20_CHARACTER_DATA_ROOT)) {
        storage_file_free(directory);
        return false;
    }
    char prefix[32];
    int prefix_len = snprintf(prefix, sizeof(prefix), "ch_%lu_", (unsigned long)profile);
    bool found = false;
    FileInfo info;
    char filename[128];
    while(prefix_len > 0 && storage_dir_read(directory, &info, filename, sizeof(filename))) {
        if(file_info_is_dir(&info) || strncmp(filename, prefix, (size_t)prefix_len) != 0) continue;
        size_t length = strlen(filename);
        if(length < (size_t)prefix_len + 6U || strcmp(filename + length - 4U, ".txt") != 0)
            continue;
        const char* level = filename + length - 5U;
        while(level > filename + prefix_len && *level != '_') --level;
        if(*level != '_' || level[1] < '0' || level[1] > '9') continue;
        bool numeric = true;
        for(const char* digit = level + 1U; digit < filename + length - 4U; ++digit)
            if(*digit < '0' || *digit > '9') numeric = false;
        if(!numeric) continue;
        int written = snprintf(
            output, size, "%s/%s", POCKET_D20_CHARACTER_DATA_ROOT, filename);
        found = written > 0 && (size_t)written < size;
        if(found) break;
    }
    storage_dir_close(directory);
    storage_file_free(directory);
    return found;
}

static uint8_t initiative_parse_csv(const char* value, int32_t* values, uint8_t capacity) {
    uint8_t count = 0U;
    const char* cursor = value;
    while(cursor && *cursor && count < capacity) {
        char* end = NULL;
        long parsed = strtol(cursor, &end, 10);
        if(end == cursor) break;
        values[count++] = parsed > INT32_MAX ? INT32_MAX : parsed < INT32_MIN ? INT32_MIN : (int32_t)parsed;
        cursor = *end == ',' ? end + 1U : end;
        if(*end && *end != ',') break;
    }
    return count;
}

static bool initiative_read_main_character(
    InitiativeApp* app, uint32_t profile, InitiativeMember* member) {
    char path[INIT_PATH_LEN];
    if(!initiative_character_path(app->storage, profile, path, sizeof(path))) return false;
    File* file = storage_file_alloc(app->storage);
    if(!file) return false;
    if(!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_free(file);
        return false;
    }
    InitiativeReader reader = {.file = file};
    char line[256];
    int8_t dexterity = 10;
    int8_t initiative_misc = 0;
    bool named = false;
    memset(member, 0, sizeof(*member));
    member->hp_current = member->hp_max = 1;
    member->armor_class = 10;
    while(initiative_read_line(&reader, line, sizeof(line))) {
        char* value = strchr(line, '=');
        if(!value) continue;
        *value++ = '\0';
        if(strcmp(line, "Name") == 0) {
            initiative_decode_string(member->name, sizeof(member->name), value);
            named = member->name[0] != '\0';
        } else if(strcmp(line, "Conditions") == 0) {
            initiative_decode_string(member->conditions, sizeof(member->conditions), value);
        } else if(strcmp(line, "AbilityScores") == 0) {
            int32_t values[6];
            uint8_t count = initiative_parse_csv(value, values, 6U);
            if(count >= 2U) dexterity = (int8_t)initiative_clamp(values[1], 1, 30);
        } else if(strcmp(line, "Vitals") == 0) {
            int32_t values[12];
            uint8_t count = initiative_parse_csv(value, values, 12U);
            if(count >= 1U) member->hp_current = initiative_clamp(values[0], -999, 999);
            if(count >= 2U) member->hp_max = initiative_clamp(values[1], 0, 999);
            if(count >= 4U) member->armor_class = initiative_clamp(values[3], 0, 99);
            if(count >= 6U) initiative_misc = (int8_t)initiative_clamp(values[5], -50, 50);
        }
    }
    bool io_ok = storage_file_get_error(file) == FSE_OK;
    storage_file_close(file);
    storage_file_free(file);
    int16_t dex_mod = dexterity >= 10 ? (dexterity - 10) / 2 : -((11 - dexterity) / 2);
    member->modifier = (int8_t)initiative_clamp(dex_mod + initiative_misc, -50, 50);
    return io_ok && named;
}

static bool initiative_sync_main_character(InitiativeApp* app) {
    if(!app || !app->storage) return false;
    uint32_t profile = app->character_id;
    InitiativeMember fresh;
    bool loaded = profile && initiative_read_main_character(app, profile, &fresh);
    if(!loaded) {
        uint32_t active = 0U;
        if(dnd_profile_ref_active(app->storage, &active) &&
           initiative_read_main_character(app, active, &fresh)) {
            profile = active;
            loaded = true;
        }
    }
    if(!loaded) return false;
    app->character_id = profile;

    uint8_t match = INIT_MAX;
    for(uint8_t i = 0U; i < app->roster_count; ++i) {
        if(strcmp(app->roster[i].name, fresh.name) == 0) {
            match = i;
            break;
        }
    }
    if(match < app->roster_count) {
        char conditions[INIT_CONDITION_LEN];
        initiative_copy(conditions, sizeof(conditions), app->roster[match].conditions);
        app->roster[match] = fresh;
        if(conditions[0])
            initiative_copy(
                app->roster[match].conditions, sizeof(app->roster[match].conditions), conditions);
    } else if(app->roster_count < INIT_MAX) {
        app->roster[app->roster_count++] = fresh;
    } else {
        initiative_copy(app->status, sizeof(app->status), "Roster full; character skipped");
        return false;
    }
    initiative_save(app);
    return true;
}

static bool initiative_parse_i32_strict(const char* text, int32_t* output) {
    if(!text || !text[0] || !output) return false;
    bool negative = false;
    if(*text == '-' || *text == '+') {
        negative = *text == '-';
        ++text;
    }
    if(*text < '0' || *text > '9') return false;
    uint32_t value = 0U;
    uint32_t maximum = negative ? (uint32_t)INT32_MAX + 1U : (uint32_t)INT32_MAX;
    while(*text >= '0' && *text <= '9') {
        uint32_t digit = (uint32_t)(*text - '0');
        if(value > (maximum - digit) / 10U) return false;
        value = value * 10U + digit;
        ++text;
    }
    if(*text) return false;
    *output = negative ? (value == (uint32_t)INT32_MAX + 1U ? INT32_MIN : -(int32_t)value) :
                         (int32_t)value;
    return true;
}

static bool initiative_indexed_key(
    const char* key,
    const char* prefix,
    const char* suffix,
    uint8_t* index) {
    size_t prefix_length = strlen(prefix);
    if(strncmp(key, prefix, prefix_length) != 0) return false;
    const char* cursor = key + prefix_length;
    const char* digits = cursor;
    uint32_t value = 0U;
    while(*cursor >= '0' && *cursor <= '9') {
        value = value * 10U + (uint32_t)(*cursor - '0');
        if(value >= INIT_MAX) return false;
        ++cursor;
    }
    if(cursor == digits || strcmp(cursor, suffix) != 0) return false;
    *index = (uint8_t)value;
    return true;
}

static bool initiative_write_named(File* file, const char* key, const char* value) {
    char line[160];
    size_t used = 0U;
    int prefix = snprintf(line, sizeof(line), "%s=", key);
    if(prefix <= 0 || (size_t)prefix >= sizeof(line)) return false;
    used = (size_t)prefix;
    for(size_t i = 0U; value && value[i]; ++i) {
        char ch = value[i];
        if(ch == '\r' || ch == '\n') ch = ' ';
        if(used + 2U >= sizeof(line)) return false;
        line[used++] = ch;
    }
    line[used++] = '\n';
    return storage_file_write(file, line, used) == used;
}

static bool initiative_write_number(File* file, const char* key, int32_t value) {
    char line[64];
    int length = snprintf(line, sizeof(line), "%s=%ld\n", key, (long)value);
    return length > 0 && (size_t)length < sizeof(line) &&
           storage_file_write(file, line, (size_t)length) == (size_t)length;
}

static bool initiative_write_member_named(
    File* file,
    const char* prefix,
    uint8_t index,
    const InitiativeMember* member) {
    char key[40];
#define INIT_MEMBER_STRING(suffix, field) \
    do { snprintf(key, sizeof(key), "%s%u%s", prefix, index, suffix); if(!initiative_write_named(file, key, field)) return false; } while(false)
#define INIT_MEMBER_NUMBER(suffix, field) \
    do { snprintf(key, sizeof(key), "%s%u%s", prefix, index, suffix); if(!initiative_write_number(file, key, field)) return false; } while(false)
    INIT_MEMBER_STRING("Name", member->name);
    INIT_MEMBER_NUMBER("HpCurrent", member->hp_current);
    INIT_MEMBER_NUMBER("HpMax", member->hp_max);
    INIT_MEMBER_NUMBER("ArmorClass", member->armor_class);
    INIT_MEMBER_NUMBER("Modifier", member->modifier);
    INIT_MEMBER_NUMBER("Total", member->total);
    INIT_MEMBER_STRING("Conditions", member->conditions);
#undef INIT_MEMBER_STRING
#undef INIT_MEMBER_NUMBER
    return true;
}


static bool initiative_save(InitiativeApp* app) {
    storage_common_mkdir(app->storage, APP_DATA_PATH(""));
    char path[INIT_PATH_LEN], temp[INIT_PATH_LEN], backup[INIT_PATH_LEN];
    if(!initiative_path(path, sizeof(path), app->character_id, "txt") ||
       !initiative_path(temp, sizeof(temp), app->character_id, "tmp") ||
       !initiative_path(backup, sizeof(backup), app->character_id, "bak"))
        return false;
    File* file = storage_file_alloc(app->storage);
    if(!file) return false;
    bool ok = storage_file_open(file, temp, FSAM_WRITE, FSOM_CREATE_ALWAYS) &&
              initiative_write_number(file, "DNDInitiative", 1) &&
              initiative_write_number(file, "CharacterId", (int32_t)app->character_id) &&
              initiative_write_number(file, "RosterCount", app->roster_count);
    for(uint8_t i = 0U; ok && i < app->roster_count; ++i)
        ok = initiative_write_member_named(file, "Roster", i, &app->roster[i]);
    if(ok) ok = initiative_write_number(file, "Active", app->active);
    if(ok) ok = initiative_write_number(file, "Round", app->round);
    if(ok) ok = initiative_write_number(file, "CurrentTurn", app->current_turn);
    if(ok) ok = initiative_write_number(file, "CombatCount", app->combat_count);
    for(uint8_t i = 0U; ok && i < app->combat_count; ++i)
        ok = initiative_write_member_named(file, "Combat", i, &app->combat[i]);
    if(ok) ok = storage_file_write(file, "End=OK\n", 7U) == 7U && storage_file_sync(file);
    storage_file_close(file);
    storage_file_free(file);
    if(!ok) {
        storage_common_remove(app->storage, temp);
        return false;
    }
    storage_common_remove(app->storage, backup);
    if(storage_file_exists(app->storage, path) &&
       storage_common_rename(app->storage, path, backup) != FSE_OK)
        return false;
    if(storage_common_rename(app->storage, temp, path) != FSE_OK) {
        if(storage_file_exists(app->storage, backup))
            storage_common_rename(app->storage, backup, path);
        return false;
    }
    storage_common_remove(app->storage, backup);
    return true;
}


static bool initiative_apply_member_field(
    const char* key,
    const char* value,
    const char* prefix,
    InitiativeMember* list,
    uint8_t* count) {
    uint8_t index = 0U;
    InitiativeMember* member = NULL;
    int32_t number = 0;
    bool applied = false;

    if(initiative_indexed_key(key, prefix, "Name", &index)) {
        member = &list[index];
        initiative_copy(member->name, sizeof(member->name), value);
        applied = true;
    } else if(initiative_indexed_key(key, prefix, "HpCurrent", &index)) {
        member = &list[index];
        if(initiative_parse_i32_strict(value, &number)) {
            member->hp_current = initiative_clamp(number, -999, 999);
            applied = true;
        }
    } else if(initiative_indexed_key(key, prefix, "HpMax", &index)) {
        member = &list[index];
        if(initiative_parse_i32_strict(value, &number)) {
            member->hp_max = initiative_clamp(number, 0, 999);
            applied = true;
        }
    } else if(initiative_indexed_key(key, prefix, "ArmorClass", &index)) {
        member = &list[index];
        if(initiative_parse_i32_strict(value, &number)) {
            member->armor_class = initiative_clamp(number, 0, 99);
            applied = true;
        }
    } else if(initiative_indexed_key(key, prefix, "Modifier", &index)) {
        member = &list[index];
        if(initiative_parse_i32_strict(value, &number)) {
            member->modifier = (int8_t)initiative_clamp(number, -50, 50);
            applied = true;
        }
    } else if(initiative_indexed_key(key, prefix, "Total", &index)) {
        member = &list[index];
        if(initiative_parse_i32_strict(value, &number)) {
            member->total = initiative_clamp(number, -99, 199);
            applied = true;
        }
    } else if(initiative_indexed_key(key, prefix, "Conditions", &index)) {
        member = &list[index];
        initiative_copy(member->conditions, sizeof(member->conditions), value);
        applied = true;
    } else {
        return false;
    }

    if(applied && *count <= index) *count = (uint8_t)(index + 1U);
    return applied;
}

static void initiative_load(InitiativeApp* app) {
    char path[INIT_PATH_LEN];
    if(!initiative_path(path, sizeof(path), app->character_id, "txt")) return;
    File* file = storage_file_alloc(app->storage);
    if(!file || !storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        if(file) storage_file_free(file);
        return;
    }

    memset(app->roster, 0, sizeof(app->roster));
    memset(app->combat, 0, sizeof(app->combat));
    app->roster_count = 0U;
    app->combat_count = 0U;
    app->active = 0U;
    app->round = 1U;
    app->current_turn = 0U;

    InitiativeReader reader = {.file = file};
    char line[192];
    while(initiative_read_line(&reader, line, sizeof(line))) {
        char* value = strchr(line, '=');
        if(!value) continue;
        *value++ = '\0';
        int32_t number = 0;

        if(!strcmp(line, "Active")) {
            if(initiative_parse_i32_strict(value, &number)) app->active = number ? 1U : 0U;
        } else if(!strcmp(line, "Round")) {
            if(initiative_parse_i32_strict(value, &number) && number >= 1 && number <= (int32_t)UINT16_MAX)
                app->round = (uint16_t)number;
        } else if(!strcmp(line, "CurrentTurn")) {
            if(initiative_parse_i32_strict(value, &number) && number >= 0 && number < (int32_t)INIT_MAX)
                app->current_turn = (uint8_t)number;
        } else if(initiative_apply_member_field(
                      line, value, "Roster", app->roster, &app->roster_count)) {
            /* Applied by field name. */
        } else if(initiative_apply_member_field(
                      line, value, "Combat", app->combat, &app->combat_count)) {
            /* Applied by field name. */
        }
        /* DNDInitiative, CharacterId, counts, End, malformed values, and unknown
           future fields are informational and never invalidate neighboring data. */
    }

    storage_file_close(file);
    storage_file_free(file);
    if(app->current_turn >= app->combat_count) app->current_turn = 0U;
    if(!app->round) app->round = 1U;
}

static void initiative_import_args(InitiativeApp* app, const char* args) {
    const char* cursor = args;
    const char* end = NULL;
    uint32_t id = 0U;
    if(!initiative_parse_u32(cursor, &end, &id)) return;
    app->character_id = id;
    cursor = end;
    while(*cursor == ';' && app->roster_count < INIT_MAX) {
        ++cursor;
        const char* record_end = strchr(cursor, ';');
        if(!record_end) record_end = cursor + strlen(cursor);
        char record[128];
        size_t length = (size_t)(record_end - cursor);
        if(length >= sizeof(record)) length = sizeof(record) - 1U;
        memcpy(record, cursor, length);
        record[length] = '\0';
        char* a = strchr(record, ',');
        char* b = a ? strchr(a + 1U, ',') : NULL;
        char* c = b ? strchr(b + 1U, ',') : NULL;
        if(a && b && c) {
            *a++ = '\0';
            *b++ = '\0';
            *c++ = '\0';
            InitiativeMember* member = &app->roster[app->roster_count];
            memset(member, 0, sizeof(*member));
            initiative_copy(member->name, sizeof(member->name), record);
            member->hp_current = member->hp_max =
                initiative_clamp(initiative_parse_i32(a), 0, 999);
            member->armor_class = initiative_clamp(initiative_parse_i32(b), 0, 99);
            member->modifier =
                (int8_t)initiative_clamp(initiative_parse_i32(c), -50, 50);
            if(member->name[0]) ++app->roster_count;
        }
        cursor = record_end;
    }
}

static void initiative_row(Canvas* canvas, uint8_t row, bool selected, const char* text) {
    uint8_t y = (uint8_t)(13U + row * 10U);
    if(selected) {
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_box(canvas, 0, y, 128, 10);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_set_font(canvas, FontSecondary);
    char shown[27];
    initiative_copy(shown, sizeof(shown), text);
    canvas_draw_str(canvas, 2, (uint8_t)(y + 8U), shown);
    if(selected) canvas_set_color(canvas, ColorBlack);
}

static void initiative_draw(Canvas* canvas, void* model) {
    InitiativeApp* app = *(InitiativeApp**)model;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 9, "DNDInitiative");
    if(app->screen == InitiativeScreenMenu) {
        char rows[4][32];
        snprintf(rows[0], sizeof(rows[0]), "Start Combat (%u)", app->roster_count);
        snprintf(rows[1], sizeof(rows[1]), "Resume%s", app->active ? "" : " (none)");
        snprintf(rows[2], sizeof(rows[2]), "Party Roster (%u)", app->roster_count);
        initiative_copy(rows[3], sizeof(rows[3]), "Return to DNDolphins");
        for(uint8_t i = 0U; i < 4U; ++i) initiative_row(canvas, i, i == app->selection, rows[i]);
    } else if(app->screen == InitiativeScreenRoster) {
        uint8_t total = (uint8_t)(app->roster_count + 1U);
        for(uint8_t row = 0U; row < 5U; ++row) {
            uint8_t i = (uint8_t)(app->scroll + row);
            if(i >= total) break;
            char text[48];
            if(i == app->roster_count)
                initiative_copy(text, sizeof(text), "+ New Member");
            else
                snprintf(text, sizeof(text), "%.24s HP%d AC%d", app->roster[i].name, app->roster[i].hp_current, app->roster[i].armor_class);
            initiative_row(canvas, row, i == app->selection, text);
        }
    } else if(app->screen == InitiativeScreenSetup) {
        uint8_t total = (uint8_t)(app->combat_count + 3U);
        for(uint8_t row = 0U; row < 5U; ++row) {
            uint8_t i = (uint8_t)(app->scroll + row);
            if(i >= total) break;
            char text[48];
            if(i == 0U)
                initiative_copy(text, sizeof(text), "Roll for All");
            else if(i <= app->combat_count) {
                InitiativeMember* member = &app->combat[i - 1U];
                snprintf(text, sizeof(text), "%.25s: %d", member->name, member->total);
            } else if(i == (uint8_t)(app->combat_count + 1U))
                initiative_copy(text, sizeof(text), "+ Temporary Member");
            else
                initiative_copy(text, sizeof(text), "Begin Combat");
            initiative_row(canvas, row, i == app->selection, text);
        }
    } else if(app->screen == InitiativeScreenCombat) {
        char header[32];
        snprintf(header, sizeof(header), "Round %u", app->round);
        canvas_draw_str(canvas, 86, 9, header);
        for(uint8_t row = 0U; row < 5U; ++row) {
            uint8_t i = (uint8_t)(app->scroll + row);
            if(i >= app->combat_count) break;
            char text[48];
            snprintf(text, sizeof(text), "%c %.20s %d HP%d", i == app->current_turn ? '>' : ' ', app->combat[i].name, app->combat[i].total, app->combat[i].hp_current);
            initiative_row(canvas, row, i == app->selection, text);
        }
    } else {
        InitiativeMember* member = initiative_edit_member(app);
        if(!member) return;
        for(uint8_t row = 0U; row < 5U; ++row) {
            uint8_t field = (uint8_t)(app->scroll + row);
            if(field >= 7U) break;
            char text[48];
            if(field == 0U)
                snprintf(text, sizeof(text), "Name: %.34s", member->name);
            else if(field == 1U)
                snprintf(text, sizeof(text), "%s: %d", app->edit_combat ? "Total" : "Initiative", app->edit_combat ? member->total : member->modifier);
            else if(field == 2U)
                snprintf(text, sizeof(text), "HP: %d", member->hp_current);
            else if(field == 3U)
                snprintf(text, sizeof(text), "Max HP: %d", member->hp_max);
            else if(field == 4U)
                snprintf(text, sizeof(text), "AC: %d", member->armor_class);
            else if(field == 5U)
                snprintf(text, sizeof(text), "Conditions: %.26s", member->conditions);
            else
                initiative_copy(text, sizeof(text), "Delete");
            initiative_row(canvas, row, field == app->edit_field, text);
        }
    }
}

static void initiative_prepare_combat(InitiativeApp* app) {
    initiative_sync_main_character(app);
    app->combat_count = app->roster_count;
    memcpy(app->combat, app->roster, app->roster_count * sizeof(InitiativeMember));
    for(uint8_t i = 0U; i < app->combat_count; ++i) app->combat[i].total = 0;
    app->active = 0U;
    app->round = 1U;
    app->current_turn = 0U;
    app->selection = 0U;
    app->scroll = 0U;
    app->screen = InitiativeScreenSetup;
    initiative_save(app);
}

static void initiative_begin_combat(InitiativeApp* app) {
    if(!app->combat_count) {
        initiative_copy(app->status, sizeof(app->status), "No combatants");
        return;
    }
    for(uint8_t i = 0U; i < app->combat_count; ++i)
        if(app->combat[i].total == 0) initiative_roll_member(&app->combat[i]);
    initiative_sort_combat(app);
    app->active = 1U;
    app->round = 1U;
    app->current_turn = 0U;
    app->selection = 0U;
    app->scroll = 0U;
    app->screen = InitiativeScreenCombat;
    initiative_save(app);
}

static void initiative_move(uint8_t* selection, uint8_t count, int8_t direction) {
    if(!count) return;
    int16_t next = (int16_t)*selection + direction;
    if(next < 0) next = count - 1U;
    if(next >= count) next = 0;
    *selection = (uint8_t)next;
}

static bool initiative_input(InputEvent* event, void* context) {
    InitiativeApp* app = context;
    if(event->type != InputTypeShort && event->type != InputTypeRepeat && event->type != InputTypeLong)
        return true;

    if(event->type == InputTypeLong && event->key == InputKeyBack) {
        if(app->screen == InitiativeScreenCombat || app->screen == InitiativeScreenSetup ||
           app->screen == InitiativeScreenEdit) {
            app->screen = InitiativeScreenMenu;
            app->selection = app->scroll = 0U;
            initiative_redraw(app);
        } else {
            app->return_to_dnd = 1U;
            view_dispatcher_stop(app->dispatcher);
        }
        return true;
    }

    if(app->screen == InitiativeScreenMenu) {
        if(event->key == InputKeyUp)
            initiative_move(&app->selection, 4U, -1);
        else if(event->key == InputKeyDown)
            initiative_move(&app->selection, 4U, 1);
        else if(event->key == InputKeyBack) {
            app->return_to_dnd = 1U;
            view_dispatcher_stop(app->dispatcher);
        } else if(event->key == InputKeyOk && event->type == InputTypeShort) {
            if(app->selection == 0U)
                initiative_prepare_combat(app);
            else if(app->selection == 1U && app->active) {
                app->screen = InitiativeScreenCombat;
                app->selection = app->current_turn;
                app->scroll = app->selection >= 5U ? (uint8_t)(app->selection - 4U) : 0U;
            } else if(app->selection == 2U) {
                app->screen = InitiativeScreenRoster;
                app->selection = app->scroll = 0U;
            } else if(app->selection == 3U) {
                app->return_to_dnd = 1U;
                view_dispatcher_stop(app->dispatcher);
            }
        }
    } else if(app->screen == InitiativeScreenRoster) {
        uint8_t total = (uint8_t)(app->roster_count + 1U);
        if(event->key == InputKeyUp)
            initiative_move(&app->selection, total, -1);
        else if(event->key == InputKeyDown)
            initiative_move(&app->selection, total, 1);
        else if(event->key == InputKeyBack)
            app->screen = InitiativeScreenMenu;
        else if(event->key == InputKeyOk && event->type == InputTypeShort) {
            if(app->selection == app->roster_count && app->roster_count < INIT_MAX) {
                InitiativeMember* member = &app->roster[app->roster_count++];
                memset(member, 0, sizeof(*member));
                initiative_copy(member->name, sizeof(member->name), "New Member");
                member->hp_current = member->hp_max = 1;
                member->armor_class = 10;
                app->selection = (uint8_t)(app->roster_count - 1U);
                initiative_save(app);
            }
            if(app->selection < app->roster_count) {
                app->screen = InitiativeScreenEdit;
                app->edit_return_screen = InitiativeScreenRoster;
                app->edit_combat = 0U;
                app->edit_field = 0U;
                app->scroll = 0U;
            }
        }
    } else if(app->screen == InitiativeScreenSetup) {
        uint8_t total = (uint8_t)(app->combat_count + 3U);
        if(event->key == InputKeyUp)
            initiative_move(&app->selection, total, -1);
        else if(event->key == InputKeyDown)
            initiative_move(&app->selection, total, 1);
        else if(event->key == InputKeyBack && event->type == InputTypeShort) {
            app->screen = InitiativeScreenMenu;
            app->selection = app->scroll = 0U;
        } else if(event->key == InputKeyOk && event->type == InputTypeLong &&
                  app->selection >= 1U && app->selection <= app->combat_count) {
            initiative_begin_setup_number(app, (uint8_t)(app->selection - 1U));
            return true;
        } else if(event->key == InputKeyOk && event->type == InputTypeShort) {
            if(app->selection == 0U) {
                for(uint8_t i = 0U; i < app->combat_count; ++i) initiative_roll_member(&app->combat[i]);
                initiative_copy(app->status, sizeof(app->status), "Initiative rolled");
                initiative_save(app);
            } else if(app->selection <= app->combat_count) {
                initiative_roll_member(&app->combat[app->selection - 1U]);
                initiative_save(app);
            } else if(app->selection == (uint8_t)(app->combat_count + 1U)) {
                if(app->combat_count < INIT_MAX) {
                    InitiativeMember* member = &app->combat[app->combat_count++];
                    memset(member, 0, sizeof(*member));
                    initiative_copy(member->name, sizeof(member->name), "New Member");
                    member->hp_current = member->hp_max = 1;
                    member->armor_class = 10;
                    app->selection = (uint8_t)(app->combat_count - 1U);
                    app->edit_combat = 1U;
                    app->edit_return_screen = InitiativeScreenSetup;
                    app->edit_field = 0U;
                    app->scroll = 0U;
                    app->screen = InitiativeScreenEdit;
                }
            } else {
                initiative_begin_combat(app);
            }
        }
    } else if(app->screen == InitiativeScreenCombat) {
        if(event->key == InputKeyUp)
            initiative_move(&app->selection, app->combat_count, -1);
        else if(event->key == InputKeyDown)
            initiative_move(&app->selection, app->combat_count, 1);
        else if(event->key == InputKeyBack && event->type == InputTypeShort && app->combat_count) {
            if(app->current_turn == 0U) {
                app->current_turn = (uint8_t)(app->combat_count - 1U);
                if(app->round > 1U) --app->round;
            } else {
                --app->current_turn;
            }
            app->selection = app->current_turn;
            initiative_save(app);
        } else if(event->type == InputTypeLong && event->key == InputKeyOk && app->combat_count) {
            app->selection = app->selection < app->combat_count ? app->selection : 0U;
            app->edit_combat = 1U;
            app->edit_return_screen = InitiativeScreenCombat;
            app->edit_field = 0U;
            app->scroll = 0U;
            app->screen = InitiativeScreenEdit;
        } else if(event->key == InputKeyOk && event->type == InputTypeShort && app->combat_count) {
            app->current_turn = (uint8_t)((app->current_turn + 1U) % app->combat_count);
            if(app->current_turn == 0U) ++app->round;
            app->selection = app->current_turn;
            initiative_save(app);
        }
    } else {
        InitiativeMember* member = initiative_edit_member(app);
        if(!member) {
            app->screen = app->edit_return_screen;
        } else if(event->key == InputKeyUp) {
            initiative_move(&app->edit_field, 7U, -1);
        } else if(event->key == InputKeyDown) {
            initiative_move(&app->edit_field, 7U, 1);
        } else if(event->key == InputKeyBack && event->type == InputTypeShort) {
            InitiativeScreen destination = app->edit_return_screen;
            uint8_t member_index = app->selection;
            app->screen = destination;
            app->selection = destination == InitiativeScreenSetup ? (uint8_t)(member_index + 1U) : member_index;
            app->scroll = app->selection >= 5U ? (uint8_t)(app->selection - 4U) : 0U;
        } else if(event->key == InputKeyLeft || event->key == InputKeyRight) {
            int16_t delta = event->key == InputKeyRight ? 1 : -1;
            if(app->edit_field == 1U) {
                if(app->edit_combat)
                    member->total = initiative_clamp(member->total + delta, -99, 199);
                else
                    member->modifier = (int8_t)initiative_clamp(member->modifier + delta, -50, 50);
            } else if(app->edit_field == 2U)
                member->hp_current = initiative_clamp(member->hp_current + delta, -999, 999);
            else if(app->edit_field == 3U)
                member->hp_max = initiative_clamp(member->hp_max + delta, 0, 999);
            else if(app->edit_field == 4U)
                member->armor_class = initiative_clamp(member->armor_class + delta, 0, 99);
            else
                goto redraw;
            initiative_save(app);
        } else if(event->key == InputKeyOk && event->type == InputTypeShort) {
            if(app->edit_field == 0U) {
                initiative_begin_text(app, 0U, "Character name", member->name);
                return true;
            } else if(app->edit_field == 5U) {
                initiative_begin_text(app, 1U, "Conditions", member->conditions);
                return true;
            } else if(app->edit_field == 6U) {
                InitiativeMember* list = app->edit_combat ? app->combat : app->roster;
                uint8_t* count = app->edit_combat ? &app->combat_count : &app->roster_count;
                memmove(
                    &list[app->selection],
                    &list[app->selection + 1U],
                    (*count - app->selection - 1U) * sizeof(*list));
                --*count;
                if(app->selection >= *count && *count) app->selection = (uint8_t)(*count - 1U);
                app->screen = app->edit_return_screen;
                if(app->screen == InitiativeScreenSetup) app->selection = app->selection + 1U;
                initiative_save(app);
            }
        }
    }

redraw:
    if(app->screen == InitiativeScreenEdit) {
        if(app->edit_field < app->scroll) app->scroll = app->edit_field;
        if(app->edit_field >= app->scroll + 5U) app->scroll = (uint8_t)(app->edit_field - 4U);
    } else {
        if(app->selection < app->scroll) app->scroll = app->selection;
        if(app->selection >= app->scroll + 5U) app->scroll = (uint8_t)(app->selection - 4U);
    }
    initiative_redraw(app);
    return true;
}

static bool initiative_navigation(void* context) {
    InitiativeApp* app = context;
    if(app->text_input_active || app->number_input_active) {
        app->text_input_active = 0U;
        app->number_input_active = 0U;
        view_dispatcher_switch_to_view(app->dispatcher, InitiativeViewMain);
        initiative_redraw(app);
    } else if(app->screen == InitiativeScreenMenu) {
        app->return_to_dnd = 1U;
        view_dispatcher_stop(app->dispatcher);
    } else {
        app->screen = InitiativeScreenMenu;
        app->selection = app->scroll = 0U;
        initiative_redraw(app);
    }
    return true;
}

static InitiativeApp* initiative_alloc(const char* args) {
    InitiativeApp* app = calloc(1U, sizeof(InitiativeApp));
    if(!app) return NULL;
    app->gui = furi_record_open(RECORD_GUI);
    app->storage = furi_record_open(RECORD_STORAGE);
    if(!app->gui || !app->storage) goto fail;
    const char* end = NULL;
    if(!(args && initiative_parse_u32(args, &end, &app->character_id)) &&
       !dnd_profile_ref_active(app->storage, &app->character_id))
        app->character_id = 0U;
    initiative_load(app);
    if(args) initiative_import_args(app, args);
    if(args && strchr(args, ';')) initiative_save(app);
    app->dispatcher = view_dispatcher_alloc();
    app->view = view_alloc();
    if(!app->dispatcher || !app->view) goto fail;
    view_dispatcher_set_event_callback_context(app->dispatcher, app);
    view_dispatcher_set_navigation_event_callback(app->dispatcher, initiative_navigation);
    view_allocate_model(app->view, ViewModelTypeLockFree, sizeof(InitiativeApp*));
    InitiativeApp** model = view_get_model(app->view);
    if(!model) goto fail;
    *model = app;
    view_commit_model(app->view, false);
    view_set_context(app->view, app);
    view_set_draw_callback(app->view, initiative_draw);
    view_set_input_callback(app->view, initiative_input);
    view_dispatcher_add_view(app->dispatcher, InitiativeViewMain, app->view);
    view_dispatcher_attach_to_gui(app->dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    return app;
fail:
    if(app->text_input) text_input_free(app->text_input);
    if(app->number_input) number_input_free(app->number_input);
    if(app->view) view_free(app->view);
    if(app->dispatcher) view_dispatcher_free(app->dispatcher);
    if(app->storage) furi_record_close(RECORD_STORAGE);
    if(app->gui) furi_record_close(RECORD_GUI);
    free(app);
    return NULL;
}

static void initiative_free(InitiativeApp* app) {
    if(!app) return;
    if(app->dispatcher && app->text_input)
        view_dispatcher_remove_view(app->dispatcher, InitiativeViewText);
    if(app->dispatcher && app->number_input)
        view_dispatcher_remove_view(app->dispatcher, InitiativeViewNumber);
    if(app->dispatcher && app->view)
        view_dispatcher_remove_view(app->dispatcher, InitiativeViewMain);
    if(app->text_input) text_input_free(app->text_input);
    if(app->number_input) number_input_free(app->number_input);
    if(app->view) view_free(app->view);
    if(app->dispatcher) view_dispatcher_free(app->dispatcher);
    if(app->storage) furi_record_close(RECORD_STORAGE);
    if(app->gui) furi_record_close(RECORD_GUI);
    free(app);
}

int32_t dndinitiative_app(void* context) {
    InitiativeApp* app = initiative_alloc(context);
    if(!app) return -1;
    view_dispatcher_switch_to_view(app->dispatcher, InitiativeViewMain);
    view_dispatcher_run(app->dispatcher);
    bool return_to_dnd = app->return_to_dnd;
    initiative_save(app);
    initiative_free(app);
    if(return_to_dnd) {
        if(!dnd_handoff_launch(DNDOLPHINS_FAP_PATH, NULL)) return -1;
    }
    return 0;
}
