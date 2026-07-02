#include "disp.h"

#include <stdio.h>
#include <string.h>

#ifndef RAYNET_PROJECT_DIR
#define RAYNET_PROJECT_DIR "."
#endif

static const char* k_default_class_name = "RaynetWindowClass";

static LRESULT CALLBACK disp_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	Display* window = (Display*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

	switch (msg) {
	case WM_NCCREATE: {
		CREATESTRUCTA* cs = (CREATESTRUCTA*)lparam;
		SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
		return DefWindowProcA(hwnd, msg, wparam, lparam);
	}
	case WM_PAINT: {
            static PAINTSTRUCT paint;
            static HDC device_context;
            device_context = BeginPaint(hwnd, &paint);
			if (window != NULL && window->frame_device_context != NULL) {
				BitBlt(device_context,
					   paint.rcPaint.left, paint.rcPaint.top,
					   paint.rcPaint.right - paint.rcPaint.left, paint.rcPaint.bottom - paint.rcPaint.top,
					   window->frame_device_context,
					   paint.rcPaint.left, paint.rcPaint.top,
					   SRCCOPY);
			}
            EndPaint(hwnd, &paint);
        } 
		break;
	case WM_CLOSE:
		if (window != NULL) {
		}
		DestroyWindow(hwnd);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProcA(hwnd, msg, wparam, lparam);
	}

	return 0;
}

int disp_init(Display* window, int width, int height, const char* title) {
	WNDCLASSEXA wc;
	RECT rect;
	DWORD style;
	char icon_path[MAX_PATH];
	void* frame_bits;

	if (window == NULL || title == NULL) {
		return 0;
	}

	memset(window, 0, sizeof(*window));
	window->instance = GetModuleHandleA(NULL);
	window->width = width;
	window->height = height;
	

	window->frame_bitmap_info.bmiHeader.biSize = sizeof(window->frame_bitmap_info.bmiHeader);
    window->frame_bitmap_info.bmiHeader.biWidth = width;
    window->frame_bitmap_info.bmiHeader.biHeight = -height;
    window->frame_bitmap_info.bmiHeader.biPlanes = 1;
    window->frame_bitmap_info.bmiHeader.biBitCount = 32;
    window->frame_bitmap_info.bmiHeader.biCompression = BI_RGB;
    window->frame_device_context = CreateCompatibleDC(0);
    if (window->frame_device_context == NULL) {
		return 0;
	}

	window->frame_bitmap = CreateDIBSection(
		window->frame_device_context,
		&window->frame_bitmap_info,
		DIB_RGB_COLORS,
		&frame_bits,
		NULL,
		0);
	if (window->frame_bitmap == NULL || frame_bits == NULL) {
		DeleteDC(window->frame_device_context);
		window->frame_device_context = NULL;
		return 0;
	}
	SelectObject(window->frame_device_context, window->frame_bitmap);
	window->frame_pixels = (uint32_t*)frame_bits;

	memset(&wc, 0, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW; 	
	wc.lpfnWndProc = disp_wndproc;
	wc.hInstance = window->instance;
	wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = k_default_class_name;

	snprintf(icon_path, sizeof(icon_path), "%s/resources/icon.ico", RAYNET_PROJECT_DIR);
	wc.hIcon = (HICON)LoadImageA(NULL, icon_path, IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
	if (wc.hIcon == NULL) {
		wc.hIcon = LoadIconA(NULL, IDI_APPLICATION);
	}
	wc.hIconSm = wc.hIcon;

	if (!RegisterClassExA(&wc)) {
		return 0;
	}

	style = WS_OVERLAPPEDWINDOW;
	rect.left = 0;
	rect.top = 0;
	rect.right = width;
	rect.bottom = height;
	AdjustWindowRect(&rect, style, FALSE);

	window->hwnd = CreateWindowExA(
		0,
		k_default_class_name,
		title,
		style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rect.right - rect.left,
		rect.bottom - rect.top,
		NULL,
		NULL,
		window->instance,
		window);

	if (window->hwnd == NULL) {
		DeleteObject(window->frame_bitmap);
		window->frame_bitmap = NULL;
		DeleteDC(window->frame_device_context);
		window->frame_device_context = NULL;
		window->frame_pixels = NULL;
		UnregisterClassA(k_default_class_name, window->instance);
		return 0;
	}

	ShowWindow(window->hwnd, SW_SHOW);
	UpdateWindow(window->hwnd);
	return 1;
}

void disp_poll_events(Display* window) {
	MSG msg;

	(void)window;
	while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}

void disp_destroy(Display* window) {
	if (window == NULL) {
		return;
	}

	if (window->frame_bitmap != NULL) {
		DeleteObject(window->frame_bitmap);
		window->frame_bitmap = NULL;
	}

	if (window->frame_device_context != NULL) {
		DeleteDC(window->frame_device_context);
		window->frame_device_context = NULL;
	}

	window->frame_pixels = NULL;

	if (window->hwnd != NULL) {
		DestroyWindow(window->hwnd);
		window->hwnd = NULL;
	}

	if (window->instance != NULL && k_default_class_name != NULL) {
		UnregisterClassA(k_default_class_name, window->instance);
	}
}
