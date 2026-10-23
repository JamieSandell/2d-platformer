//
// Created by jamie on 13/10/2026.
//

#include "colour.h"
#include "game.h"

void game_render(Framebuffer *frame_buffer) {
    int width = frame_buffer->width;
    int height = frame_buffer->height;

    for (int row = 0; row < height; ++row) {
        for (int column = 0; column < width; ++column) {
            uint32_t *pixel = frame_buffer->pixels;
            pixel += row * width;
            pixel += column;
            *pixel = COLOUR_RED.rgba;
        }
    }
}