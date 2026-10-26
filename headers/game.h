//
// Created by jamie on 20/10/2026.
//

#ifndef INC_2D_PLATFORMER_GAME_H
#define INC_2D_PLATFORMER_GAME_H

#include "platform.h"

extern const int GAME_WIDTH;
extern const int GAME_HEIGHT;
extern const int GAME_BYTES_PER_PIXEL;

// renders to a frame buffer in RGBA.
void game_render(FrameBuffer *frame_buffer);

#endif //INC_2D_PLATFORMER_GAME_H
