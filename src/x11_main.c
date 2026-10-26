//
// Created by jamie on 13/10/2026.
//

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>

#include <game.h>
#include <platform.h>

#define UNUSED(x) (void)(x)

static XImage *shm_image = NULL;
static XShmSegmentInfo shm_segment_info;
static unsigned int current_window_width = 800;
static unsigned int current_window_height = 600;

bool allocate_shm_buffer(Display *display, Visual *visual, unsigned int width, unsigned int height);
void free_shm_buffer(Display *display);

int main(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);

    FrameBuffer framebuffer = {0};
    framebuffer.height = GAME_HEIGHT;
    framebuffer.width = GAME_WIDTH;
    framebuffer.pixels = mmap(
        NULL,
        (size_t)(GAME_WIDTH * GAME_HEIGHT * 4),
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1, 0
        );

    Display *display = XOpenDisplay(NULL);

    if (!display) {
        fprintf(stderr, "Cannot connect to X server.\n");
        exit(1);
    }

    Window window = XCreateSimpleWindow(
        display,
        DefaultRootWindow(display), // parents
        0, 0, // x, y
        800, 600, // width, height
        0, // border width
        0x00000000, // border colour
        0x00000000 // background colour
    );

    XSelectInput(display, window, ExposureMask | KeyPressMask);
    XMapWindow(display, window);

    // assumes the game resolution never changes
    FrameBuffer *game_frame_buffer = mmap(
        NULL,
        (size_t)(GAME_WIDTH * GAME_HEIGHT * GAME_BYTES_PER_PIXEL),
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0); // assumes the resolution of the game or bytes per pixel does not change

    if (game_frame_buffer == MAP_FAILED) {
        perror("mmap allocation for game_framebuffer failed");
        return EXIT_FAILURE;
    }

    bool is_running = true;

    while (is_running) {
        while (XPending(display) > 0) {
            XEvent event = {0};
            XNextEvent(display, &event);

            if (event.type == KeyPress) {
                is_running = false;
            }
        }

        game_render(game_frame_buffer);
    }

    XCloseDisplay(display);
}

bool allocate_shm_buffer(Display *display, Visual *visual, unsigned int width, unsigned int height) {
    free_shm_buffer(display);
    shm_image = XShmCreateImage(
        display,
        visual,
        24,
        ZPixmap,
        NULL,
        &shm_segment_info,
        width,
        height);

    if (!shm_image) {
        return false;
    }

    shm_segment_info.shmid = shmget(
        IPC_PRIVATE,
        (size_t)(shm_image->bytes_per_line * shm_image->height),
        IPC_CREAT | 0777);

    if (shm_segment_info.shmid == -1) {
        return false;
    }

    shm_segment_info.shmaddr = shmat(shm_segment_info.shmid, NULL, 0);

    if (shm_segment_info.shmaddr == (void *)-1) {
        return false;
    }

    shm_image->data = shm_segment_info.shmaddr;
    shm_segment_info.readOnly = false;

    if (!XShmAttach(display, &shm_segment_info)) {
        return false;
    }

    current_window_height = height;
    current_window_width = width;
    return true;
}

void free_shm_buffer(Display *display) {
    if (shm_image) {
        XShmDetach(display, &shm_segment_info);
        XDestroyImage(shm_image);
        shmdt(shm_segment_info.shmaddr);
        shmctl(shm_segment_info.shmid, IPC_RMID, 0);
        shm_image = NULL;
    }
}