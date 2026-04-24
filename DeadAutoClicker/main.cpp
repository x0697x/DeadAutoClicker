#include <windows.h>
#include <string>
#include <thread>
#include <atomic>

// Global variables
bool isClicking = false;
bool useLeftClick = true;
int clickDelay = 100; // Time between clicks in ms
std::atomic<bool> runThread(true);

// Run in background to handle clicking
void ClickerThread() {
    while (runThread) {
        if (isClicking) {
            INPUT input = { 0 };
            input.type = INPUT_MOUSE;

            // Down
            input.mi.dwFlags = useLeftClick ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
            SendInput(1, &input, sizeof(INPUT));

            // Up
            input.mi.dwFlags = useLeftClick ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP;
            SendInput(1, &input, sizeof(INPUT));

            // Wait
            Sleep(clickDelay);
        }
        else {
            Sleep(10);
        }
    }
}

// Main window
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        // Start/stop
        CreateWindowW(L"Static", L"Press F6 to start/stop\nF7 to toggle left/right click",
            WS_VISIBLE | WS_CHILD | SS_CENTER, 20, 20, 240, 50, hwnd, NULL, NULL, NULL);

        CreateWindowW(L"Static", L"v0.1.0",
            WS_VISIBLE | WS_CHILD | SS_RIGHT, 180, 130, 100, 20, hwnd, NULL, NULL, NULL);
        break;

    case WM_HOTKEY:
        if (wParam == 1) { // F6
            isClicking = !isClicking;
            MessageBeep(MB_OK);

            // Update window title
            SetWindowTextW(hwnd, isClicking ? L"DeadAutoClicker [ON]" : L"DeadAutoClicker [OFF]");
        }
        if (wParam == 2) { // F7
            useLeftClick = !useLeftClick;
            MessageBoxW(hwnd, useLeftClick ? L"Switched to left click" : L"Switched to right click", L"Settings", MB_OK);
        }
        break;

    case WM_DESTROY:
        runThread = false;
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Main entry point
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrev, PWSTR pCmdLine, int nCmdShow) {
    // Window create
    const wchar_t CLASS_NAME[] = L"AutoClickerClass";
    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Dark soon
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"DeadAutoClicker",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 325, 200, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) return 0;

    RegisterHotKey(hwnd, 1, 0, VK_F6);
    RegisterHotKey(hwnd, 2, 0, VK_F7);

    std::thread worker(ClickerThread);
    worker.detach();

    ShowWindow(hwnd, nCmdShow);

    // Loop
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Clean hotkeys
    UnregisterHotKey(hwnd, 1);
    UnregisterHotKey(hwnd, 2);

    return 0;
}