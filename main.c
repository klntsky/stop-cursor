#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xinerama.h>
#include <unistd.h>

int main() {
    Display *dpy;
    Window root;
    int x, y;
    Window child;
    int win_x, win_y;
    unsigned int mask;
    XineramaScreenInfo *screens;
    int num_screens, i;
    int screen_width, screen_height;

    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        fprintf(stderr, "Unable to open display\n");
        return 1;
    }

    root = DefaultRootWindow(dpy);

    if (!XineramaIsActive(dpy)) {
        fprintf(stderr, "Xinerama is not active\n");
        return 1;
    }

    screens = XineramaQueryScreens(dpy, &num_screens);
    
    while (1) {
        XQueryPointer(dpy, root, &root, &child, &x, &y, &win_x, &win_y, &mask);
        printf("Cursor Position: x=%d, y=%d\n", x, y);

        // Find which screen (monitor) the cursor is currently on
        for (i = 0; i < num_screens; i++) {
            if (x >= screens[i].x_org && x < screens[i].x_org + screens[i].width &&
                y >= screens[i].y_org && y < screens[i].y_org + screens[i].height) {
                screen_width = screens[i].width;
                screen_height = screens[i].height;
                break;
            }
        }

        printf("Current Screen Dimensions: width=%d, height=%d\n", screen_width, screen_height);

        // Check boundaries of the current screen and adjust position if necessary
        if (x <= screens[i].x_org || y <= screens[i].y_org || x >= screens[i].x_org + screen_width - 1 || y >= screens[i].y_org + screen_height - 1) {
            if (x <= screens[i].x_org) x = screens[i].x_org + 1;
            if (y <= screens[i].y_org) y = screens[i].y_org + 1;
            if (x >= screens[i].x_org + screen_width - 1) x = screens[i].x_org + screen_width - 2;
            if (y >= screens[i].y_org + screen_height - 1) y = screens[i].y_org + screen_height - 2;

            XWarpPointer(dpy, None, root, 0, 0, 0, 0, x, y);
            printf("Cursor Warped to: x=%d, y=%d\n", x, y);
        }

        usleep(10000); // Sleep for 10 milliseconds to reduce CPU usage
    }

    XFree(screens);
    XCloseDisplay(dpy);
    return 0;
}
