#include "dndspellbook_collection.h"

#include "dnd_handoff.h"
#include "dnd_data.h"
#include "dnd_spell_eligibility.h"
#include "dnd_storage.h"

#include <furi.h>
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

#define TAG "DndSpellbook"
#define DNDSPELLBOOK_COLLECTION_VIEW_MAIN 0U
#define DNDSPELLBOOK_COLLECTION_VIEW_TEXT 1U
#define DNDSPELLBOOK_COLLECTION_VIEW_NUMBER 2U
#define DNDSPELLBOOK_COLLECTION_ROWS 5U
#define DNDSPELLBOOK_COLLECTION_CATALOG_PAGE 10U
#define DNDSPELLBOOK_COLLECTION_LINE_MAX 256U
#define DNDSPELLBOOK_COLLECTION_SPELL_CATALOG APP_ASSETS_PATH("catalogs/spells.txt")

typedef enum {
    DndSpellbookCollectionScreenNoCharacter,
    DndSpellbookCollectionScreenList,
    DndSpellbookCollectionScreenDetail,
    DndSpellbookCollectionScreenCatalog,
    DndSpellbookCollectionScreenFilters,
} DndSpellbookCollectionScreen;

typedef enum {
    DndSpellbookCollectionEditNone,
    DndSpellbookCollectionEditName,
    DndSpellbookCollectionEditDetail,
    DndSpellbookCollectionEditStableId,
    DndSpellbookCollectionEditSource,
    DndSpellbookCollectionEditSchool,
    DndSpellbookCollectionEditGrantName,
} DndSpellbookCollectionEdit;

typedef enum {
    DndSpellbookSourceAny,
    DndSpellbookSourceCore,
    DndSpellbookSourceXanathar,
    DndSpellbookSourceForgottenRealms,
    DndSpellbookSourceRavenloft,
    DndSpellbookSourceOther,
    DndSpellbookSourceCount,
} DndSpellbookSource;

enum {
    DndSpellbookClassMaskArtificer = 1U << 0,
    DndSpellbookClassMaskBarbarian = 1U << 1,
    DndSpellbookClassMaskBard = 1U << 2,
    DndSpellbookClassMaskCleric = 1U << 3,
    DndSpellbookClassMaskDruid = 1U << 4,
    DndSpellbookClassMaskFighter = 1U << 5,
    DndSpellbookClassMaskMonk = 1U << 6,
    DndSpellbookClassMaskPaladin = 1U << 7,
    DndSpellbookClassMaskRanger = 1U << 8,
    DndSpellbookClassMaskRogue = 1U << 9,
    DndSpellbookClassMaskSorcerer = 1U << 10,
    DndSpellbookClassMaskWarlock = 1U << 11,
    DndSpellbookClassMaskWizard = 1U << 12,
};

typedef struct {
    char name[POCKET_D20_CATALOG_NAME_LEN];
    uint8_t level;
    uint16_t class_mask;
    uint8_t school;
    uint8_t source;
    uint8_t ritual;
    uint16_t absolute_index;
} DndSpellbookCatalogEntry;

static const char* const dndspellbook_collection_school_names[] = {
    "Any", "Abjuration", "Conjuration", "Divination", "Enchantment",
    "Evocation", "Illusion", "Necromancy", "Transmutation"};
static const char* const dndspellbook_collection_source_names[] = {
    "Any", "Core", "Xanathar", "Forgotten Realms", "Ravenloft", "Other"};

typedef struct {
    Gui* gui;
    Storage* storage;
    ViewDispatcher* dispatcher;
    View* view;
    TextInput* text_input;
    NumberInput* number_input;
    PocketSaveData data;
    DndSpellbookCollectionScreen screen;
    DndSpellbookCollectionEdit edit;
    uint32_t profile;
    uint8_t have_profile;
    uint8_t return_to_dnd;
    uint8_t total;
    uint8_t cache_start;
    uint16_t selection;
    uint16_t scroll;
    uint8_t record_index;
    uint8_t detail_selection;
    uint16_t detail_scroll;
    char edit_buffer[POCKET_D20_DETAIL_LEN];
    uint8_t number_field;
    uint8_t input_active;
    char status[32];
    uint8_t status_transient;
    uint8_t action_ack_active;
    uint16_t action_ack_selection;
    DndSpellbookCatalogEntry catalog[DNDSPELLBOOK_COLLECTION_CATALOG_PAGE];
    uint8_t catalog_count;
    uint16_t catalog_page_start;
    uint16_t catalog_total;
    uint8_t catalog_has_more;
    int8_t filter_level;
    uint8_t filter_class;
    uint8_t filter_ritual;
    uint8_t filter_school;
    uint8_t filter_source;
    uint8_t filter_status;
    uint8_t filter_show_all;
    uint8_t filter_selection;
    DndSpellbookCollectionScreen filter_return_screen;
} DndSpellbookCollectionApp;

static bool dndspellbook_collection_load_page(DndSpellbookCollectionApp* app, uint8_t start);
static bool dndspellbook_collection_save_page(DndSpellbookCollectionApp* app);
static bool dndspellbook_collection_prepare_record(DndSpellbookCollectionApp* app, uint8_t logical);
static PocketSpell* dndspellbook_collection_spell(DndSpellbookCollectionApp* app, uint8_t logical, uint8_t* local_out);
static void dndspellbook_collection_redraw(DndSpellbookCollectionApp* app);
static void dndspellbook_collection_begin_text(
    DndSpellbookCollectionApp* app,
    DndSpellbookCollectionEdit edit,
    const char* header,
    const char* initial);
static bool dndspellbook_collection_begin_number(DndSpellbookCollectionApp* app, uint8_t field);

static void dndspellbook_collection_copy(char* destination, size_t size, const char* source) {
    if(!destination || !size) return;
    if(!source) source = "";
    strncpy(destination, source, size - 1U);
    destination[size - 1U] = '\0';
}

static bool dndspellbook_collection_parse_u32(const char* text, uint32_t* output) {
    if(!text || !*text || !output) return false;
    uint32_t value = 0U;
    const char* cursor = text;
    while(*cursor >= '0' && *cursor <= '9') {
        uint32_t digit = (uint32_t)(*cursor - '0');
        if(value > (UINT32_MAX - digit) / 10U) return false;
        value = value * 10U + digit;
        ++cursor;
    }
    if(cursor == text) return false;
    while(*cursor == ' ' || *cursor == '\r' || *cursor == '\n' || *cursor == '\t') ++cursor;
    if(*cursor) return false;
    *output = value;
    return true;
}

static uint8_t dndspellbook_collection_clamp_u8(int32_t value, uint8_t maximum) {
    if(value < 0) return 0U;
    if(value > maximum) return maximum;
    return (uint8_t)value;
}

static void dndspellbook_collection_set_status(DndSpellbookCollectionApp* app, const char* text) {
    dndspellbook_collection_copy(app->status, sizeof(app->status), text);
    app->status_transient = 0U;
}

static void dndspellbook_collection_set_transient_status(
    DndSpellbookCollectionApp* app,
    const char* text) {
    dndspellbook_collection_copy(app->status, sizeof(app->status), text);
    app->status_transient = 1U;
}

static void dndspellbook_collection_draw_header(Canvas* canvas, DndSpellbookCollectionApp* app, const char* title, const char* status) {
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(canvas, 0, 0, 128, 10);
    canvas_set_color(canvas, ColorWhite);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 8, title);

    /* Character ID belongs only to the main Spellbook list. Catalog paging
       hints are shown only inside the explicit Name/catalog picker. */
    bool main_list = app && app->screen == DndSpellbookCollectionScreenList && app->profile != UINT32_MAX;
    uint16_t status_right = 126U;
    if(main_list) {
        char profile_id[16];
        snprintf(profile_id, sizeof(profile_id), "[%lu]", (unsigned long)app->profile);
        uint16_t id_width = canvas_string_width(canvas, profile_id);
        uint8_t id_x = id_width < 125U ? (uint8_t)(126U - id_width) : 1U;
        canvas_draw_str(canvas, id_x, 8, profile_id);
        status_right = id_x > 2U ? (uint16_t)(id_x - 2U) : 0U;
    }

    if(status && status[0]) {
        uint16_t title_width = canvas_string_width(canvas, title);
        uint16_t status_width = canvas_string_width(canvas, status);
        if(status_width < status_right) {
            uint16_t status_x = status_right - status_width;
            if(status_x > title_width + 4U)
                canvas_draw_str(canvas, (uint8_t)status_x, 8, status);
        }
    }
    canvas_set_color(canvas, ColorBlack);
}

