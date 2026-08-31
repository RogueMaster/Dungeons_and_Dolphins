#include "dndinventory_internal.h"

#include <furi.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define POCKET_D20_DEFAULT_CLASS_EQUIPMENT APP_ASSETS_PATH("equipment/default_class.txt")
#define POCKET_D20_DEFAULT_RACE_EQUIPMENT APP_ASSETS_PATH("equipment/default_race.txt")
#define POCKET_D20_DEFAULT_BACKGROUND_EQUIPMENT APP_ASSETS_PATH("equipment/default_background.txt")
#define POCKET_D20_DEFAULT_TRINKETS APP_ASSETS_PATH("equipment/trinkets.txt")

static int32_t dndinventory_items_add_saturated(int32_t value, int32_t add) {
    int64_t total = (int64_t)value + add;
    if(total > INT32_MAX) return INT32_MAX;
    if(total < INT32_MIN) return INT32_MIN;
    return (int32_t)total;
}

bool dndinventory_items_initialize_inventory(
    Storage* storage,
    uint32_t profile,
    PocketSaveData* data,
    bool* initialized) {
    if(initialized) *initialized = false;
    if(!storage || !data) return false;
    PocketCharacter* character = &data->character;
    if(dnd_storage_items_exist(storage, profile)) {
        bool granted = false;
        if(!dnd_storage_inventory_initial_granted(storage, profile, &granted))
            return false;
        if(granted) return true;
        uint8_t existing_items = 0U;
        if(!dnd_storage_visit_items(storage, profile, NULL, NULL, &existing_items))
            return false;
        if(existing_items) return true;

        /* Currency-only inventory files are valid now that Inventory owns
           money. Preserve that balance, then recreate the empty sidecar with
           the granted equipment below. This lets a player edit currency before
           granting starting gear without permanently disabling the grant. */
        int32_t existing_currency[5];
        bool currency_found = false;
        if(!dnd_storage_load_inventory_currency(
               storage, profile, existing_currency, &currency_found))
            return false;
        if(currency_found) {
            character->currency_cp = existing_currency[0];
            character->currency_sp = existing_currency[1];
            character->currency_ep = existing_currency[2];
            character->currency_gp = existing_currency[3];
            character->currency_pp = existing_currency[4];
        }
        if(!dnd_storage_remove_live_items(storage, profile)) return false;
    }

    PocketD20ItemSeedAsset assets[3] = {
        {.path = POCKET_D20_DEFAULT_CLASS_EQUIPMENT,
         .match = character->class_count ? character->classes[0].name : ""},
        {.path = POCKET_D20_DEFAULT_RACE_EQUIPMENT, .match = character->species},
        {.path = POCKET_D20_DEFAULT_BACKGROUND_EQUIPMENT, .match = character->background},
    };
    int32_t currency[5] = {0, 0, 0, 0, 0};
    bool created = false;
    bool composed = dnd_storage_create_items_from_assets(
        storage, profile, character, assets, 3U, currency, &created);

    uint8_t seeded_items = 0U;
    bool has_currency = false;
    for(uint8_t i = 0U; i < 5U; ++i)
        if(currency[i] != 0) has_currency = true;
    bool have_seeded_equipment =
        composed && created &&
        dnd_storage_visit_items(storage, profile, NULL, NULL, &seeded_items) &&
        (seeded_items > 0U || has_currency);
    if(!have_seeded_equipment) {
        dnd_storage_remove_live_items(storage, profile);
        memset(currency, 0, sizeof(currency));
        char trinket_key[4];
        uint8_t trinket_roll = dnd_rules_core_roll_die(100U);
        snprintf(trinket_key, sizeof(trinket_key), "%u", trinket_roll);
        PocketD20ItemSeedAsset fallback = {
            .path = POCKET_D20_DEFAULT_TRINKETS,
            .match = trinket_key,
        };
        created = false;
        if(!dnd_storage_create_items_from_assets(
               storage, profile, character, &fallback, 1U, currency, &created))
            return false;
    }
    if(!created) return true;

    int32_t combined_currency[5] = {
        dndinventory_items_add_saturated(character->currency_cp, currency[0]),
        dndinventory_items_add_saturated(character->currency_sp, currency[1]),
        dndinventory_items_add_saturated(character->currency_ep, currency[2]),
        dndinventory_items_add_saturated(character->currency_gp, currency[3]),
        dndinventory_items_add_saturated(character->currency_pp, currency[4]),
    };
    if(!dnd_storage_save_inventory_currency(
           storage, profile, character, combined_currency)) {
        dnd_storage_remove_live_items(storage, profile);
        return false;
    }
    character->currency_cp = combined_currency[0];
    character->currency_sp = combined_currency[1];
    character->currency_ep = combined_currency[2];
    character->currency_gp = combined_currency[3];
    character->currency_pp = combined_currency[4];
    if(initialized) *initialized = true;
    return true;
}

bool dndinventory_items_regrant_inventory_once(
    Storage* storage,
    uint32_t profile,
    PocketSaveData* data,
    bool* regranted) {
    if(regranted) *regranted = false;
    if(!storage || !data || !regranted) return false;

    uint8_t grant_state = 0U;
    if(!dnd_storage_inventory_initial_grant_state(storage, profile, &grant_state)) return false;
    if(grant_state != 1U) return true;

    PocketCharacter* character = &data->character;
    PocketD20ItemSeedAsset assets[3] = {
        {.path = POCKET_D20_DEFAULT_CLASS_EQUIPMENT,
         .match = character->class_count ? character->classes[0].name : ""},
        {.path = POCKET_D20_DEFAULT_RACE_EQUIPMENT, .match = character->species},
        {.path = POCKET_D20_DEFAULT_BACKGROUND_EQUIPMENT, .match = character->background},
    };
    char trinket_key[4];
    uint8_t trinket_roll = dnd_rules_core_roll_die(100U);
    snprintf(trinket_key, sizeof(trinket_key), "%u", trinket_roll);
    PocketD20ItemSeedAsset fallback = {
        .path = POCKET_D20_DEFAULT_TRINKETS,
        .match = trinket_key,
    };
    int32_t added_currency[5] = {0, 0, 0, 0, 0};
    bool applied = false;
    if(!dnd_storage_regrant_items_from_assets(
           storage,
           profile,
           character,
           assets,
           3U,
           &fallback,
           added_currency,
           &applied))
        return false;
    if(!applied) return true;

    character->currency_cp = dndinventory_items_add_saturated(
        character->currency_cp, added_currency[0]);
    character->currency_sp = dndinventory_items_add_saturated(
        character->currency_sp, added_currency[1]);
    character->currency_ep = dndinventory_items_add_saturated(
        character->currency_ep, added_currency[2]);
    character->currency_gp = dndinventory_items_add_saturated(
        character->currency_gp, added_currency[3]);
    character->currency_pp = dndinventory_items_add_saturated(
        character->currency_pp, added_currency[4]);
    *regranted = true;
    return true;
}

