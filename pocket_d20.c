#include "pocket_d20.h"
#include "pocket_d20_rules.h"
#include "pocket_d20_storage.h"

#include <furi.h>
#include <gui/gui.h>
#include <gui/modules/text_input.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAG "PocketD20"
#define POCKET_D20_MAX_GENERIC_ROLLS 20U
#define POCKET_D20_DICE_ANIMATION_FRAMES 8U
#define POCKET_D20_DICE_ANIMATION_EVENT 0xD120U
#define POCKET_D20_MAX_CATALOG_ENTRIES 384U

typedef enum {
    PocketViewMain,
    PocketViewTextInput,
} PocketViewId;

typedef enum {
    PocketScreenHome,
    PocketScreenProfiles,
    PocketScreenCharacter,
    PocketScreenVitals,
    PocketScreenAbilities,
    PocketScreenSkills,
    PocketScreenMagic,
    PocketScreenCurrency,
    PocketScreenRecordList,
    PocketScreenRecordDetail,
    PocketScreenCatalog,
    PocketScreenCombat,
    PocketScreenDice,
    PocketScreenDiceResult,
    PocketScreenAttackList,
    PocketScreenAttackResult,
    PocketScreenInitiativeMenu,
    PocketScreenInitiativeSetup,
    PocketScreenInitiativeCombat,
    PocketScreenAbout,
} PocketScreen;

typedef enum {
    PocketListClasses,
    PocketListSpells,
    PocketListFeatures,
    PocketListItems,
    PocketListLanguages,
    PocketListJournal,
    PocketListParty,
} PocketListKind;

typedef enum {
    PocketCatalogClasses,
    PocketCatalogSubclasses,
    PocketCatalogBackgrounds,
    PocketCatalogSpells,
    PocketCatalogFeats,
    PocketCatalogItems,
    PocketCatalogCount,
} PocketCatalogKind;

enum {
    PocketClassMaskArtificer = 1U << 0,
    PocketClassMaskBard = 1U << 1,
    PocketClassMaskCleric = 1U << 2,
    PocketClassMaskDruid = 1U << 3,
    PocketClassMaskPaladin = 1U << 4,
    PocketClassMaskRanger = 1U << 5,
    PocketClassMaskSorcerer = 1U << 6,
    PocketClassMaskWarlock = 1U << 7,
    PocketClassMaskWizard = 1U << 8,
};

typedef struct {
    const char* name;
    uint8_t level;
    uint16_t class_mask;
} PocketBuiltinSpell;

typedef enum {
    PocketEditNone,
    PocketEditCharacterName,
    PocketEditPlayerName,
    PocketEditSpecies,
    PocketEditBackground,
    PocketEditAlignment,
    PocketEditOtherProficiencies,
    PocketEditClassName,
    PocketEditSubclass,
    PocketEditSpellName,
    PocketEditSpellDetail,
    PocketEditFeatureName,
    PocketEditFeatureDetail,
    PocketEditItemName,
    PocketEditItemDetail,
    PocketEditLanguageName,
    PocketEditJournalTitle,
    PocketEditJournalBody,
    PocketEditPartyName,
    PocketEditTemporaryInitiativeName,
} PocketEditTarget;

typedef struct {
    Gui* gui;
    Storage* storage;
    ViewDispatcher* dispatcher;
    View* main_view;
    TextInput* text_input;
    FuriTimer* dice_timer;

    PocketSaveData data;
    PocketProfileState profiles;
    uint32_t saved_fingerprint;
    PocketScreen screen;
    PocketScreen return_screen;
    PocketListKind list_kind;
    uint16_t selection;
    uint16_t scroll;
    uint8_t record_index;
    PocketCatalogKind catalog_kind;
    PocketEditTarget catalog_target;
    uint16_t catalog_count;
    uint16_t catalog_return_selection;
    char catalog_entries[POCKET_D20_MAX_CATALOG_ENTRIES][POCKET_D20_NAME_LEN];
    uint8_t catalog_levels[POCKET_D20_MAX_CATALOG_ENTRIES];
    uint16_t catalog_class_masks[POCKET_D20_MAX_CATALOG_ENTRIES];
    uint8_t catalog_has_metadata[POCKET_D20_MAX_CATALOG_ENTRIES];
    uint8_t catalog_show_all;
    uint8_t edit_slot_max;
    uint8_t edit_modifier_mode;
    uint8_t arcane_recovery_active;
    uint8_t arcane_recovery_budget;
    uint8_t arcane_recovery_spent;
    uint8_t arcane_recovery_restored[6];

    PocketEditTarget edit_target;
    char edit_buffer[POCKET_D20_DETAIL_LEN];

    PocketRollMode roll_mode;
    uint8_t dice_count;
    uint8_t dice_sides;
    int16_t dice_modifier;
    int16_t dice_result;
    uint8_t dice_first;
    uint8_t dice_second;
    uint8_t dice_roll_values[POCKET_D20_MAX_GENERIC_ROLLS];
    uint8_t dice_roll_value_count;
    uint16_t dice_roll_sum;
    uint8_t damage_roll_page;
    uint8_t dice_animating;
    uint8_t dice_anim_frame;
    uint8_t dice_anim_sides;
    uint8_t dice_anim_count;

    uint8_t attack_item_index;
    uint8_t attack_phase;
    PocketAttackRoll attack_roll;
    PocketDamageRoll damage_roll;

    char status[32];
} PocketD20App;

static void pocket_text_done(void* context);

static const char* const pocket_home_items[] = {
    "Characters",
    "Character",
    "Vitals",
    "Abilities & Saves",
    "Skills",
    "Magic & Spells",
    "Features & Perks",
    "Inventory",
    "Currency",
    "Journal",
    "Combat",
    "Initiative",
    "Dice Roller",
    "Save Now",
    "About",
};

static const uint8_t pocket_die_choices[] = {4U, 6U, 8U, 10U, 12U, 20U, 100U};
static const uint8_t pocket_damage_die_choices[] = {4U, 6U, 8U, 10U, 12U};
static const char* const pocket_roll_mode_names[] = {"Normal", "Advantage", "Disadvantage"};
static const char* const pocket_attack_ability_names[] = {"Auto", "Strength", "Dexterity", "Best"};
static const char* const pocket_recharge_names[] = {"Manual", "Short/Long", "Long"};

/* Group the standard skills by governing ability without changing their save indexes. */
static const uint8_t pocket_skill_display_order[POCKET_D20_SKILL_COUNT] = {
    3U,                    /* STR: Athletics */
    0U, 15U, 16U,         /* DEX: Acrobatics, Sleight of Hand, Stealth */
    2U, 5U, 8U, 10U, 14U, /* INT: Arcana, History, Investigation, Nature, Religion */
    1U, 6U, 9U, 11U, 17U, /* WIS: Animal Handling, Insight, Medicine, Perception, Survival */
    4U, 7U, 12U, 13U,     /* CHA: Deception, Intimidation, Performance, Persuasion */
};

static const char* const pocket_catalog_classes[] = {
    "Artificer", "Barbarian", "Bard", "Cleric", "Druid", "Fighter", "Monk",
    "Paladin", "Ranger", "Rogue", "Sorcerer", "Warlock", "Wizard"};

static const char* const pocket_catalog_subclasses[] = {
    "Path of the Berserker",
    "College of Lore",
    "Life Domain",
    "Circle of the Land",
    "Champion",
    "Warrior of the Open Hand",
    "Oath of Devotion",
    "Hunter",
    "Thief",
    "Draconic Sorcery",
    "Fiend Patron",
    "Evoker",
    "College of the Moon",
    "Knowledge Domain",
    "Banneret",
    "Oath of the Noble Genies",
    "Winter Walker",
    "Scion of the Three",
    "Spellfire Sorcery",
    "Bladesinger",
    "College of Spirits",
    "The Undead",
    "Alchemist",
    "Armorer",
    "Artillerist",
    "Battle Smith",
    "Cartographer",
};

static const char* const pocket_catalog_backgrounds[] = {
    "Acolyte",
    "Artisan",
    "Charlatan",
    "Criminal",
    "Entertainer",
    "Farmer",
    "Guard",
    "Guide",
    "Hermit",
    "Merchant",
    "Noble",
    "Sage",
    "Sailor",
    "Scribe",
    "Soldier",
    "Wayfarer",
    "Haunted One",
    "Investigator",
    "Chondathan Freebooter",
    "Dead Magic Dweller",
    "Dragon Cultist",
    "Emerald Enclave Caretaker",
    "Flaming Fist Mercenary",
    "Genie Touched",
    "Harper",
    "Ice Fisher",
    "Knight of the Gauntlet",
    "Lords' Alliance Vassal",
    "Moonwell Pilgrim",
    "Mulhorandi Tomb Raider",
    "Mythalkeeper",
    "Purple Dragon Squire",
    "Rashemi Wanderer",
    "Shadowmasters Exile",
    "Spellfire Initiate",
    "Zhentarim Mercenary",
};

static const char* const pocket_catalog_feats[] = {
    "Alert",
    "Magic Initiate",
    "Savage Attacker",
    "Skilled",
    "Ability Score Improvement",
    "Grappler",
    "Archery",
    "Defense",
    "Great Weapon Fighting",
    "Two-Weapon Fighting",
    "Boon of Combat Prowess",
    "Boon of Dimensional Travel",
    "Boon of Fate",
    "Boon of Irresistible Offense",
    "Boon of the Night Spirit",
    "Boon of Spell Recall",
    "Boon of Truesight",
};

static const PocketBuiltinSpell pocket_catalog_spells[] = {
    {"Acid Splash", 0U, PocketClassMaskSorcerer | PocketClassMaskWizard},
    {"Aid", 2U, PocketClassMaskCleric | PocketClassMaskPaladin | PocketClassMaskRanger},
    {"Alarm", 1U, PocketClassMaskRanger | PocketClassMaskWizard},
    {"Animal Friendship", 1U, PocketClassMaskBard | PocketClassMaskDruid | PocketClassMaskRanger},
    {"Aura of Life", 4U, PocketClassMaskCleric | PocketClassMaskPaladin},
    {"Bless", 1U, PocketClassMaskCleric | PocketClassMaskPaladin},
    {"Burning Hands", 1U, PocketClassMaskSorcerer | PocketClassMaskWizard},
    {"Charm Monster", 4U, PocketClassMaskBard | PocketClassMaskDruid | PocketClassMaskSorcerer | PocketClassMaskWarlock | PocketClassMaskWizard},
    {"Charm Person", 1U, PocketClassMaskBard | PocketClassMaskDruid | PocketClassMaskSorcerer | PocketClassMaskWarlock | PocketClassMaskWizard},
    {"Chromatic Orb", 1U, PocketClassMaskSorcerer | PocketClassMaskWizard},
    {"Cure Wounds", 1U, PocketClassMaskBard | PocketClassMaskCleric | PocketClassMaskDruid | PocketClassMaskPaladin | PocketClassMaskRanger},
    {"Detect Magic", 1U, PocketClassMaskBard | PocketClassMaskCleric | PocketClassMaskDruid | PocketClassMaskPaladin | PocketClassMaskRanger | PocketClassMaskSorcerer | PocketClassMaskWizard},
    {"Dispel Magic", 3U, PocketClassMaskBard | PocketClassMaskCleric | PocketClassMaskDruid | PocketClassMaskPaladin | PocketClassMaskSorcerer | PocketClassMaskWarlock | PocketClassMaskWizard},
    {"Dissonant Whispers", 1U, PocketClassMaskBard},
    {"Divine Smite", 1U, PocketClassMaskPaladin},
    {"Dragon's Breath", 2U, PocketClassMaskSorcerer | PocketClassMaskWizard},
    {"Elementalism", 0U, PocketClassMaskDruid | PocketClassMaskSorcerer | PocketClassMaskWizard},
    {"Ensnaring Strike", 1U, PocketClassMaskRanger},
    {"Fire Bolt", 0U, PocketClassMaskSorcerer | PocketClassMaskWizard},
    {"Fireball", 3U, PocketClassMaskSorcerer | PocketClassMaskWizard},
    {"Guidance", 0U, PocketClassMaskCleric | PocketClassMaskDruid},
    {"Healing Word", 1U, PocketClassMaskBard | PocketClassMaskCleric | PocketClassMaskDruid},
    {"Hex", 1U, PocketClassMaskWarlock},
    {"Hold Person", 2U, PocketClassMaskBard | PocketClassMaskCleric | PocketClassMaskDruid | PocketClassMaskSorcerer | PocketClassMaskWarlock | PocketClassMaskWizard},
    {"Ice Knife", 1U, PocketClassMaskDruid | PocketClassMaskSorcerer | PocketClassMaskWizard},
    {"Identify", 1U, PocketClassMaskBard | PocketClassMaskWizard},
    {"Invisibility", 2U, PocketClassMaskBard | PocketClassMaskSorcerer | PocketClassMaskWarlock | PocketClassMaskWizard},
    {"Light", 0U, PocketClassMaskBard | PocketClassMaskCleric | PocketClassMaskSorcerer | PocketClassMaskWizard},
    {"Mage Armor", 1U, PocketClassMaskSorcerer | PocketClassMaskWizard},
    {"Mage Hand", 0U, PocketClassMaskBard | PocketClassMaskSorcerer | PocketClassMaskWarlock | PocketClassMaskWizard},
    {"Magic Missile", 1U, PocketClassMaskSorcerer | PocketClassMaskWizard},
    {"Mind Spike", 2U, PocketClassMaskSorcerer | PocketClassMaskWarlock | PocketClassMaskWizard},
    {"Phantasmal Force", 2U, PocketClassMaskBard | PocketClassMaskSorcerer | PocketClassMaskWizard},
    {"Power Word Heal", 9U, PocketClassMaskBard | PocketClassMaskCleric},
    {"Ray of Sickness", 1U, PocketClassMaskSorcerer | PocketClassMaskWizard},
    {"Searing Smite", 1U, PocketClassMaskPaladin},
    {"Shield", 1U, PocketClassMaskSorcerer | PocketClassMaskWizard},
    {"Shocking Grasp", 0U, PocketClassMaskSorcerer | PocketClassMaskWizard},
    {"Sorcerous Burst", 0U, PocketClassMaskSorcerer},
    {"Starry Wisp", 0U, PocketClassMaskBard | PocketClassMaskDruid},
    {"Summon Dragon", 5U, PocketClassMaskDruid | PocketClassMaskSorcerer | PocketClassMaskWizard},
    {"Thaumaturgy", 0U, PocketClassMaskCleric},
    {"Thunderwave", 1U, PocketClassMaskBard | PocketClassMaskDruid | PocketClassMaskSorcerer | PocketClassMaskWizard},
    {"Tsunami", 8U, PocketClassMaskDruid},
    {"Vitriolic Sphere", 4U, PocketClassMaskSorcerer | PocketClassMaskWizard},
};

static const char* const pocket_catalog_items[] = {
    "Battleaxe", "Club", "Dagger", "Dart", "Greatclub", "Greataxe", "Greatsword",
    "Handaxe", "Javelin", "Light Crossbow", "Longbow", "Longsword", "Mace",
    "Musket", "Pistol", "Quarterstaff", "Rapier", "Shortbow", "Shortsword", "Spear",
    "Bead of Nourishment", "Cloak of Invisibility", "Elixir of Health", "Energy Bow",
    "Gloves of Thievery", "Hat of Many Spells", "Potion of Healing",
    "Potion of Invulnerability", "Potion of Longevity", "Potion of Vitality",
    "Quarterstaff of the Acrobat", "Rod of Resurrection", "Sending Stones",
    "Sentinel Shield", "Shield of the Cavalier", "Thunderous Greatclub",
};

static const char* const pocket_catalog_paths[PocketCatalogCount] = {
    APP_DATA_PATH("catalogs/classes.txt"),
    APP_DATA_PATH("catalogs/subclasses.txt"),
    APP_DATA_PATH("catalogs/backgrounds.txt"),
    APP_DATA_PATH("catalogs/spells.txt"),
    APP_DATA_PATH("catalogs/feats.txt"),
    APP_DATA_PATH("catalogs/items.txt"),
};

static const char* const pocket_catalog_abilities_path =
    APP_DATA_PATH("catalogs/abilities.txt");

static void pocket_copy(char* destination, size_t size, const char* source) {
    if(size == 0U) return;
    strncpy(destination, source, size - 1U);
    destination[size - 1U] = '\0';
}

static int16_t pocket_clamp_i16(int16_t value, int16_t minimum, int16_t maximum) {
    if(value < minimum) return minimum;
    if(value > maximum) return maximum;
    return value;
}

static uint8_t pocket_clamp_u8(int16_t value, uint8_t maximum) {
    if(value < 0) return 0U;
    if(value > maximum) return maximum;
    return (uint8_t)value;
}

static void pocket_clear_status(PocketD20App* app) {
    app->status[0] = '\0';
}

static void pocket_set_status(PocketD20App* app, const char* status) {
    pocket_copy(app->status, sizeof(app->status), status);
}

static void pocket_refresh(PocketD20App* app) {
    (void)view_get_model(app->main_view);
    view_commit_model(app->main_view, true);
}

static void pocket_dice_timer_callback(void* context) {
    PocketD20App* app = context;
    view_dispatcher_send_custom_event(app->dispatcher, POCKET_D20_DICE_ANIMATION_EVENT);
}

static void pocket_start_dice_animation(PocketD20App* app, uint8_t count, uint8_t sides) {
    app->dice_animating = 1U;
    app->dice_anim_frame = 0U;
    app->dice_anim_count = count ? count : 1U;
    app->dice_anim_sides = sides >= 2U ? sides : 20U;
    furi_timer_stop(app->dice_timer);
    furi_timer_start(app->dice_timer, furi_ms_to_ticks(100U));
}

static bool pocket_custom_event_callback(void* context, uint32_t event) {
    PocketD20App* app = context;
    if(event != POCKET_D20_DICE_ANIMATION_EVENT) return false;
    if(app->dice_animating) {
        ++app->dice_anim_frame;
        if(app->dice_anim_frame >= POCKET_D20_DICE_ANIMATION_FRAMES) {
            app->dice_animating = 0U;
            furi_timer_stop(app->dice_timer);
        }
        pocket_refresh(app);
    }
    return true;
}

static uint32_t pocket_data_fingerprint(const PocketSaveData* data) {
    const uint8_t* bytes = (const uint8_t*)data;
    uint32_t hash = 2166136261UL;
    for(size_t i = 0U; i < sizeof(*data); ++i) {
        hash ^= bytes[i];
        hash *= 16777619UL;
    }
    return hash;
}

static bool pocket_save(PocketD20App* app, bool report) {
    uint8_t active = app->profiles.active_profile;
    if(active >= POCKET_D20_MAX_PROFILES) active = 0U;
    bool result = pocket_d20_storage_save_profile(app->storage, active, &app->data);
    if(strcmp(app->profiles.names[active], app->data.character.name) != 0) {
        pocket_copy(
            app->profiles.names[active],
            sizeof(app->profiles.names[active]),
            app->data.character.name);
        result = pocket_d20_profiles_save(app->storage, &app->profiles) && result;
    }
    if(result) app->saved_fingerprint = pocket_data_fingerprint(&app->data);
    if(report) pocket_set_status(app, result ? "Saved" : "Save failed");
    return result;
}

