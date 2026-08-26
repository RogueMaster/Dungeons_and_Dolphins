#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <storage/storage.h>

#define POCKET_I18N_HEAP_LIMIT 2048U
#define POCKET_I18N_MAX_ENTRIES 40U

typedef struct PocketI18nRuntime PocketI18nRuntime;

typedef struct {
    char id[12];
    char name[24];
    char file[24];
    uint8_t bundled;
} PocketLanguageSummary;

uint16_t pocket_i18n_language_count(Storage* storage);
bool pocket_i18n_language_at(Storage* storage, uint16_t index, PocketLanguageSummary* output);
PocketI18nRuntime* pocket_i18n_load(
    Storage* storage,
    const PocketLanguageSummary* language,
    size_t* heap_bytes);
void pocket_i18n_free(PocketI18nRuntime* runtime);
const char* pocket_i18n_get(
    const PocketI18nRuntime* runtime,
    const char* key,
    const char* english_fallback);
