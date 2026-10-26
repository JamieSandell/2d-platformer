//
// Created by jamie on 13/10/2026.
//

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>

#include <game.h>
#include <platform.h>

#define UNUSED(x) (void)(x)

static XImage *shm_image = NULL;
static uint32_t *shm_pixels = NULL;
static XShmSegmentInfo shm_segment_info;
static unsigned int current_window_width = 800;
static unsigned int current_window_height = 600;
static const unsigned int target_fps = 60;
static const double frame_duration = 1.0 / target_fps;

static bool allocate_shm_buffer(Display *display, Visual *visual, unsigned int width, unsigned int height);
static void free_shm_buffer(Display *display);
static double now_seconds(void);

static void scale_and_swizzle(
    const uint8_t *src_rgba,
    uint32_t *dst_bgra,
    size_t src_w,
    size_t src_h,
    size_t dst_w,
    size_t dst_h);

static void sleep_until(double target);

int main(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);

    Display *display = XOpenDisplay(NULL);

    if (!display) {
        fprintf(stderr, "Cannot connect to X server.\n");
        exit(EXIT_FAILURE);
    }

    if (!XShmQueryExtension(display)) {
        fprintf(stderr, "X server does not support MIT-SHM extension.\n");
        XCloseDisplay(display);
        exit(EXIT_FAILURE);
    }

    const int screen = DefaultScreen(display);
    Visual *visual = DefaultVisual(display, screen);
    GC gc = DefaultGC(display, screen);

    Window window = XCreateSimpleWindow(
        display,
        DefaultRootWindow(display), // parents
        0, 0, // x, y
        current_window_width, current_window_height, // width, height
        0, // border width
        0x00000000, // border colour
        0x00000000 // background colour
    );

    XSelectInput(display, window, ExposureMask | KeyPressMask | StructureNotifyMask);
    XMapWindow(display, window);

    if (!allocate_shm_buffer(display, visual, current_window_width, current_window_height)) {
        fprintf(stderr, "Failed to allocate shm buffer.\n");
        XCloseDisplay(display);
        return EXIT_FAILURE;
    }

    // assumes the game resolution never changes
    FrameBuffer game_frame_buffer;
    game_frame_buffer.height = GAME_HEIGHT;
    game_frame_buffer.width = GAME_WIDTH;
    game_frame_buffer.pixels = mmap(
        NULL,
        (size_t)(GAME_WIDTH * GAME_HEIGHT * GAME_BYTES_PER_PIXEL),
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0); // assumes the resolution of the game or bytes per pixel does not change

    if (game_frame_buffer.pixels == MAP_FAILED) {
        perror("mmap allocation for game_framebuffer failed");
        return EXIT_FAILURE;
    }

    Atom wm_delete = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wm_delete, 1);
    GameState game_state = {0};
    XSetForeground(display, gc, WhitePixel(display, screen));
    double fps_timer = now_seconds();
    unsigned int fps_frames = 0;
    char fps_text[32] = "FPS: --";
    bool is_running = true;
    double next_frame_time = now_seconds() + frame_duration;

    while (is_running) {
        while (XPending(display) > 0) {
            XEvent event = {0};
            XNextEvent(display, &event);

            switch (event.type) {
                case KeyPress:
                    is_running = false;
                    break;

                case ClientMessage:
                    if ((Atom)event.xclient.data.l[0] == wm_delete) {
                        is_running = false;
                    }
                    break;

                case ConfigureNotify: {
                    const XConfigureEvent xce = event.xconfigure;
                    if (xce.width != (int)current_window_width ||
                        xce.height != (int)current_window_height) {
                        allocate_shm_buffer(display, visual,
                                            (unsigned int)xce.width,
                                            (unsigned int)xce.height);
                        }
                    break;
                }

                default:
                    break;
            }
        }

        game_update(&game_state);
        game_render(&game_state, &game_frame_buffer);

        if (shm_pixels) {
            scale_and_swizzle(
                (const uint8_t *)game_frame_buffer.pixels,
                shm_pixels,
                GAME_WIDTH,
                GAME_HEIGHT,
                current_window_width,
                current_window_height);

            XShmPutImage(
                display,
                window,
                gc,
                shm_image,
                0,
                0,
                0,
                0,
                current_window_width,
                current_window_height,
                false);

            XDrawString(display, window, gc, 10, 20, fps_text, (int)strlen(fps_text));
            XFlush(display);
        }

        sleep_until(next_frame_time);
        next_frame_time += frame_duration;

        // If we fell badly behind (window drag, debugger), don't try to catch up
        const double now = now_seconds();
        if (now > next_frame_time + frame_duration) {
            next_frame_time = now + frame_duration;
        }

        fps_frames++;
        const double fps_now = now_seconds();
        if (fps_now - fps_timer >= 1.0) {
            snprintf(fps_text, sizeof fps_text, "FPS: %.1f",
                     fps_frames / (fps_now - fps_timer));
            fps_frames = 0;
            fps_timer = fps_now;
        }
    }

    free_shm_buffer(display);
    XCloseDisplay(display);
}