static void dndspellbook_collection_draw_row(Canvas* canvas, uint8_t row, bool selected, const char* text) {
    uint8_t y = (uint8_t)(11U + row * 10U);
    char display[27];
    size_t length = strlen(text);
    size_t copy = length > 25U ? 25U : length;
    memcpy(display, text, copy);
    display[copy] = '\0';
    if(selected) {
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_box(canvas, 0, y, 128, 10);
        canvas_set_color(canvas, ColorWhite);
    } else {
        canvas_set_color(canvas, ColorBlack);
    }
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 3, y + 8U, display);
    canvas_set_color(canvas, ColorBlack);
}

static void dndspellbook_collection_redraw(DndSpellbookCollectionApp* app) {
    if(!app || !app->view) return;
    DndSpellbookCollectionApp** model = view_get_model(app->view);
    if(!model) return;
    *model = app;
    view_commit_model(app->view, true);
}

static uint16_t dndspellbook_collection_class_mask_from_name(const char* name) {
    if(!name) return 0U;
    if(strcmp(name, "Artificer") == 0) return DndSpellbookClassMaskArtificer;
    if(strcmp(name, "Barbarian") == 0) return DndSpellbookClassMaskBarbarian;
    if(strcmp(name, "Bard") == 0) return DndSpellbookClassMaskBard;
    if(strcmp(name, "Cleric") == 0) return DndSpellbookClassMaskCleric;
    if(strcmp(name, "Druid") == 0) return DndSpellbookClassMaskDruid;
    if(strcmp(name, "Fighter") == 0) return DndSpellbookClassMaskFighter;
    if(strcmp(name, "Monk") == 0) return DndSpellbookClassMaskMonk;
    if(strcmp(name, "Paladin") == 0) return DndSpellbookClassMaskPaladin;
    if(strcmp(name, "Ranger") == 0) return DndSpellbookClassMaskRanger;
    if(strcmp(name, "Rogue") == 0) return DndSpellbookClassMaskRogue;
    if(strcmp(name, "Sorcerer") == 0) return DndSpellbookClassMaskSorcerer;
    if(strcmp(name, "Warlock") == 0) return DndSpellbookClassMaskWarlock;
    if(strcmp(name, "Wizard") == 0) return DndSpellbookClassMaskWizard;
    return 0U;
}