static uint8_t pocket_profile_count(const PocketD20App* app) {
    uint8_t count = 0U;
    for(uint8_t slot = 0U; slot < POCKET_D20_MAX_PROFILES; ++slot) {
        if(app->profiles.occupied_mask & (1U << slot)) ++count;
    }
    return count;
}

static uint8_t pocket_profile_slot_at(const PocketD20App* app, uint8_t list_index) {
    uint8_t found = 0U;
    for(uint8_t slot = 0U; slot < POCKET_D20_MAX_PROFILES; ++slot) {
        if(!(app->profiles.occupied_mask & (1U << slot))) continue;
        if(found == list_index) return slot;
        ++found;
    }
    return POCKET_D20_MAX_PROFILES;
}

static uint8_t pocket_find_empty_profile(const PocketD20App* app) {
    for(uint8_t slot = 1U; slot < POCKET_D20_MAX_PROFILES; ++slot) {
        if(!(app->profiles.occupied_mask & (1U << slot))) return slot;
    }
    return POCKET_D20_MAX_PROFILES;
}

static void pocket_enter_screen(PocketD20App* app, PocketScreen screen) {
    app->screen = screen;
    app->selection = 0U;
    app->scroll = 0U;
    app->edit_modifier_mode = 0U;
    pocket_clear_status(app);
}

static void pocket_switch_profile(PocketD20App* app, uint8_t slot) {
    if(slot >= POCKET_D20_MAX_PROFILES ||
       !(app->profiles.occupied_mask & (1U << slot)))
        return;
    if(slot == app->profiles.active_profile) {
        pocket_set_status(app, "Already active");
        return;
    }
    if(!pocket_save(app, false)) {
        pocket_set_status(app, "Save failed");
        return;
    }

    app->profiles.active_profile = slot;
    app->arcane_recovery_active = 0U;
    bool recovered_backup = false;
    bool loaded = pocket_d20_storage_load_profile(
        app->storage, slot, &app->data, &recovered_backup);
    bool character_ready = loaded;
    if(!loaded)
        character_ready = pocket_d20_storage_save_profile(app->storage, slot, &app->data);
    pocket_copy(
        app->profiles.names[slot],
        sizeof(app->profiles.names[slot]),
        app->data.character.name);
    bool metadata_saved = pocket_d20_profiles_save(app->storage, &app->profiles);
    if(character_ready && metadata_saved)
        app->saved_fingerprint = pocket_data_fingerprint(&app->data);
    pocket_enter_screen(app, PocketScreenHome);
    if(!character_ready || !metadata_saved)
        pocket_set_status(app, "Profile save failed");
    else if(recovered_backup)
        pocket_set_status(app, "Backup recovered");
    else if(loaded)
        pocket_set_status(app, "Character switched");
    else
        pocket_set_status(app, "Fresh character");
}

static void pocket_create_profile(PocketD20App* app) {
    uint8_t slot = pocket_find_empty_profile(app);
    if(slot >= POCKET_D20_MAX_PROFILES) {
        pocket_set_status(app, "Six character limit");
        return;
    }
    if(!pocket_save(app, false)) {
        pocket_set_status(app, "Save failed");
        return;
    }

    app->profiles.active_profile = slot;
    app->profiles.occupied_mask |= (uint8_t)(1U << slot);
    app->arcane_recovery_active = 0U;
    pocket_d20_data_set_defaults(&app->data);
    snprintf(
        app->data.character.name,
        sizeof(app->data.character.name),
        "New Hero %u",
        (unsigned int)(slot + 1U));
    pocket_copy(
        app->profiles.names[slot],
        sizeof(app->profiles.names[slot]),
        app->data.character.name);
    bool character_saved = pocket_d20_storage_save_profile(app->storage, slot, &app->data);
    bool metadata_saved = pocket_d20_profiles_save(app->storage, &app->profiles);
    if(character_saved && metadata_saved)
        app->saved_fingerprint = pocket_data_fingerprint(&app->data);
    pocket_enter_screen(app, PocketScreenCharacter);
    pocket_set_status(
        app,
        character_saved && metadata_saved ? "New character" : "Save failed");
}

static void pocket_delete_profile(PocketD20App* app, uint8_t slot) {
    if(slot == 0U) {
        pocket_set_status(app, "Main cannot delete");
        return;
    }
    if(slot == app->profiles.active_profile) {
        pocket_set_status(app, "Switch before delete");
        return;
    }
    if(slot >= POCKET_D20_MAX_PROFILES ||
       !(app->profiles.occupied_mask & (1U << slot)))
        return;

    app->profiles.occupied_mask &= (uint8_t)~(1U << slot);
    memset(app->profiles.names[slot], 0, sizeof(app->profiles.names[slot]));
    bool metadata_saved = pocket_d20_profiles_save(app->storage, &app->profiles);
    bool character_deleted = pocket_d20_storage_delete_profile(app->storage, slot);
    app->selection = 0U;
    app->scroll = 0U;
    pocket_set_status(
        app,
        metadata_saved && character_deleted ? "Character deleted" : "Delete failed");
}

static uint8_t pocket_wizard_level(const PocketCharacter* character) {
    for(uint8_t i = 0U; i < character->class_count; ++i)
        if(strcmp(character->classes[i].name, "Wizard") == 0)
            return character->classes[i].level;
    return 0U;
}

static bool pocket_begin_arcane_recovery(PocketD20App* app) {
    PocketCharacter* character = &app->data.character;
    uint8_t wizard_level = pocket_wizard_level(character);
    if(!wizard_level) {
        pocket_set_status(app, "No Wizard class found");
        return false;
    }
    if(character->arcane_recovery_used) {
        pocket_set_status(app, "Recovery already used");
        return false;
    }
    bool has_expended_slot = false;
    for(uint8_t level = 1U; level <= 5U; ++level)
        if(character->spell_slots_current[level] < character->spell_slots_max[level])
            has_expended_slot = true;
    if(!has_expended_slot) {
        pocket_set_status(app, "No eligible slots spent");
        return false;
    }
    app->arcane_recovery_active = 1U;
    app->arcane_recovery_budget = (wizard_level + 1U) / 2U;
    app->arcane_recovery_spent = 0U;
    memset(app->arcane_recovery_restored, 0, sizeof(app->arcane_recovery_restored));
    pocket_enter_screen(app, PocketScreenMagic);
    app->selection = 6U;
    app->scroll = 2U;
    pocket_set_status(app, "Choose slots, OK done");
    return true;
}

static void pocket_menu_move(PocketD20App* app, uint16_t count, int8_t delta) {
    if(count == 0U) return;
    int32_t next = (int32_t)app->selection + delta;
    if(next < 0) next = count - 1U;
    if(next >= count) next = 0;
    app->selection = (uint16_t)next;
    if(app->selection < app->scroll) app->scroll = app->selection;
    if(app->selection >= app->scroll + 5U) app->scroll = app->selection - 4U;
}

static const char* pocket_proficiency_mark(uint8_t proficiency) {
    if(proficiency == PocketProficiencyExpertise) return "E";
    if(proficiency == PocketProficiencyProficient) return "P";
    return "-";
}

static char pocket_spell_status(const PocketCharacter* character, uint8_t index) {
    if(character->spell_always_prepared[index]) return 'A';
    if(character->spells[index].prepared) return 'P';
    if(character->spell_known[index]) return 'K';
    return '-';
}

static uint8_t pocket_cycle_die(uint8_t current, int8_t delta, bool damage_only) {
    const uint8_t* choices = damage_only ? pocket_damage_die_choices : pocket_die_choices;
    uint8_t count = damage_only ? sizeof(pocket_damage_die_choices) : sizeof(pocket_die_choices);
    uint8_t index = 0U;
    for(uint8_t i = 0U; i < count; ++i) {
        if(choices[i] == current) {
            index = i;
            break;
        }
    }
    int16_t next = (int16_t)index + delta;
    if(next < 0) next = count - 1U;
    if(next >= count) next = 0;
    return choices[next];
}

static uint16_t pocket_class_mask_from_name(const char* name) {
    if(strcmp(name, "Artificer") == 0) return PocketClassMaskArtificer;
    if(strcmp(name, "Bard") == 0) return PocketClassMaskBard;
    if(strcmp(name, "Cleric") == 0) return PocketClassMaskCleric;
    if(strcmp(name, "Druid") == 0) return PocketClassMaskDruid;
    if(strcmp(name, "Paladin") == 0) return PocketClassMaskPaladin;
    if(strcmp(name, "Ranger") == 0) return PocketClassMaskRanger;
    if(strcmp(name, "Sorcerer") == 0) return PocketClassMaskSorcerer;
    if(strcmp(name, "Warlock") == 0) return PocketClassMaskWarlock;
    if(strcmp(name, "Wizard") == 0) return PocketClassMaskWizard;
    return 0U;
}

static uint8_t pocket_class_max_spell_level(const PocketClassLevel* class_level) {
    uint8_t level = class_level->level;
    uint16_t mask = pocket_class_mask_from_name(class_level->name);
    if(mask & (PocketClassMaskBard | PocketClassMaskCleric | PocketClassMaskDruid |
               PocketClassMaskSorcerer | PocketClassMaskWizard)) {
        uint8_t maximum = (level + 1U) / 2U;
        return maximum > 9U ? 9U : maximum;
    }
    if(mask & (PocketClassMaskArtificer | PocketClassMaskPaladin | PocketClassMaskRanger)) {
        uint8_t maximum = (level + 3U) / 4U;
        return maximum > 5U ? 5U : maximum;
    }
    if(mask & PocketClassMaskWarlock) {
        uint8_t maximum = (level + 1U) / 2U;
        return maximum > 5U ? 5U : maximum;
    }
    return 0U;
}

static bool pocket_spell_allowed(
    const PocketD20App* app,
    uint8_t level,
    uint16_t class_mask,
    bool has_metadata) {
    if(app->catalog_show_all) return true;
    if(!has_metadata || app->record_index >= app->data.character.spell_count) return false;
    const PocketSpell* spell = &app->data.character.spells[app->record_index];
    if(spell->class_index >= app->data.character.class_count) return false;
    const PocketClassLevel* class_level = &app->data.character.classes[spell->class_index];
    uint16_t selected_class = pocket_class_mask_from_name(class_level->name);
    return selected_class && (class_mask & selected_class) &&
           level <= pocket_class_max_spell_level(class_level);
}

static const char* pocket_catalog_title(const PocketD20App* app) {
    switch(app->catalog_kind) {
    case PocketCatalogClasses:
        return "Choose Class";
    case PocketCatalogSubclasses:
        return "Choose Subclass";
    case PocketCatalogBackgrounds:
        return "Choose Background";
    case PocketCatalogSpells:
        return app->catalog_show_all ? "Spells: All" : "Spells: Allowed";
    case PocketCatalogFeats:
        return "Choose Feat/Perk";
    case PocketCatalogItems:
        return "Choose Item";
    default:
        return "Choose Name";
    }
}

static bool pocket_catalog_add_metadata(
    PocketD20App* app,
    const char* name,
    uint8_t level,
    uint16_t class_mask,
    bool has_metadata) {
    if(app->catalog_kind == PocketCatalogSpells &&
       !pocket_spell_allowed(app, level, class_mask, has_metadata))
        return false;
    if(!name[0] || app->catalog_count >= POCKET_D20_MAX_CATALOG_ENTRIES) return false;
    for(uint16_t i = 0U; i < app->catalog_count; ++i) {
        if(strcmp(app->catalog_entries[i], name) == 0) {
            if(has_metadata) {
                app->catalog_levels[i] = level;
                app->catalog_class_masks[i] |= class_mask;
                app->catalog_has_metadata[i] = 1U;
            }
            return true;
        }
    }
    pocket_copy(
        app->catalog_entries[app->catalog_count],
        sizeof(app->catalog_entries[app->catalog_count]),
        name);
    app->catalog_levels[app->catalog_count] = level;
    app->catalog_class_masks[app->catalog_count] = class_mask;
    app->catalog_has_metadata[app->catalog_count] = has_metadata ? 1U : 0U;
    ++app->catalog_count;
    return true;
}

static bool pocket_catalog_add(PocketD20App* app, const char* name) {
    return pocket_catalog_add_metadata(app, name, 0U, 0U, false);
}

static void pocket_catalog_add_builtins(PocketD20App* app, PocketCatalogKind kind) {
    const char* const* entries = NULL;
    size_t count = 0U;
    switch(kind) {
    case PocketCatalogClasses:
        entries = pocket_catalog_classes;
        count = sizeof(pocket_catalog_classes) / sizeof(pocket_catalog_classes[0]);
        break;
    case PocketCatalogSubclasses:
        entries = pocket_catalog_subclasses;
        count = sizeof(pocket_catalog_subclasses) / sizeof(pocket_catalog_subclasses[0]);
        break;
    case PocketCatalogBackgrounds:
        entries = pocket_catalog_backgrounds;
        count = sizeof(pocket_catalog_backgrounds) / sizeof(pocket_catalog_backgrounds[0]);
        break;
    case PocketCatalogSpells:
        for(size_t i = 0U; i < sizeof(pocket_catalog_spells) / sizeof(pocket_catalog_spells[0]);
            ++i) {
            pocket_catalog_add_metadata(
                app,
                pocket_catalog_spells[i].name,
                pocket_catalog_spells[i].level,
                pocket_catalog_spells[i].class_mask,
                true);
        }
        return;
    case PocketCatalogFeats:
        entries = pocket_catalog_feats;
        count = sizeof(pocket_catalog_feats) / sizeof(pocket_catalog_feats[0]);
        break;
    case PocketCatalogItems:
        entries = pocket_catalog_items;
        count = sizeof(pocket_catalog_items) / sizeof(pocket_catalog_items[0]);
        break;
    default:
        break;
    }
    for(size_t i = 0U; i < count; ++i) pocket_catalog_add(app, entries[i]);
}

static void pocket_catalog_process_line(PocketD20App* app, char* line) {
    char* start = line;
    while(*start == ' ' || *start == '\t') ++start;
    char* end = start + strlen(start);
    while(end > start && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r')) --end;
    *end = '\0';
    if(!start[0] || start[0] == '#') return;
    if(app->catalog_kind != PocketCatalogSpells) {
        pocket_catalog_add(app, start);
        return;
    }

    char* level_separator = strchr(start, '|');
    if(!level_separator) {
        pocket_catalog_add_metadata(app, start, 0U, 0U, false);
        return;
    }
    *level_separator = '\0';
    char* class_separator = strchr(level_separator + 1U, '|');
    if(!class_separator) {
        pocket_catalog_add_metadata(app, start, 0U, 0U, false);
        return;
    }
    *class_separator = '\0';
    long parsed_level = strtol(level_separator + 1U, NULL, 10);
    if(parsed_level < 0 || parsed_level > 9) {
        pocket_catalog_add_metadata(app, start, 0U, 0U, false);
        return;
    }
    uint16_t mask = 0U;
    char* class_name = class_separator + 1U;
    while(class_name && class_name[0]) {
        char* comma = strchr(class_name, ',');
        if(comma) *comma = '\0';
        while(*class_name == ' ' || *class_name == '\t') ++class_name;
        char* class_end = class_name + strlen(class_name);
        while(class_end > class_name && (class_end[-1] == ' ' || class_end[-1] == '\t'))
            --class_end;
        *class_end = '\0';
        mask |= pocket_class_mask_from_name(class_name);
        class_name = comma ? comma + 1U : NULL;
    }
    pocket_catalog_add_metadata(app, start, (uint8_t)parsed_level, mask, mask != 0U);
}

static void pocket_catalog_load_path(PocketD20App* app, const char* path) {
    File* file = storage_file_alloc(app->storage);
    if(!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_free(file);
        return;
    }
    char line[192];
    size_t position = 0U;
    char byte = '\0';
    while(storage_file_read(file, &byte, 1U) == 1U) {
        if(byte == '\n') {
            line[position] = '\0';
            pocket_catalog_process_line(app, line);
            position = 0U;
        } else if(position + 1U < sizeof(line)) {
            line[position++] = byte;
        }
    }
    if(position) {
        line[position] = '\0';
        pocket_catalog_process_line(app, line);
    }
    storage_file_close(file);
    storage_file_free(file);
}

static void pocket_catalog_load_external(PocketD20App* app, PocketCatalogKind kind) {
    if(kind >= PocketCatalogCount) return;
    pocket_catalog_load_path(app, pocket_catalog_paths[kind]);
    if(kind == PocketCatalogFeats)
        pocket_catalog_load_path(app, pocket_catalog_abilities_path);
}

static void pocket_open_catalog(
    PocketD20App* app,
    PocketCatalogKind kind,
    PocketEditTarget target,
    const char* current) {
    app->catalog_kind = kind;
    app->catalog_target = target;
    app->catalog_return_selection = app->selection;
    app->catalog_show_all = 0U;
    app->catalog_count = 0U;
    memset(app->catalog_has_metadata, 0, sizeof(app->catalog_has_metadata));
    pocket_catalog_add_builtins(app, kind);
    pocket_catalog_load_external(app, kind);
    pocket_enter_screen(app, PocketScreenCatalog);
    for(uint16_t i = 0U; i < app->catalog_count; ++i) {
        if(strcmp(app->catalog_entries[i], current) == 0) {
            app->selection = i;
            if(i >= 5U) app->scroll = i - 4U;
            break;
        }
    }
}

static void pocket_draw_header(Canvas* canvas, const char* title, const char* status) {
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(canvas, 0, 0, 128, 10);
    canvas_set_color(canvas, ColorWhite);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 8, title);
    if(status && status[0] != '\0') {
        uint16_t width = canvas_string_width(canvas, status);
        if(width < 62U) canvas_draw_str(canvas, 126 - width, 8, status);
    }
    canvas_set_color(canvas, ColorBlack);
}

static void pocket_truncate(char* output, size_t size, const char* text, size_t maximum) {
    if(size == 0U) return;
    size_t length = strlen(text);
    if(length <= maximum) {
        pocket_copy(output, size, text);
        return;
    }
    size_t copy_length = maximum > 3U ? maximum - 3U : maximum;
    if(copy_length >= size) copy_length = size - 1U;
    memcpy(output, text, copy_length);
    output[copy_length] = '\0';
    if(maximum > 3U && copy_length + 3U < size) strcat(output, "...");
}

