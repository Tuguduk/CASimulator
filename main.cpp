#include <windows.h>
#include <cstdlib>
#include <ctime>
#include "CASimulator.h"
#include "UI.h"

// глобальный экземпляр симулятора
CASimulator g_simulator;

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 1. Инициализация генератора случайных чисел
    srand((unsigned)time(nullptr));

    // 2. Регистрация класса главного окна
    WNDCLASSA wc = {};
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "CAInteractive";
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.style = CS_VREDRAW | CS_HREDRAW;

    if (!RegisterClassA(&wc)) {
        MessageBoxA(nullptr, "Error registering window class", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // 3. Создание главного окна
    HWND hwnd = CreateWindowA("CAInteractive",
        "Cellular Automata - 5 Algorithms",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        100, 100, 610, 480,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwnd) {
        MessageBoxA(nullptr, "Error creating main window", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Убедимся, что окно видимо
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // 4. Главный цикл обработки сообщений
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 5. Возврат кода выхода
    return (int)msg.wParam;
}
