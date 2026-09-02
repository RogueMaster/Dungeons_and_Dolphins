#include "dnd_settings.h"
#include "dnd_fs.h"
#include "dnd_profile_handoff.h"

#include <stdio.h>
#include <string.h>

#define DND_SETTINGS_PATH DND_CHARACTER_DATA_ROOT "/settings.txt"

static bool dnd_settings_write_line(File* file, const char* key, uint8_t value) {
    char line[40];
    int length = snprintf(line, sizeof(line), "%s=%u\n", key, value ? 1U : 0U);
    return length > 0 && (size_t)length < sizeof(line) &&
           storage_file_write(file, line, (size_t)length) == (size_t)length;
}

void dnd_settings_defaults(DndSettings* settings) {
    if(!settings) return;
    settings->skip_dice_loading = 0U;
    settings->debug = 0U;
    settings->extra_items = 1U;
    settings->catalog_all = 0U;
    settings->homebrew = 1U;
}

static bool dnd_settings_apply_bool(
    const char* line,
    const char* key,
    uint8_t* target) {
    if(!line || !key || !target) return false;
    size_t key_length = strlen(key);
    if(strncmp(line, key, key_length) != 0 || line[key_length] != '=') return false;
    char value = line[key_length + 1U];
    if((value != '0' && value != '1') || line[key_length + 2U] != '\0') return true;
    *target = value == '1' ? 1U : 0U;
    return true;
}

static void dnd_settings_apply_line(DndSettings* settings, const char* line) {
    if(!settings || !line) return;
    if(dnd_settings_apply_bool(line, "SkipDiceLoading", &settings->skip_dice_loading)) return;
    if(dnd_settings_apply_bool(line, "Debug", &settings->debug)) return;
    if(dnd_settings_apply_bool(line, "ExtraItems", &settings->extra_items)) return;
    if(dnd_settings_apply_bool(line, "CatalogAll", &settings->catalog_all)) return;
    dnd_settings_apply_bool(line, "Homebrew", &settings->homebrew);
}

bool dnd_settings_load(Storage* storage, DndSettings* settings) {
    if(!storage || !settings) return false;
    dnd_settings_defaults(settings);
    if(!storage_file_exists(storage, DND_SETTINGS_PATH)) return true;

    File* file = storage_file_alloc(storage);
    if(!file) return true;
    if(!storage_file_open(file, DND_SETTINGS_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_free(file);
        return true;
    }

    char line[64];
    size_t used = 0U;
    bool overflow = false;
    uint8_t buffer[128];
    while(true) {
        size_t count = storage_file_read(file, buffer, sizeof(buffer));
        if(!count) break;
        for(size_t i = 0U; i < count; ++i) {
            char ch = (char)buffer[i];
            if(ch == '\r') continue;
            if(ch == '\n') {
                if(!overflow) {
                    line[used] = '\0';
                    dnd_settings_apply_line(settings, line);
                }
                used = 0U;
                overflow = false;
                continue;
            }
            if(overflow) continue;
            if(used + 1U < sizeof(line))
                line[used++] = ch;
            else
                overflow = true;
        }
    }
    bool read_complete = storage_file_get_error(file) == FSE_OK;
    if(used && !overflow && read_complete) {
        line[used] = '\0';
        dnd_settings_apply_line(settings, line);
    }
    storage_file_close(file);
    storage_file_free(file);
    return true;
}

bool dnd_settings_save(Storage* storage, const DndSettings* settings) {
    if(!storage || !settings) return false;
    if(!dnd_fs_ensure_directory(storage, DND_CHARACTER_DATA_ROOT)) return false;

    File* file = storage_file_alloc(storage);
    if(!file) return false;
    bool ok = storage_file_open(file, DND_SETTINGS_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS) &&
              dnd_settings_write_line(file, "SkipDiceLoading", settings->skip_dice_loading) &&
              dnd_settings_write_line(file, "Debug", settings->debug) &&
              dnd_settings_write_line(file, "ExtraItems", settings->extra_items) &&
              dnd_settings_write_line(file, "CatalogAll", settings->catalog_all) &&
              dnd_settings_write_line(file, "Homebrew", settings->homebrew) &&
              storage_file_sync(file);
    storage_file_close(file);
    storage_file_free(file);
    return ok;
}