static void pocket_draw_row(Canvas* canvas, uint8_t row, bool selected, const char* text) {
    uint8_t y = (uint8_t)(11U + (row * 10U));
    char display[32];
    pocket_truncate(display, sizeof(display), text, 20U);
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

static void pocket_draw_menu_rows(
    Canvas* canvas,
    PocketD20App* app,
    const char* const* rows,
    uint16_t count) {
    for(uint8_t visible = 0U; visible < 5U; ++visible) {
        uint16_t index = app->scroll + visible;
        if(index >= count) break;
        pocket_draw_row(canvas, visible, index == app->selection, rows[index]);
    }
}

static uint8_t pocket_list_count(const PocketD20App* app) {
    const PocketCharacter* character = &app->data.character;
    switch(app->list_kind) {
    case PocketListClasses:
        return character->class_count;
    case PocketListSpells:
        return character->spell_count;
    case PocketListFeatures:
        return character->feature_count;
    case PocketListItems:
        return character->item_count;
    case PocketListLanguages:
        return character->language_count;
    case PocketListJournal:
        return character->journal_count;
    case PocketListParty:
        return app->data.party_count;
    default:
        return 0U;
    }
}

static const char* pocket_list_title(PocketListKind kind) {
    switch(kind) {
    case PocketListClasses:
        return "Classes";
    case PocketListSpells:
        return "Spells";
    case PocketListFeatures:
        return "Features / Perks";
    case PocketListItems:
        return "Inventory";
    case PocketListLanguages:
        return "Languages";
    case PocketListJournal:
        return "Journal";
    case PocketListParty:
        return "Party Roster";
    default:
        return "List";
    }
}

static void pocket_format_list_entry(
    const PocketD20App* app,
    uint8_t index,
    char* output,
    size_t size) {
    const PocketCharacter* character = &app->data.character;
    switch(app->list_kind) {
    case PocketListClasses: {
        const PocketClassLevel* class_level = &character->classes[index];
        snprintf(output, size, "%s L%u", class_level->name, class_level->level);
        break;
    }
    case PocketListSpells: {
        const PocketSpell* spell = &character->spells[index];
        snprintf(
            output,
            size,
            "%c%c L%u %s",
            pocket_spell_status(character, index),
            character->spell_free_casts_current[index] ? 'F' : ' ',
            spell->level,
            spell->name);
        break;
    }
    case PocketListFeatures: {
        const PocketFeature* feature = &character->features[index];
        snprintf(output, size, "%s", feature->name);
        break;
    }
    case PocketListItems: {
        const PocketItem* item = &character->items[index];
        snprintf(output, size, "%c %dx %s", item->equipped ? '*' : ' ', item->quantity, item->name);
        break;
    }
    case PocketListLanguages:
        snprintf(output, size, "%s", character->languages[index]);
        break;
    case PocketListJournal: {
        const PocketJournalEntry* entry = &character->journal[index];
        snprintf(
            output,
            size,
            "%c %s: %s",
            entry->completed ? 'X' : ' ',
            pocket_d20_journal_category_names[entry->category],
            entry->title);
        break;
    }
    case PocketListParty: {
        const PocketPartyMember* member = &app->data.party[index];
        snprintf(output, size, "%s %+d", member->name, member->initiative_modifier);
        break;
    }
    }
}

static uint8_t pocket_weapon_count(const PocketD20App* app) {
    uint8_t count = 0U;
    for(uint8_t i = 0U; i < app->data.character.item_count; ++i) {
        if(app->data.character.items[i].is_weapon) ++count;
    }
    return count;
}

static uint8_t pocket_weapon_index(const PocketD20App* app, uint8_t weapon_number) {
    uint8_t count = 0U;
    for(uint8_t i = 0U; i < app->data.character.item_count; ++i) {
        if(app->data.character.items[i].is_weapon) {
            if(count == weapon_number) return i;
            ++count;
        }
    }
    return 0U;
}

static uint8_t pocket_record_detail_count(const PocketD20App* app) {
    switch(app->list_kind) {
    case PocketListClasses:
        return 4U;
    case PocketListSpells:
        return 12U;
    case PocketListFeatures:
        return 8U;
    case PocketListItems:
        return 29U;
    case PocketListLanguages:
        return 2U;
    case PocketListJournal:
        return 8U;
    case PocketListParty:
        return 3U;
    default:
        return 0U;
    }
}

static void pocket_begin_text(
    PocketD20App* app,
    PocketEditTarget target,
    const char* header,
    const char* initial) {
    app->edit_target = target;
    pocket_copy(app->edit_buffer, sizeof(app->edit_buffer), initial);
    text_input_reset(app->text_input);
    text_input_set_header_text(app->text_input, header);
    text_input_set_result_callback(
        app->text_input,
        pocket_text_done,
        app,
        app->edit_buffer,
        sizeof(app->edit_buffer),
        false);
    view_dispatcher_switch_to_view(app->dispatcher, PocketViewTextInput);
}

static void pocket_draw_home(Canvas* canvas, PocketD20App* app) {
    char title[48];
    snprintf(title, sizeof(title), "Pocket d20 - %s", app->data.character.name);
    pocket_draw_header(canvas, title, app->status);
    pocket_draw_menu_rows(
        canvas,
        app,
        pocket_home_items,
        sizeof(pocket_home_items) / sizeof(pocket_home_items[0]));
}

static void pocket_draw_profiles(Canvas* canvas, PocketD20App* app) {
    char rows[POCKET_D20_MAX_PROFILES + 1U][48];
    const char* row_ptrs[POCKET_D20_MAX_PROFILES + 1U];
    uint8_t count = pocket_profile_count(app);
    for(uint8_t index = 0U; index < count; ++index) {
        uint8_t slot = pocket_profile_slot_at(app, index);
        const char* name = app->profiles.names[slot][0] ? app->profiles.names[slot] : "Unnamed";
        char active = slot == app->profiles.active_profile ? '*' : ' ';
        if(slot == 0U)
            snprintf(rows[index], sizeof(rows[index]), "%c Main: %s", active, name);
        else
            snprintf(
                rows[index],
                sizeof(rows[index]),
                "%c Slot %u: %s",
                active,
                (unsigned int)(slot + 1U),
                name);
        row_ptrs[index] = rows[index];
    }
    snprintf(rows[count], sizeof(rows[count]), "+ New Character");
    row_ptrs[count] = rows[count];
    pocket_draw_header(canvas, "Characters - hold OK delete", app->status);
    pocket_draw_menu_rows(canvas, app, row_ptrs, count + 1U);
}

static void pocket_draw_character(Canvas* canvas, PocketD20App* app) {
    PocketCharacter* character = &app->data.character;
    char rows[12][48];
    const char* row_ptrs[12];
    for(uint8_t i = 0U; i < 12U; ++i) row_ptrs[i] = rows[i];

    snprintf(rows[0], sizeof(rows[0]), "Name: %s", character->name);
    snprintf(rows[1], sizeof(rows[1]), "Player: %s", character->player);
    snprintf(rows[2], sizeof(rows[2]), "Species: %s", character->species);
    snprintf(rows[3], sizeof(rows[3]), "Background: %s", character->background);
    snprintf(rows[4], sizeof(rows[4]), "Alignment: %s", character->alignment);
    snprintf(rows[5], sizeof(rows[5]), "Classes (%u)", character->class_count);
    snprintf(
        rows[6],
        sizeof(rows[6]),
        "Total L%u / PB +%u",
        pocket_d20_total_level(character),
        pocket_d20_proficiency_bonus(character));
    snprintf(rows[7], sizeof(rows[7]), "XP: %lu", (unsigned long)character->experience);
    snprintf(
        rows[8],
        sizeof(rows[8]),
        "Leveling: %s",
        character->milestone_leveling ? "Milestone" : "XP");
    snprintf(rows[9], sizeof(rows[9]), "Languages (%u)", character->language_count);
    snprintf(rows[10], sizeof(rows[10]), "Other proficiencies");
    snprintf(rows[11], sizeof(rows[11]), "Inspiration: %s", character->inspiration ? "Yes" : "No");
    pocket_draw_header(canvas, "Character", app->status);
    pocket_draw_menu_rows(canvas, app, row_ptrs, 12U);
}

static void pocket_draw_vitals(Canvas* canvas, PocketD20App* app) {
    PocketCharacter* character = &app->data.character;
    char rows[14][40];
    const char* row_ptrs[14];
    for(uint8_t i = 0U; i < 14U; ++i) row_ptrs[i] = rows[i];

    snprintf(rows[0], sizeof(rows[0]), "HP: %d/%d", character->hp_current, character->hp_max);
    snprintf(rows[1], sizeof(rows[1]), "Temporary HP: %d", character->hp_temporary);
    snprintf(rows[2], sizeof(rows[2]), "Armor Class: %d", character->armor_class);
    if(character->exhaustion)
        snprintf(
            rows[3],
            sizeof(rows[3]),
            "Speed: %d -> %d ft",
            character->speed,
            pocket_d20_effective_speed(character));
    else
        snprintf(rows[3], sizeof(rows[3]), "Speed: %d ft", character->speed);
    snprintf(
        rows[4],
        sizeof(rows[4]),
        "Initiative: %+d",
        pocket_d20_initiative_modifier(character));
    snprintf(rows[5], sizeof(rows[5]), "Initiative misc: %+d", character->initiative_misc);
    snprintf(rows[6], sizeof(rows[6]), "Exhaustion: %u", character->exhaustion);
    snprintf(rows[7], sizeof(rows[7]), "Death saves: %u/%u", character->death_successes, 3U);
    snprintf(rows[8], sizeof(rows[8]), "Death fails: %u/%u", character->death_failures, 3U);
    snprintf(rows[9], sizeof(rows[9]), "Hit die: d%u", character->hit_die);
    snprintf(
        rows[10],
        sizeof(rows[10]),
        "Hit dice: %u/%u",
        character->hit_dice_current,
        character->hit_dice_max);
    snprintf(
        rows[11],
        sizeof(rows[11]),
        "Passive Perc: %d",
        10 + pocket_d20_skill_base_modifier(character, 11U));
    snprintf(
        rows[12],
        sizeof(rows[12]),
        "Passive Insight: %d",
        10 + pocket_d20_skill_base_modifier(character, 6U));
    snprintf(
        rows[13],
        sizeof(rows[13]),
        "Passive Invest: %d",
        10 + pocket_d20_skill_base_modifier(character, 8U));
    pocket_draw_header(canvas, "Vitals & Combat Stats", app->status);
    pocket_draw_menu_rows(canvas, app, row_ptrs, 14U);
}

static void pocket_draw_abilities(Canvas* canvas, PocketD20App* app) {
    PocketCharacter* character = &app->data.character;
    char rows[POCKET_D20_ABILITY_COUNT][40];
    const char* row_ptrs[POCKET_D20_ABILITY_COUNT];
    for(uint8_t i = 0U; i < POCKET_D20_ABILITY_COUNT; ++i) {
        row_ptrs[i] = rows[i];
        if(app->edit_modifier_mode) {
            snprintf(
                rows[i],
                sizeof(rows[i]),
                "%s Save M%+d = %+d",
                pocket_d20_ability_names[i],
                character->saving_throw_misc[i],
                pocket_d20_saving_throw_modifier(character, i));
        } else {
            snprintf(
                rows[i],
                sizeof(rows[i]),
                "%s %d(%+d) %s Save %+d",
                pocket_d20_ability_names[i],
                character->ability_scores[i],
                pocket_d20_ability_modifier(character->ability_scores[i]),
                pocket_proficiency_mark(character->saving_throw_proficiency[i]),
                pocket_d20_saving_throw_modifier(character, i));
        }
    }
    pocket_draw_header(
        canvas,
        app->edit_modifier_mode ? "Saves: <> misc, OK prof" : "Abilities: <> score, OK prof",
        app->status);
    pocket_draw_menu_rows(canvas, app, row_ptrs, POCKET_D20_ABILITY_COUNT);
}

static void pocket_draw_skills(Canvas* canvas, PocketD20App* app) {
    PocketCharacter* character = &app->data.character;
    char title[32];
    snprintf(
        title,
        sizeof(title),
        "Skills PB+%u: <> %s",
        pocket_d20_proficiency_bonus(character),
        app->edit_modifier_mode ? "misc" : "prof");
    pocket_draw_header(canvas, title, app->status);
    for(uint8_t visible = 0U; visible < 5U; ++visible) {
        uint16_t display_index = app->scroll + visible;
        if(display_index >= POCKET_D20_SKILL_COUNT) break;
        uint8_t index = pocket_skill_display_order[display_index];
        uint8_t ability = pocket_d20_skill_abilities[index];
        char row[48];
        if(app->edit_modifier_mode) {
            snprintf(
                row,
                sizeof(row),
                "%s %s M%+d=%+d",
                pocket_d20_ability_names[ability],
                pocket_d20_skill_names[index],
                character->skill_misc[index],
                pocket_d20_skill_modifier(character, index));
        } else {
            snprintf(
                row,
                sizeof(row),
                "%s %s %s %+d",
                pocket_d20_ability_names[ability],
                pocket_d20_skill_names[index],
                pocket_proficiency_mark(character->skill_proficiency[index]),
                pocket_d20_skill_modifier(character, index));
        }
        pocket_draw_row(canvas, visible, display_index == app->selection, row);
    }
}

static void pocket_draw_magic(Canvas* canvas, PocketD20App* app) {
    PocketCharacter* character = &app->data.character;
    char rows[16][48];
    const char* row_ptrs[16];
    for(uint8_t i = 0U; i < 16U; ++i) row_ptrs[i] = rows[i];
    snprintf(rows[0], sizeof(rows[0]), "Spells (%u)", character->spell_count);
    snprintf(
        rows[1],
        sizeof(rows[1]),
        "Casting ability: %s",
        pocket_d20_ability_names[character->spellcasting_ability]);
    snprintf(
        rows[2],
        sizeof(rows[2]),
        "PB +%u Atk %+d DC %d",
        pocket_d20_proficiency_bonus(character),
        pocket_d20_spell_attack_modifier(character),
        pocket_d20_spell_save_dc(character));
    snprintf(
        rows[3],
        sizeof(rows[3]),
        "Spell attack misc: %+d",
        character->spell_attack_misc);
    snprintf(
        rows[4],
        sizeof(rows[4]),
        "Spell save misc: %+d",
        character->spell_save_misc);
    snprintf(
        rows[5],
        sizeof(rows[5]),
        "Edit slots: %s",
        app->edit_slot_max ? "Maximum" : "Current");
    uint8_t wizard_level = pocket_wizard_level(character);
    if(app->arcane_recovery_active)
        snprintf(
            rows[6],
            sizeof(rows[6]),
            "Arcane Recovery: %u left",
            app->arcane_recovery_budget - app->arcane_recovery_spent);
    else if(!wizard_level)
        snprintf(rows[6], sizeof(rows[6]), "Arcane Recovery: no Wizard");
    else
        snprintf(
            rows[6],
            sizeof(rows[6]),
            "Arcane Recovery: %s",
            character->arcane_recovery_used ? "Used" : "Ready");
    for(uint8_t level = 1U; level <= 9U; ++level) {
        snprintf(
            rows[level + 6U],
            sizeof(rows[level + 6U]),
            "Level %u slots: %u/%u",
            level,
            character->spell_slots_current[level],
            character->spell_slots_max[level]);
    }
    pocket_draw_header(
        canvas,
        app->arcane_recovery_active ? "Magic: Arcane Recovery" : "Magic",
        app->status);
    pocket_draw_menu_rows(canvas, app, row_ptrs, 16U);
}

static void pocket_draw_currency(Canvas* canvas, PocketD20App* app) {
    const PocketCharacter* character = &app->data.character;
    char rows[5][40];
    const char* row_ptrs[5];
    for(uint8_t i = 0U; i < 5U; ++i) row_ptrs[i] = rows[i];
    snprintf(rows[0], sizeof(rows[0]), "Copper (CP): %ld", (long)character->currency_cp);
    snprintf(rows[1], sizeof(rows[1]), "Silver (SP): %ld", (long)character->currency_sp);
    snprintf(rows[2], sizeof(rows[2]), "Electrum (EP): %ld", (long)character->currency_ep);
    snprintf(rows[3], sizeof(rows[3]), "Gold (GP): %ld", (long)character->currency_gp);
    snprintf(rows[4], sizeof(rows[4]), "Platinum (PP): %ld", (long)character->currency_pp);
    pocket_draw_header(canvas, "Currency - hold for fast change", app->status);
    pocket_draw_menu_rows(canvas, app, row_ptrs, 5U);
}

static void pocket_draw_catalog(Canvas* canvas, PocketD20App* app) {
    pocket_draw_header(canvas, pocket_catalog_title(app), app->status);
    if(app->catalog_count == 0U) {
        pocket_draw_row(
            canvas,
            0U,
            false,
            app->catalog_kind == PocketCatalogSpells ? "No allowed spells (hold OK)" :
                                                       "Catalog is empty");
        return;
    }
    for(uint8_t visible = 0U; visible < 5U; ++visible) {
        uint16_t index = app->scroll + visible;
        if(index >= app->catalog_count) break;
        char row[48];
        if(app->catalog_kind == PocketCatalogSpells && app->catalog_has_metadata[index])
            snprintf(
                row,
                sizeof(row),
                "L%u %s",
                app->catalog_levels[index],
                app->catalog_entries[index]);
        else
            pocket_copy(row, sizeof(row), app->catalog_entries[index]);
        pocket_draw_row(
            canvas,
            visible,
            index == app->selection,
            row);
    }
}

static void pocket_draw_record_list(Canvas* canvas, PocketD20App* app) {
    uint8_t count = pocket_list_count(app);
    pocket_draw_header(canvas, pocket_list_title(app->list_kind), app->status);
    for(uint8_t visible = 0U; visible < 5U; ++visible) {
        uint16_t index = app->scroll + visible;
        if(index >= (uint16_t)count + 1U) break;
        char row[64];
        if(index == 0U) {
            snprintf(row, sizeof(row), "+ Add New");
        } else {
            pocket_format_list_entry(app, (uint8_t)(index - 1U), row, sizeof(row));
        }
        pocket_draw_row(canvas, visible, index == app->selection, row);
    }
}

static void pocket_format_record_detail(
    const PocketD20App* app,
    uint8_t field,
    char* output,
    size_t size) {
    const PocketCharacter* character = &app->data.character;
    uint8_t index = app->record_index;
    switch(app->list_kind) {
    case PocketListClasses: {
        const PocketClassLevel* class_level = &character->classes[index];
        if(field == 0U) snprintf(output, size, "Name: %s", class_level->name);
        else if(field == 1U) snprintf(output, size, "Subclass: %s", class_level->subclass);
        else if(field == 2U) snprintf(output, size, "Class level: %u", class_level->level);
        else snprintf(output, size, "Delete class");
        break;
    }
    case PocketListSpells: {
        const PocketSpell* spell = &character->spells[index];
        if(field == 0U) snprintf(output, size, "Name: %s", spell->name);
        else if(field == 1U) snprintf(output, size, "Notes: %s", spell->detail);
        else if(field == 2U)
            snprintf(
                output,
                size,
                "Source class: %s",
                spell->class_index < character->class_count ?
                    character->classes[spell->class_index].name :
                    "Primary");
        else if(field == 3U) snprintf(output, size, "Level: %u", spell->level);
        else if(field == 4U)
            snprintf(output, size, "Known: %s", character->spell_known[index] ? "Yes" : "No");
        else if(field == 5U)
            snprintf(output, size, "Prepared: %s", spell->prepared ? "Yes" : "No");
        else if(field == 6U)
            snprintf(
                output,
                size,
                "Always prepared: %s",
                character->spell_always_prepared[index] ? "Yes" : "No");
        else if(field == 7U) snprintf(output, size, "Ritual: %s", spell->ritual ? "Yes" : "No");
        else if(field == 8U)
            snprintf(
                output,
                size,
                "Free casts: %u/%u",
                character->spell_free_casts_current[index],
                character->spell_free_casts_max[index]);
        else if(field == 9U)
            snprintf(
                output,
                size,
                "Free casts max: %u",
                character->spell_free_casts_max[index]);
        else if(field == 10U)
            snprintf(
                output,
                size,
                "%s",
                character->spell_free_casts_current[index] ? "Use one free cast" :
                                                               "No free casts left");
        else
            snprintf(output, size, "Delete spell");
        break;
    }
    case PocketListFeatures: {
        const PocketFeature* feature = &character->features[index];
        const char* class_name = feature->class_index < character->class_count ?
                                     character->classes[feature->class_index].name :
                                     "General";
        if(field == 0U) snprintf(output, size, "Name: %s", feature->name);
        else if(field == 1U) snprintf(output, size, "Notes: %s", feature->detail);
        else if(field == 2U) snprintf(output, size, "Source class: %s", class_name);
        else if(field == 3U)
            snprintf(output, size, "Gained at class L%u", feature->class_level_gained);
        else if(field == 4U)
            snprintf(output, size, "Uses: %d/%d", feature->uses_current, feature->uses_max);
        else if(field == 5U) snprintf(output, size, "Maximum uses: %d", feature->uses_max);
        else if(field == 6U)
            snprintf(output, size, "Recharge: %s", pocket_recharge_names[feature->recharge]);
        else snprintf(output, size, "Delete feature");
        break;
    }
    case PocketListItems: {
        const PocketItem* item = &character->items[index];
        switch(field) {
        case 0:
            snprintf(output, size, "Name: %s", item->name);
            break;
        case 1:
            snprintf(output, size, "Notes: %s", item->detail);
            break;
        case 2:
            snprintf(output, size, "Quantity: %d", item->quantity);
            break;
        case 3:
            snprintf(output, size, "Weight: %d.%d lb", item->weight_tenths / 10, abs(item->weight_tenths % 10));
            break;
        case 4:
            snprintf(output, size, "Equipped: %s", item->equipped ? "Yes" : "No");
            break;
        case 5:
            snprintf(output, size, "Attuned: %s", item->attuned ? "Yes" : "No");
            break;
        case 6:
            snprintf(output, size, "Weapon: %s", item->is_weapon ? "Yes" : "No");
            break;
        case 7:
            snprintf(output, size, "Attack ability: %s", pocket_attack_ability_names[item->attack_ability]);
            break;
        case 8:
            snprintf(output, size, "Proficient: %s", item->proficient ? "Yes" : "No");
            break;
        case 9:
            snprintf(output, size, "Magic bonus: %+d", item->magic_bonus);
            break;
        case 10:
            snprintf(output, size, "Damage dice: %u", item->damage_dice);
            break;
        case 11:
            snprintf(output, size, "Damage die: d%u", item->damage_die);
            break;
        case 12:
            snprintf(output, size, "Versatile: %s", item->versatile_die ? "Yes" : "No");
            break;
        case 13:
            snprintf(output, size, "Versatile die: d%u", item->versatile_die);
            break;
        case 14:
            snprintf(output, size, "Use versatile: %s", item->use_versatile ? "Yes" : "No");
            break;
        case 15:
            snprintf(output, size, "Type: %s", pocket_d20_damage_names[item->damage_type]);
            break;
        case 16:
            snprintf(output, size, "Finesse: %s", (item->weapon_properties & PocketWeaponFinesse) ? "Yes" : "No");
            break;
        case 17:
            snprintf(output, size, "Ranged: %s", (item->weapon_properties & PocketWeaponRanged) ? "Yes" : "No");
            break;
        case 18:
            snprintf(output, size, "Light: %s", (item->weapon_properties & PocketWeaponLight) ? "Yes" : "No");
            break;
        case 19:
            snprintf(output, size, "Heavy: %s", (item->weapon_properties & PocketWeaponHeavy) ? "Yes" : "No");
            break;
        case 20:
            snprintf(output, size, "Thrown: %s", (item->weapon_properties & PocketWeaponThrown) ? "Yes" : "No");
            break;
        case 21:
            snprintf(output, size, "Ammunition: %s", (item->weapon_properties & PocketWeaponAmmunition) ? "Yes" : "No");
            break;
        case 22:
            snprintf(output, size, "Add ability dmg: %s", item->add_ability_damage ? "Yes" : "No");
            break;
        case 23:
            snprintf(output, size, "Extra dice: %u", item->extra_dice);
            break;
        case 24:
            snprintf(output, size, "Extra die: d%u", item->extra_die);
            break;
        case 25:
            snprintf(output, size, "Ammo: %d/%d", item->ammo_current, item->ammo_max);
            break;
        case 26:
            snprintf(output, size, "Maximum ammo: %d", item->ammo_max);
            break;
        case 27:
            snprintf(
                output,
                size,
                "Attack %+d / %ud%u",
                pocket_d20_weapon_attack_modifier(character, item),
                item->damage_dice,
                item->use_versatile ? item->versatile_die : item->damage_die);
            break;
        default:
            snprintf(output, size, "Delete item");
            break;
        }
        break;
    }
    case PocketListLanguages:
        if(field == 0U) snprintf(output, size, "Language: %s", character->languages[index]);
        else snprintf(output, size, "Delete language");
        break;
    case PocketListJournal: {
        const PocketJournalEntry* entry = &character->journal[index];
        const char* class_name = entry->class_index < character->class_count ?
                                     character->classes[entry->class_index].name :
                                     "Primary";
        if(field == 0U)
            snprintf(output, size, "Category: %s", pocket_d20_journal_category_names[entry->category]);
        else if(field == 1U) snprintf(output, size, "Title: %s", entry->title);
        else if(field == 2U) snprintf(output, size, "Body: %s", entry->body);
        else if(field == 3U) snprintf(output, size, "Complete: %s", entry->completed ? "Yes" : "No");
        else if(field == 4U) snprintf(output, size, "Level class: %s", class_name);
        else if(field == 5U)
            snprintf(output, size, "%s", entry->level_granted ? "Level already applied" : "Apply milestone level");
        else if(field == 6U) snprintf(output, size, "Create inventory item");
        else snprintf(output, size, "Delete journal entry");
        break;
    }
    case PocketListParty: {
        const PocketPartyMember* member = &app->data.party[index];
        if(field == 0U) snprintf(output, size, "Name: %s", member->name);
        else if(field == 1U) snprintf(output, size, "Initiative mod: %+d", member->initiative_modifier);
        else snprintf(output, size, "Delete party member");
        break;
    }
    }
}

static void pocket_draw_record_detail(Canvas* canvas, PocketD20App* app) {
    uint8_t count = pocket_record_detail_count(app);
    pocket_draw_header(canvas, pocket_list_title(app->list_kind), app->status);
    for(uint8_t visible = 0U; visible < 5U; ++visible) {
        uint16_t field = app->scroll + visible;
        if(field >= count) break;
        char row[80];
        pocket_format_record_detail(app, (uint8_t)field, row, sizeof(row));
        pocket_draw_row(canvas, visible, field == app->selection, row);
    }
}

static void pocket_draw_combat(Canvas* canvas, PocketD20App* app) {
    PocketCharacter* character = &app->data.character;
    char rows[10][48];
    const char* row_ptrs[10];
    for(uint8_t i = 0U; i < 10U; ++i) row_ptrs[i] = rows[i];
    snprintf(rows[0], sizeof(rows[0]), "Weapon Attacks");
    snprintf(rows[1], sizeof(rows[1]), "Initiative Tracker");
    snprintf(rows[2], sizeof(rows[2]), "HP: %d/%d", character->hp_current, character->hp_max);
    snprintf(rows[3], sizeof(rows[3]), "Temporary HP: %d", character->hp_temporary);
    snprintf(rows[4], sizeof(rows[4]), "Short Rest");
    snprintf(
        rows[5],
        sizeof(rows[5]),
        "Spend Hit Die: %u/%u",
        character->hit_dice_current,
        character->hit_dice_max);
    snprintf(rows[6], sizeof(rows[6]), "Long Rest");
    snprintf(rows[7], sizeof(rows[7]), "Death success: %u", character->death_successes);
    snprintf(rows[8], sizeof(rows[8]), "Death failure: %u", character->death_failures);
    snprintf(rows[9], sizeof(rows[9]), "Exhaustion: %u", character->exhaustion);
    pocket_draw_header(canvas, "Combat", app->status);
    pocket_draw_menu_rows(canvas, app, row_ptrs, 10U);
}

static void pocket_draw_dice(Canvas* canvas, PocketD20App* app) {
    char rows[5][48];
    const char* row_ptrs[5];
    for(uint8_t i = 0U; i < 5U; ++i) row_ptrs[i] = rows[i];
    snprintf(rows[0], sizeof(rows[0]), "Dice count: %u", app->dice_count);
    snprintf(rows[1], sizeof(rows[1]), "Die: d%u", app->dice_sides);
    snprintf(rows[2], sizeof(rows[2]), "Modifier: %+d", app->dice_modifier);
    snprintf(rows[3], sizeof(rows[3]), "Mode: %s", pocket_roll_mode_names[app->roll_mode]);
    if(app->dice_second) {
        snprintf(
            rows[4],
            sizeof(rows[4]),
            "Roll %u/%u = %d",
            app->dice_first,
            app->dice_second,
            app->dice_result);
    } else if(app->dice_result || app->dice_modifier) {
        snprintf(rows[4], sizeof(rows[4]), "Roll result: %d", app->dice_result);
    } else {
        snprintf(rows[4], sizeof(rows[4]), "Roll now");
    }
    pocket_draw_header(canvas, "Dice Roller", app->status);
    pocket_draw_menu_rows(canvas, app, row_ptrs, 5U);
}

static void pocket_format_roll_row(
    char* output,
    size_t size,
    const uint8_t* values,
    uint8_t start,
    uint8_t count) {
    output[0] = '\0';
    size_t position = 0U;
    for(uint8_t i = 0U; i < count && start + i < POCKET_D20_MAX_GENERIC_ROLLS; ++i) {
        int written = snprintf(
            output + position,
            size - position,
            "%s%u",
            i ? ", " : "",
            (unsigned int)values[start + i]);
        if(written < 0 || (size_t)written >= size - position) break;
        position += (size_t)written;
    }
}

static void pocket_draw_dice_result(Canvas* canvas, PocketD20App* app) {
    char title[48];
    char rows[5][48];
    const char* row_ptrs[5];
    for(uint8_t i = 0U; i < 5U; ++i) {
        rows[i][0] = '\0';
        row_ptrs[i] = rows[i];
    }

    if(app->dice_second) {
        uint8_t chosen = app->roll_mode == PocketRollAdvantage ?
                             (app->dice_first > app->dice_second ? app->dice_first :
                                                                   app->dice_second) :
                             (app->dice_first < app->dice_second ? app->dice_first :
                                                                   app->dice_second);
        snprintf(
            title,
            sizeof(title),
            "%s d20 - total %d",
            app->roll_mode == PocketRollAdvantage ? "Advantage" : "Disadvantage",
            app->dice_result);
        snprintf(rows[0], sizeof(rows[0]), "Rolls: %u, %u", app->dice_first, app->dice_second);
        snprintf(rows[1], sizeof(rows[1]), "Dice sum: %u", app->dice_roll_sum);
        snprintf(rows[2], sizeof(rows[2]), "Chosen: %u", chosen);
        snprintf(rows[3], sizeof(rows[3]), "Modifier: %+d", app->dice_modifier);
        snprintf(rows[4], sizeof(rows[4]), "Total: %d (OK reroll)", app->dice_result);
    } else if(app->dice_roll_value_count > 1U) {
        snprintf(
            title,
            sizeof(title),
            "%ud%u sum %u total %d",
            app->dice_roll_value_count,
            app->dice_sides,
            app->dice_roll_sum,
            app->dice_result);
        for(uint8_t row = 0U; row < 5U; ++row) {
            uint8_t start = row * 4U;
            if(start >= app->dice_roll_value_count) break;
            uint8_t remaining = app->dice_roll_value_count - start;
            pocket_format_roll_row(
                rows[row],
                sizeof(rows[row]),
                app->dice_roll_values,
                start,
                remaining > 4U ? 4U : remaining);
        }
    } else {
        snprintf(title, sizeof(title), "d%u total %d", app->dice_sides, app->dice_result);
        snprintf(rows[0], sizeof(rows[0]), "Roll: %u", app->dice_first);
        snprintf(rows[1], sizeof(rows[1]), "Dice sum: %u", app->dice_roll_sum);
        snprintf(rows[2], sizeof(rows[2]), "Modifier: %+d", app->dice_modifier);
        snprintf(rows[4], sizeof(rows[4]), "OK to reroll");
    }
    pocket_draw_header(canvas, title, app->status);
    for(uint8_t row = 0U; row < 5U; ++row) {
        if(rows[row][0]) pocket_draw_row(canvas, row, false, row_ptrs[row]);
    }
}

static void pocket_draw_attack_list(Canvas* canvas, PocketD20App* app) {
    uint8_t count = pocket_weapon_count(app);
    char title[32];
    snprintf(title, sizeof(title), "Attacks: %s", pocket_roll_mode_names[app->roll_mode]);
    pocket_draw_header(canvas, title, app->status);
    if(count == 0U) {
        pocket_draw_row(canvas, 0U, false, "No weapon items");
        pocket_draw_row(canvas, 1U, false, "Add one in Inventory");
        return;
    }
    for(uint8_t visible = 0U; visible < 5U; ++visible) {
        uint16_t weapon_number = app->scroll + visible;
        if(weapon_number >= count) break;
        uint8_t item_index = pocket_weapon_index(app, weapon_number);
        const PocketItem* item = &app->data.character.items[item_index];
        char row[64];
        snprintf(
            row,
            sizeof(row),
            "%s %+d %ud%u",
            item->name,
            pocket_d20_weapon_attack_modifier(&app->data.character, item),
            item->damage_dice,
            item->use_versatile ? item->versatile_die : item->damage_die);
        pocket_draw_row(canvas, visible, weapon_number == app->selection, row);
    }
}

static void pocket_draw_attack_result(Canvas* canvas, PocketD20App* app) {
    const PocketItem* item = &app->data.character.items[app->attack_item_index];
    pocket_draw_header(canvas, item->name, app->status);
    char row[64];
    if(app->attack_phase == 0U) {
        if(app->attack_roll.second_die) {
            snprintf(
                row,
                sizeof(row),
                "d20: %u / %u",
                app->attack_roll.first_die,
                app->attack_roll.second_die);
        } else {
            snprintf(row, sizeof(row), "d20: %u", app->attack_roll.first_die);
        }
        pocket_draw_row(canvas, 0U, false, row);
        uint8_t detail_row = 1U;
        if(app->attack_roll.second_die) {
            snprintf(
                row,
                sizeof(row),
                "Dice sum: %u",
                app->attack_roll.first_die + app->attack_roll.second_die);
            pocket_draw_row(canvas, detail_row++, false, row);
        }
        snprintf(row, sizeof(row), "Modifier: %+d", app->attack_roll.modifier);
        pocket_draw_row(canvas, detail_row++, false, row);
        snprintf(row, sizeof(row), "Attack total: %d", app->attack_roll.total);
        pocket_draw_row(canvas, detail_row++, false, row);
        if(app->attack_roll.critical)
            snprintf(row, sizeof(row), "Critical! OK damage");
        else if(app->attack_roll.automatic_miss)
            snprintf(row, sizeof(row), "Natural 1 - miss");
        else
            snprintf(row, sizeof(row), "OK damage; Right crit");
        pocket_draw_row(canvas, detail_row, false, row);
        if(detail_row < 4U) pocket_draw_row(canvas, 4U, false, "Up: reroll attack");
    } else {
        uint8_t roll_count =
            app->damage_roll.weapon_roll_count + app->damage_roll.extra_roll_count;
        if(roll_count > 1U) {
            uint8_t page_count = (roll_count + 15U) / 16U;
            if(app->damage_roll_page >= page_count) app->damage_roll_page = page_count - 1U;
            char damage_title[48];
            snprintf(
                damage_title,
                sizeof(damage_title),
                "Damage %u/%u - total %d",
                app->damage_roll_page + 1U,
                page_count,
                app->damage_roll.total);
            pocket_draw_header(canvas, damage_title, app->status);
            uint8_t start = app->damage_roll_page * 16U;
            for(uint8_t display_row = 0U; display_row < 4U; ++display_row) {
                uint8_t first = start + (display_row * 4U);
                if(first >= roll_count) break;
                char values[64] = "";
                size_t position = 0U;
                for(uint8_t i = 0U; i < 4U && first + i < roll_count; ++i) {
                    uint8_t roll_index = first + i;
                    int written = snprintf(
                        values + position,
                        sizeof(values) - position,
                        "%s%c%u",
                        i ? " " : "",
                        roll_index < app->damage_roll.weapon_roll_count ? 'W' : 'E',
                        app->damage_roll.rolls[roll_index]);
                    if(written < 0 || (size_t)written >= sizeof(values) - position) break;
                    position += (size_t)written;
                }
                pocket_draw_row(canvas, display_row, false, values);
            }
            snprintf(
                row,
                sizeof(row),
                "Sum %d %+d = %d",
                app->damage_roll.weapon_total + app->damage_roll.extra_total,
                app->damage_roll.modifier,
                app->damage_roll.total);
            pocket_draw_row(canvas, 4U, false, row);
        } else {
            snprintf(
                row,
                sizeof(row),
                "%s damage",
                app->damage_roll.critical ? "Critical" : "Normal");
            pocket_draw_row(canvas, 0U, false, row);
            snprintf(row, sizeof(row), "Weapon dice: %d", app->damage_roll.weapon_total);
            pocket_draw_row(canvas, 1U, false, row);
            snprintf(row, sizeof(row), "Extra dice: %d", app->damage_roll.extra_total);
            pocket_draw_row(canvas, 2U, false, row);
            snprintf(row, sizeof(row), "Modifier: %+d", app->damage_roll.modifier);
            pocket_draw_row(canvas, 3U, false, row);
            snprintf(row, sizeof(row), "Total: %d (OK reroll)", app->damage_roll.total);
            pocket_draw_row(canvas, 4U, false, row);
        }
    }
}

static void pocket_draw_initiative_menu(Canvas* canvas, PocketD20App* app) {
    char rows[5][48];
    const char* row_ptrs[5];
    for(uint8_t i = 0U; i < 5U; ++i) row_ptrs[i] = rows[i];
    snprintf(rows[0], sizeof(rows[0]), "Start New Combat");
    snprintf(
        rows[1],
        sizeof(rows[1]),
        "Resume Combat%s",
        app->data.initiative.active ? "" : " (none)");
    snprintf(rows[2], sizeof(rows[2]), "Party Roster (%u)", app->data.party_count);
    snprintf(rows[3], sizeof(rows[3]), "Edit Current Order");
    snprintf(rows[4], sizeof(rows[4]), "End Current Combat");
    pocket_draw_header(canvas, "Initiative", app->status);
    pocket_draw_menu_rows(canvas, app, row_ptrs, 5U);
}

static void pocket_draw_initiative_setup(Canvas* canvas, PocketD20App* app) {
    PocketInitiativeState* initiative = &app->data.initiative;
    pocket_draw_header(canvas, "Set Initiative: <> / hold OK", app->status);
    uint16_t count = initiative->count + 2U;
    for(uint8_t visible = 0U; visible < 5U; ++visible) {
        uint16_t index = app->scroll + visible;
        if(index >= count) break;
        char row[48];
        if(index < initiative->count) {
            PocketInitiativeEntry* entry = &initiative->entries[index];
            snprintf(row, sizeof(row), "%s: %d", entry->name, entry->initiative_total);
        } else if(index == initiative->count) {
            snprintf(row, sizeof(row), "+ Temporary participant");
        } else {
            snprintf(row, sizeof(row), "Begin Combat");
        }
        pocket_draw_row(canvas, visible, index == app->selection, row);
    }
}

static void pocket_draw_initiative_combat(Canvas* canvas, PocketD20App* app) {
    PocketInitiativeState* initiative = &app->data.initiative;
    char title[32];
    snprintf(title, sizeof(title), "Round %u - OK Next", initiative->round);
    pocket_draw_header(canvas, title, app->status);
    for(uint8_t visible = 0U; visible < 5U; ++visible) {
        uint16_t index = app->scroll + visible;
        if(index >= initiative->count) break;
        PocketInitiativeEntry* entry = &initiative->entries[index];
        char row[48];
        snprintf(
            row,
            sizeof(row),
            "%c %s: %d",
            index == initiative->current_turn ? '>' : ' ',
            entry->name,
            entry->initiative_total);
        pocket_draw_row(canvas, visible, index == app->selection, row);
    }
}

static void pocket_draw_about(Canvas* canvas, PocketD20App* app) {
    pocket_draw_header(canvas, "About Pocket d20 v0.3", app->status);
    pocket_draw_row(canvas, 0U, false, "5E-compatible tracker");
    pocket_draw_row(canvas, 1U, false, "Native RogueMaster app");
    pocket_draw_row(canvas, 2U, false, "Six character profiles");
    pocket_draw_row(canvas, 3U, false, "Separate text saves");
    pocket_draw_row(canvas, 4U, false, "Back to return");
}

static void pocket_draw_animated_die(
    Canvas* canvas,
    int32_t center_x,
    int32_t center_y,
    uint8_t frame,
    uint8_t face) {
    if(frame & 1U) {
        canvas_draw_line(canvas, center_x, center_y - 14, center_x + 14, center_y);
        canvas_draw_line(canvas, center_x + 14, center_y, center_x, center_y + 14);
        canvas_draw_line(canvas, center_x, center_y + 14, center_x - 14, center_y);
        canvas_draw_line(canvas, center_x - 14, center_y, center_x, center_y - 14);
    } else {
        int8_t offset = (frame & 2U) ? 2 : 0;
        canvas_draw_rframe(canvas, center_x - 14 + offset, center_y - 14, 28, 28, 4);
    }
    char value[5];
    snprintf(value, sizeof(value), "%u", face);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, center_x, center_y + 1, AlignCenter, AlignCenter, value);
}

