#include "pocket_d20_i18n_runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define I18N_BUNDLED_INDEX APP_ASSETS_PATH("i18n/index.txt")
#define I18N_USER_INDEX APP_ASSETS_PATH("i18n/custom_index.txt")
#define I18N_BUNDLED_FILE APP_ASSETS_PATH("i18n/%s")
#define I18N_USER_FILE APP_ASSETS_PATH("i18n/custom_%s")

typedef struct {
    uint16_t key;
    uint16_t value;
} PocketI18nEntry;

struct PocketI18nRuntime {
    uint8_t count;
    uint16_t used;
    PocketI18nEntry entries[POCKET_I18N_MAX_ENTRIES];
    char data[1500];
};

_Static_assert(sizeof(PocketI18nRuntime) <= POCKET_I18N_HEAP_LIMIT, "language heap limit");

static void i18n_copy(char* out, size_t size, const char* value) {
    if(!size) return;
    strncpy(out, value ? value : "", size - 1U);
    out[size - 1U] = '\0';
}

static bool i18n_read_line(File* file, char* line, size_t size) {
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

static bool i18n_parse_summary(char* line, bool bundled, PocketLanguageSummary* output) {
    if(!line[0] || line[0] == '#') return false;
    char* name = strchr(line, '|');
    if(!name) return false;
    *name++ = '\0';
    char* file = strchr(name, '|');
    if(!file) return false;
    *file++ = '\0';
    memset(output, 0, sizeof(*output));
    i18n_copy(output->id, sizeof(output->id), line);
    i18n_copy(output->name, sizeof(output->name), name);
    i18n_copy(output->file, sizeof(output->file), file);
    output->bundled = bundled ? 1U : 0U;
    return output->id[0] && output->name[0] && output->file[0];
}

static uint16_t i18n_count_path(Storage* storage, const char* path, bool bundled) {
    File* file = storage_file_alloc(storage);
    uint16_t count = 0U;
    if(storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        char line[256];
        PocketLanguageSummary summary;
        while(i18n_read_line(file, line, sizeof(line)))
            if(i18n_parse_summary(line, bundled, &summary)) ++count;
    }
    storage_file_close(file);
    storage_file_free(file);
    return count;
}

static bool i18n_at_path(
    Storage* storage,
    const char* path,
    bool bundled,
    uint16_t wanted,
    PocketLanguageSummary* output) {
    File* file = storage_file_alloc(storage);
    bool found = false;
    if(storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        char line[256];
        uint16_t index = 0U;
        while(i18n_read_line(file, line, sizeof(line))) {
            PocketLanguageSummary summary;
            if(!i18n_parse_summary(line, bundled, &summary)) continue;
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

uint16_t pocket_i18n_language_count(Storage* storage) {
    return i18n_count_path(storage, I18N_BUNDLED_INDEX, true) +
           i18n_count_path(storage, I18N_USER_INDEX, false);
}

bool pocket_i18n_language_at(Storage* storage, uint16_t index, PocketLanguageSummary* output) {
    uint16_t bundled = i18n_count_path(storage, I18N_BUNDLED_INDEX, true);
    return index < bundled ? i18n_at_path(storage, I18N_BUNDLED_INDEX, true, index, output) :
                             i18n_at_path(storage, I18N_USER_INDEX, false, index - bundled, output);
}

PocketI18nRuntime* pocket_i18n_load(
    Storage* storage,
    const PocketLanguageSummary* language,
    size_t* heap_bytes) {
    *heap_bytes = 0U;
    if(!strcmp(language->id, "en")) return NULL;
    PocketI18nRuntime* runtime = calloc(1U, sizeof(PocketI18nRuntime));
    if(!runtime) return NULL;
    char path[128];
    snprintf(path, sizeof(path), I18N_USER_FILE, language->file);
    if(!storage_file_exists(storage, path) && language->bundled)
        snprintf(path, sizeof(path), I18N_BUNDLED_FILE, language->file);
    File* file = storage_file_alloc(storage);
    bool ok = storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING);
    char line[256];
    while(ok && runtime->count < POCKET_I18N_MAX_ENTRIES &&
          i18n_read_line(file, line, sizeof(line))) {
        if(!line[0] || line[0] == '#') continue;
        char* value = strchr(line, '=');
        if(!value) continue;
        *value++ = '\0';
        size_t key_size = strlen(line) + 1U, value_size = strlen(value) + 1U;
        if(runtime->used + key_size + value_size > sizeof(runtime->data)) {
            ok = false;
            break;
        }
        PocketI18nEntry* entry = &runtime->entries[runtime->count++];
        entry->key = runtime->used;
        memcpy(runtime->data + runtime->used, line, key_size);
        runtime->used += key_size;
        entry->value = runtime->used;
        memcpy(runtime->data + runtime->used, value, value_size);
        runtime->used += value_size;
    }
    storage_file_close(file);
    storage_file_free(file);
    if(!ok) {
        free(runtime);
        return NULL;
    }
    *heap_bytes = sizeof(PocketI18nRuntime);
    return runtime;
}

void pocket_i18n_free(PocketI18nRuntime* runtime) {
    free(runtime);
}

const char* pocket_i18n_get(
    const PocketI18nRuntime* runtime,
    const char* key,
    const char* english_fallback) {
    if(!runtime) return english_fallback;
    for(uint8_t i = 0U; i < runtime->count; ++i)
        if(!strcmp(runtime->data + runtime->entries[i].key, key))
            return runtime->data + runtime->entries[i].value;
    return english_fallback;
}
