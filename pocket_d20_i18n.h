#pragma once

/*
 * Zero-allocation translation hook. A localized build may define
 * POCKET_D20_TRANSLATE(key, fallback) before including this header.
 * The stable keys and English fallbacks are mirrored in assets/i18n/en.txt.
 */
#ifndef POCKET_D20_TRANSLATE
#define POCKET_D20_TRANSLATE(key, fallback) (fallback)
#endif

#define POCKET_D20_TR(key, fallback) POCKET_D20_TRANSLATE((key), (fallback))
