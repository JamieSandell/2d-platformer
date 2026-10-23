//
// Created by jamie on 23/10/2026.
//

#ifndef INC_2D_PLATFORMER_COLOUR_H
#define INC_2D_PLATFORMER_COLOUR_H
#include <stdint.h>

typedef union {
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
    uint32_t rgba;
} Colour;

const Colour COLOUR_BLUE = {.r = 0, .g = 0, .b = 255, .a = 255};
const Colour COLOUR_GREEN = {.r = 0, .g = 255, .b = 0, .a = 255};
const Colour COLOUR_RED = {.r = 255, .g = 0, .b = 0, .a = 255};
#endif //INC_2D_PLATFORMER_COLOUR_H
