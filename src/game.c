//
// Created by jamie on 13/10/2026.
//
#include <stddef.h>

#include "colour.h"
#include "game.h"

const size_t GAME_WIDTH = 320;
const size_t GAME_HEIGHT = 180;
const size_t GAME_BYTES_PER_PIXEL = 4;

void game_update(GameState *state) {
    state->frame++;

    // Move one row every 6 frames (about 10 steps per second at 60 fps)
    if (state->frame % 6 == 0) {
        state->scroll_offset++;
    }
}

void game_render(const GameState *state, const FrameBuffer *frame_buffer) {
    const size_t width = frame_buffer->width;
    const size_t height = frame_buffer->height;

    const uint32_t colours[3] = {
        COLOUR_RED.rgba,
        COLOUR_GREEN.rgba,
        COLOUR_BLUE.rgba,
    };

    uint32_t *pixel = frame_buffer->pixels;
    for (size_t y = 0; y < height; ++y) {
        const uint32_t colour = colours[(y + state->scroll_offset) % 3];

        for (size_t x = 0; x < width; ++x) {
            *pixel++ = colour;
        }
    }
}