#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "map.h"
#include "disp.h"
#include "render.h"

#define ENET_IMPLEMENTATION
#include <enet.h>


#define Rand32() ((rand() << 16) + (rand() << 1) + (rand() & 1))

static void center_cursor(const Display* disp) {
    POINT center = { disp->width / 2, disp->height / 2 };
    ClientToScreen(disp->hwnd, &center);
    SetCursorPos(center.x, center.y);
}

static void set_mouse_capture(const Display* disp, bool capture) {
    if (capture) {
        RECT client_rect = {0};
        POINT top_left = {0, 0};
        POINT bottom_right = { disp->width, disp->height };

        SetCapture(disp->hwnd);
        GetClientRect(disp->hwnd, &client_rect);
        (void)client_rect;

        ClientToScreen(disp->hwnd, &top_left);
        ClientToScreen(disp->hwnd, &bottom_right);

        client_rect.left = top_left.x;
        client_rect.top = top_left.y;
        client_rect.right = bottom_right.x;
        client_rect.bottom = bottom_right.y;
        ClipCursor(&client_rect);

        while (ShowCursor(FALSE) >= 0) {}
        center_cursor(disp);
    } else {
        ClipCursor(NULL);
        ReleaseCapture();
        while (ShowCursor(TRUE) < 0) {}
    }
}

int main() {

    printf("starting...");

    Camera cam;


    init_camera(&cam, 103.0, 32, 1280);
    cam.pos[0] = 30.5;
    cam.pos[1] = 30.5;
    cam.pos[2] = 0.3;

    Map map = create_map(128,128);

    for (uint32_t r = 0; r < 128; r++) {
        for (uint32_t c = 0; c < 128; c++) {
            map.chunk_map[r][c].blocks[0] = Rand32() % 12 ? 0 : Rand32() % 5;
        }
    }

    for (uint32_t r = 8; r <= 15; r++) {
        for (uint32_t c = 8; c <= 15; c++) {
            map.chunk_map[r][c].blocks[0] = 3;
        }
    }

    map.chunk_map[10][9].blocks[0] = 0;
    map.chunk_map[10][10].blocks[0] = 0;
    map.chunk_map[10][11].blocks[0] = 0;
    map.chunk_map[10][12].blocks[0] = 0;
    map.chunk_map[10][8].blocks[0] = 0;
    map.chunk_map[10][13].blocks[0] = 0;
    map.chunk_map[10][14].blocks[0] = 0;

    Display disp;
    if (!disp_init(&disp, 1280, 720, "Raynet")) {
        return 1;
    }

    Frame frame = {0};
    LARGE_INTEGER perf_frequency = {0};
    LARGE_INTEGER last_counter = {0};

    frame.width = disp.width;
    frame.height = disp.height;
    frame.pixels = disp.frame_pixels;

    if (frame.pixels == NULL) {
        disp_destroy(&disp);
        return 1;
    }

    QueryPerformanceFrequency(&perf_frequency);
    QueryPerformanceCounter(&last_counter);

    bool mouse_captured = false;
    bool mouse_look_enabled = true;
    bool prev_escape_down = false;
    bool prev_lbutton_down = false;
    double fps_print_accum = 0.0;

    while(IsWindow(disp.hwnd)) {
        const double turn_speed = 3.0;
        const double move_speed = 3.8;
        const double mouse_sensitivity = 0.0012;
        LARGE_INTEGER current_counter = {0};
        double dt = 0.0;
        double next_x = cam.pos[0];
        double next_y = cam.pos[1];
        bool in_focus = false;
        bool escape_down = false;
        bool lbutton_down = false;

        disp_poll_events(&disp);

        in_focus = (GetForegroundWindow() == disp.hwnd);
        escape_down = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
        lbutton_down = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

        if (escape_down && !prev_escape_down) {
            mouse_look_enabled = false;
            if (mouse_captured) {
                set_mouse_capture(&disp, false);
                mouse_captured = false;
            }
        }

        if (!in_focus && mouse_captured) {
            set_mouse_capture(&disp, false);
            mouse_captured = false;
        }

        // Click in the window to re-enable mouse look after Escape.
        if (!mouse_look_enabled && in_focus && lbutton_down && !prev_lbutton_down) {
            mouse_look_enabled = true;
        }

        if (mouse_look_enabled && in_focus && !mouse_captured) {
            set_mouse_capture(&disp, true);
            mouse_captured = true;
        }

        prev_escape_down = escape_down;
        prev_lbutton_down = lbutton_down;

        QueryPerformanceCounter(&current_counter);
        dt = (double)(current_counter.QuadPart - last_counter.QuadPart) /
            (double)perf_frequency.QuadPart;
        last_counter = current_counter;

        if (dt < 0.0) {
            dt = 0.0;
        } else if (dt > 0.05) {
            dt = 0.05;
        }

        if (mouse_captured) {
            POINT mouse_pos = {0};
            if (GetCursorPos(&mouse_pos) && ScreenToClient(disp.hwnd, &mouse_pos)) {
                int delta_x = mouse_pos.x - (disp.width / 2);
                int delta_y = mouse_pos.y - (disp.height / 2);

                if (delta_x != 0) {cam.roll += (double)delta_x * mouse_sensitivity;}
                if (delta_y != 0) {cam.pitch -= (double)delta_y * mouse_sensitivity;}
            }
            center_cursor(&disp);
        }

        if (cam.pitch > 1.5f) {cam.pitch = 1.5f;}
        if (cam.pitch < -1.5f) {cam.pitch = -1.5f;}

        if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
            cam.roll -= turn_speed * dt;
        }

        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
            cam.roll += turn_speed * dt;
        }

        fix_angle_inplace(&cam.roll);

        if (GetAsyncKeyState('W') & 0x8000) {
            next_x += cos(cam.roll) * move_speed * dt;
            next_y -= sin(cam.roll) * move_speed * dt;
        }

        if (GetAsyncKeyState('S') & 0x8000) {
            next_x -= cos(cam.roll) * move_speed * dt;
            next_y += sin(cam.roll) * move_speed * dt;
        }
        

        if (GetAsyncKeyState('A') & 0x8000) {
            next_x += sin(cam.roll) * move_speed * dt;
            next_y += cos(cam.roll) * move_speed * dt;
        }

        if (GetAsyncKeyState('D') & 0x8000) {
            next_x -= sin(cam.roll) * move_speed * dt;
            next_y -= cos(cam.roll) * move_speed * dt;
        }
        
        if (GetAsyncKeyState(VK_UP) & 0x8000) {
            cam.pos[2] += move_speed * dt;
        }

        if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
            cam.pos[2] -= move_speed * dt;
        }


        rendercast_auto(&cam, &map, &frame);

        InvalidateRect(disp.hwnd, NULL, FALSE);
        UpdateWindow(disp.hwnd);

        memset(frame.pixels, 0, (size_t)frame.width * (size_t)frame.height * sizeof(uint32_t));

    
        cam.pos[0] = next_x;
        cam.pos[1] = next_y;

        fps_print_accum += dt;
        if (fps_print_accum >= 0.25) {
            double fps = (dt > 0.0) ? (1.0 / dt) : 0.0;
            snprintf(disp.buf, sizeof(disp.buf), "fps: %d\npos: %.2f %.2f %.2f\nangle: %.2f %.2f \n",(int)fps, cam.pos[0], cam.pos[1], cam.pos[2], cam.roll, cam.pitch);
            fps_print_accum = 0.0;
        }
    }

    if (mouse_captured) {
        set_mouse_capture(&disp, false);
    }

    return 0;

}