//
// Created by jamie on 13/10/2026.
//

#include "colour.h"
#include "game.h"

const int GAME_WIDTH = 320;
const int GAME_HEIGHT = 180;
const int GAME_BYTES_PER_PIXEL = 4;

void game_render(FrameBuffer *frame_buffer) {
    const int width = frame_buffer->width;
    const int height = frame_buffer->height;

    for (int row = 0; row < height; ++row) {
        for (int column = 0; column < width; ++column) {
            uint32_t *pixel = frame_buffer->pixels;
            pixel += row * width;
            pixel += column;
            *pixel = COLOUR_RED.rgba;
        }
    }
}