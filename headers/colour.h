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

#define COLOUR_BLUE (Colour){.r = 0, .g = 0, .b = 255, .a = 255}
#define COLOUR_GREEN (Colour){.r = 0, .g = 255, .b = 0, .a = 255}
#define COLOUR_RED ((Colour){.r = 255, .g = 0, .b = 0, .a = 255})
#endif //INC_2D_PLATFORMER_COLOUR_H