static char* dndspellbook_collection_trim(char* text) {
    if(!text) return text;
    while(*text == ' ' || *text == '\t') ++text;
    char* end = text + strlen(text);
    while(end > text && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n')) --end;
    *end = '\0';
    return text;
}

static bool dndspellbook_collection_equals_ci(const char* left, const char* right) {
    if(!left || !right) return false;
    while(*left && *right) {
        char a = *left++;
        char b = *right++;
        if(a >= 'A' && a <= 'Z') a = (char)(a - 'A' + 'a');
        if(b >= 'A' && b <= 'Z') b = (char)(b - 'A' + 'a');
        if(a != b) return false;
    }
    return *left == '\0' && *right == '\0';
}

static uint8_t dndspellbook_collection_school(const char* school) {
    if(!school || !*school) return 0U;
    for(uint8_t i = 1U; i < 9U; ++i)
        if(dndspellbook_collection_equals_ci(school, dndspellbook_collection_school_names[i])) return i;
    return 0U;
}

static uint8_t dndspellbook_collection_source(const char* source) {
    if(!source || !*source) return DndSpellbookSourceOther;
    if(dndspellbook_collection_equals_ci(source, "Core") || strstr(source, "SRD")) return DndSpellbookSourceCore;
    if(strstr(source, "Xanathar") || strstr(source, "XGE")) return DndSpellbookSourceXanathar;
    if(strstr(source, "Forgotten Realms") || strstr(source, "FR")) return DndSpellbookSourceForgottenRealms;
    if(strstr(source, "Ravenloft")) return DndSpellbookSourceRavenloft;
    return DndSpellbookSourceOther;
}

static uint16_t dndspellbook_collection_class_mask(char* classes) {
    uint16_t mask = 0U;
    char* cursor = classes;
    while(cursor && *cursor) {
        char* comma = strchr(cursor, ',');
        if(comma) *comma = '\0';
        mask |= dndspellbook_collection_class_mask_from_name(dndspellbook_collection_trim(cursor));
        if(!comma) break;
        cursor = comma + 1U;
    }
    return mask;
}

static bool dndspellbook_collection_class_allows(
    const PocketCharacter* character,
    uint8_t class_index,
    uint8_t level,
    uint16_t mask) {
    if(!character || class_index >= character->class_count) return false;
    const PocketClassLevel* class_level = &character->classes[class_index];
    uint16_t selected = dnd_spell_eligibility_class_uses_wizard_spell_list(class_level) ?
                            DndSpellbookClassMaskWizard :
                            dndspellbook_collection_class_mask_from_name(class_level->name);
    return selected && (mask & selected) &&
           level <= dnd_spell_eligibility_class_max_spell_level(class_level);
}

static bool dndspellbook_collection_spell_allowed(
    DndSpellbookCollectionApp* app,
    uint8_t level,
    uint16_t mask) {
    PocketCharacter* character = &app->data.character;
    if(app->filter_show_all) return true;
    if(app->filter_class < character->class_count)
        return dndspellbook_collection_class_allows(character, app->filter_class, level, mask);
    for(uint8_t i = 0U; i < character->class_count; ++i)
        if(dndspellbook_collection_class_allows(character, i, level, mask)) return true;
    return false;
}

static uint8_t dndspellbook_collection_resolve_class(
    DndSpellbookCollectionApp* app,
    uint8_t level,
    uint16_t mask,
    uint8_t preferred) {
    PocketCharacter* character = &app->data.character;
    if(app->filter_class < character->class_count &&
       dndspellbook_collection_class_allows(character, app->filter_class, level, mask))
        return app->filter_class;
    if(preferred < character->class_count &&
       dndspellbook_collection_class_allows(character, preferred, level, mask))
        return preferred;
    for(uint8_t i = 0U; i < character->class_count; ++i)
        if(dndspellbook_collection_class_allows(character, i, level, mask)) return i;
    return preferred < character->class_count ? preferred : 0U;
}

static bool dndspellbook_collection_load_page(DndSpellbookCollectionApp* app, uint8_t start) {
    uint8_t total = 0U;
    if(!dnd_storage_load_spellbook_window(
           app->storage, app->profile, start, &app->data.character, &total))
        return false;
    app->total = total;
    app->cache_start = start;
    return true;
}

static bool dndspellbook_collection_save_page(DndSpellbookCollectionApp* app) {
    bool ok = dnd_storage_save_spellbook_window(
        app->storage, app->profile, app->cache_start, &app->data.character);
    if(ok) dndspellbook_collection_set_transient_status(app, "Saved");
    else dndspellbook_collection_set_status(app, "UNSAVED");
    return ok;
}

static bool dndspellbook_collection_prepare_record(DndSpellbookCollectionApp* app, uint8_t logical) {
    if(logical >= app->total) return false;
    uint8_t target = (uint8_t)((logical / POCKET_D20_COLLECTION_CACHE_SIZE) * POCKET_D20_COLLECTION_CACHE_SIZE);
    if(target != app->cache_start && !dndspellbook_collection_load_page(app, target)) {
        dndspellbook_collection_set_status(app, "Read failed");
        return false;
    }
    return logical >= app->cache_start &&
           logical < (uint8_t)(app->cache_start + app->data.character.spell_count);
}

static uint8_t dndspellbook_collection_local(const DndSpellbookCollectionApp* app, uint8_t logical) {
    return (uint8_t)(logical - app->cache_start);
}

static PocketSpell* dndspellbook_collection_spell(
    DndSpellbookCollectionApp* app,
    uint8_t logical,
    uint8_t* local_out) {
    if(!dndspellbook_collection_prepare_record(app, logical)) return NULL;
    uint8_t local = dndspellbook_collection_local(app, logical);
    if(local >= app->data.character.spell_count) return NULL;
    if(local_out) *local_out = local;
    return &app->data.character.spells[local];
}

static PocketSpell* dndspellbook_collection_spell_cached(
    DndSpellbookCollectionApp* app,
    uint8_t logical,
    uint8_t* local_out) {
    if(!app || logical < app->cache_start) return NULL;
    uint8_t local = (uint8_t)(logical - app->cache_start);
    if(local >= app->data.character.spell_count) return NULL;
    if(local_out) *local_out = local;
    return &app->data.character.spells[local];
}

static void dndspellbook_collection_focus_list(DndSpellbookCollectionApp* app, uint8_t logical) {
    app->selection = (uint16_t)logical + 1U;
    uint16_t page_min = (uint16_t)app->cache_start + 1U;
    uint16_t page_max = page_min + POCKET_D20_COLLECTION_CACHE_SIZE - 1U;
    uint16_t max_selection = app->total;
    if(page_max > max_selection) page_max = max_selection;
    uint16_t scroll = app->selection > 4U ? app->selection - 4U : 0U;
    if(scroll && scroll < page_min) scroll = page_min;
    if(scroll + 4U > page_max && page_max >= 4U) scroll = page_max - 4U;
    if(app->selection == 0U) scroll = 0U;
    app->scroll = scroll;
}

static bool dndspellbook_collection_add_blank(DndSpellbookCollectionApp* app) {
    if(app->total >= POCKET_D20_MAX_SPELLS) {
        dndspellbook_collection_set_status(app, "Collection full");
        return false;
    }
    uint8_t target = (uint8_t)((app->total / POCKET_D20_COLLECTION_CACHE_SIZE) * POCKET_D20_COLLECTION_CACHE_SIZE);
    if(target != app->cache_start && !dndspellbook_collection_load_page(app, target)) {
        dndspellbook_collection_set_status(app, "Tail read failed");
        return false;
    }
    PocketCharacter* character = &app->data.character;
    uint8_t expected = (uint8_t)(app->total - target);
    if(character->spell_count != expected ||
       character->spell_count >= POCKET_D20_COLLECTION_CACHE_SIZE ||
       !dnd_data_reserve_spells(character, (uint8_t)(character->spell_count + 1U))) {
        dndspellbook_collection_set_status(app, "Spell add failed");
        return false;
    }
    uint8_t local = character->spell_count;
    PocketSpell* spell = &character->spells[local];
    memset(spell, 0, sizeof(*spell));
    dndspellbook_collection_copy(spell->name, sizeof(spell->name), "New Spell");
    character->spell_known[local] = 1U;
    ++character->spell_count;
    app->record_index = app->total++;
    bool saved = dndspellbook_collection_save_page(app);
    dndspellbook_collection_focus_list(app, app->record_index);
    app->detail_selection = 0U;
    app->detail_scroll = 0U;
    app->screen = DndSpellbookCollectionScreenDetail;
    if(saved)
        dndspellbook_collection_set_transient_status(app, "Spell added");
    else
        dndspellbook_collection_set_status(app, "Added - UNSAVED");
    return true;
}

static bool dndspellbook_collection_delete_current(DndSpellbookCollectionApp* app) {
    if(app->record_index >= app->total) return false;
    if(!dnd_storage_delete_spell(
           app->storage, app->profile, &app->data.character, app->record_index)) {
        dndspellbook_collection_set_status(app, "Delete failed");
        return false;
    }
    if(app->total) --app->total;
    if(app->total) {
        uint8_t logical = app->record_index < app->total ?
                              app->record_index :
                              (uint8_t)(app->total - 1U);
        uint8_t target = (uint8_t)((logical / POCKET_D20_COLLECTION_CACHE_SIZE) * POCKET_D20_COLLECTION_CACHE_SIZE);
        if(!dndspellbook_collection_load_page(app, target)) {
            dndspellbook_collection_set_status(app, "Deleted; read failed");
            return true;
        }
        dndspellbook_collection_focus_list(app, logical);
    } else {
        (void)dndspellbook_collection_load_page(app, 0U);
        app->selection = app->scroll = 0U;
    }
    app->screen = DndSpellbookCollectionScreenList;
    dndspellbook_collection_set_status(app, "Deleted");
    return true;
}

static bool dndspellbook_collection_read_line(File* file, char* line, size_t size) {
    if(!file || !line || size < 2U) return false;
    size_t used = 0U;
    bool got = false;
    while(true) {
        char ch = '\0';
        size_t count = storage_file_read(file, &ch, 1U);
        if(count != 1U) break;
        got = true;
        if(ch == '\r') continue;
        if(ch == '\n') break;
        if(used + 1U < size) line[used++] = ch;
    }
    line[used] = '\0';
    return got || used;
}

typedef struct {
    const char* name;
    uint8_t filter;
    bool matched;
} DndSpellbookStatusContext;

static bool dndspellbook_collection_status_visitor(
    uint8_t logical_index,
    const PocketSpell* spell,
    uint8_t known,
    uint8_t always_prepared,
    uint8_t free_casts_current,
    uint8_t free_casts_max,
    void* context) {
    UNUSED(logical_index);
    UNUSED(free_casts_current);
    UNUSED(free_casts_max);
    DndSpellbookStatusContext* status = context;
    if(!status || !spell || strcmp(spell->name, status->name) != 0) return true;
    if(status->filter == 1U)
        status->matched = spell->prepared != 0U;
    else if(status->filter == 2U)
        status->matched = known != 0U;
    else
        status->matched = always_prepared != 0U;
    return !status->matched;
}

static bool dndspellbook_collection_status_matches(DndSpellbookCollectionApp* app, const char* name) {
    if(!app->filter_status) return true;
    DndSpellbookStatusContext status = {
        .name = name,
        .filter = app->filter_status,
        .matched = false,
    };
    if(!dnd_storage_visit_spells(
           app->storage,
           app->profile,
           dndspellbook_collection_status_visitor,
           &status,
           NULL))
        return false;
    return status.matched;
}

static bool dndspellbook_collection_catalog_matches(
    DndSpellbookCollectionApp* app,
    const char* name,
    uint8_t level,
    uint16_t mask,
    uint8_t school,
    uint8_t source,
    bool ritual) {
    if(!dndspellbook_collection_spell_allowed(app, level, mask)) return false;
    if(app->filter_level >= 0 && level != (uint8_t)app->filter_level) return false;
    if(app->filter_ritual && !ritual) return false;
    if(app->filter_school && school != app->filter_school) return false;
    if(app->filter_source && source != app->filter_source) return false;
    if(app->filter_status && !dndspellbook_collection_status_matches(app, name)) return false;
    return true;
}

static bool dndspellbook_collection_parse_catalog_line(
    DndSpellbookCollectionApp* app,
    char* line,
    DndSpellbookCatalogEntry* entry) {
    if(!line || !entry) return false;
    char* start = dndspellbook_collection_trim(line);
    if(!*start || *start == '#') return false;
    memset(entry, 0, sizeof(*entry));
    char* level_text = strchr(start, '|');
    if(!level_text) return false;
    *level_text++ = '\0';
    char* classes = strchr(level_text, '|');
    if(!classes) return false;
    *classes++ = '\0';
    char* school = strchr(classes, '|');
    if(!school) return false;
    *school++ = '\0';
    char* ritual = strchr(school, '|');
    if(ritual) *ritual++ = '\0';
    char* source = ritual ? strchr(ritual, '|') : NULL;
    if(source) *source++ = '\0';
    uint32_t level_number = 0U;
    if(!dndspellbook_collection_parse_u32(
           dndspellbook_collection_trim(level_text), &level_number) ||
       level_number > 9U)
        return false;
    uint16_t mask = dndspellbook_collection_class_mask(dndspellbook_collection_trim(classes));
    uint8_t school_id = dndspellbook_collection_school(dndspellbook_collection_trim(school));
    bool ritual_flag = ritual &&
                       (dndspellbook_collection_equals_ci(dndspellbook_collection_trim(ritual), "Yes") ||
                        dndspellbook_collection_equals_ci(dndspellbook_collection_trim(ritual), "True"));
    uint8_t source_id = source ?
                            dndspellbook_collection_source(dndspellbook_collection_trim(source)) :
                            DndSpellbookSourceOther;
    start = dndspellbook_collection_trim(start);
    if(!dndspellbook_collection_catalog_matches(
           app,
           start,
           (uint8_t)level_number,
           mask,
           school_id,
           source_id,
           ritual_flag))
        return false;
    dndspellbook_collection_copy(entry->name, sizeof(entry->name), start);
    entry->level = (uint8_t)level_number;
    entry->class_mask = mask;
    entry->school = school_id;
    entry->source = source_id;
    entry->ritual = ritual_flag ? 1U : 0U;
    return entry->name[0] != '\0';
}

static bool dndspellbook_collection_load_catalog(DndSpellbookCollectionApp* app) {
    app->catalog_count = 0U;
    app->catalog_total = 0U;
    app->catalog_has_more = 0U;
    File* file = storage_file_alloc(app->storage);
    if(!file) return false;
    if(!storage_file_open(
           file,
           DNDSPELLBOOK_COLLECTION_SPELL_CATALOG,
           FSAM_READ,
           FSOM_OPEN_EXISTING)) {
        storage_file_free(file);
        dndspellbook_collection_set_status(app, "Catalog unavailable");
        return false;
    }
    char line[DNDSPELLBOOK_COLLECTION_LINE_MAX];
    uint16_t matched = 0U;
    while(dndspellbook_collection_read_line(file, line, sizeof(line))) {
        DndSpellbookCatalogEntry parsed;
        if(!dndspellbook_collection_parse_catalog_line(app, line, &parsed)) continue;
        parsed.absolute_index = matched;
        if(matched >= app->catalog_page_start &&
           matched < (uint16_t)(app->catalog_page_start + DNDSPELLBOOK_COLLECTION_CATALOG_PAGE))
            app->catalog[app->catalog_count++] = parsed;
        ++matched;
    }
    app->catalog_total = matched;
    app->catalog_has_more = (uint16_t)(app->catalog_page_start + app->catalog_count) < matched;
    storage_file_close(file);
    storage_file_free(file);
    if(app->catalog_page_start >= matched && app->catalog_page_start) {
        app->catalog_page_start = matched ?
                                      (uint16_t)(((matched - 1U) / DNDSPELLBOOK_COLLECTION_CATALOG_PAGE) * DNDSPELLBOOK_COLLECTION_CATALOG_PAGE) :
                                      0U;
        return dndspellbook_collection_load_catalog(app);
    }
    return true;
}

static void dndspellbook_collection_open_catalog(DndSpellbookCollectionApp* app) {
    app->screen = DndSpellbookCollectionScreenCatalog;
    app->catalog_page_start = 0U;
    app->selection = 0U;
    if(!dndspellbook_collection_load_catalog(app))
        dndspellbook_collection_set_status(app, "Catalog unavailable");
}

static bool dndspellbook_collection_apply_catalog(DndSpellbookCollectionApp* app) {
    if(app->selection >= app->catalog_count) return false;
    DndSpellbookCatalogEntry* selected = &app->catalog[app->selection];
    uint8_t local = 0U;
    PocketSpell* spell = dndspellbook_collection_spell(app, app->record_index, &local);
    if(!spell) return false;
    dndspellbook_collection_copy(spell->name, sizeof(spell->name), selected->name);
    spell->level = selected->level;
    spell->class_index = dndspellbook_collection_resolve_class(
        app, selected->level, selected->class_mask, spell->class_index);
    snprintf(
        spell->stable_id,
        sizeof(spell->stable_id),
        "spell-%u-%u",
        spell->level,
        selected->absolute_index);
    spell->school[0] = '\0';
    if(selected->school)
        dndspellbook_collection_copy(
            spell->school,
            sizeof(spell->school),
            dndspellbook_collection_school_names[selected->school]);
    spell->source[0] = '\0';
    if(selected->source < DndSpellbookSourceCount)
        dndspellbook_collection_copy(
            spell->source,
            sizeof(spell->source),
            dndspellbook_collection_source_names[selected->source]);
    spell->ritual = selected->ritual;
    app->data.character.spell_known[local] = 1U;
    bool saved = dndspellbook_collection_save_page(app);
    app->screen = DndSpellbookCollectionScreenDetail;
    app->detail_selection = 0U;
    app->detail_scroll = 0U;
    if(saved) dndspellbook_collection_set_transient_status(app, "Catalog choice saved");
    else dndspellbook_collection_set_status(app, "Choice - UNSAVED");
    return saved;
}

static void dndspellbook_collection_list_adjust_scroll(DndSpellbookCollectionApp* app) {
    if(!app) return;
    uint16_t count = (uint16_t)app->total + 1U;
    if(!count) return;
    if(app->selection >= count) app->selection = count - 1U;
    if(app->selection == 0U || app->total == 0U) {
        app->scroll = 0U;
        return;
    }

    uint8_t logical = (uint8_t)(app->selection - 1U);
    uint8_t page_start = (uint8_t)(
        (logical / POCKET_D20_COLLECTION_CACHE_SIZE) *
        POCKET_D20_COLLECTION_CACHE_SIZE);
    uint16_t first = (uint16_t)page_start + 1U;
    uint8_t page_records = (uint8_t)(app->total - page_start);
    if(page_records > POCKET_D20_COLLECTION_CACHE_SIZE)
        page_records = POCKET_D20_COLLECTION_CACHE_SIZE;
    uint16_t last = first + page_records - 1U;

    /* Keep + Add New visible with up to four spells. A fifth spell is the first
       point where the five-row viewport needs to scroll it away. */
    if(page_start == 0U && app->selection <= 4U) {
        app->scroll = 0U;
        return;
    }
    if(page_records <= DNDSPELLBOOK_COLLECTION_ROWS) {
        app->scroll = first;
        return;
    }

    uint16_t scroll = app->selection > first + 3U ? app->selection - 4U : first;
    uint16_t maximum = last - (DNDSPELLBOOK_COLLECTION_ROWS - 1U);
    if(scroll > maximum) scroll = maximum;
    if(scroll < first) scroll = first;
    app->scroll = scroll;
}

static bool dndspellbook_collection_ensure_list_page(
    DndSpellbookCollectionApp* app,
    uint16_t selection) {
    if(!app) return false;
    if(selection == 0U) {
        if(app->total && app->cache_start != 0U)
            return dndspellbook_collection_load_page(app, 0U);
        return true;
    }
    uint8_t logical = (uint8_t)(selection - 1U);
    uint8_t target = (uint8_t)(
        (logical / POCKET_D20_COLLECTION_CACHE_SIZE) *
        POCKET_D20_COLLECTION_CACHE_SIZE);
    if(target == app->cache_start) return true;
    return dndspellbook_collection_load_page(app, target);
}

static bool dndspellbook_collection_move_list(DndSpellbookCollectionApp* app, int8_t delta) {
    uint16_t count = (uint16_t)app->total + 1U;
    if(!count) return false;
    int32_t next = (int32_t)app->selection + delta;
    if(next < 0) next = count - 1U;
    if(next >= count) next = 0;
    if(!dndspellbook_collection_ensure_list_page(app, (uint16_t)next)) {
        dndspellbook_collection_set_status(app, "Read failed");
        return false;
    }
    app->selection = (uint16_t)next;
    app->status[0] = '\0';
    dndspellbook_collection_list_adjust_scroll(app);
    return true;
}

static bool dndspellbook_collection_page_list(DndSpellbookCollectionApp* app, int8_t delta) {
    if(!app || !app->total || !delta) return false;
    uint16_t target = app->cache_start;
    if(delta < 0) {
        if(!app->cache_start) return false;
        target = app->cache_start >= POCKET_D20_COLLECTION_CACHE_SIZE ?
                     (uint16_t)(app->cache_start - POCKET_D20_COLLECTION_CACHE_SIZE) :
                     0U;
    } else {
        target = (uint16_t)app->cache_start + POCKET_D20_COLLECTION_CACHE_SIZE;
        if(target >= app->total) return false;
    }
    if(!dndspellbook_collection_load_page(app, (uint8_t)target)) {
        dndspellbook_collection_set_status(app, "Read failed");
        return false;
    }
    app->selection = target + 1U;
    app->status[0] = '\0';
    dndspellbook_collection_list_adjust_scroll(app);
    return true;
}

static uint8_t dndspellbook_collection_detail_count(void) {
    return 17U;
}

static void dndspellbook_collection_format_detail(
    DndSpellbookCollectionApp* app,
    uint8_t field,
    char* out,
    size_t size) {
    PocketCharacter* character = &app->data.character;
    uint8_t local = 0U;
    PocketSpell* spell = dndspellbook_collection_spell_cached(app, app->record_index, &local);
    if(!spell) {
        dndspellbook_collection_copy(out, size, "Read error");
        return;
    }
    switch(field) {
    case 0: snprintf(out, size, "Name: %.31s", spell->name); break;
    case 1: snprintf(out, size, "Notes: %.31s", spell->detail); break;
    case 2: snprintf(out, size, "Source class: %s", spell->class_index < character->class_count ? character->classes[spell->class_index].name : "Primary"); break;
    case 3: snprintf(out, size, "Level: %u", spell->level); break;
    case 4: snprintf(out, size, "Known: %s", character->spell_known[local] ? "Yes" : "No"); break;
    case 5: snprintf(out, size, "Prepared: %s", spell->prepared ? "Yes" : "No"); break;
    case 6: snprintf(out, size, "Always prepared: %s", character->spell_always_prepared[local] ? "Yes" : "No"); break;
    case 7: snprintf(out, size, "Ritual: %s", spell->ritual ? "Yes" : "No"); break;
    case 8: snprintf(out, size, "Free casts: %u/%u", character->spell_free_casts_current[local], character->spell_free_casts_max[local]); break;
    case 9: snprintf(out, size, "Free casts max: %u", character->spell_free_casts_max[local]); break;
    case 10: dndspellbook_collection_copy(out, size, character->spell_free_casts_current[local] ? "Use one free cast" : "No free casts left"); break;
    case 11: snprintf(out, size, "Stable ID: %.23s", spell->stable_id); break;
    case 12: snprintf(out, size, "Source: %.23s", spell->source); break;
    case 13: snprintf(out, size, "School: %.23s", spell->school); break;
    case 14: snprintf(out, size, "Grant source: %.23s", spell->grant_name); break;
    case 15: snprintf(out, size, "Grant type: %u", spell->grant_source); break;
    default: dndspellbook_collection_copy(out, size, "Delete spell"); break;
    }
}

static void dndspellbook_collection_draw_list(Canvas* canvas, DndSpellbookCollectionApp* app) {
    char title[32];
    snprintf(title, sizeof(title), "Spellbook %.7s", app->data.character.name);
    dndspellbook_collection_draw_header(canvas, app, title, app->status);
    uint16_t count = (uint16_t)app->total + 1U;
    for(uint8_t row = 0U; row < DNDSPELLBOOK_COLLECTION_ROWS; ++row) {
        uint16_t index = app->scroll + row;
        if(index >= count) break;
        char text[52];
        if(index == 0U) {
            dndspellbook_collection_copy(text, sizeof(text), "+ Add New");
        } else {
            uint8_t logical = (uint8_t)(index - 1U);
            if(logical < app->cache_start ||
               logical >= app->cache_start + POCKET_D20_COLLECTION_CACHE_SIZE) {
                dndspellbook_collection_copy(text, sizeof(text), "Page unavailable");
            } else {
                uint8_t local = dndspellbook_collection_local(app, logical);
                if(local < app->data.character.spell_count) {
                    PocketSpell* spell = &app->data.character.spells[local];
                    char status_mark = app->data.character.spell_always_prepared[local] ?
                                           'A' :
                                           spell->prepared ?
                                               'P' :
                                               app->data.character.spell_known[local] ? 'K' : '-';
                    char free_mark = app->data.character.spell_free_casts_current[local] ? 'F' : ' ';
                    snprintf(
                        text,
                        sizeof(text),
                        "%c%c L%u %.41s",
                        status_mark,
                        free_mark,
                        spell->level,
                        spell->name);
                } else {
                    dndspellbook_collection_copy(text, sizeof(text), "Read error");
                }
            }
        }
        if(app->action_ack_active && index == app->action_ack_selection) {
            char confirmed[52];
            snprintf(confirmed, sizeof(confirmed), "[X] %.46s", text);
            dndspellbook_collection_copy(text, sizeof(text), confirmed);
        }
        dndspellbook_collection_draw_row(canvas, row, index == app->selection, text);
    }
}

static void dndspellbook_collection_draw_detail(Canvas* canvas, DndSpellbookCollectionApp* app) {
    dndspellbook_collection_draw_header(canvas, app, "Spell Editor", app->status);
    uint8_t count = dndspellbook_collection_detail_count();
    for(uint8_t row = 0U; row < DNDSPELLBOOK_COLLECTION_ROWS; ++row) {
        uint16_t field = app->detail_scroll + row;
        if(field >= count) break;
        char text[64];
        dndspellbook_collection_format_detail(app, (uint8_t)field, text, sizeof(text));
        dndspellbook_collection_draw_row(canvas, row, field == app->detail_selection, text);
    }
}

static void dndspellbook_collection_draw_catalog(Canvas* canvas, DndSpellbookCollectionApp* app) {
    char title[48];
    snprintf(
        title,
        sizeof(title),
        "Spells: %s",
        app->filter_class < app->data.character.class_count ?
            app->data.character.classes[app->filter_class].name :
            "All Classes");
    char page[16];
    snprintf(
        page,
        sizeof(page),
        "P%u%s <>",
        (unsigned)(app->catalog_page_start / DNDSPELLBOOK_COLLECTION_CATALOG_PAGE + 1U),
        app->catalog_has_more ? "+" : "");
    dndspellbook_collection_draw_header(
        canvas, app, title, app->status[0] ? app->status : page);
    if(!app->catalog_count) {
        dndspellbook_collection_draw_row(canvas, 0U, false, "No matching entries");
        return;
    }
    uint16_t scroll = app->selection > 4U ? app->selection - 4U : 0U;
    for(uint8_t row = 0U; row < DNDSPELLBOOK_COLLECTION_ROWS; ++row) {
        uint16_t index = scroll + row;
        if(index >= app->catalog_count) break;
        DndSpellbookCatalogEntry* entry = &app->catalog[index];
        char text[56];
        snprintf(text, sizeof(text), "L%u %.49s", entry->level, entry->name);
        dndspellbook_collection_draw_row(canvas, row, index == app->selection, text);
    }
}

static void dndspellbook_collection_draw_filters(Canvas* canvas, DndSpellbookCollectionApp* app) {
    char rows[7][48];
    PocketCharacter* character = &app->data.character;
    if(app->filter_level < 0)
        dndspellbook_collection_copy(rows[0], sizeof(rows[0]), "Level: Any");
    else if(app->filter_level == 0)
        dndspellbook_collection_copy(rows[0], sizeof(rows[0]), "Level: Cantrip");
    else
        snprintf(rows[0], sizeof(rows[0]), "Level: %d", app->filter_level);
    snprintf(
        rows[1],
        sizeof(rows[1]),
        "Spell Class: %s",
        app->filter_class < character->class_count ?
            character->classes[app->filter_class].name :
            "All Classes");
    snprintf(rows[2], sizeof(rows[2]), "Ritual: %s", app->filter_ritual ? "Only" : "Any");
    snprintf(rows[3], sizeof(rows[3]), "School: %s", dndspellbook_collection_school_names[app->filter_school]);
    snprintf(rows[4], sizeof(rows[4]), "Source: %s", dndspellbook_collection_source_names[app->filter_source]);
    snprintf(rows[5], sizeof(rows[5]), "Status: %s", app->filter_status == 1U ? "Prepared" : app->filter_status == 2U ? "Known" : app->filter_status == 3U ? "Always" : "Any");
    snprintf(
        rows[6],
        sizeof(rows[6]),
        "Eligibility: %s",
        app->filter_show_all ? "All Spells" : "Allowed");
    dndspellbook_collection_draw_header(canvas, app, "Spell Filters", app->status);
    uint8_t scroll = app->filter_selection > 4U ? app->filter_selection - 4U : 0U;
    for(uint8_t row = 0U; row < DNDSPELLBOOK_COLLECTION_ROWS; ++row) {
        uint8_t index = scroll + row;
        if(index >= 7U) break;
        dndspellbook_collection_draw_row(canvas, row, index == app->filter_selection, rows[index]);
    }
}

static void dndspellbook_collection_draw(Canvas* canvas, void* model) {
    /* View draw callbacks receive the model buffer, not view context. */
    if(!model) return;
    DndSpellbookCollectionApp* app = *(DndSpellbookCollectionApp**)model;
    if(!app) return;
    canvas_clear(canvas);
    switch(app->screen) {
    case DndSpellbookCollectionScreenNoCharacter:
        dndspellbook_collection_draw_header(canvas, app, "DNDSpellbook", NULL);
        dndspellbook_collection_draw_row(canvas, 0U, false, "No character");
        dndspellbook_collection_draw_row(canvas, 1U, true, "OK: Open DNDolphins");
        break;
    case DndSpellbookCollectionScreenList: dndspellbook_collection_draw_list(canvas, app); break;
    case DndSpellbookCollectionScreenDetail: dndspellbook_collection_draw_detail(canvas, app); break;
    case DndSpellbookCollectionScreenCatalog: dndspellbook_collection_draw_catalog(canvas, app); break;
    case DndSpellbookCollectionScreenFilters: dndspellbook_collection_draw_filters(canvas, app); break;
    }
}

static void dndspellbook_collection_release_text(DndSpellbookCollectionApp* app) {
    if(!app->text_input || app->input_active) return;
    view_dispatcher_remove_view(app->dispatcher, DNDSPELLBOOK_COLLECTION_VIEW_TEXT);
    text_input_free(app->text_input);
    app->text_input = NULL;
}

static void dndspellbook_collection_release_number(DndSpellbookCollectionApp* app) {
    if(!app->number_input || app->input_active) return;
    view_dispatcher_remove_view(app->dispatcher, DNDSPELLBOOK_COLLECTION_VIEW_NUMBER);
    number_input_free(app->number_input);
    app->number_input = NULL;
}

static void dndspellbook_collection_text_done(void* context);
static void dndspellbook_collection_number_done(void* context, int32_t number);

static void dndspellbook_collection_begin_text(
    DndSpellbookCollectionApp* app,
    DndSpellbookCollectionEdit edit,
    const char* header,
    const char* initial) {
    dndspellbook_collection_release_number(app);
    if(!app->text_input) {
        app->text_input = text_input_alloc();
        if(!app->text_input) {
            dndspellbook_collection_set_status(app, "Text memory low");
            return;
        }
        view_dispatcher_add_view(app->dispatcher, DNDSPELLBOOK_COLLECTION_VIEW_TEXT, text_input_get_view(app->text_input));
    }
    app->edit = edit;
    app->input_active = 1U;
    dndspellbook_collection_copy(app->edit_buffer, sizeof(app->edit_buffer), initial);
    text_input_reset(app->text_input);
    text_input_set_header_text(app->text_input, header);
    text_input_set_result_callback(app->text_input, dndspellbook_collection_text_done, app, app->edit_buffer, sizeof(app->edit_buffer), false);
    view_dispatcher_switch_to_view(app->dispatcher, DNDSPELLBOOK_COLLECTION_VIEW_TEXT);
}

static bool dndspellbook_collection_number_spec(
    DndSpellbookCollectionApp* app,
    uint8_t field,
    const char** header,
    int32_t* value,
    int32_t* minimum,
    int32_t* maximum) {
    *header = NULL;
    *value = 0;
    *minimum = 0;
    *maximum = 999;
    uint8_t local = 0U;
    PocketSpell* spell = dndspellbook_collection_spell(app, app->record_index, &local);
    if(!spell) return false;
    if(field == 3U) {
        *header = "Spell level";
        *value = spell->level;
        *maximum = 9;
    } else if(field == 8U) {
        *header = "Free casts current";
        *value = app->data.character.spell_free_casts_current[local];
        *maximum = app->data.character.spell_free_casts_max[local];
    } else if(field == 9U) {
        *header = "Free casts maximum";
        *value = app->data.character.spell_free_casts_max[local];
        *maximum = 20;
    } else {
        return false;
    }
    return true;
}

static bool dndspellbook_collection_begin_number(DndSpellbookCollectionApp* app, uint8_t field) {
    const char* header = NULL;
    int32_t value = 0;
    int32_t minimum = 0;
    int32_t maximum = 0;
    if(!dndspellbook_collection_number_spec(app, field, &header, &value, &minimum, &maximum)) return false;
    dndspellbook_collection_release_text(app);
    if(!app->number_input) {
        app->number_input = number_input_alloc();
        if(!app->number_input) {
            dndspellbook_collection_set_status(app, "Number memory low");
            return true;
        }
        view_dispatcher_add_view(app->dispatcher, DNDSPELLBOOK_COLLECTION_VIEW_NUMBER, number_input_get_view(app->number_input));
    }
    app->number_field = field;
    app->input_active = 1U;
    number_input_set_header_text(app->number_input, header);
    number_input_set_result_callback(app->number_input, dndspellbook_collection_number_done, app, value, minimum, maximum);
    view_dispatcher_switch_to_view(app->dispatcher, DNDSPELLBOOK_COLLECTION_VIEW_NUMBER);
    return true;
}

static void dndspellbook_collection_adjust(DndSpellbookCollectionApp* app, uint8_t field, int8_t delta) {
    PocketCharacter* character = &app->data.character;
    uint8_t local = 0U;
    PocketSpell* spell = dndspellbook_collection_spell(app, app->record_index, &local);
    if(!spell) return;
    if(field == 2U) {
        if(!character->class_count) return;
        int16_t next = (int16_t)spell->class_index + delta;
        if(next < 0) next = (int16_t)character->class_count - 1;
        if(next >= (int16_t)character->class_count) next = 0;
        spell->class_index = (uint8_t)next;
    } else if(field == 3U) {
        spell->level = dndspellbook_collection_clamp_u8((int16_t)spell->level + delta, 9U);
    } else if(field == 4U) {
        character->spell_known[local] = !character->spell_known[local];
        if(!character->spell_known[local]) {
            spell->prepared = 0U;
            character->spell_always_prepared[local] = 0U;
        }
    } else if(field == 5U) {
        spell->prepared = !spell->prepared;
        if(spell->prepared) character->spell_known[local] = 1U;
    } else if(field == 6U) {
        character->spell_always_prepared[local] = !character->spell_always_prepared[local];
        if(character->spell_always_prepared[local]) character->spell_known[local] = 1U;
    } else if(field == 7U) {
        spell->ritual = !spell->ritual;
    } else if(field == 8U) {
        character->spell_free_casts_current[local] = dndspellbook_collection_clamp_u8(
            (int16_t)character->spell_free_casts_current[local] + delta,
            character->spell_free_casts_max[local]);
    } else if(field == 9U) {
        character->spell_free_casts_max[local] = dndspellbook_collection_clamp_u8(
            (int16_t)character->spell_free_casts_max[local] + delta, 20U);
        if(character->spell_free_casts_current[local] > character->spell_free_casts_max[local])
            character->spell_free_casts_current[local] = character->spell_free_casts_max[local];
    } else if(field == 15U) {
        int16_t source = (int16_t)spell->grant_source + delta;
        if(source < 0) source = PocketGrantSourceCount - 1U;
        if(source >= PocketGrantSourceCount) source = 0;
        spell->grant_source = (uint8_t)source;
    } else {
        return;
    }
    (void)dndspellbook_collection_save_page(app);
}

static void dndspellbook_collection_text_done(void* context) {
    DndSpellbookCollectionApp* app = context;
    if(!app) return;
    PocketSpell* spell = dndspellbook_collection_spell(app, app->record_index, NULL);
    if(spell) {
        if(app->edit == DndSpellbookCollectionEditName)
            dndspellbook_collection_copy(spell->name, sizeof(spell->name), app->edit_buffer);
        else if(app->edit == DndSpellbookCollectionEditDetail)
            dndspellbook_collection_copy(spell->detail, sizeof(spell->detail), app->edit_buffer);
        else if(app->edit == DndSpellbookCollectionEditStableId)
            dndspellbook_collection_copy(spell->stable_id, sizeof(spell->stable_id), app->edit_buffer);
        else if(app->edit == DndSpellbookCollectionEditSource)
            dndspellbook_collection_copy(spell->source, sizeof(spell->source), app->edit_buffer);
        else if(app->edit == DndSpellbookCollectionEditSchool)
            dndspellbook_collection_copy(spell->school, sizeof(spell->school), app->edit_buffer);
        else if(app->edit == DndSpellbookCollectionEditGrantName)
            dndspellbook_collection_copy(spell->grant_name, sizeof(spell->grant_name), app->edit_buffer);
    }
    app->input_active = 0U;
    app->edit = DndSpellbookCollectionEditNone;
    (void)dndspellbook_collection_save_page(app);
    view_dispatcher_switch_to_view(app->dispatcher, DNDSPELLBOOK_COLLECTION_VIEW_MAIN);
    dndspellbook_collection_redraw(app);
}

static void dndspellbook_collection_number_done(void* context, int32_t number) {
    DndSpellbookCollectionApp* app = context;
    if(!app) return;
    uint8_t local = 0U;
    PocketSpell* spell = dndspellbook_collection_spell(app, app->record_index, &local);
    if(spell) {
        if(app->number_field == 3U) {
            spell->level = (uint8_t)number;
        } else if(app->number_field == 8U) {
            app->data.character.spell_free_casts_current[local] = (uint8_t)number;
        } else if(app->number_field == 9U) {
            app->data.character.spell_free_casts_max[local] = (uint8_t)number;
            if(app->data.character.spell_free_casts_current[local] > (uint8_t)number)
                app->data.character.spell_free_casts_current[local] = (uint8_t)number;
        }
    }
    app->input_active = 0U;
    (void)dndspellbook_collection_save_page(app);
    view_dispatcher_switch_to_view(app->dispatcher, DNDSPELLBOOK_COLLECTION_VIEW_MAIN);
    dndspellbook_collection_redraw(app);
}

static void dndspellbook_collection_detail_ok(DndSpellbookCollectionApp* app) {
    uint8_t field = app->detail_selection;
    uint8_t local = 0U;
    PocketSpell* spell = dndspellbook_collection_spell(app, app->record_index, &local);
    if(!spell) return;
    if(field == 0U)
        dndspellbook_collection_open_catalog(app);
    else if(field == 1U)
        dndspellbook_collection_begin_text(app, DndSpellbookCollectionEditDetail, "Spell notes", spell->detail);
    else if(field < 10U)
        dndspellbook_collection_adjust(app, field, 1);
    else if(field == 10U) {
        if(app->data.character.spell_free_casts_current[local]) {
            --app->data.character.spell_free_casts_current[local];
            (void)dndspellbook_collection_save_page(app);
            dndspellbook_collection_set_status(app, "Free cast used");
        } else {
            dndspellbook_collection_set_status(app, "No free casts left");
        }
    } else if(field == 11U)
        dndspellbook_collection_begin_text(app, DndSpellbookCollectionEditStableId, "Stable ID", spell->stable_id);
    else if(field == 12U)
        dndspellbook_collection_begin_text(app, DndSpellbookCollectionEditSource, "Spell source", spell->source);
    else if(field == 13U)
        dndspellbook_collection_begin_text(app, DndSpellbookCollectionEditSchool, "Spell school", spell->school);
    else if(field == 14U)
        dndspellbook_collection_begin_text(app, DndSpellbookCollectionEditGrantName, "Grant source name", spell->grant_name);
    else if(field == 15U)
        dndspellbook_collection_adjust(app, field, 1);
    else
        (void)dndspellbook_collection_delete_current(app);
}

static void dndspellbook_collection_detail_hold_ok(DndSpellbookCollectionApp* app) {
    if(dndspellbook_collection_begin_number(app, app->detail_selection)) return;
    if(app->detail_selection != 0U) return;
    PocketSpell* spell = dndspellbook_collection_spell(app, app->record_index, NULL);
    if(spell)
        dndspellbook_collection_begin_text(app, DndSpellbookCollectionEditName, "Custom spell", spell->name);
}

static void dndspellbook_collection_filter_adjust(DndSpellbookCollectionApp* app, int8_t delta) {
    PocketCharacter* character = &app->data.character;
    switch(app->filter_selection) {
    case 0: {
        int16_t next = app->filter_level + delta;
        if(next < -1) next = 9;
        if(next > 9) next = -1;
        app->filter_level = (int8_t)next;
        break;
    }
    case 1: {
        int16_t next = app->filter_class == UINT8_MAX ? -1 : app->filter_class;
        next += delta;
        if(next < -1) next = (int16_t)character->class_count - 1;
        if(next >= (int16_t)character->class_count) next = -1;
        app->filter_class = next < 0 ? UINT8_MAX : (uint8_t)next;
        break;
    }
    case 2: app->filter_ritual = !app->filter_ritual; break;
    case 3: {
        int16_t next = (int16_t)app->filter_school + delta;
        if(next < 0) next = 8;
        if(next > 8) next = 0;
        app->filter_school = (uint8_t)next;
        break;
    }
    case 4: {
        int16_t next = (int16_t)app->filter_source + delta;
        if(next < 0) next = DndSpellbookSourceCount - 1U;
        if(next >= DndSpellbookSourceCount) next = 0;
        app->filter_source = (uint8_t)next;
        break;
    }
    case 5: {
        int16_t next = (int16_t)app->filter_status + delta;
        if(next < 0) next = 3;
        if(next > 3) next = 0;
        app->filter_status = (uint8_t)next;
        break;
    }
    default:
        app->filter_show_all = !app->filter_show_all;
        break;
    }
}

static bool dndspellbook_collection_input(InputEvent* event, void* context) {
    DndSpellbookCollectionApp* app = context;
    if(!app || !event) return false;
    bool move = event->type == InputTypeShort || event->type == InputTypeRepeat;
    if(event->type != InputTypeShort && event->type != InputTypeRepeat &&
       event->type != InputTypeLong)
        return true;

    if(app->status_transient) {
        app->status[0] = '\0';
        app->status_transient = 0U;
    }
    app->action_ack_active = 0U;

    if(event->key == InputKeyBack && event->type == InputTypeLong) {
        app->return_to_dnd = 0U;
        view_dispatcher_stop(app->dispatcher);
        return true;
    }

    if(event->key == InputKeyBack && event->type == InputTypeShort) {
        if(app->screen == DndSpellbookCollectionScreenNoCharacter ||
           app->screen == DndSpellbookCollectionScreenList) {
            app->return_to_dnd = 1U;
            view_dispatcher_stop(app->dispatcher);
            return true;
        } else if(app->screen == DndSpellbookCollectionScreenDetail) {
            app->screen = DndSpellbookCollectionScreenList;
            dndspellbook_collection_focus_list(app, app->record_index);
        } else if(app->screen == DndSpellbookCollectionScreenCatalog) {
            app->screen = DndSpellbookCollectionScreenDetail;
        } else if(app->screen == DndSpellbookCollectionScreenFilters) {
            app->screen = app->filter_return_screen;
            if(app->screen == DndSpellbookCollectionScreenCatalog) {
                app->catalog_page_start = 0U;
                app->selection = 0U;
                (void)dndspellbook_collection_load_catalog(app);
            }
        }
        dndspellbook_collection_redraw(app);
        return true;
    }

    if(app->screen == DndSpellbookCollectionScreenNoCharacter) {
        if(event->key == InputKeyOk && event->type == InputTypeShort) {
            app->return_to_dnd = 1U;
            view_dispatcher_stop(app->dispatcher);
        }
        return true;
    }

    if(app->screen == DndSpellbookCollectionScreenList) {
        if(event->type == InputTypeLong && event->key == InputKeyUp) {
            app->filter_return_screen = DndSpellbookCollectionScreenList;
            app->filter_selection = 0U;
            app->screen = DndSpellbookCollectionScreenFilters;
        } else if(move && event->key == InputKeyUp)
            (void)dndspellbook_collection_move_list(app, -1);
        else if(move && event->key == InputKeyDown)
            (void)dndspellbook_collection_move_list(app, 1);
        else if(event->type == InputTypeShort && event->key == InputKeyLeft)
            (void)dndspellbook_collection_page_list(app, -1);
        else if(event->type == InputTypeShort && event->key == InputKeyRight)
            (void)dndspellbook_collection_page_list(app, 1);
        else if(event->key == InputKeyOk &&
                (event->type == InputTypeShort || event->type == InputTypeLong) &&
                app->selection == 0U)
            (void)dndspellbook_collection_add_blank(app);
        else if(event->key == InputKeyOk && event->type == InputTypeLong && app->selection) {
            uint8_t logical = (uint8_t)(app->selection - 1U);
            uint8_t local = 0U;
            PocketSpell* spell = dndspellbook_collection_spell(app, logical, &local);
            if(spell) {
                if(app->data.character.spell_always_prepared[local])
                    dndspellbook_collection_set_transient_status(app, "Always prepared");
                else if(!app->data.character.spell_known[local])
                    dndspellbook_collection_set_status(app, "Spell not known");
                else {
                    spell->prepared = !spell->prepared;
                    bool saved = dndspellbook_collection_save_page(app);
                    if(saved) {
                        app->action_ack_active = 1U;
                        app->action_ack_selection = app->selection;
                        dndspellbook_collection_set_transient_status(
                            app, spell->prepared ? "Spell prepared" : "Spell unprepared");
                    }
                }
            }
        } else if(event->key == InputKeyOk && event->type == InputTypeShort && app->selection) {
            app->record_index = (uint8_t)(app->selection - 1U);
            if(dndspellbook_collection_prepare_record(app, app->record_index)) {
                app->detail_selection = app->detail_scroll = 0U;
                app->screen = DndSpellbookCollectionScreenDetail;
            }
        }
    } else if(app->screen == DndSpellbookCollectionScreenDetail) {
        uint8_t count = dndspellbook_collection_detail_count();
        if(move && event->key == InputKeyUp)
            app->detail_selection = app->detail_selection ? app->detail_selection - 1U : count - 1U;
        else if(move && event->key == InputKeyDown)
            app->detail_selection = app->detail_selection + 1U < count ? app->detail_selection + 1U : 0U;
        else if(move && (event->key == InputKeyLeft || event->key == InputKeyRight))
            dndspellbook_collection_adjust(app, app->detail_selection, event->key == InputKeyRight ? 1 : -1);
        else if(event->key == InputKeyOk && event->type == InputTypeLong)
            dndspellbook_collection_detail_hold_ok(app);
        else if(event->key == InputKeyOk && event->type == InputTypeShort)
            dndspellbook_collection_detail_ok(app);
        if(app->detail_selection < app->detail_scroll) app->detail_scroll = app->detail_selection;
        if(app->detail_selection >= app->detail_scroll + DNDSPELLBOOK_COLLECTION_ROWS)
            app->detail_scroll = app->detail_selection - (DNDSPELLBOOK_COLLECTION_ROWS - 1U);
    } else if(app->screen == DndSpellbookCollectionScreenCatalog) {
        if(move && event->key == InputKeyUp && app->catalog_count)
            app->selection = app->selection ? app->selection - 1U : app->catalog_count - 1U;
        else if(move && event->key == InputKeyDown && app->catalog_count)
            app->selection = app->selection + 1U < app->catalog_count ? app->selection + 1U : 0U;
        else if(move && event->key == InputKeyLeft && app->catalog_page_start) {
            app->catalog_page_start = app->catalog_page_start >= DNDSPELLBOOK_COLLECTION_CATALOG_PAGE ?
                                          app->catalog_page_start - DNDSPELLBOOK_COLLECTION_CATALOG_PAGE :
                                          0U;
            app->selection = 0U;
            (void)dndspellbook_collection_load_catalog(app);
        } else if(move && event->key == InputKeyRight && app->catalog_has_more) {
            app->catalog_page_start += DNDSPELLBOOK_COLLECTION_CATALOG_PAGE;
            app->selection = 0U;
            (void)dndspellbook_collection_load_catalog(app);
        } else if(event->key == InputKeyOk && event->type == InputTypeLong) {
            app->filter_return_screen = DndSpellbookCollectionScreenCatalog;
            app->screen = DndSpellbookCollectionScreenFilters;
            app->filter_selection = 0U;
        } else if(event->key == InputKeyOk && event->type == InputTypeShort && app->catalog_count) {
            (void)dndspellbook_collection_apply_catalog(app);
        }
    } else if(app->screen == DndSpellbookCollectionScreenFilters) {
        if(move && event->key == InputKeyUp)
            app->filter_selection = app->filter_selection ? app->filter_selection - 1U : 6U;
        else if(move && event->key == InputKeyDown)
            app->filter_selection = app->filter_selection < 6U ? app->filter_selection + 1U : 0U;
        else if(move && (event->key == InputKeyLeft || event->key == InputKeyRight))
            dndspellbook_collection_filter_adjust(app, event->key == InputKeyRight ? 1 : -1);
        else if(event->key == InputKeyOk && event->type == InputTypeShort) {
            app->screen = app->filter_return_screen;
            if(app->screen == DndSpellbookCollectionScreenCatalog) {
                app->catalog_page_start = 0U;
                app->selection = 0U;
                (void)dndspellbook_collection_load_catalog(app);
            }
        }
    }
    dndspellbook_collection_redraw(app);
    return true;
}

static bool dndspellbook_collection_navigation(void* context) {
    DndSpellbookCollectionApp* app = context;
    if(!app) return false;
    app->input_active = 0U;
    app->edit = DndSpellbookCollectionEditNone;
    view_dispatcher_switch_to_view(app->dispatcher, DNDSPELLBOOK_COLLECTION_VIEW_MAIN);
    dndspellbook_collection_redraw(app);
    return true;
}

static bool dndspellbook_collection_load_profile(
    DndSpellbookCollectionApp* app,
    const char* args) {
    UNUSED(args);
    if(!app || !app->storage) return false;

    /* DNDSpellbook already links dnd_storage.c for character/collection I/O, so
       use that module's exact Active=<id> reader as the single storage path here.
       This never scans for or substitutes another character. Assign the ID before
       loading the character so a character-load failure still reports the exact
       persisted ID in the header. */
    uint32_t requested = 0U;
    if(!dnd_storage_load_active_profile_id(app->storage, &requested)) return false;
    app->profile = requested;

    bool recovered = false;
    if(!dnd_storage_load_profile(app->storage, requested, &app->data, &recovered)) return false;
    return true;
}

static DndSpellbookCollectionApp* dndspellbook_collection_alloc(const char* args) {
    DndSpellbookCollectionApp* app = calloc(1U, sizeof(DndSpellbookCollectionApp));
    if(!app) return NULL;
    app->filter_level = -1;
    app->filter_class = UINT8_MAX;
    app->filter_return_screen = DndSpellbookCollectionScreenList;
    app->gui = furi_record_open(RECORD_GUI);
    app->storage = furi_record_open(RECORD_STORAGE);
    if(!app->gui || !app->storage) goto fail;

    /* Reserve the complete fixed UI/runtime footprint before character and spell
       parsing can make variable heap allocations. This mirrors Adventure's startup
       ordering so a successful collection read cannot consume memory required for
       the main view/model. */
    app->dispatcher = view_dispatcher_alloc();
    app->view = view_alloc();
    if(!app->dispatcher || !app->view) goto fail;
    view_dispatcher_set_event_callback_context(app->dispatcher, app);
    view_dispatcher_set_navigation_event_callback(app->dispatcher, dndspellbook_collection_navigation);
    view_allocate_model(app->view, ViewModelTypeLockFree, sizeof(DndSpellbookCollectionApp*));
    DndSpellbookCollectionApp** model = view_get_model(app->view);
    if(!model) goto fail;
    *model = app;
    view_commit_model(app->view, false);
    view_set_context(app->view, app);
    view_set_draw_callback(app->view, dndspellbook_collection_draw);
    view_set_input_callback(app->view, dndspellbook_collection_input);

    app->have_profile = dndspellbook_collection_load_profile(app, args) ? 1U : 0U;
    if(app->have_profile) {
        /* Opening Spellbook is read-only with respect to its sidecar. A missing
           sidecar is an empty spell list; the first actual Add New/save creates
           the file. Always enter directly on the list with + Add New selected. */
        if(!dndspellbook_collection_load_page(app, 0U))
            dndspellbook_collection_set_status(app, "Collection read failed");
        app->selection = 0U;
        app->scroll = 0U;
        app->screen = DndSpellbookCollectionScreenList;
    } else {
        app->screen = DndSpellbookCollectionScreenNoCharacter;
    }
    view_dispatcher_add_view(app->dispatcher, DNDSPELLBOOK_COLLECTION_VIEW_MAIN, app->view);
    view_dispatcher_attach_to_gui(app->dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    return app;

fail:
    if(app->text_input) text_input_free(app->text_input);
    if(app->number_input) number_input_free(app->number_input);
    if(app->view) view_free(app->view);
    if(app->dispatcher) view_dispatcher_free(app->dispatcher);
    dnd_data_clear(&app->data);
    if(app->storage) furi_record_close(RECORD_STORAGE);
    if(app->gui) furi_record_close(RECORD_GUI);
    free(app);
    return NULL;
}

static void dndspellbook_collection_free(DndSpellbookCollectionApp* app) {
    if(!app) return;
    if(app->dispatcher && app->text_input)
        view_dispatcher_remove_view(app->dispatcher, DNDSPELLBOOK_COLLECTION_VIEW_TEXT);
    if(app->dispatcher && app->number_input)
        view_dispatcher_remove_view(app->dispatcher, DNDSPELLBOOK_COLLECTION_VIEW_NUMBER);
    if(app->dispatcher && app->view)
        view_dispatcher_remove_view(app->dispatcher, DNDSPELLBOOK_COLLECTION_VIEW_MAIN);
    if(app->text_input) text_input_free(app->text_input);
    if(app->number_input) number_input_free(app->number_input);
    if(app->view) view_free(app->view);
    if(app->dispatcher) view_dispatcher_free(app->dispatcher);
    dnd_data_clear(&app->data);
    if(app->storage) furi_record_close(RECORD_STORAGE);
    if(app->gui) furi_record_close(RECORD_GUI);
    free(app);
}

int32_t dndspellbook_collection_run(void* context) {
    DndSpellbookCollectionApp* app = dndspellbook_collection_alloc(context);
    if(!app) return -1;
    view_dispatcher_switch_to_view(app->dispatcher, DNDSPELLBOOK_COLLECTION_VIEW_MAIN);
    view_dispatcher_run(app->dispatcher);
    bool return_to_dnd = app->return_to_dnd;
    dndspellbook_collection_free(app);
    if(return_to_dnd)
        (void)dnd_handoff_launch_if_present(DNDOLPHINS_FAP_PATH, NULL);
    return 0;
}
