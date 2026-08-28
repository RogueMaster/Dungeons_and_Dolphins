#pragma once

#include "dndolphins.h"

#include <stdbool.h>

/* Synchronize deterministic, level-derived character resources. This never
   chooses player-facing options such as feats, subclasses, learned spells,
   fighting styles, invocations, metamagic, or ASI allocation. */
bool pocket_d20_progression_sync_resources(PocketCharacter* character);
