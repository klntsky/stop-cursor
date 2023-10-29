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
    int num_screens;
    int last_screen_id = -1;  // Initially set to an invalid ID
    int last_x;       // Store last cursor position relative to the display
    int prev_x = 0, prev_y = 0;  // Previous cursor position
    double speed_x, speed_y;

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

        // Calculate speed based on position difference and fixed time interval
        speed_x = abs(x - prev_x) / 10.0;  // 10 milliseconds
        speed_y = abs(y - prev_y) / 10.0;  // 10 milliseconds

        prev_x = x;
        prev_y = y;

        int left_mouse_button_down = mask & Button1Mask;
        int ctrl_key_pressed = mask & ControlMask;

        // Find which screen (monitor) the cursor is currently on
        int current_screen_id = -1;
        for (int i = 0; i < num_screens; i++) {
            if (x >= screens[i].x_org && x < screens[i].x_org + screens[i].width &&
                y >= screens[i].y_org && y < screens[i].y_org + screens[i].height) {
                current_screen_id = i;
                break;
            }
        }

        // If the left mouse button is down or speed is too high, skip the rest of the loop
        if (left_mouse_button_down) {
            usleep(10000);
            continue;
        }

        // If CTRL key is pressed, allow cursor to move to another display and update last_screen_id
        if (ctrl_key_pressed || speed_x >= 10 || speed_y >= 10) {
            last_screen_id = current_screen_id;
            usleep(10000);
            continue;
        }

        if (last_screen_id != -1 && current_screen_id != last_screen_id) {
            // If there's a change in active screen ID and CTRL isn't pressed, restore the x-coordinate of the cursor position
            XWarpPointer(dpy, None, root, 0, 0, 0, 0, last_x, y);
            printf("Cursor x-coordinate Warped back to: x=%d\n", last_x);
        } else {
            // Update the last known x-coordinate position and screen ID
            last_x = x;
            last_screen_id = current_screen_id;
        }

        usleep(10000); // Sleep for 10 milliseconds to reduce CPU usage
    }

    XFree(screens);
    XCloseDisplay(dpy);
    return 0;
}
