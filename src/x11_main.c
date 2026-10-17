//
// Created by jamie on 13/10/2026.
//

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

int main(int argc, char **argv) {
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

    bool is_running = true;

    while (is_running) {
        while (XPending(display) > 0) {
            XEvent event = {0};
            XNextEvent(display, &event);

            if (event.type == KeyPress) {
                is_running = false;
            }
        }
    }

    XCloseDisplay(display);
}
