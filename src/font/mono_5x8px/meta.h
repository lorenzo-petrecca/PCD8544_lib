#pragma once
#include <stdint.h>
#include "../FontCompact.h"
#include "../FontInfo.h"

static constexpr uint8_t FLAGS = 0x10;
static constexpr uint16_t FIRST_CODEPOINT = 32;
static constexpr uint16_t LAST_CODEPOINT = 126;
static constexpr uint8_t GLYPH_HEIGHT = 8;
static constexpr uint8_t GLYPH_WIDTH = 5;
static constexpr uint8_t GLYPH_SPACING = 1;

extern const uint8_t MONO_5x7_DATA[] FONT_PROGMEM;

static constexpr pcd8544::FontInfo MONO_5x7 {
    FLAGS,
    FIRST_CODEPOINT,
    LAST_CODEPOINT,
    GLYPH_HEIGHT,
    GLYPH_WIDTH,
    GLYPH_SPACING,
    MONO_5x7_DATA
};