static void pocket_draw_dice_animation(Canvas* canvas, PocketD20App* app) {
    char title[40];
    snprintf(
        title,
        sizeof(title),
        "Rolling %ud%u...",
        app->dice_anim_count,
        app->dice_anim_sides);
    pocket_draw_header(canvas, title, NULL);
    uint8_t visible_dice = app->dice_anim_count > 1U ? 3U : 1U;
    for(uint8_t i = 0U; i < visible_dice; ++i) {
        int32_t x = visible_dice == 1U ? 64 : 22 + (i * 42);
        uint8_t face =
            (uint8_t)(((app->dice_anim_frame * 7U) + (i * 5U) + app->dice_anim_sides) %
                      app->dice_anim_sides) +
            1U;
        pocket_draw_animated_die(canvas, x, 34, app->dice_anim_frame + i, face);
    }
    canvas_draw_frame(canvas, 14, 55, 100, 5);
    uint8_t progress = (uint8_t)(((app->dice_anim_frame + 1U) * 98U) /
                                 POCKET_D20_DICE_ANIMATION_FRAMES);
    canvas_draw_box(canvas, 15, 56, progress, 3);
}

static void pocket_draw_callback(Canvas* canvas, void* model) {
    PocketD20App* app = *(PocketD20App**)model;
    canvas_clear(canvas);
    if(app->dice_animating) {
        pocket_draw_dice_animation(canvas, app);
        return;
    }
    switch(app->screen) {
    case PocketScreenHome:
        pocket_draw_home(canvas, app);
        break;
    case PocketScreenProfiles:
        pocket_draw_profiles(canvas, app);
        break;
    case PocketScreenCharacter:
        pocket_draw_character(canvas, app);
        break;
    case PocketScreenVitals:
        pocket_draw_vitals(canvas, app);
        break;
    case PocketScreenAbilities:
        pocket_draw_abilities(canvas, app);
        break;
    case PocketScreenSkills:
        pocket_draw_skills(canvas, app);
        break;
    case PocketScreenMagic:
        pocket_draw_magic(canvas, app);
        break;
    case PocketScreenCurrency:
        pocket_draw_currency(canvas, app);
        break;
    case PocketScreenRecordList:
        pocket_draw_record_list(canvas, app);
        break;
    case PocketScreenRecordDetail:
        pocket_draw_record_detail(canvas, app);
        break;
    case PocketScreenCatalog:
        pocket_draw_catalog(canvas, app);
        break;
    case PocketScreenCombat:
        pocket_draw_combat(canvas, app);
        break;
    case PocketScreenDice:
        pocket_draw_dice(canvas, app);
        break;
    case PocketScreenDiceResult:
        pocket_draw_dice_result(canvas, app);
        break;
    case PocketScreenAttackList:
        pocket_draw_attack_list(canvas, app);
        break;
    case PocketScreenAttackResult:
        pocket_draw_attack_result(canvas, app);
        break;
    case PocketScreenInitiativeMenu:
        pocket_draw_initiative_menu(canvas, app);
        break;
    case PocketScreenInitiativeSetup:
        pocket_draw_initiative_setup(canvas, app);
        break;
    case PocketScreenInitiativeCombat:
        pocket_draw_initiative_combat(canvas, app);
        break;
    case PocketScreenAbout:
        pocket_draw_about(canvas, app);
        break;
    }
}

