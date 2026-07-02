#ifndef RAYNET_DISP_H
#define RAYNET_DISP_H

#include <windows.h>
#include <stdbool.h>
#include <stdint.h>


typedef struct {
    int width;
    int height;
    uint32_t *pixels;
} Frame;

typedef struct Display {

	HINSTANCE instance;
	HWND hwnd;

	int width;
	int height;

	BITMAPINFO frame_bitmap_info;
	HBITMAP frame_bitmap;
	HDC frame_device_context;
	uint32_t* frame_pixels;

	char buf[128];

    //properties
    bool resizable;
} Display;

int disp_init(Display* window, int width, int height, const char* title);
void disp_poll_events(Display* window);
void disp_destroy(Display* window);

#endif