static bool allocate_shm_buffer(Display *display, Visual *visual, unsigned int width, unsigned int height) {
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

    if (shm_image->bits_per_pixel != 32) {
        fprintf(stderr, "Unsupported pixel format: %d bpp\n", shm_image->bits_per_pixel);
        XDestroyImage(shm_image);
        shm_image = NULL;
        return false;
    }

    shm_segment_info.shmid = shmget(
        IPC_PRIVATE,
        ((size_t)shm_image->bytes_per_line * (size_t)shm_image->height),
        IPC_CREAT | 0777);

    if (shm_segment_info.shmid == -1) {
        // Nothing attached yet, so only the image needs destroying.
        // shm_image->data is NULL here, so XDestroyImage won't free anything else.
        XDestroyImage(shm_image);
        shm_image = NULL;
        return false;
    }

    shm_segment_info.shmaddr = shmat(shm_segment_info.shmid, NULL, 0);

    if (shm_segment_info.shmaddr == (void *)-1) {
        shmctl(shm_segment_info.shmid, IPC_RMID, NULL);// Segment exists but we couldn't map it, so, remove it, destroy the image.
        shm_segment_info.shmaddr = NULL;
        XDestroyImage(shm_image);
        shm_image = NULL;
        return false;
    }

    shm_image->data = shm_segment_info.shmaddr;
    shm_segment_info.readOnly = False;

    if (!XShmAttach(display, &shm_segment_info)) {
        // Mapped locally but the server refused, so, undo everything.
        shmdt(shm_segment_info.shmaddr);
        shmctl(shm_segment_info.shmid, IPC_RMID, NULL);
        shm_segment_info.shmaddr = NULL;
        shm_image->data = NULL;   // stop XDestroyImage freeing shared memory
        XDestroyImage(shm_image);
        shm_image = NULL;
        return false;
    }

    XSync(display, false);                           // ensure the server has attached
    shmctl(shm_segment_info.shmid, IPC_RMID, NULL);  // auto-destroyed once everyone detaches

    // Safe: shmat() returns page-aligned memory.
    shm_pixels = (uint32_t *)(void *)shm_segment_info.shmaddr;
    current_window_width = width;
    current_window_height = height;
    return true;
}

static void free_shm_buffer(Display *display) {
    if (shm_image) {
        XShmDetach(display, &shm_segment_info);
        XSync(display, false);          // make sure the server has let go first
        shmdt(shm_segment_info.shmaddr);
        shm_image->data = NULL;         // stop XDestroyImage free()ing shared memory
        XDestroyImage(shm_image);
        shm_image = NULL;
        shm_pixels = NULL;
    }
}

static double now_seconds(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec + (double)t.tv_nsec / 1e9;
}

// Nearest neighbour scaler and colour swizzler (RGBA -> BGRA)
static void scale_and_swizzle(
    const uint8_t *src_rgba,
    uint32_t *dst_bgra,
    size_t src_w,
    size_t src_h,
    size_t dst_w,
    size_t dst_h) {
    const uint32_t x_ratio = (uint32_t)(((src_w) << 16) / dst_w) + 1;
    const uint32_t y_ratio = (uint32_t)(((src_h) << 16) / dst_h) + 1;

    for (size_t y = 0; y < dst_h; y++) {
        const uint32_t src_y = (uint32_t)((y * y_ratio) >> 16);
        const uint8_t *src_row = &src_rgba[src_y * src_w * 4];
        uint32_t *dst_row = &dst_bgra[y * dst_w];

        for (size_t x = 0; x < dst_w; x++) {
            const uint32_t src_x = (uint32_t)((x * x_ratio) >> 16);
            const uint8_t *pixel = &src_row[src_x * 4];

            const uint8_t r = pixel[0];
            const uint8_t g = pixel[1];
            const uint8_t b = pixel[2];

            dst_row[x] = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
        }
    }
}

static void sleep_until(double target) {
    const double remaining = target - now_seconds();
    if (remaining > 0.0) {
        struct timespec ts;
        ts.tv_sec = (time_t)remaining;
        ts.tv_nsec = (long)((remaining - (double)ts.tv_sec) * 1e9);
        nanosleep(&ts, NULL);
    }
}