static void pocket_open_list(
    PocketD20App* app,
    PocketListKind kind,
    PocketScreen return_screen) {
    app->list_kind = kind;
    app->return_screen = return_screen;
    pocket_enter_screen(app, PocketScreenRecordList);
}

static bool pocket_add_record(PocketD20App* app) {
    PocketCharacter* character = &app->data.character;
    switch(app->list_kind) {
    case PocketListClasses:
        if(character->class_count >= POCKET_D20_MAX_CLASSES ||
           pocket_d20_total_level(character) >= 20U)
            return false;
        app->record_index = character->class_count++;
        memset(&character->classes[app->record_index], 0, sizeof(PocketClassLevel));
        pocket_copy(
            character->classes[app->record_index].name,
            sizeof(character->classes[app->record_index].name),
            "New Class");
        pocket_copy(
            character->classes[app->record_index].subclass,
            sizeof(character->classes[app->record_index].subclass),
            "None");
        character->classes[app->record_index].level = 1U;
        break;
    case PocketListSpells:
        if(character->spell_count >= POCKET_D20_MAX_SPELLS) return false;
        app->record_index = character->spell_count++;
        memset(&character->spells[app->record_index], 0, sizeof(PocketSpell));
        character->spell_known[app->record_index] = 1U;
        character->spell_always_prepared[app->record_index] = 0U;
        character->spell_free_casts_current[app->record_index] = 0U;
        character->spell_free_casts_max[app->record_index] = 0U;
        pocket_copy(
            character->spells[app->record_index].name,
            sizeof(character->spells[app->record_index].name),
            "New Spell");
        break;
    case PocketListFeatures:
        if(character->feature_count >= POCKET_D20_MAX_FEATURES) return false;
        app->record_index = character->feature_count++;
        memset(&character->features[app->record_index], 0, sizeof(PocketFeature));
        pocket_copy(
            character->features[app->record_index].name,
            sizeof(character->features[app->record_index].name),
            "New Feature");
        character->features[app->record_index].class_index = 0U;
        character->features[app->record_index].class_level_gained =
            character->classes[0].level;
        break;
    case PocketListItems: {
        if(character->item_count >= POCKET_D20_MAX_ITEMS) return false;
        app->record_index = character->item_count++;
        PocketItem* item = &character->items[app->record_index];
        memset(item, 0, sizeof(*item));
        pocket_copy(item->name, sizeof(item->name), "New Item");
        item->quantity = 1;
        item->damage_dice = 1U;
        item->damage_die = 6U;
        item->extra_die = 6U;
        item->add_ability_damage = 1U;
        break;
    }
    case PocketListLanguages:
        if(character->language_count >= POCKET_D20_MAX_LANGUAGES) return false;
        app->record_index = character->language_count++;
        memset(character->languages[app->record_index], 0, POCKET_D20_SHORT_LEN);
        pocket_copy(
            character->languages[app->record_index],
            POCKET_D20_SHORT_LEN,
            "New Language");
        break;
    case PocketListJournal:
        if(character->journal_count >= POCKET_D20_MAX_JOURNAL) return false;
        app->record_index = character->journal_count++;
        memset(&character->journal[app->record_index], 0, sizeof(PocketJournalEntry));
        pocket_copy(
            character->journal[app->record_index].title,
            sizeof(character->journal[app->record_index].title),
            "New Note");
        break;
    case PocketListParty:
        if(app->data.party_count >= POCKET_D20_MAX_PARTY) return false;
        app->record_index = app->data.party_count++;
        memset(&app->data.party[app->record_index], 0, sizeof(PocketPartyMember));
        pocket_copy(
            app->data.party[app->record_index].name,
            sizeof(app->data.party[app->record_index].name),
            "Party Member");
        break;
    }
    pocket_save(app, false);
    pocket_enter_screen(app, PocketScreenRecordDetail);
    return true;
}

static void pocket_delete_record(PocketD20App* app) {
    PocketCharacter* character = &app->data.character;
    uint8_t index = app->record_index;
    switch(app->list_kind) {
    case PocketListClasses:
        if(character->class_count <= 1U) {
            pocket_set_status(app, "Keep at least one class");
            return;
        }
        memmove(
            &character->classes[index],
            &character->classes[index + 1U],
            (character->class_count - index - 1U) * sizeof(PocketClassLevel));
        --character->class_count;
        memset(&character->classes[character->class_count], 0, sizeof(PocketClassLevel));
        for(uint8_t i = 0U; i < character->feature_count; ++i) {
            if(character->features[i].class_index == index)
                character->features[i].class_index = 0U;
            else if(character->features[i].class_index > index)
                --character->features[i].class_index;
        }
        for(uint8_t i = 0U; i < character->spell_count; ++i) {
            if(character->spells[i].class_index == index)
                character->spells[i].class_index = 0U;
            else if(character->spells[i].class_index > index)
                --character->spells[i].class_index;
        }
        for(uint8_t i = 0U; i < character->journal_count; ++i) {
            if(character->journal[i].class_index == index)
                character->journal[i].class_index = 0U;
            else if(character->journal[i].class_index > index)
                --character->journal[i].class_index;
        }
        break;
    case PocketListSpells:
        memmove(
            &character->spells[index],
            &character->spells[index + 1U],
            (character->spell_count - index - 1U) * sizeof(PocketSpell));
        memmove(
            &character->spell_known[index],
            &character->spell_known[index + 1U],
            character->spell_count - index - 1U);
        memmove(
            &character->spell_always_prepared[index],
            &character->spell_always_prepared[index + 1U],
            character->spell_count - index - 1U);
        memmove(
            &character->spell_free_casts_current[index],
            &character->spell_free_casts_current[index + 1U],
            character->spell_count - index - 1U);
        memmove(
            &character->spell_free_casts_max[index],
            &character->spell_free_casts_max[index + 1U],
            character->spell_count - index - 1U);
        --character->spell_count;
        memset(&character->spells[character->spell_count], 0, sizeof(PocketSpell));
        character->spell_known[character->spell_count] = 0U;
        character->spell_always_prepared[character->spell_count] = 0U;
        character->spell_free_casts_current[character->spell_count] = 0U;
        character->spell_free_casts_max[character->spell_count] = 0U;
        break;
    case PocketListFeatures:
        memmove(
            &character->features[index],
            &character->features[index + 1U],
            (character->feature_count - index - 1U) * sizeof(PocketFeature));
        --character->feature_count;
        memset(&character->features[character->feature_count], 0, sizeof(PocketFeature));
        break;
    case PocketListItems:
        memmove(
            &character->items[index],
            &character->items[index + 1U],
            (character->item_count - index - 1U) * sizeof(PocketItem));
        --character->item_count;
        memset(&character->items[character->item_count], 0, sizeof(PocketItem));
        break;
    case PocketListLanguages:
        memmove(
            &character->languages[index],
            &character->languages[index + 1U],
            (character->language_count - index - 1U) * POCKET_D20_SHORT_LEN);
        --character->language_count;
        memset(character->languages[character->language_count], 0, POCKET_D20_SHORT_LEN);
        break;
    case PocketListJournal:
        memmove(
            &character->journal[index],
            &character->journal[index + 1U],
            (character->journal_count - index - 1U) * sizeof(PocketJournalEntry));
        --character->journal_count;
        memset(&character->journal[character->journal_count], 0, sizeof(PocketJournalEntry));
        break;
    case PocketListParty:
        memmove(
            &app->data.party[index],
            &app->data.party[index + 1U],
            (app->data.party_count - index - 1U) * sizeof(PocketPartyMember));
        --app->data.party_count;
        memset(&app->data.party[app->data.party_count], 0, sizeof(PocketPartyMember));
        break;
    }
    pocket_save(app, false);
    pocket_enter_screen(app, PocketScreenRecordList);
}

static void pocket_text_done(void* context) {
    PocketD20App* app = context;
    PocketCharacter* character = &app->data.character;
    uint8_t index = app->record_index;
    switch(app->edit_target) {
    case PocketEditCharacterName:
        pocket_copy(character->name, sizeof(character->name), app->edit_buffer);
        break;
    case PocketEditPlayerName:
        pocket_copy(character->player, sizeof(character->player), app->edit_buffer);
        break;
    case PocketEditSpecies:
        pocket_copy(character->species, sizeof(character->species), app->edit_buffer);
        break;
    case PocketEditBackground:
        pocket_copy(character->background, sizeof(character->background), app->edit_buffer);
        break;
    case PocketEditAlignment:
        pocket_copy(character->alignment, sizeof(character->alignment), app->edit_buffer);
        break;
    case PocketEditOtherProficiencies:
        pocket_copy(
            character->other_proficiencies,
            sizeof(character->other_proficiencies),
            app->edit_buffer);
        break;
    case PocketEditClassName:
        pocket_copy(
            character->classes[index].name,
            sizeof(character->classes[index].name),
            app->edit_buffer);
        break;
    case PocketEditSubclass:
        pocket_copy(
            character->classes[index].subclass,
            sizeof(character->classes[index].subclass),
            app->edit_buffer);
        break;
    case PocketEditSpellName:
        pocket_copy(
            character->spells[index].name,
            sizeof(character->spells[index].name),
            app->edit_buffer);
        break;
    case PocketEditSpellDetail:
        pocket_copy(
            character->spells[index].detail,
            sizeof(character->spells[index].detail),
            app->edit_buffer);
        break;
    case PocketEditFeatureName:
        pocket_copy(
            character->features[index].name,
            sizeof(character->features[index].name),
            app->edit_buffer);
        break;
    case PocketEditFeatureDetail:
        pocket_copy(
            character->features[index].detail,
            sizeof(character->features[index].detail),
            app->edit_buffer);
        break;
    case PocketEditItemName:
        pocket_copy(
            character->items[index].name,
            sizeof(character->items[index].name),
            app->edit_buffer);
        break;
    case PocketEditItemDetail:
        pocket_copy(
            character->items[index].detail,
            sizeof(character->items[index].detail),
            app->edit_buffer);
        break;
    case PocketEditLanguageName:
        pocket_copy(character->languages[index], POCKET_D20_SHORT_LEN, app->edit_buffer);
        break;
    case PocketEditJournalTitle:
        pocket_copy(
            character->journal[index].title,
            sizeof(character->journal[index].title),
            app->edit_buffer);
        break;
    case PocketEditJournalBody:
        pocket_copy(
            character->journal[index].body,
            sizeof(character->journal[index].body),
            app->edit_buffer);
        break;
    case PocketEditPartyName:
        pocket_copy(
            app->data.party[index].name,
            sizeof(app->data.party[index].name),
            app->edit_buffer);
        break;
    case PocketEditTemporaryInitiativeName:
        pocket_copy(
            app->data.initiative.entries[index].name,
            sizeof(app->data.initiative.entries[index].name),
            app->edit_buffer);
        break;
    case PocketEditNone:
        break;
    }
    app->edit_target = PocketEditNone;
    pocket_save(app, false);
    view_dispatcher_switch_to_view(app->dispatcher, PocketViewMain);
    pocket_refresh(app);
}

static bool pocket_is_move_event(const InputEvent* event) {
    return event->type == InputTypeShort || event->type == InputTypeRepeat;
}

static void pocket_handle_back(PocketD20App* app) {
    switch(app->screen) {
    case PocketScreenHome:
        pocket_save(app, false);
        view_dispatcher_stop(app->dispatcher);
        break;
    case PocketScreenRecordList:
        pocket_enter_screen(app, app->return_screen);
        break;
    case PocketScreenRecordDetail:
        pocket_enter_screen(app, PocketScreenRecordList);
        app->selection = app->record_index + 1U;
        break;
    case PocketScreenCatalog:
        pocket_enter_screen(
            app,
            app->catalog_target == PocketEditBackground ? PocketScreenCharacter :
                                                          PocketScreenRecordDetail);
        app->selection = app->catalog_return_selection;
        if(app->selection >= 5U) app->scroll = app->selection - 4U;
        break;
    case PocketScreenMagic:
        app->arcane_recovery_active = 0U;
        pocket_enter_screen(app, PocketScreenHome);
        break;
    case PocketScreenAttackList:
    case PocketScreenAttackResult:
        pocket_enter_screen(app, PocketScreenCombat);
        break;
    case PocketScreenDiceResult:
        pocket_enter_screen(app, PocketScreenDice);
        break;
    case PocketScreenInitiativeSetup:
    case PocketScreenInitiativeCombat:
        pocket_enter_screen(app, PocketScreenInitiativeMenu);
        break;
    default:
        pocket_enter_screen(app, PocketScreenHome);
        break;
    }
}

static void pocket_handle_profiles(PocketD20App* app, const InputEvent* event) {
    uint8_t profile_count = pocket_profile_count(app);
    uint16_t row_count = profile_count + 1U;
    if(pocket_is_move_event(event) && event->key == InputKeyUp)
        pocket_menu_move(app, row_count, -1);
    else if(pocket_is_move_event(event) && event->key == InputKeyDown)
        pocket_menu_move(app, row_count, 1);
    else if(event->type == InputTypeShort && event->key == InputKeyOk) {
        if(app->selection == profile_count)
            pocket_create_profile(app);
        else
            pocket_switch_profile(
                app, pocket_profile_slot_at(app, (uint8_t)app->selection));
    } else if(event->type == InputTypeLong && event->key == InputKeyOk &&
              app->selection < profile_count) {
        pocket_delete_profile(
            app, pocket_profile_slot_at(app, (uint8_t)app->selection));
    }
}

