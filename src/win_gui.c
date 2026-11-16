#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "core.h"

static void draw_text(HDC dc, const char *text, int x, int y, int width, int height,
                      int font_height, int weight, COLORREF color) {
    HFONT font = CreateFontA(font_height, 0, 0, 0, weight, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    HGDIOBJ previous = NULL;
    RECT rect;
    if (font != NULL) previous = SelectObject(dc, font);
    SetTextColor(dc, color);
    SetBkMode(dc, TRANSPARENT);
    rect.left = x;
    rect.top = y;
    rect.right = x + width;
    rect.bottom = y + height;
    DrawTextA(dc, text, -1, &rect, DT_LEFT | DT_TOP | DT_WORDBREAK);
    if (previous != NULL) SelectObject(dc, previous);
    if (font != NULL) DeleteObject(font);
}

static LRESULT CALLBACK wndproc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    (void)wparam;
    (void)lparam;

    if (message == WM_CREATE) {
        pl_cpu_info cpu;
        pl_benchmark benchmark = {0};
        char text[768];
        char *stored;
        (void)pl_cpu_profile(&cpu);
        (void)pl_run_benchmark(UINT64_C(1000000), &benchmark);
        (void)snprintf(text, sizeof(text),
                       "CPU vendor: %s\nCPU model: %s\nLogical processors: %u\n"
                       "Family / model: %u / %u\n\n"
                       "Bounded local benchmark: %.2f ms (1,000,000 iterations)\n\n"
                       "Static review is read-only. No firmware, BIOS, voltage, clock, "
                       "or CPU-control changes are performed.",
                       cpu.vendor, cpu.model, cpu.logical_cpus, cpu.family,
                       cpu.model_id, benchmark.elapsed_ms);
        stored = (char *)GlobalAlloc(GPTR, strlen(text) + 1u);
        if (stored != NULL) {
            (void)lstrcpyA(stored, text);
            SetWindowLongPtrA(window, GWLP_USERDATA, (LONG_PTR)stored);
        }
        return 0;
    }

    if (message == WM_ERASEBKGND) return 1;

    if (message == WM_PAINT) {
        PAINTSTRUCT paint;
        HDC dc = BeginPaint(window, &paint);
        RECT client;
        RECT header;
        RECT accent;
        RECT card;
        HBRUSH background = CreateSolidBrush(RGB(248, 250, 253));
        HBRUSH navy = CreateSolidBrush(RGB(20, 55, 105));
        HBRUSH blue = CreateSolidBrush(RGB(36, 105, 190));
        HBRUSH red = CreateSolidBrush(RGB(210, 40, 55));
        HBRUSH white = CreateSolidBrush(RGB(255, 255, 255));
        HBRUSH border = CreateSolidBrush(RGB(215, 225, 238));
        char *details = (char *)GetWindowLongPtrA(window, GWLP_USERDATA);

        GetClientRect(window, &client);
        FillRect(dc, &client, background);
        header.left = 0; header.top = 0; header.right = client.right; header.bottom = 92;
        FillRect(dc, &header, navy);
        accent.left = 0; accent.top = 92; accent.right = client.right; accent.bottom = 98;
        FillRect(dc, &accent, red);

        draw_text(dc, "PYTYLIKI", 28, 19, 320, 38, -31, FW_BOLD, RGB(255, 255, 255));
        draw_text(dc, "CPU LAB  |  NATIVE C11", 30, 58, 360, 24, -15, FW_NORMAL, RGB(220, 235, 255));

        card.left = 24; card.top = 120; card.right = client.right - 24; card.bottom = client.bottom - 24;
        FillRect(dc, &card, white);
        FrameRect(dc, &card, border);
        draw_text(dc, "SYSTEM PROFILE", 44, 138, 280, 26, -18, FW_BOLD, RGB(36, 105, 190));
        draw_text(dc, details != NULL ? details : "CPU profile unavailable.",
                  44, 174, client.right - 88, client.bottom - 205,
                  -16, FW_NORMAL, RGB(38, 48, 62));

        {
            RECT footer;
            footer.left = 24; footer.right = client.right - 24;
            footer.top = client.bottom - 18; footer.bottom = client.bottom - 12;
            FillRect(dc, &footer, blue);
        }
        if (background) DeleteObject(background);
        if (navy) DeleteObject(navy);
        if (blue) DeleteObject(blue);
        if (red) DeleteObject(red);
        if (white) DeleteObject(white);
        if (border) DeleteObject(border);
        EndPaint(window, &paint);
        return 0;
    }

    if (message == WM_DESTROY) {
        char *stored = (char *)GetWindowLongPtrA(window, GWLP_USERDATA);
        if (stored != NULL) GlobalFree(stored);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(window, message, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR command, int show) {
    WNDCLASSA wc;
    HWND window;
    MSG message;
    (void)previous;
    (void)command;

    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = wndproc;
    wc.hInstance = instance;
    wc.lpszClassName = "PytylikiCpuLab";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    if (!RegisterClassA(&wc)) return 1;

    window = CreateWindowA(wc.lpszClassName, "Pytyliki CPU Lab 0.5.0",
                           WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                           760, 520, NULL, NULL, instance, NULL);
    if (window == NULL) return 1;
    ShowWindow(window, show);
    while (GetMessageA(&message, NULL, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
    return (int)message.wParam;
}
