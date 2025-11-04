#pragma once

#include <stdint.h>

namespace pcd8544 {
struct FontInfo {
    uint8_t flags;
    uint16_t first; 
    uint16_t last; 
    uint8_t gHeight; 
    uint8_t gWidth; 
    uint8_t gSpacing;
    const uint8_t* data;
};
}