static void pocket_handle_home(PocketD20App* app, const InputEvent* event) {
    uint16_t count = sizeof(pocket_home_items) / sizeof(pocket_home_items[0]);
    if(pocket_is_move_event(event) && event->key == InputKeyUp)
        pocket_menu_move(app, count, -1);
    else if(pocket_is_move_event(event) && event->key == InputKeyDown)
        pocket_menu_move(app, count, 1);
    else if(event->type == InputTypeShort && event->key == InputKeyOk) {
        switch(app->selection) {
        case 0:
            pocket_enter_screen(app, PocketScreenProfiles);
            break;
        case 1:
            pocket_enter_screen(app, PocketScreenCharacter);
            break;
        case 2:
            pocket_enter_screen(app, PocketScreenVitals);
            break;
        case 3:
            pocket_enter_screen(app, PocketScreenAbilities);
            break;
        case 4:
            pocket_enter_screen(app, PocketScreenSkills);
            break;
        case 5:
            pocket_enter_screen(app, PocketScreenMagic);
            break;
        case 6:
            pocket_open_list(app, PocketListFeatures, PocketScreenHome);
            break;
        case 7:
            pocket_open_list(app, PocketListItems, PocketScreenHome);
            break;
        case 8:
            pocket_enter_screen(app, PocketScreenCurrency);
            break;
        case 9:
            pocket_open_list(app, PocketListJournal, PocketScreenHome);
            break;
        case 10:
            pocket_enter_screen(app, PocketScreenCombat);
            break;
        case 11:
            pocket_enter_screen(app, PocketScreenInitiativeMenu);
            break;
        case 12:
            pocket_enter_screen(app, PocketScreenDice);
            break;
        case 13:
            pocket_save(app, true);
            break;
        case 14:
            pocket_enter_screen(app, PocketScreenAbout);
            break;
        }
    }
}

static void pocket_handle_character(PocketD20App* app, const InputEvent* event) {
    PocketCharacter* character = &app->data.character;
    if(pocket_is_move_event(event) && event->key == InputKeyUp)
        pocket_menu_move(app, 12U, -1);
    else if(pocket_is_move_event(event) && event->key == InputKeyDown)
        pocket_menu_move(app, 12U, 1);
    else if(pocket_is_move_event(event) &&
            (event->key == InputKeyLeft || event->key == InputKeyRight)) {
        int8_t delta = event->key == InputKeyRight ? 1 : -1;
        if(app->selection == 7U) {
            int64_t value = (int64_t)character->experience + (delta * 100);
            if(value < 0) value = 0;
            if(value > 1000000) value = 1000000;
            character->experience = (uint32_t)value;
            pocket_save(app, false);
        } else if(app->selection == 8U) {
            character->milestone_leveling = !character->milestone_leveling;
            pocket_save(app, false);
        } else if(app->selection == 11U) {
            character->inspiration = !character->inspiration;
            pocket_save(app, false);
        }
    } else if(event->type == InputTypeLong && event->key == InputKeyOk &&
              app->selection == 3U) {
        pocket_begin_text(app, PocketEditBackground, "Custom background", character->background);
    } else if(event->type == InputTypeShort && event->key == InputKeyOk) {
        switch(app->selection) {
        case 0:
            pocket_begin_text(app, PocketEditCharacterName, "Character name", character->name);
            break;
        case 1:
            pocket_begin_text(app, PocketEditPlayerName, "Player name", character->player);
            break;
        case 2:
            pocket_begin_text(app, PocketEditSpecies, "Species", character->species);
            break;
        case 3:
            pocket_open_catalog(
                app,
                PocketCatalogBackgrounds,
                PocketEditBackground,
                character->background);
            break;
        case 4:
            pocket_begin_text(app, PocketEditAlignment, "Alignment", character->alignment);
            break;
        case 5:
            pocket_open_list(app, PocketListClasses, PocketScreenCharacter);
            break;
        case 7:
            character->experience += 100U;
            pocket_save(app, false);
            break;
        case 8:
            character->milestone_leveling = !character->milestone_leveling;
            pocket_save(app, false);
            break;
        case 9:
            pocket_open_list(app, PocketListLanguages, PocketScreenCharacter);
            break;
        case 10:
            pocket_begin_text(
                app,
                PocketEditOtherProficiencies,
                "Other proficiencies",
                character->other_proficiencies);
            break;
        case 11:
            character->inspiration = !character->inspiration;
            pocket_save(app, false);
            break;
        default:
            break;
        }
    }
}

static void pocket_handle_vitals(PocketD20App* app, const InputEvent* event) {
    PocketCharacter* character = &app->data.character;
    if(pocket_is_move_event(event) && event->key == InputKeyUp)
        pocket_menu_move(app, 14U, -1);
    else if(pocket_is_move_event(event) && event->key == InputKeyDown)
        pocket_menu_move(app, 14U, 1);
    else if(pocket_is_move_event(event) &&
            (event->key == InputKeyLeft || event->key == InputKeyRight)) {
        int16_t delta = event->key == InputKeyRight ? 1 : -1;
        switch(app->selection) {
        case 0:
            character->hp_current = pocket_clamp_i16(character->hp_current + delta, 0, 999);
            break;
        case 1:
            character->hp_temporary = pocket_clamp_i16(character->hp_temporary + delta, 0, 999);
            break;
        case 2:
            character->armor_class = pocket_clamp_i16(character->armor_class + delta, 0, 99);
            break;
        case 3:
            character->speed = pocket_clamp_i16(character->speed + (delta * 5), 0, 255);
            break;
        case 5:
            character->initiative_misc =
                (int8_t)pocket_clamp_i16(character->initiative_misc + delta, -20, 20);
            break;
        case 6:
            character->exhaustion = pocket_clamp_u8(character->exhaustion + delta, 6U);
            break;
        case 7:
            character->death_successes =
                pocket_clamp_u8(character->death_successes + delta, 3U);
            break;
        case 8:
            character->death_failures =
                pocket_clamp_u8(character->death_failures + delta, 3U);
            break;
        case 9:
            character->hit_die = pocket_cycle_die(character->hit_die, delta, true);
            break;
        case 10:
            character->hit_dice_current =
                pocket_clamp_u8(character->hit_dice_current + delta, 20U);
            break;
        case 11:
            character->skill_misc[11U] = (int8_t)pocket_clamp_i16(
                character->skill_misc[11U] + delta, -20, 20);
            break;
        case 12:
            character->skill_misc[6U] = (int8_t)pocket_clamp_i16(
                character->skill_misc[6U] + delta, -20, 20);
            break;
        case 13:
            character->skill_misc[8U] = (int8_t)pocket_clamp_i16(
                character->skill_misc[8U] + delta, -20, 20);
            break;
        default:
            return;
        }
        pocket_save(app, false);
    } else if(event->type == InputTypeShort && event->key == InputKeyOk) {
        if(app->selection == 0U) {
            character->hp_max = pocket_clamp_i16(character->hp_max + 1, 1, 999);
            pocket_save(app, false);
        } else if(app->selection == 10U) {
            character->hit_dice_max = pocket_clamp_u8(character->hit_dice_max + 1, 20U);
            pocket_save(app, false);
        }
    }
}

static void pocket_handle_abilities(PocketD20App* app, const InputEvent* event) {
    PocketCharacter* character = &app->data.character;
    if(pocket_is_move_event(event) && event->key == InputKeyUp)
        pocket_menu_move(app, POCKET_D20_ABILITY_COUNT, -1);
    else if(pocket_is_move_event(event) && event->key == InputKeyDown)
        pocket_menu_move(app, POCKET_D20_ABILITY_COUNT, 1);
    else if(pocket_is_move_event(event) &&
            (event->key == InputKeyLeft || event->key == InputKeyRight)) {
        int8_t delta = event->key == InputKeyRight ? 1 : -1;
        uint8_t index = app->selection;
        if(app->edit_modifier_mode) {
            character->saving_throw_misc[index] = (int8_t)pocket_clamp_i16(
                character->saving_throw_misc[index] + delta, -20, 20);
        } else {
            character->ability_scores[index] = (int8_t)pocket_clamp_i16(
                character->ability_scores[index] + delta, 1, 30);
        }
        pocket_save(app, false);
    } else if(event->type == InputTypeLong && event->key == InputKeyOk) {
        app->edit_modifier_mode = !app->edit_modifier_mode;
        pocket_set_status(app, app->edit_modifier_mode ? "Editing save misc" : "Editing scores");
    } else if(event->type == InputTypeShort && event->key == InputKeyOk) {
        uint8_t index = app->selection;
        character->saving_throw_proficiency[index] =
            (character->saving_throw_proficiency[index] + 1U) % 2U;
        pocket_save(app, false);
    }
}

static void pocket_handle_skills(PocketD20App* app, const InputEvent* event) {
    PocketCharacter* character = &app->data.character;
    if(pocket_is_move_event(event) && event->key == InputKeyUp)
        pocket_menu_move(app, POCKET_D20_SKILL_COUNT, -1);
    else if(pocket_is_move_event(event) && event->key == InputKeyDown)
        pocket_menu_move(app, POCKET_D20_SKILL_COUNT, 1);
    else if(pocket_is_move_event(event) &&
            (event->key == InputKeyLeft || event->key == InputKeyRight)) {
        int8_t delta = event->key == InputKeyRight ? 1 : -1;
        uint8_t index = pocket_skill_display_order[app->selection];
        if(app->edit_modifier_mode) {
            character->skill_misc[index] = (int8_t)pocket_clamp_i16(
                character->skill_misc[index] + delta, -20, 20);
        } else {
            int16_t proficiency = character->skill_proficiency[index] + delta;
            if(proficiency < 0) proficiency = PocketProficiencyExpertise;
            if(proficiency > PocketProficiencyExpertise) proficiency = PocketProficiencyNone;
            character->skill_proficiency[index] = (uint8_t)proficiency;
        }
        pocket_save(app, false);
    } else if(event->type == InputTypeLong && event->key == InputKeyOk) {
        app->edit_modifier_mode = !app->edit_modifier_mode;
        pocket_set_status(app, app->edit_modifier_mode ? "Editing skill misc" : "Editing proficiency");
    } else if(event->type == InputTypeShort && event->key == InputKeyOk) {
        uint8_t index = pocket_skill_display_order[app->selection];
        character->skill_proficiency[index] =
            (character->skill_proficiency[index] + 1U) % 3U;
        pocket_save(app, false);
    }
}

static void pocket_handle_magic(PocketD20App* app, const InputEvent* event) {
    PocketCharacter* character = &app->data.character;
    if(pocket_is_move_event(event) && event->key == InputKeyUp)
        pocket_menu_move(app, 16U, -1);
    else if(pocket_is_move_event(event) && event->key == InputKeyDown)
        pocket_menu_move(app, 16U, 1);
    else if(pocket_is_move_event(event) &&
            (event->key == InputKeyLeft || event->key == InputKeyRight)) {
        int8_t delta = event->key == InputKeyRight ? 1 : -1;
        if(app->arcane_recovery_active) {
            if(app->selection < 7U || app->selection > 11U) {
                pocket_set_status(app, "Choose a level 1-5 slot");
                return;
            }
            uint8_t level = app->selection - 6U;
            if(delta > 0) {
                uint8_t remaining = app->arcane_recovery_budget - app->arcane_recovery_spent;
                if(level > remaining) {
                    pocket_set_status(app, "Not enough recovery");
                    return;
                }
                if(character->spell_slots_current[level] >=
                   character->spell_slots_max[level]) {
                    pocket_set_status(app, "That slot level is full");
                    return;
                }
                ++character->spell_slots_current[level];
                ++app->arcane_recovery_restored[level];
                app->arcane_recovery_spent += level;
                character->arcane_recovery_used = 1U;
            } else {
                if(!app->arcane_recovery_restored[level]) {
                    pocket_set_status(app, "Nothing to undo here");
                    return;
                }
                --character->spell_slots_current[level];
                --app->arcane_recovery_restored[level];
                app->arcane_recovery_spent -= level;
                if(!app->arcane_recovery_spent) character->arcane_recovery_used = 0U;
            }
            pocket_save(app, false);
            pocket_set_status(app, "<> recover, row 6 done");
        } else if(app->selection == 1U) {
            int16_t ability = character->spellcasting_ability + delta;
            if(ability < 0) ability = PocketAbilityCharisma;
            if(ability > PocketAbilityCharisma) ability = PocketAbilityStrength;
            character->spellcasting_ability = (uint8_t)ability;
        } else if(app->selection == 3U) {
            character->spell_attack_misc = (int8_t)pocket_clamp_i16(
                character->spell_attack_misc + delta, -20, 20);
        } else if(app->selection == 4U) {
            character->spell_save_misc = (int8_t)pocket_clamp_i16(
                character->spell_save_misc + delta, -20, 20);
        } else if(app->selection == 5U) {
            app->edit_slot_max = !app->edit_slot_max;
            return;
        } else if(app->selection >= 7U) {
            uint8_t level = app->selection - 6U;
            uint8_t* slots = app->edit_slot_max ? character->spell_slots_max :
                                                   character->spell_slots_current;
            slots[level] = pocket_clamp_u8(slots[level] + delta, 20U);
            if(character->spell_slots_current[level] > character->spell_slots_max[level])
                character->spell_slots_current[level] = character->spell_slots_max[level];
        } else {
            return;
        }
        pocket_save(app, false);
    } else if(event->type == InputTypeShort && event->key == InputKeyOk) {
        if(app->selection == 0U)
            pocket_open_list(app, PocketListSpells, PocketScreenMagic);
        else if(app->selection == 5U)
            app->edit_slot_max = !app->edit_slot_max;
        else if(app->selection == 6U) {
            if(app->arcane_recovery_active) {
                app->arcane_recovery_active = 0U;
                pocket_set_status(
                    app,
                    app->arcane_recovery_spent ? "Arcane Recovery used" :
                                                 "Recovery skipped");
            } else if(character->arcane_recovery_used) {
                pocket_set_status(app, "Recovery already used");
            } else {
                pocket_set_status(app, "Finish a Short Rest first");
            }
        }
    }
}

static void pocket_handle_currency(PocketD20App* app, const InputEvent* event) {
    PocketCharacter* character = &app->data.character;
    int32_t* values[5] = {
        &character->currency_cp,
        &character->currency_sp,
        &character->currency_ep,
        &character->currency_gp,
        &character->currency_pp,
    };
    if(pocket_is_move_event(event) && event->key == InputKeyUp)
        pocket_menu_move(app, 5U, -1);
    else if(pocket_is_move_event(event) && event->key == InputKeyDown)
        pocket_menu_move(app, 5U, 1);
    else if(pocket_is_move_event(event) &&
            (event->key == InputKeyLeft || event->key == InputKeyRight)) {
        int64_t next = *values[app->selection] +
                       (event->key == InputKeyRight ? 1 : -1);
        if(next < 0) next = 0;
        if(next > 999999999L) next = 999999999L;
        *values[app->selection] = (int32_t)next;
        pocket_save(app, false);
    }
}

static void pocket_handle_record_list(PocketD20App* app, const InputEvent* event) {
    uint16_t count = pocket_list_count(app) + 1U;
    if(pocket_is_move_event(event) && event->key == InputKeyUp)
        pocket_menu_move(app, count, -1);
    else if(pocket_is_move_event(event) && event->key == InputKeyDown)
        pocket_menu_move(app, count, 1);
    else if(event->type == InputTypeShort && event->key == InputKeyOk) {
        if(app->selection == 0U) {
            if(!pocket_add_record(app)) pocket_set_status(app, "List is full");
        } else {
            app->record_index = app->selection - 1U;
            pocket_enter_screen(app, PocketScreenRecordDetail);
        }
    }
}

static void pocket_toggle_item_property(PocketItem* item, uint16_t property) {
    item->weapon_properties ^= property;
}

