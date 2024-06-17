#include "Windows.h"
#include "stdio.h"
#include "magnification.h"
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Magnification.lib")
#include "assert.h"

#define ArrayCount(x) (sizeof((x)) / sizeof((x)[0]))

#define ScreenColorsName "Screen Colors"
#define WM_ScreenColors_ShowTray (WM_USER + 1)

struct ColorMap {
    char name[128];
    MAGCOLOREFFECT effect;
};

struct AppState {
    HICON icon;
    ColorMap colors[256];
    unsigned color_count;
};
static AppState App;

static void AddColorMap(const char *name, MAGCOLOREFFECT effect) {
    assert(App.color_count < ArrayCount(App.colors));
    ColorMap *color = App.colors + App.color_count;
    App.color_count += 1;

    strncpy(color->name, name, sizeof(color->name));
    color->effect = effect;
}

static void ChangeColor(unsigned color_index) {
    if (color_index >= App.color_count) return;

    ColorMap *color = App.colors + color_index;
    MagSetFullscreenColorEffect(&color->effect);
}

static void AddTrayIcon(HWND window, HICON icon) {
    NOTIFYICONDATAA data = {};
    data.cbSize = sizeof(data);
    data.hWnd = window,
    data.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP,
    data.uCallbackMessage = WM_ScreenColors_ShowTray,
    data.hIcon = icon,
    strncpy(data.szTip, ScreenColorsName, sizeof(data.szTip));
    assert(Shell_NotifyIconA(NIM_ADD, &data));
}

static void ShowTrayMenu(HWND window) {
    enum TrayMenu {
        TrayMenu_None,
        TrayMenu_OpenDir,
        TrayMenu_Exit,
        TrayMenu_ColorOffset,
    };

    HMENU menu = CreatePopupMenu();
    assert(menu);

    for (unsigned color_index = 0;
         color_index < App.color_count;
         color_index += 1)
    {
        ColorMap *color = App.colors + color_index;
        unsigned tray_menu_index = TrayMenu_ColorOffset + color_index;

        char buf[1024];
        snprintf(buf, sizeof(buf), "Set colors: %s", color->name);
        AppendMenuA(menu, MF_STRING, tray_menu_index, buf);
    }

    AppendMenuA(menu, MF_STRING, TrayMenu_OpenDir, "Open program directory");
    AppendMenuA(menu, MF_STRING, TrayMenu_Exit, "Exit");

    POINT mouse;
    GetCursorPos(&mouse);
    SetForegroundWindow(window);
    int command = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_NONOTIFY, mouse.x, mouse.y, 0, window, 0);

    if (command == TrayMenu_OpenDir) {
        ShellExecuteA(nullptr, "explore", ".", nullptr, nullptr, SW_NORMAL);
    } else if (command == TrayMenu_Exit) {
        ExitProcess(0);
    } else if (command >= TrayMenu_ColorOffset) {
        unsigned color_index = command - TrayMenu_ColorOffset;
        ChangeColor(color_index);
    }

    DestroyMenu(menu);
}

static LRESULT WinMessageCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    if (message == WM_CREATE) {
        AddTrayIcon(window, App.icon);
    } else if (message == WM_CLOSE) {
        ExitProcess(0);
    } else if (message == WM_ScreenColors_ShowTray && lParam == WM_RBUTTONUP) {
        ShowTrayMenu(window);
    } else {
        result = DefWindowProcW(window, message, wParam, lParam);
    }
    return result;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    App.icon = LoadIconW(GetModuleHandleW(0), MAKEINTRESOURCEW(1));

    MagInitialize();

#define WINDOW_CLASS ScreenColorsName
    WNDCLASSA window_class = {};
    window_class.style = 0;
    window_class.lpfnWndProc = WinMessageCallback;
    window_class.hInstance = hInstance;
    window_class.hIcon = App.icon;
    window_class.lpszClassName = WINDOW_CLASS;
    ATOM atom = RegisterClassA(&window_class);

    DWORD style = WS_OVERLAPPEDWINDOW;
    HWND window = CreateWindowA(WINDOW_CLASS, ScreenColorsName, style,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                400, 400,
                                0, 0, hInstance, 0);

    AddColorMap("Default", {
        1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  0.0f,  1.0f
    });
    AddColorMap("Grayscale", {
        0.3f,  0.3f,  0.3f,  0.0f,  0.0f,
        0.6f,  0.6f,  0.6f,  0.0f,  0.0f,
        0.1f,  0.1f,  0.1f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  0.0f,  1.0f
    });
    AddColorMap("Negative", {
        -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        0.0f,  -1.0f,  0.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  -1.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        1.0f,  1.0f,  1.0f,  0.0f,  1.0f
    });
    AddColorMap("Redscale", {
        0.6f,  0.0f,  0.0f,  0.0f,  0.0f,
        0.3f,  0.0f,  0.0f,  0.0f,  0.0f,
        0.1f,  0.0f,  0.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  0.0f,  1.0f
    });
    AddColorMap("Greenscale", {
        0.0f,  0.3f,  0.0f,  0.0f,  0.0f,
        0.0f,  0.6f,  0.0f,  0.0f,  0.0f,
        0.0f,  0.1f,  0.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  0.0f,  1.0f
    });
    AddColorMap("Bluescale", {
        0.0f,  0.0f,  0.1f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.3f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.6f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  0.0f,  1.0f
    });
    AddColorMap("No red", {
        0.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  0.0f,  1.0f
    });
    AddColorMap("No green", {
        1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  0.0f,  1.0f
    });
    AddColorMap("No blue (Szymon's theme)", {
        1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  0.0f,  1.0f
    });
    AddColorMap("Shift right", {
        0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
        1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  0.0f,  1.0f
    });
    AddColorMap("Shift left", {
        0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
        1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  0.0f,  1.0f
    });
    AddColorMap("Cavern fire", {
        0.6f,  0.0f,  0.0f,  0.0f,  0.0f,
        0.3f,  0.3f,  0.0f,  0.0f,  0.0f,
        0.1f,  0.0f,  0.1f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  0.0f,  1.0f
    });
    AddColorMap("Cavern candle", {
        0.3f,  0.0f,  0.0f,  0.0f,  0.0f,
        0.15f,  0.15f,  0.0f,  0.0f,  0.0f,
        0.05f,  0.0f,  0.05f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  0.0f,  1.0f
    });
    AddColorMap("Darkroom", {
        -0.6f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.3f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.1f,  0.0f,  0.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        1.0f,  0.0f,  0.0f,  0.0f,  1.0f
    });

    while (true) {
        MSG msg;
        GetMessageW(&msg, 0, 0, 0);
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    MagUninitialize();
    return 0;
}
