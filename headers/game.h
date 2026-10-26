//
// Created by jamie on 20/10/2026.
//

#ifndef INC_2D_PLATFORMER_GAME_H
#define INC_2D_PLATFORMER_GAME_H

#include "platform.h"

extern const size_t GAME_WIDTH;
extern const size_t GAME_HEIGHT;
extern const size_t GAME_BYTES_PER_PIXEL;

typedef struct {
    uint64_t frame;
    size_t scroll_offset;   // how many rows the stripes have moved
} GameState;

void game_update(GameState *state);
void game_render(const GameState *state, const FrameBuffer *frame_buffer); // renders to a frame buffer in RGBA.

#endif //INC_2D_PLATFORMER_GAME_H