static void pocket_adjust_record(PocketD20App* app, int8_t delta) {
    PocketCharacter* character = &app->data.character;
    uint8_t index = app->record_index;
    uint8_t field = app->selection;
    switch(app->list_kind) {
    case PocketListClasses: {
        PocketClassLevel* class_level = &character->classes[index];
        if(field == 2U) {
            uint8_t total = pocket_d20_total_level(character);
            int16_t maximum = 20 - (total - class_level->level);
            class_level->level = (uint8_t)pocket_clamp_i16(
                class_level->level + delta, 1, maximum < 1 ? 1 : maximum);
        } else {
            return;
        }
        break;
    }
    case PocketListSpells: {
        PocketSpell* spell = &character->spells[index];
        if(field == 2U) {
            int16_t class_index = spell->class_index + delta;
            if(class_index < 0) class_index = character->class_count - 1U;
            if(class_index >= character->class_count) class_index = 0;
            spell->class_index = (uint8_t)class_index;
        } else if(field == 3U)
            spell->level = pocket_clamp_u8(spell->level + delta, 9U);
        else if(field == 4U) {
            character->spell_known[index] = !character->spell_known[index];
            if(!character->spell_known[index]) {
                spell->prepared = 0U;
                character->spell_always_prepared[index] = 0U;
            }
        } else if(field == 5U) {
            spell->prepared = !spell->prepared;
            if(spell->prepared) character->spell_known[index] = 1U;
        } else if(field == 6U) {
            character->spell_always_prepared[index] =
                !character->spell_always_prepared[index];
            if(character->spell_always_prepared[index])
                character->spell_known[index] = 1U;
        } else if(field == 7U) {
            spell->ritual = !spell->ritual;
        } else if(field == 8U) {
            character->spell_free_casts_current[index] = pocket_clamp_u8(
                character->spell_free_casts_current[index] + delta,
                character->spell_free_casts_max[index]);
        } else if(field == 9U) {
            character->spell_free_casts_max[index] = pocket_clamp_u8(
                character->spell_free_casts_max[index] + delta, 20U);
            if(character->spell_free_casts_current[index] >
               character->spell_free_casts_max[index])
                character->spell_free_casts_current[index] =
                    character->spell_free_casts_max[index];
        } else {
            return;
        }
        break;
    }
    case PocketListFeatures: {
        PocketFeature* feature = &character->features[index];
        if(field == 2U) {
            int16_t class_index = feature->class_index + delta;
            if(class_index < 0) class_index = character->class_count - 1U;
            if(class_index >= character->class_count) class_index = 0;
            feature->class_index = (uint8_t)class_index;
        } else if(field == 3U) {
            feature->class_level_gained =
                pocket_clamp_u8(feature->class_level_gained + delta, 20U);
        } else if(field == 4U) {
            feature->uses_current =
                pocket_clamp_i16(feature->uses_current + delta, 0, feature->uses_max);
        } else if(field == 5U) {
            feature->uses_max = pocket_clamp_i16(feature->uses_max + delta, 0, 99);
            if(feature->uses_current > feature->uses_max)
                feature->uses_current = feature->uses_max;
        } else if(field == 6U) {
            int16_t recharge = feature->recharge + delta;
            if(recharge < PocketRechargeManual) recharge = PocketRechargeCount - 1U;
            if(recharge >= PocketRechargeCount) recharge = PocketRechargeManual;
            feature->recharge = (uint8_t)recharge;
        } else {
            return;
        }
        break;
    }
    case PocketListItems: {
        PocketItem* item = &character->items[index];
        switch(field) {
        case 2:
            item->quantity = pocket_clamp_i16(item->quantity + delta, 0, 999);
            break;
        case 3:
            item->weight_tenths = pocket_clamp_i16(item->weight_tenths + delta, 0, 9999);
            break;
        case 4:
            item->equipped = !item->equipped;
            break;
        case 5:
            item->attuned = !item->attuned;
            break;
        case 6:
            item->is_weapon = !item->is_weapon;
            break;
        case 7: {
            int16_t ability = item->attack_ability + delta;
            if(ability < 0) ability = PocketAttackAbilityBest;
            if(ability > PocketAttackAbilityBest) ability = PocketAttackAbilityAuto;
            item->attack_ability = (uint8_t)ability;
            break;
        }
        case 8:
            item->proficient = !item->proficient;
            break;
        case 9:
            item->magic_bonus = (int8_t)pocket_clamp_i16(item->magic_bonus + delta, -10, 10);
            break;
        case 10:
            item->damage_dice = pocket_clamp_u8(item->damage_dice + delta, 20U);
            break;
        case 11:
            item->damage_die = pocket_cycle_die(item->damage_die, delta, true);
            break;
        case 12:
            if(item->versatile_die)
                item->versatile_die = 0U;
            else
                item->versatile_die = pocket_cycle_die(item->damage_die, 1, true);
            break;
        case 13:
            item->versatile_die = pocket_cycle_die(
                item->versatile_die ? item->versatile_die : item->damage_die,
                delta,
                true);
            break;
        case 14:
            item->use_versatile = !item->use_versatile;
            break;
        case 15: {
            int16_t type = item->damage_type + delta;
            if(type < 0) type = PocketDamageTypeCount - 1U;
            if(type >= PocketDamageTypeCount) type = 0;
            item->damage_type = (uint8_t)type;
            break;
        }
        case 16:
            pocket_toggle_item_property(item, PocketWeaponFinesse);
            break;
        case 17:
            pocket_toggle_item_property(item, PocketWeaponRanged);
            break;
        case 18:
            pocket_toggle_item_property(item, PocketWeaponLight);
            break;
        case 19:
            pocket_toggle_item_property(item, PocketWeaponHeavy);
            break;
        case 20:
            pocket_toggle_item_property(item, PocketWeaponThrown);
            break;
        case 21:
            pocket_toggle_item_property(item, PocketWeaponAmmunition);
            break;
        case 22:
            item->add_ability_damage = !item->add_ability_damage;
            break;
        case 23:
            item->extra_dice = pocket_clamp_u8(item->extra_dice + delta, 20U);
            break;
        case 24:
            item->extra_die = pocket_cycle_die(item->extra_die, delta, true);
            break;
        case 25:
            item->ammo_current = pocket_clamp_i16(item->ammo_current + delta, 0, item->ammo_max);
            break;
        case 26:
            item->ammo_max = pocket_clamp_i16(item->ammo_max + delta, 0, 999);
            if(item->ammo_current > item->ammo_max) item->ammo_current = item->ammo_max;
            break;
        default:
            return;
        }
        break;
    }
    case PocketListJournal: {
        PocketJournalEntry* entry = &character->journal[index];
        if(field == 0U) {
            int16_t category = entry->category + delta;
            if(category < 0) category = PocketJournalCategoryCount - 1U;
            if(category >= PocketJournalCategoryCount) category = 0;
            entry->category = (uint8_t)category;
        } else if(field == 3U) {
            entry->completed = !entry->completed;
        } else if(field == 4U) {
            int16_t class_index = entry->class_index + delta;
            if(class_index < 0) class_index = character->class_count - 1U;
            if(class_index >= character->class_count) class_index = 0;
            entry->class_index = (uint8_t)class_index;
        } else {
            return;
        }
        break;
    }
    case PocketListParty:
        if(field == 1U) {
            app->data.party[index].initiative_modifier = (int8_t)pocket_clamp_i16(
                app->data.party[index].initiative_modifier + delta, -20, 20);
        } else {
            return;
        }
        break;
    case PocketListLanguages:
        return;
    }
    pocket_save(app, false);
}

static void pocket_create_item_from_journal(PocketD20App* app) {
    PocketCharacter* character = &app->data.character;
    if(character->item_count >= POCKET_D20_MAX_ITEMS) {
        pocket_set_status(app, "Inventory is full");
        return;
    }
    PocketJournalEntry* entry = &character->journal[app->record_index];
    uint8_t item_index = character->item_count++;
    PocketItem* item = &character->items[item_index];
    memset(item, 0, sizeof(*item));
    pocket_copy(item->name, sizeof(item->name), entry->title);
    pocket_copy(item->detail, sizeof(item->detail), entry->body);
    item->quantity = 1;
    item->damage_dice = 1U;
    item->damage_die = 6U;
    item->extra_die = 6U;
    item->add_ability_damage = 1U;
    app->list_kind = PocketListItems;
    app->record_index = item_index;
    pocket_save(app, false);
    pocket_enter_screen(app, PocketScreenRecordDetail);
}

static void pocket_apply_milestone_level(PocketD20App* app) {
    PocketCharacter* character = &app->data.character;
    PocketJournalEntry* entry = &character->journal[app->record_index];
    if(entry->category != PocketJournalMilestone) {
        pocket_set_status(app, "Set category Milestone");
        return;
    }
    if(entry->level_granted) {
        pocket_set_status(app, "Level already applied");
        return;
    }
    if(pocket_d20_total_level(character) >= 20U) {
        pocket_set_status(app, "Maximum total level");
        return;
    }
    if(entry->class_index >= character->class_count) entry->class_index = 0U;
    ++character->classes[entry->class_index].level;
    entry->completed = 1U;
    entry->level_granted = 1U;
    character->hit_dice_max = pocket_clamp_u8(character->hit_dice_max + 1, 20U);
    pocket_save(app, false);
    pocket_set_status(app, "Class level increased");
}

static void pocket_handle_record_detail_ok(PocketD20App* app) {
    PocketCharacter* character = &app->data.character;
    uint8_t index = app->record_index;
    uint8_t field = app->selection;
    switch(app->list_kind) {
    case PocketListClasses:
        if(field == 0U)
            pocket_open_catalog(
                app,
                PocketCatalogClasses,
                PocketEditClassName,
                character->classes[index].name);
        else if(field == 1U)
            pocket_open_catalog(
                app,
                PocketCatalogSubclasses,
                PocketEditSubclass,
                character->classes[index].subclass);
        else if(field == 2U)
            pocket_adjust_record(app, 1);
        else
            pocket_delete_record(app);
        break;
    case PocketListSpells:
        if(field == 0U)
            pocket_open_catalog(
                app,
                PocketCatalogSpells,
                PocketEditSpellName,
                character->spells[index].name);
        else if(field == 1U)
            pocket_begin_text(app, PocketEditSpellDetail, "Spell notes", character->spells[index].detail);
        else if(field < 10U)
            pocket_adjust_record(app, 1);
        else if(field == 10U) {
            if(character->spell_free_casts_current[index]) {
                --character->spell_free_casts_current[index];
                pocket_save(app, false);
                pocket_set_status(app, "Free cast used");
            } else {
                pocket_set_status(app, "No free casts left");
            }
        } else {
            pocket_delete_record(app);
        }
        break;
    case PocketListFeatures:
        if(field == 0U)
            pocket_open_catalog(
                app,
                PocketCatalogFeats,
                PocketEditFeatureName,
                character->features[index].name);
        else if(field == 1U)
            pocket_begin_text(app, PocketEditFeatureDetail, "Feature notes", character->features[index].detail);
        else if(field < 7U)
            pocket_adjust_record(app, 1);
        else
            pocket_delete_record(app);
        break;
    case PocketListItems:
        if(field == 0U)
            pocket_open_catalog(
                app,
                PocketCatalogItems,
                PocketEditItemName,
                character->items[index].name);
        else if(field == 1U)
            pocket_begin_text(app, PocketEditItemDetail, "Item notes", character->items[index].detail);
        else if(field < 27U)
            pocket_adjust_record(app, 1);
        else if(field == 28U)
            pocket_delete_record(app);
        break;
    case PocketListLanguages:
        if(field == 0U)
            pocket_begin_text(
                app,
                PocketEditLanguageName,
                "Language",
                character->languages[index]);
        else
            pocket_delete_record(app);
        break;
    case PocketListJournal:
        if(field == 0U || field == 3U || field == 4U)
            pocket_adjust_record(app, 1);
        else if(field == 1U)
            pocket_begin_text(
                app,
                PocketEditJournalTitle,
                "Journal title",
                character->journal[index].title);
        else if(field == 2U)
            pocket_begin_text(
                app,
                PocketEditJournalBody,
                "Journal note",
                character->journal[index].body);
        else if(field == 5U)
            pocket_apply_milestone_level(app);
        else if(field == 6U)
            pocket_create_item_from_journal(app);
        else
            pocket_delete_record(app);
        break;
    case PocketListParty:
        if(field == 0U)
            pocket_begin_text(
                app,
                PocketEditPartyName,
                "Party member",
                app->data.party[index].name);
        else if(field == 1U)
            pocket_adjust_record(app, 1);
        else
            pocket_delete_record(app);
        break;
    }
}

static void pocket_handle_record_detail_custom_name(PocketD20App* app) {
    PocketCharacter* character = &app->data.character;
    uint8_t index = app->record_index;
    if(app->list_kind == PocketListClasses && app->selection == 0U)
        pocket_begin_text(
            app, PocketEditClassName, "Custom class", character->classes[index].name);
    else if(app->list_kind == PocketListClasses && app->selection == 1U)
        pocket_begin_text(
            app, PocketEditSubclass, "Custom subclass", character->classes[index].subclass);
    else if(app->list_kind == PocketListSpells && app->selection == 0U)
        pocket_begin_text(
            app, PocketEditSpellName, "Custom spell", character->spells[index].name);
    else if(app->list_kind == PocketListFeatures && app->selection == 0U)
        pocket_begin_text(
            app, PocketEditFeatureName, "Custom feat/perk", character->features[index].name);
    else if(app->list_kind == PocketListItems && app->selection == 0U)
        pocket_begin_text(
            app, PocketEditItemName, "Custom item", character->items[index].name);
}

static void pocket_handle_record_detail(PocketD20App* app, const InputEvent* event) {
    uint8_t count = pocket_record_detail_count(app);
    if(pocket_is_move_event(event) && event->key == InputKeyUp)
        pocket_menu_move(app, count, -1);
    else if(pocket_is_move_event(event) && event->key == InputKeyDown)
        pocket_menu_move(app, count, 1);
    else if(pocket_is_move_event(event) &&
            (event->key == InputKeyLeft || event->key == InputKeyRight))
        pocket_adjust_record(app, event->key == InputKeyRight ? 1 : -1);
    else if(event->type == InputTypeLong && event->key == InputKeyOk)
        pocket_handle_record_detail_custom_name(app);
    else if(event->type == InputTypeShort && event->key == InputKeyOk)
        pocket_handle_record_detail_ok(app);
}

static void pocket_apply_catalog_selection(PocketD20App* app) {
    if(app->selection >= app->catalog_count) return;
    const char* selected = app->catalog_entries[app->selection];
    PocketCharacter* character = &app->data.character;
    uint8_t index = app->record_index;
    switch(app->catalog_target) {
    case PocketEditClassName:
        pocket_copy(character->classes[index].name, sizeof(character->classes[index].name), selected);
        break;
    case PocketEditSubclass:
        pocket_copy(
            character->classes[index].subclass,
            sizeof(character->classes[index].subclass),
            selected);
        break;
    case PocketEditSpellName:
        pocket_copy(character->spells[index].name, sizeof(character->spells[index].name), selected);
        if(app->catalog_has_metadata[app->selection])
            character->spells[index].level = app->catalog_levels[app->selection];
        break;
    case PocketEditFeatureName:
        pocket_copy(character->features[index].name, sizeof(character->features[index].name), selected);
        break;
    case PocketEditItemName:
        pocket_copy(character->items[index].name, sizeof(character->items[index].name), selected);
        break;
    case PocketEditBackground:
        pocket_copy(character->background, sizeof(character->background), selected);
        break;
    default:
        return;
    }
    pocket_save(app, false);
    pocket_enter_screen(
        app,
        app->catalog_target == PocketEditBackground ? PocketScreenCharacter :
                                                      PocketScreenRecordDetail);
    app->selection = app->catalog_return_selection;
    if(app->selection >= 5U) app->scroll = app->selection - 4U;
    pocket_set_status(app, "Catalog choice saved");
}

static void pocket_handle_catalog(PocketD20App* app, const InputEvent* event) {
    if(app->catalog_count && pocket_is_move_event(event) && event->key == InputKeyUp)
        pocket_menu_move(app, app->catalog_count, -1);
    else if(app->catalog_count && pocket_is_move_event(event) && event->key == InputKeyDown)
        pocket_menu_move(app, app->catalog_count, 1);
    else if(event->type == InputTypeLong && event->key == InputKeyOk &&
            app->catalog_kind == PocketCatalogSpells) {
        app->catalog_show_all = !app->catalog_show_all;
        app->catalog_count = 0U;
        app->selection = 0U;
        app->scroll = 0U;
        memset(app->catalog_has_metadata, 0, sizeof(app->catalog_has_metadata));
        pocket_catalog_add_builtins(app, app->catalog_kind);
        pocket_catalog_load_external(app, app->catalog_kind);
        pocket_set_status(app, app->catalog_show_all ? "Showing all" : "Class + level filter");
    } else if(event->type == InputTypeShort && event->key == InputKeyOk)
        pocket_apply_catalog_selection(app);
}

static void pocket_handle_combat(PocketD20App* app, const InputEvent* event) {
    PocketCharacter* character = &app->data.character;
    if(pocket_is_move_event(event) && event->key == InputKeyUp)
        pocket_menu_move(app, 10U, -1);
    else if(pocket_is_move_event(event) && event->key == InputKeyDown)
        pocket_menu_move(app, 10U, 1);
    else if(pocket_is_move_event(event) &&
            (event->key == InputKeyLeft || event->key == InputKeyRight)) {
        int16_t delta = event->key == InputKeyRight ? 1 : -1;
        if(app->selection == 2U)
            character->hp_current = pocket_clamp_i16(character->hp_current + delta, 0, 999);
        else if(app->selection == 3U)
            character->hp_temporary = pocket_clamp_i16(character->hp_temporary + delta, 0, 999);
        else if(app->selection == 7U)
            character->death_successes = pocket_clamp_u8(character->death_successes + delta, 3U);
        else if(app->selection == 8U)
            character->death_failures = pocket_clamp_u8(character->death_failures + delta, 3U);
        else if(app->selection == 9U)
            character->exhaustion = pocket_clamp_u8(character->exhaustion + delta, 6U);
        else
            return;
        pocket_save(app, false);
    } else if(event->type == InputTypeShort && event->key == InputKeyOk) {
        switch(app->selection) {
        case 0:
            pocket_enter_screen(app, PocketScreenAttackList);
            break;
        case 1:
            pocket_enter_screen(app, PocketScreenInitiativeMenu);
            break;
        case 2:
            character->hp_current = pocket_clamp_i16(character->hp_current + 1, 0, 999);
            pocket_save(app, false);
            break;
        case 3:
            character->hp_temporary = pocket_clamp_i16(character->hp_temporary + 1, 0, 999);
            pocket_save(app, false);
            break;
        case 4:
            if(character->hp_current < 1) {
                pocket_set_status(app, "Need at least 1 HP");
                break;
            }
            pocket_d20_short_rest(character);
            pocket_save(app, false);
            if(pocket_wizard_level(character) && !character->arcane_recovery_used &&
               pocket_begin_arcane_recovery(app))
                break;
            pocket_set_status(app, "Short rest applied");
            break;
        case 5:
            if(character->hp_current < 1) {
                pocket_set_status(app, "Need at least 1 HP");
            } else if(character->hp_current >= character->hp_max) {
                pocket_set_status(app, "HP already full");
            } else if(!character->hit_dice_current) {
                pocket_set_status(app, "No Hit Dice left");
            } else {
                uint8_t roll = 0U;
                int16_t healed = pocket_d20_spend_hit_die(character, &roll);
                int8_t constitution = pocket_d20_ability_modifier(
                    character->ability_scores[PocketAbilityConstitution]);
                pocket_save(app, false);
                snprintf(
                    app->status,
                    sizeof(app->status),
                    "d%u:%u %+d, healed %d",
                    character->hit_die,
                    roll,
                    constitution,
                    healed);
                pocket_start_dice_animation(app, 1U, character->hit_die);
            }
            break;
        case 6:
            if(character->hp_current < 1) {
                pocket_set_status(app, "Need at least 1 HP");
                break;
            }
            pocket_d20_long_rest(character);
            pocket_save(app, false);
            pocket_set_status(app, "Long rest applied");
            break;
        case 7:
            character->death_successes = pocket_clamp_u8(character->death_successes + 1, 3U);
            pocket_save(app, false);
            break;
        case 8:
            character->death_failures = pocket_clamp_u8(character->death_failures + 1, 3U);
            pocket_save(app, false);
            break;
        case 9:
            character->exhaustion = pocket_clamp_u8(character->exhaustion + 1, 6U);
            pocket_save(app, false);
            break;
        }
    }
}

static void pocket_roll_generic(PocketD20App* app) {
    app->dice_first = 0U;
    app->dice_second = 0U;
    app->dice_roll_value_count = 0U;
    app->dice_roll_sum = 0U;
    memset(app->dice_roll_values, 0, sizeof(app->dice_roll_values));
    if(app->roll_mode != PocketRollNormal && app->dice_count == 1U && app->dice_sides == 20U) {
        app->dice_first = (uint8_t)pocket_d20_roll_dice(1U, 20U);
        app->dice_second = (uint8_t)pocket_d20_roll_dice(1U, 20U);
        app->dice_roll_values[0] = app->dice_first;
        app->dice_roll_values[1] = app->dice_second;
        app->dice_roll_value_count = 2U;
        app->dice_roll_sum = app->dice_first + app->dice_second;
        uint8_t chosen = app->roll_mode == PocketRollAdvantage ?
                             (app->dice_first > app->dice_second ? app->dice_first : app->dice_second) :
                             (app->dice_first < app->dice_second ? app->dice_first : app->dice_second);
        app->dice_result = chosen + app->dice_modifier;
    } else {
        app->dice_roll_value_count = app->dice_count;
        app->dice_roll_sum = pocket_d20_roll_dice_values(
            app->dice_count,
            app->dice_sides,
            app->dice_roll_values,
            sizeof(app->dice_roll_values));
        if(app->dice_count == 1U) app->dice_first = app->dice_roll_values[0];
        app->dice_result = (int16_t)app->dice_roll_sum + app->dice_modifier;
    }
    pocket_enter_screen(app, PocketScreenDiceResult);
    pocket_start_dice_animation(
        app, app->dice_roll_value_count, app->dice_sides);
}

