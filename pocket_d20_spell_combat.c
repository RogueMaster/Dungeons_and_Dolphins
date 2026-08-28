#include "pocket_d20_spell_combat.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char* name;
    uint8_t primary_dice;
    uint8_t primary_die;
    uint8_t secondary_dice;
    uint8_t secondary_die;
    int8_t flat_bonus;
    uint8_t primary_upcast;
    uint8_t secondary_upcast;
    int8_t flat_upcast;
    uint8_t cantrip_scale;
    uint8_t add_spellcasting_modifier;
    uint8_t resolution;
} PocketSpellDamageMap;

/*
 * Direct-damage mappings for the SRD 5.2 spell catalog. Persistent spells roll one
 * damage instance; spells with multiple simultaneous damage groups retain both groups.
 * Special riders (bounces, exploding dice, repeat-turn damage, target-count changes)
 * remain player-managed rather than being silently approximated.
 */
static const PocketSpellDamageMap pocket_spell_damage_map[] = {
    {"Acid Splash", 1, 6, 0, 0, 0, 0, 0, 0, 1, 0, PocketSpellResolutionSave},
    {"Chill Touch", 1, 10, 0, 0, 0, 0, 0, 0, 1, 0, PocketSpellResolutionAttack},
    {"Eldritch Blast", 1, 10, 0, 0, 0, 0, 0, 0, 1, 0, PocketSpellResolutionAttack},
    {"Fire Bolt", 1, 10, 0, 0, 0, 0, 0, 0, 1, 0, PocketSpellResolutionAttack},
    {"Poison Spray", 1, 12, 0, 0, 0, 0, 0, 0, 1, 0, PocketSpellResolutionAttack},
    {"Produce Flame", 1, 8, 0, 0, 0, 0, 0, 0, 1, 0, PocketSpellResolutionAttack},
    {"Ray of Frost", 1, 8, 0, 0, 0, 0, 0, 0, 1, 0, PocketSpellResolutionAttack},
    {"Sacred Flame", 1, 8, 0, 0, 0, 0, 0, 0, 1, 0, PocketSpellResolutionSave},
    {"Shocking Grasp", 1, 8, 0, 0, 0, 0, 0, 0, 1, 0, PocketSpellResolutionAttack},
    {"Sorcerous Burst", 1, 8, 0, 0, 0, 0, 0, 0, 1, 0, PocketSpellResolutionAttack},
    {"Starry Wisp", 1, 8, 0, 0, 0, 0, 0, 0, 1, 0, PocketSpellResolutionAttack},
    {"Vicious Mockery", 1, 6, 0, 0, 0, 0, 0, 0, 1, 0, PocketSpellResolutionSave},

    {"Burning Hands", 3, 6, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Chromatic Orb", 3, 8, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionAttack},
    {"Dissonant Whispers", 3, 6, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Divine Smite", 2, 8, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionTriggered},
    {"Guiding Bolt", 4, 6, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionAttack},
    {"Hellish Rebuke", 2, 10, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Ice Knife", 1, 10, 2, 6, 0, 0, 1, 0, 0, 0, PocketSpellResolutionAttack},
    {"Inflict Wounds", 2, 10, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Magic Missile", 3, 4, 0, 0, 3, 1, 0, 1, 0, 0, PocketSpellResolutionAutomatic},
    {"Ray of Sickness", 2, 8, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionAttack},
    {"Searing Smite", 1, 6, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionTriggered},
    {"Thunderwave", 2, 8, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},

    {"Acid Arrow", 4, 4, 2, 4, 0, 1, 1, 0, 0, 0, PocketSpellResolutionAttack},
    {"Dragon's Breath", 3, 6, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Flame Blade", 3, 6, 0, 0, 0, 1, 0, 0, 0, 1, PocketSpellResolutionAttack},
    {"Flaming Sphere", 2, 6, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Heat Metal", 2, 8, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionAutomatic},
    {"Mind Spike", 3, 8, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Moonbeam", 2, 10, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Scorching Ray", 6, 6, 0, 0, 0, 2, 0, 0, 0, 0, PocketSpellResolutionAttack},
    {"Shatter", 3, 8, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Shining Smite", 2, 6, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionTriggered},
    {"Spike Growth", 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, PocketSpellResolutionAutomatic},
    {"Spiritual Weapon", 1, 8, 0, 0, 0, 1, 0, 0, 0, 1, PocketSpellResolutionAttack},

    {"Call Lightning", 3, 10, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Conjure Animals", 3, 10, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Fireball", 8, 6, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Glyph of Warding", 5, 8, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Lightning Bolt", 8, 6, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Spirit Guardians", 3, 8, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Vampiric Touch", 3, 6, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionAttack},
    {"Wind Wall", 4, 8, 0, 0, 0, 0, 0, 0, 0, 0, PocketSpellResolutionSave},

    {"Black Tentacles", 3, 6, 0, 0, 0, 0, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Blight", 8, 8, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Conjure Minor Elementals", 2, 8, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionTriggered},
    {"Conjure Woodland Beings", 5, 8, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Faithful Hound", 4, 8, 0, 0, 0, 0, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Fire Shield", 2, 8, 0, 0, 0, 0, 0, 0, 0, 0, PocketSpellResolutionAutomatic},
    {"Guardian of Faith", 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Ice Storm", 2, 10, 4, 6, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Phantasmal Killer", 4, 10, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Vitriolic Sphere", 10, 4, 5, 4, 0, 2, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Wall of Fire", 5, 8, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},

    {"Arcane Hand", 5, 8, 0, 0, 0, 2, 0, 0, 0, 0, PocketSpellResolutionAttack},
    {"Cloudkill", 5, 8, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Cone of Cold", 8, 8, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Conjure Elemental", 8, 8, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Contagion", 11, 8, 0, 0, 0, 0, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Flame Strike", 5, 6, 5, 6, 0, 1, 1, 0, 0, 0, PocketSpellResolutionSave},
    {"Insect Plague", 4, 10, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},

    {"Blade Barrier", 6, 10, 0, 0, 0, 0, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Chain Lightning", 10, 8, 0, 0, 0, 0, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Circle of Death", 8, 8, 0, 0, 0, 2, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Conjure Fey", 3, 12, 0, 0, 0, 1, 0, 0, 0, 1, PocketSpellResolutionAttack},
    {"Disintegrate", 10, 6, 0, 0, 40, 3, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Freezing Sphere", 10, 6, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Harm", 14, 6, 0, 0, 0, 0, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Sunbeam", 6, 8, 0, 0, 0, 0, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Wall of Ice", 10, 6, 0, 0, 0, 2, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Wall of Thorns", 7, 8, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},

    {"Arcane Sword", 4, 12, 0, 0, 0, 0, 0, 0, 0, 1, PocketSpellResolutionAttack},
    {"Conjure Celestial", 6, 12, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Delayed Blast Fireball", 12, 6, 0, 0, 0, 1, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Finger of Death", 7, 8, 0, 0, 30, 0, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Fire Storm", 7, 10, 0, 0, 0, 0, 0, 0, 0, 0, PocketSpellResolutionSave},

    {"Befuddlement", 10, 12, 0, 0, 0, 0, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Incendiary Cloud", 10, 8, 0, 0, 0, 0, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Sunburst", 12, 6, 0, 0, 0, 0, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Tsunami", 6, 10, 0, 0, 0, 0, 0, 0, 0, 0, PocketSpellResolutionSave},

    {"Meteor Swarm", 20, 6, 20, 6, 0, 0, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Storm of Vengeance", 2, 6, 0, 0, 0, 0, 0, 0, 0, 0, PocketSpellResolutionSave},
    {"Weird", 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, PocketSpellResolutionSave},
};

static uint8_t pocket_cantrip_multiplier(uint8_t character_level) {
    if(character_level >= 17U) return 4U;
    if(character_level >= 11U) return 3U;
    if(character_level >= 5U) return 2U;
    return 1U;
}

static bool pocket_parse_damage_dice(const char* text, uint8_t* dice, uint8_t* die) {
    if(!text || !dice || !die) return false;
    for(const char* cursor = text; *cursor; ++cursor) {
        if(!isdigit((unsigned char)*cursor)) continue;

        uint16_t count = 0U;
        const char* position = cursor;
        while(isdigit((unsigned char)*position)) {
            uint8_t digit = (uint8_t)(*position - '0');
            if(count > 80U / 10U || (count == 80U / 10U && digit > 80U % 10U)) {
                count = 0U;
                break;
            }
            count = (uint16_t)(count * 10U + digit);
            ++position;
        }
        if(count < 1U || (*position != 'd' && *position != 'D')) continue;
        ++position;
        if(!isdigit((unsigned char)*position)) continue;

        uint16_t sides = 0U;
        while(isdigit((unsigned char)*position)) {
            uint8_t digit = (uint8_t)(*position - '0');
            if(sides > 100U / 10U || (sides == 100U / 10U && digit > 100U % 10U)) {
                sides = 0U;
                break;
            }
            sides = (uint16_t)(sides * 10U + digit);
            ++position;
        }
        if(sides < 2U) continue;
        *dice = (uint8_t)count;
        *die = (uint8_t)sides;
        return true;
    }
    return false;
}

bool pocket_d20_spell_damage_spec(
    const PocketSpell* spell,
    uint8_t cast_level,
    uint8_t character_level,
    int8_t spellcasting_modifier,
    PocketSpellDamageSpec* output) {
    if(!spell || !output) return false;
    memset(output, 0, sizeof(*output));

    for(size_t index = 0U; index < sizeof(pocket_spell_damage_map) / sizeof(pocket_spell_damage_map[0]);
        ++index) {
        const PocketSpellDamageMap* mapped = &pocket_spell_damage_map[index];
        if(strcmp(mapped->name, spell->name) != 0) continue;
        uint8_t multiplier = mapped->cantrip_scale ? pocket_cantrip_multiplier(character_level) : 1U;
        uint8_t upcast = cast_level > spell->level ? (uint8_t)(cast_level - spell->level) : 0U;
        output->primary_dice = (uint8_t)(mapped->primary_dice * multiplier + mapped->primary_upcast * upcast);
        output->primary_die = mapped->primary_die;
        output->secondary_dice = (uint8_t)(mapped->secondary_dice * multiplier + mapped->secondary_upcast * upcast);
        output->secondary_die = mapped->secondary_die;
        output->flat_bonus = (int16_t)mapped->flat_bonus + (int16_t)mapped->flat_upcast * upcast;
        if(mapped->add_spellcasting_modifier) output->flat_bonus += spellcasting_modifier;
        output->resolution = mapped->resolution;
        return output->primary_dice || output->secondary_dice || output->flat_bonus;
    }

    if(pocket_parse_damage_dice(spell->detail, &output->primary_dice, &output->primary_die)) {
        output->resolution = PocketSpellResolutionNone;
        return true;
    }
    return false;
}