static void pocket_handle_dice(PocketD20App* app, const InputEvent* event) {
    if(pocket_is_move_event(event) && event->key == InputKeyUp)
        pocket_menu_move(app, 5U, -1);
    else if(pocket_is_move_event(event) && event->key == InputKeyDown)
        pocket_menu_move(app, 5U, 1);
    else if(pocket_is_move_event(event) &&
            (event->key == InputKeyLeft || event->key == InputKeyRight)) {
        int8_t delta = event->key == InputKeyRight ? 1 : -1;
        if(app->selection == 0U)
            app->dice_count = (uint8_t)pocket_clamp_i16(app->dice_count + delta, 1, 20);
        else if(app->selection == 1U)
            app->dice_sides = pocket_cycle_die(app->dice_sides, delta, false);
        else if(app->selection == 2U)
            app->dice_modifier = pocket_clamp_i16(app->dice_modifier + delta, -99, 99);
        else if(app->selection == 3U) {
            int16_t mode = app->roll_mode + delta;
            if(mode < 0) mode = PocketRollDisadvantage;
            if(mode > PocketRollDisadvantage) mode = PocketRollNormal;
            app->roll_mode = (PocketRollMode)mode;
            if(app->roll_mode != PocketRollNormal) {
                app->dice_count = 1U;
                app->dice_sides = 20U;
            }
        } else {
            return;
        }
        app->dice_result = 0;
        app->dice_second = 0U;
    } else if(event->type == InputTypeShort && event->key == InputKeyOk) {
        if(app->selection == 4U) pocket_roll_generic(app);
    }
}

static void pocket_handle_dice_result(PocketD20App* app, const InputEvent* event) {
    if(event->type == InputTypeShort && event->key == InputKeyOk) pocket_roll_generic(app);
}

static void pocket_roll_selected_attack(PocketD20App* app) {
    uint8_t count = pocket_weapon_count(app);
    if(count == 0U) return;
    app->attack_item_index = pocket_weapon_index(app, app->selection);
    PocketItem* item = &app->data.character.items[app->attack_item_index];
    if((item->weapon_properties & PocketWeaponAmmunition) && item->ammo_current <= 0) {
        pocket_set_status(app, "No ammunition");
        return;
    }
    if(item->weapon_properties & PocketWeaponAmmunition) {
        --item->ammo_current;
        pocket_save(app, false);
    }
    app->attack_roll = pocket_d20_roll_attack(&app->data.character, item, app->roll_mode);
    app->attack_phase = 0U;
    pocket_enter_screen(app, PocketScreenAttackResult);
    pocket_start_dice_animation(app, app->attack_roll.second_die ? 2U : 1U, 20U);
}

static void pocket_handle_attack_list(PocketD20App* app, const InputEvent* event) {
    uint8_t count = pocket_weapon_count(app);
    if(pocket_is_move_event(event) && event->key == InputKeyUp)
        pocket_menu_move(app, count, -1);
    else if(pocket_is_move_event(event) && event->key == InputKeyDown)
        pocket_menu_move(app, count, 1);
    else if(pocket_is_move_event(event) &&
            (event->key == InputKeyLeft || event->key == InputKeyRight)) {
        int16_t mode = app->roll_mode + (event->key == InputKeyRight ? 1 : -1);
        if(mode < 0) mode = PocketRollDisadvantage;
        if(mode > PocketRollDisadvantage) mode = PocketRollNormal;
        app->roll_mode = (PocketRollMode)mode;
    } else if(event->type == InputTypeShort && event->key == InputKeyOk) {
        pocket_roll_selected_attack(app);
    }
}

static void pocket_handle_attack_result(PocketD20App* app, const InputEvent* event) {
    const PocketItem* item = &app->data.character.items[app->attack_item_index];
    if(app->attack_phase == 0U) {
        if(event->type == InputTypeShort && event->key == InputKeyOk) {
            app->damage_roll = pocket_d20_roll_damage(
                &app->data.character,
                item,
                app->attack_roll.critical);
            app->attack_phase = 1U;
            app->damage_roll_page = 0U;
            uint8_t count =
                app->damage_roll.weapon_roll_count + app->damage_roll.extra_roll_count;
            if(count)
                pocket_start_dice_animation(
                    app,
                    count,
                    item->use_versatile && item->versatile_die >= 2U ? item->versatile_die :
                                                                        item->damage_die);
        } else if(event->type == InputTypeShort && event->key == InputKeyRight) {
            app->damage_roll = pocket_d20_roll_damage(&app->data.character, item, true);
            app->attack_phase = 1U;
            app->damage_roll_page = 0U;
            uint8_t count =
                app->damage_roll.weapon_roll_count + app->damage_roll.extra_roll_count;
            if(count)
                pocket_start_dice_animation(
                    app,
                    count,
                    item->use_versatile && item->versatile_die >= 2U ? item->versatile_die :
                                                                        item->damage_die);
        } else if(event->type == InputTypeShort && event->key == InputKeyUp) {
            app->attack_roll = pocket_d20_roll_attack(&app->data.character, item, app->roll_mode);
            pocket_start_dice_animation(
                app, app->attack_roll.second_die ? 2U : 1U, 20U);
        }
    } else {
        uint8_t roll_count =
            app->damage_roll.weapon_roll_count + app->damage_roll.extra_roll_count;
        uint8_t page_count = roll_count > 1U ? (roll_count + 15U) / 16U : 1U;
        if(event->type == InputTypeShort && event->key == InputKeyUp && page_count > 1U) {
            if(app->damage_roll_page == 0U)
                app->damage_roll_page = page_count - 1U;
            else
                --app->damage_roll_page;
        } else if(event->type == InputTypeShort && event->key == InputKeyDown &&
                  page_count > 1U) {
            app->damage_roll_page = (app->damage_roll_page + 1U) % page_count;
        } else if(event->type == InputTypeShort && event->key == InputKeyOk) {
            app->damage_roll = pocket_d20_roll_damage(
                &app->data.character,
                item,
                app->damage_roll.critical);
            app->damage_roll_page = 0U;
            roll_count =
                app->damage_roll.weapon_roll_count + app->damage_roll.extra_roll_count;
            if(roll_count)
                pocket_start_dice_animation(
                    app,
                    roll_count,
                    item->use_versatile && item->versatile_die >= 2U ? item->versatile_die :
                                                                        item->damage_die);
        }
    }
}

static void pocket_start_new_initiative(PocketD20App* app) {
    PocketInitiativeState* initiative = &app->data.initiative;
    memset(initiative, 0, sizeof(*initiative));
    initiative->round = 1U;
    PocketInitiativeEntry* character_entry = &initiative->entries[initiative->count++];
    pocket_copy(
        character_entry->name,
        sizeof(character_entry->name),
        app->data.character.name);
    character_entry->initiative_modifier = pocket_d20_initiative_modifier(&app->data.character);
    character_entry->is_player_character = 1U;
    for(uint8_t i = 0U;
        i < app->data.party_count && initiative->count < POCKET_D20_MAX_INITIATIVE;
        ++i) {
        PocketInitiativeEntry* entry = &initiative->entries[initiative->count++];
        pocket_copy(entry->name, sizeof(entry->name), app->data.party[i].name);
        entry->initiative_modifier = app->data.party[i].initiative_modifier;
    }
    pocket_save(app, false);
    pocket_enter_screen(app, PocketScreenInitiativeSetup);
}

static void pocket_sort_initiative(PocketInitiativeState* initiative) {
    for(uint8_t i = 1U; i < initiative->count; ++i) {
        PocketInitiativeEntry value = initiative->entries[i];
        uint8_t position = i;
        while(position > 0U &&
              initiative->entries[position - 1U].initiative_total < value.initiative_total) {
            initiative->entries[position] = initiative->entries[position - 1U];
            --position;
        }
        initiative->entries[position] = value;
    }
}

static void pocket_handle_initiative_menu(PocketD20App* app, const InputEvent* event) {
    PocketInitiativeState* initiative = &app->data.initiative;
    if(pocket_is_move_event(event) && event->key == InputKeyUp)
        pocket_menu_move(app, 5U, -1);
    else if(pocket_is_move_event(event) && event->key == InputKeyDown)
        pocket_menu_move(app, 5U, 1);
    else if(event->type == InputTypeShort && event->key == InputKeyOk) {
        if(app->selection == 0U)
            pocket_start_new_initiative(app);
        else if(app->selection == 1U) {
            if(initiative->active)
                pocket_enter_screen(app, PocketScreenInitiativeCombat);
            else
                pocket_set_status(app, "No active combat");
        } else if(app->selection == 2U)
            pocket_open_list(app, PocketListParty, PocketScreenInitiativeMenu);
        else if(app->selection == 3U) {
            if(initiative->count)
                pocket_enter_screen(app, PocketScreenInitiativeSetup);
            else
                pocket_set_status(app, "No current order");
        } else {
            memset(initiative, 0, sizeof(*initiative));
            initiative->round = 1U;
            pocket_save(app, false);
            pocket_set_status(app, "Combat ended");
        }
    }
}

static void pocket_handle_initiative_setup(PocketD20App* app, const InputEvent* event) {
    PocketInitiativeState* initiative = &app->data.initiative;
    uint16_t count = initiative->count + 2U;
    if(pocket_is_move_event(event) && event->key == InputKeyUp)
        pocket_menu_move(app, count, -1);
    else if(pocket_is_move_event(event) && event->key == InputKeyDown)
        pocket_menu_move(app, count, 1);
    else if(pocket_is_move_event(event) &&
            app->selection < initiative->count &&
            (event->key == InputKeyLeft || event->key == InputKeyRight)) {
        int16_t delta = event->key == InputKeyRight ? 1 : -1;
        PocketInitiativeEntry* entry = &initiative->entries[app->selection];
        entry->initiative_total = pocket_clamp_i16(entry->initiative_total + delta, -20, 99);
        pocket_save(app, false);
    } else if(event->type == InputTypeLong && event->key == InputKeyOk &&
              app->selection < initiative->count) {
        PocketInitiativeEntry* entry = &initiative->entries[app->selection];
        entry->initiative_total =
            (int16_t)pocket_d20_roll_dice(1U, 20U) + entry->initiative_modifier;
        pocket_save(app, false);
        pocket_start_dice_animation(app, 1U, 20U);
    } else if(event->type == InputTypeShort && event->key == InputKeyOk) {
        if(app->selection < initiative->count) {
            PocketInitiativeEntry* entry = &initiative->entries[app->selection];
            entry->initiative_total =
                (int16_t)pocket_d20_roll_dice(1U, 20U) + entry->initiative_modifier;
            pocket_save(app, false);
            pocket_start_dice_animation(app, 1U, 20U);
        } else if(app->selection == initiative->count) {
            if(initiative->count >= POCKET_D20_MAX_INITIATIVE) {
                pocket_set_status(app, "Initiative list full");
            } else {
                app->record_index = initiative->count++;
                PocketInitiativeEntry* entry = &initiative->entries[app->record_index];
                memset(entry, 0, sizeof(*entry));
                pocket_copy(entry->name, sizeof(entry->name), "Temporary");
                pocket_begin_text(
                    app,
                    PocketEditTemporaryInitiativeName,
                    "Participant name",
                    entry->name);
            }
        } else {
            pocket_sort_initiative(initiative);
            initiative->active = 1U;
            initiative->round = 1U;
            initiative->current_turn = 0U;
            pocket_save(app, false);
            pocket_enter_screen(app, PocketScreenInitiativeCombat);
        }
    }
}

static void pocket_swap_initiative(
    PocketInitiativeState* initiative,
    uint8_t first,
    uint8_t second) {
    PocketInitiativeEntry temporary = initiative->entries[first];
    initiative->entries[first] = initiative->entries[second];
    initiative->entries[second] = temporary;
    if(initiative->current_turn == first)
        initiative->current_turn = second;
    else if(initiative->current_turn == second)
        initiative->current_turn = first;
}

static void pocket_handle_initiative_combat(PocketD20App* app, const InputEvent* event) {
    PocketInitiativeState* initiative = &app->data.initiative;
    if(initiative->count == 0U) return;
    if(pocket_is_move_event(event) && event->key == InputKeyUp)
        pocket_menu_move(app, initiative->count, -1);
    else if(pocket_is_move_event(event) && event->key == InputKeyDown)
        pocket_menu_move(app, initiative->count, 1);
    else if(event->type == InputTypeShort && event->key == InputKeyOk) {
        ++initiative->current_turn;
        if(initiative->current_turn >= initiative->count) {
            initiative->current_turn = 0U;
            ++initiative->round;
            if(initiative->round == 0U) initiative->round = 1U;
        }
        app->selection = initiative->current_turn;
        if(app->selection < app->scroll) app->scroll = app->selection;
        if(app->selection >= app->scroll + 5U) app->scroll = app->selection - 4U;
        pocket_save(app, false);
    } else if(event->type == InputTypeLong && event->key == InputKeyOk) {
        initiative->current_turn = app->selection;
        pocket_save(app, false);
    } else if(event->type == InputTypeLong && event->key == InputKeyLeft &&
              app->selection > 0U) {
        pocket_swap_initiative(initiative, app->selection, app->selection - 1U);
        --app->selection;
        pocket_save(app, false);
    } else if(event->type == InputTypeLong && event->key == InputKeyRight &&
              app->selection + 1U < initiative->count) {
        pocket_swap_initiative(initiative, app->selection, app->selection + 1U);
        ++app->selection;
        pocket_save(app, false);
    }
}

static bool pocket_input_callback(InputEvent* event, void* context) {
    PocketD20App* app = context;
    if(app->dice_animating) {
        if(event->type == InputTypeShort && event->key == InputKeyBack) {
            app->dice_animating = 0U;
            furi_timer_stop(app->dice_timer);
        }
        pocket_refresh(app);
        return true;
    }
    if(event->type == InputTypeShort && event->key == InputKeyBack) {
        pocket_handle_back(app);
        pocket_refresh(app);
        return true;
    }

    switch(app->screen) {
    case PocketScreenHome:
        pocket_handle_home(app, event);
        break;
    case PocketScreenProfiles:
        pocket_handle_profiles(app, event);
        break;
    case PocketScreenCharacter:
        pocket_handle_character(app, event);
        break;
    case PocketScreenVitals:
        pocket_handle_vitals(app, event);
        break;
    case PocketScreenAbilities:
        pocket_handle_abilities(app, event);
        break;
    case PocketScreenSkills:
        pocket_handle_skills(app, event);
        break;
    case PocketScreenMagic:
        pocket_handle_magic(app, event);
        break;
    case PocketScreenCurrency:
        pocket_handle_currency(app, event);
        break;
    case PocketScreenRecordList:
        pocket_handle_record_list(app, event);
        break;
    case PocketScreenRecordDetail:
        pocket_handle_record_detail(app, event);
        break;
    case PocketScreenCatalog:
        pocket_handle_catalog(app, event);
        break;
    case PocketScreenCombat:
        pocket_handle_combat(app, event);
        break;
    case PocketScreenDice:
        pocket_handle_dice(app, event);
        break;
    case PocketScreenDiceResult:
        pocket_handle_dice_result(app, event);
        break;
    case PocketScreenAttackList:
        pocket_handle_attack_list(app, event);
        break;
    case PocketScreenAttackResult:
        pocket_handle_attack_result(app, event);
        break;
    case PocketScreenInitiativeMenu:
        pocket_handle_initiative_menu(app, event);
        break;
    case PocketScreenInitiativeSetup:
        pocket_handle_initiative_setup(app, event);
        break;
    case PocketScreenInitiativeCombat:
        pocket_handle_initiative_combat(app, event);
        break;
    case PocketScreenAbout:
        break;
    }
    if(pocket_data_fingerprint(&app->data) != app->saved_fingerprint)
        pocket_save(app, false);
    pocket_refresh(app);
    return true;
}

static bool pocket_navigation_callback(void* context) {
    PocketD20App* app = context;
    app->edit_target = PocketEditNone;
    view_dispatcher_switch_to_view(app->dispatcher, PocketViewMain);
    pocket_refresh(app);
    return true;
}

static PocketD20App* pocket_app_alloc(void) {
    PocketD20App* app = malloc(sizeof(PocketD20App));
    furi_check(app);
    memset(app, 0, sizeof(*app));

    app->gui = furi_record_open(RECORD_GUI);
    app->storage = furi_record_open(RECORD_STORAGE);
    bool profiles_loaded = pocket_d20_profiles_load(app->storage, &app->profiles);
    bool recovered_backup = false;
    bool loaded = pocket_d20_storage_load_profile(
        app->storage,
        app->profiles.active_profile,
        &app->data,
        &recovered_backup);
    if(!loaded)
        pocket_d20_storage_save_profile(
            app->storage, app->profiles.active_profile, &app->data);
    app->profiles.occupied_mask |= (uint8_t)(1U << app->profiles.active_profile);
    pocket_copy(
        app->profiles.names[app->profiles.active_profile],
        sizeof(app->profiles.names[app->profiles.active_profile]),
        app->data.character.name);
    pocket_d20_profiles_save(app->storage, &app->profiles);
    app->saved_fingerprint = pocket_data_fingerprint(&app->data);

    app->screen = PocketScreenHome;
    app->roll_mode = PocketRollNormal;
    app->dice_count = 1U;
    app->dice_sides = 20U;
    if(recovered_backup)
        pocket_set_status(app, "Backup recovered");
    else if(loaded)
        pocket_set_status(app, "Loaded");
    else if(profiles_loaded)
        pocket_set_status(app, "Fresh character");
    else
        pocket_set_status(app, "New character");

    app->dispatcher = view_dispatcher_alloc();
    view_dispatcher_set_event_callback_context(app->dispatcher, app);
    view_dispatcher_set_navigation_event_callback(app->dispatcher, pocket_navigation_callback);
    view_dispatcher_set_custom_event_callback(app->dispatcher, pocket_custom_event_callback);
    app->dice_timer =
        furi_timer_alloc(pocket_dice_timer_callback, FuriTimerTypePeriodic, app);

    app->main_view = view_alloc();
    view_allocate_model(app->main_view, ViewModelTypeLockFree, sizeof(PocketD20App*));
    PocketD20App** model = view_get_model(app->main_view);
    *model = app;
    view_commit_model(app->main_view, false);
    view_set_context(app->main_view, app);
    view_set_draw_callback(app->main_view, pocket_draw_callback);
    view_set_input_callback(app->main_view, pocket_input_callback);

    app->text_input = text_input_alloc();
    view_dispatcher_add_view(app->dispatcher, PocketViewMain, app->main_view);
    view_dispatcher_add_view(
        app->dispatcher,
        PocketViewTextInput,
        text_input_get_view(app->text_input));
    view_dispatcher_attach_to_gui(app->dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    return app;
}

static void pocket_app_free(PocketD20App* app) {
    furi_assert(app);
    pocket_save(app, false);
    view_dispatcher_remove_view(app->dispatcher, PocketViewTextInput);
    view_dispatcher_remove_view(app->dispatcher, PocketViewMain);
    text_input_free(app->text_input);
    view_free(app->main_view);
    furi_timer_stop(app->dice_timer);
    furi_timer_free(app->dice_timer);
    view_dispatcher_free(app->dispatcher);
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_GUI);
    free(app);
}

int32_t pocket_d20_app(void* context) {
    UNUSED(context);
    PocketD20App* app = pocket_app_alloc();
    view_dispatcher_switch_to_view(app->dispatcher, PocketViewMain);
    view_dispatcher_run(app->dispatcher);
    pocket_app_free(app);
    return 0;
}
