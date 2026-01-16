#include "UI.h"
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include "Graphics.h"
#include "CASimulator.h"

// Глобальные переменные интерфейса
HWND g_hwndEditBox = nullptr;
HWND g_hwndResultBox = nullptr;
int g_cellSize = 14;
int g_selectedState = 1;
int g_currentHistoryIndex = 0;
int g_historyStep = 1; // сколько шагов перелистывает одна стрелка

// размеры и позиции элементов
int g_windowWidth = 600;
int g_windowHeight = 500;
int g_panelHeight = 120;
int g_gridWidth = 0;
int g_gridHeight = 0;

// Глобальные кисти для темной темы
static HBRUSH g_hBrushBg = nullptr;
static HBRUSH g_hBrushPanel = nullptr;
static HBRUSH g_hBrushText = nullptr;
static HBRUSH g_hBrushEdit = nullptr;
static HBRUSH g_hBrushAccent = nullptr;

// кнопки State 
static HWND g_hBtnState[3] = {nullptr, nullptr, nullptr};
static int g_activeStateBtn = 1; // по умолчанию

// метки для окон
static HWND g_hLabelEditor = nullptr;
static HWND g_hLabelResult = nullptr;

// внешняя переменная (определена в main.cpp)
extern CASimulator g_simulator;

// CALLBACK ДЛЯ левого окна
LRESULT CALLBACK EditBoxProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);

            // рисуем на невидимый буфер
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbm = CreateCompatibleBitmap(hdc, rc.right - rc.left, rc.bottom - rc.top);
            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbm);

            // Заполнение фона темным цветом
            HBRUSH hBrushBg = CreateSolidBrush(RGB(30, 41, 59));
            FillRect(hdcMem, &rc, hBrushBg);
            DeleteObject(hBrushBg);

            // Рисование сетки
            if (!g_simulator.current.empty()) {
                DrawGrid(hdcMem, g_simulator.current, g_cellSize, g_simulator.algorithm);
            }

            // копируем буфер на экран
            BitBlt(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, hdcMem, 0, 0, SRCCOPY);
            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbm);
            DeleteDC(hdcMem);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_LBUTTONDOWN: {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            int col = x / g_cellSize;
            int row = y / g_cellSize;
            if (row >= 0 && row < g_simulator.rows && col >= 0 && col < g_simulator.cols) {
                g_simulator.current[row][col] = g_selectedState;
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;
        }

        case WM_MOUSEMOVE: {
            if (wParam & MK_LBUTTON) {
                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);
                int col = x / g_cellSize;
                int row = y / g_cellSize;
                if (row >= 0 && row < g_simulator.rows && col >= 0 && col < g_simulator.cols) {
                    g_simulator.current[row][col] = g_selectedState;
                    InvalidateRect(hwnd, nullptr, FALSE);
                }
            }
            return 0;
        }

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}


// CALLBACK для правого окна

LRESULT CALLBACK ResultBoxProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);

            // Двойная буферизация
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbm = CreateCompatibleBitmap(hdc, rc.right - rc.left, rc.bottom - rc.top);
            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbm);

            // Заполнение фона
            HBRUSH hBrushBg = CreateSolidBrush(RGB(30, 41, 59));
            FillRect(hdcMem, &rc, hBrushBg);
            DeleteObject(hBrushBg);

            // Рисование сетки
            if (!g_simulator.history.empty() && g_currentHistoryIndex < (int)g_simulator.history.size()) {
                DrawGrid(hdcMem, g_simulator.history[g_currentHistoryIndex].grid, g_cellSize, g_simulator.algorithm);
            }

            // Копируем буфер на экран 
            BitBlt(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, hdcMem, 0, 0, SRCCOPY);
            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbm);
            DeleteDC(hdcMem);
            EndPaint(hwnd, &ps);
            return 0;
        }

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

// CALLBAC(кнопки и управление)

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {

            // 1. Выбор алгоритма
            CreateWindowA("STATIC", "Algorithm:", WS_CHILD | WS_VISIBLE,
                10, 10, 70, 20, hwnd, nullptr, nullptr, nullptr);
            HWND hCombo = CreateWindowA("COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
                85, 10, 120, 150, hwnd, (HMENU)1001, nullptr, nullptr);
            SendMessageA(hCombo, CB_ADDSTRING, 0, (LPARAM)"Rule 30");
            SendMessageA(hCombo, CB_ADDSTRING, 0, (LPARAM)"Game of Life");
            SendMessageA(hCombo, CB_ADDSTRING, 0, (LPARAM)"Traffic");
            SendMessageA(hCombo, CB_ADDSTRING, 0, (LPARAM)"Epidemic");
            SendMessageA(hCombo, CB_ADDSTRING, 0, (LPARAM)"Forest Fire");
            SendMessageA(hCombo, CB_SETCURSEL, 0, 0);

            // 2. Размер сетки
            CreateWindowA("STATIC", "Size:", WS_CHILD | WS_VISIBLE, 207, 10, 40, 20, hwnd, nullptr, nullptr, nullptr);
            CreateWindowA("EDIT", "20", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                243, 10, 40, 20, hwnd, (HMENU)1002, nullptr, nullptr);

            // 3. Кнопки управления сеткой
            CreateWindowA("BUTTON", "[New]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                390, 10, 50, 20, hwnd, (HMENU)1003, nullptr, nullptr);
            CreateWindowA("BUTTON", "[Rand]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                445, 10, 50, 20, hwnd, (HMENU)1004, nullptr, nullptr);
            CreateWindowA("BUTTON", "[Clear]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                500, 10, 50, 20, hwnd, (HMENU)1005, nullptr, nullptr);
            // Кнопки state
            for (int i = 0; i < 3; i++) {
                char label[4];
                snprintf(label, sizeof(label), "[S%d]", i);
                
                g_hBtnState[i] = CreateWindowA("BUTTON", label,
                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    10 + i * 35, 40,  // x = 10, 45, 80
                    30, 20,           // 30x20 каждая кнопка
                    hwnd, (HMENU)(2000 + i), nullptr, nullptr);
            }

            // 4. Количество шагов и запуск
            CreateWindowA("STATIC", "Steps:", WS_CHILD | WS_VISIBLE, 145, 40, 40, 20, hwnd, nullptr, nullptr, nullptr);
            CreateWindowA("EDIT", "50", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                190, 40, 40, 20, hwnd, (HMENU)1006, nullptr, nullptr);
            CreateWindowA("BUTTON", "[RUN]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                235, 40, 45, 20, hwnd, (HMENU)1007, nullptr, nullptr);

            // 4b. Шаг стрелок по истории
            CreateWindowA("STATIC", "Step jump:", WS_CHILD | WS_VISIBLE,
                285, 10, 70, 20, hwnd, nullptr, nullptr, nullptr);
            CreateWindowA("EDIT", "1", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                358, 10, 30, 20, hwnd, (HMENU)1014, nullptr, nullptr);

            // 5. Навигация по истории - ИСПРАВЛЕНЫ ID!
            CreateWindowA("BUTTON", "[<]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                285, 40, 30, 20, hwnd, (HMENU)3000, nullptr, nullptr);  // ID 3000
            CreateWindowA("BUTTON", "[>]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                320, 40, 30, 20, hwnd, (HMENU)3001, nullptr, nullptr);  // ID 3001
            CreateWindowA("STATIC", "Gen: 0", WS_CHILD | WS_VISIBLE,
                355, 40, 70, 20, hwnd, (HMENU)1011, nullptr, nullptr);

            // 6. Load и Save
            CreateWindowA("BUTTON", "[Load]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                430, 40, 50, 20, hwnd, (HMENU)1012, nullptr, nullptr);
            CreateWindowA("BUTTON", "[Save]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                485, 40, 50, 20, hwnd, (HMENU)1013, nullptr, nullptr);

            // 7. Красивая линия разделения
            CreateWindowA("STATIC", "", WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
                5, 70, 535, 1, hwnd, nullptr, nullptr, nullptr);

            // 8. Метки для окон - ВЫРОВНЕНЫ ДИНАМИЧЕСКИ
            g_hLabelEditor = CreateWindowA("STATIC", "[ EDITOR - Click to draw ]",
                WS_CHILD | WS_VISIBLE | SS_CENTER, 10, 75, 260, 15, hwnd, (HMENU)2001, nullptr, nullptr);
            g_hLabelResult = CreateWindowA("STATIC", "[ RESULT - History View ]",
                WS_CHILD | WS_VISIBLE | SS_CENTER, 275, 75, 260, 15, hwnd, (HMENU)2002, nullptr, nullptr);

            // 9. Регистрация классов для окон редактора и результатов
            WNDCLASSA wc1 = {};
            wc1.lpfnWndProc = EditBoxProc;
            wc1.hInstance = GetModuleHandle(nullptr);
            HBRUSH hBrushDark = CreateSolidBrush(RGB(30, 41, 59));
            wc1.hbrBackground = hBrushDark;
            wc1.lpszClassName = "EditBoxClass";
            RegisterClassA(&wc1);

            WNDCLASSA wc2 = {};
            wc2.lpfnWndProc = ResultBoxProc;
            wc2.hInstance = GetModuleHandle(nullptr);
            wc2.hbrBackground = hBrushDark;
            wc2.lpszClassName = "ResultBoxClass";
            RegisterClassA(&wc2);

            // 10. Создание окон редактора и результатов
            g_hwndEditBox = CreateWindowA("EditBoxClass", "", WS_CHILD | WS_VISIBLE | WS_BORDER,
                10, 95, 260, 280, hwnd, nullptr, nullptr, nullptr);
            g_hwndResultBox = CreateWindowA("ResultBoxClass", "", WS_CHILD | WS_VISIBLE | WS_BORDER,
                275, 95, 260, 280, hwnd, nullptr, nullptr, nullptr);

            // 11. Инициализация симулятора
            g_simulator.Initialize(20, 20);
            g_cellSize = 280 / 20;

            // 12. Инициализация кистей для темной темы
            g_hBrushBg = CreateSolidBrush(RGB(15, 23, 42));      // #0F172A фон
            g_hBrushPanel = CreateSolidBrush(RGB(20, 30, 48));   // Немного светлее для панели
            g_hBrushText = CreateSolidBrush(RGB(15, 23, 42));    // #0F172A
            g_hBrushEdit = CreateSolidBrush(RGB(30, 41, 59));    // #1E293B
            g_hBrushAccent = CreateSolidBrush(RGB(96, 165, 250)); // #60A5FA
            break;
        }

        case WM_ERASEBKGND: {
            // Рисуем красивый фон с цветом
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(GetParent((HWND)lParam), &rc);
            
            // Основной фон
            HBRUSH hBrush1 = CreateSolidBrush(RGB(15, 23, 42));
            FillRect(hdc, &rc, hBrush1);
            DeleteObject(hBrush1);
            
            // Панель управления 
            RECT rcPanel = {0, 0, rc.right, 72};
            HBRUSH hBrushPanel = CreateSolidBrush(RGB(20, 30, 48));
            FillRect(hdc, &rcPanel, hBrushPanel);
            DeleteObject(hBrushPanel);
            
            return 1;
        }

        case WM_SIZE: {
            // Адаптивность при изменении размера окна
            int width = GET_X_LPARAM(lParam);
            int height = GET_Y_LPARAM(lParam);
            
            g_windowWidth = width;
            g_windowHeight = height;
            
            int margin = 10;
            int panelHeight = 75;
            int labelHeight = 15;
            int gridAreaHeight = height - panelHeight - labelHeight - margin * 2;
            int gridWidth = (width - margin * 3) / 2;
            
            // Пересчет размера ячейки
            if (g_simulator.rows > 0) {
                g_cellSize = gridAreaHeight / g_simulator.rows;
            }
            
            // Пересчет позиций окон сетки и меток
            MoveWindow(g_hLabelEditor, margin, panelHeight + margin, 
                      gridWidth, labelHeight, TRUE);
            MoveWindow(g_hLabelResult, gridWidth + margin * 2, panelHeight + margin, 
                      gridWidth, labelHeight, TRUE);
            
            MoveWindow(g_hwndEditBox, margin, panelHeight + labelHeight + margin, 
                      gridWidth, gridAreaHeight, TRUE);
            MoveWindow(g_hwndResultBox, gridWidth + margin * 2, panelHeight + labelHeight + margin, 
                      gridWidth, gridAreaHeight, TRUE);
            
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_COMMAND: {
            int id = LOWORD(wParam);

            // 1001: Выбор алгоритма
            if (id == 1001) {
                int sel = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
                g_simulator.algorithm = sel + 1;
                InvalidateRect(g_hwndEditBox, nullptr, FALSE);
                InvalidateRect(g_hwndResultBox, nullptr, FALSE);
            }

        
            // 1003: Создание новой сетки
            else if (id == 1003) {
                char sizeStr[10];
                GetDlgItemTextA(hwnd, 1002, sizeStr, sizeof(sizeStr));
                int size = atoi(sizeStr);

                // Разрешаем от 1 до 100
                if (size > 0 && size <= 100) {
                    g_simulator.Initialize(size, size);

                    RECT rc;
                    GetClientRect(g_hwndEditBox, &rc);

                    // Выбираем минимальный размер по ширине/высоте, чтобы всё вошло
                    int maxCellW = (rc.right  - rc.left) / size;
                    int maxCellH = (rc.bottom - rc.top ) / size;
                    g_cellSize = (maxCellW < maxCellH ? maxCellW : maxCellH);
                    if (g_cellSize < 1) g_cellSize = 1; // защита от слишком маленького окна

                    g_currentHistoryIndex = 0;
                    InvalidateRect(g_hwndEditBox, nullptr, FALSE);
                    InvalidateRect(g_hwndResultBox, nullptr, FALSE);
                }
            }


            // 1004: Случайная генерация
            else if (id == 1004) {
                for (int i = 0; i < g_simulator.rows; i++) {
                    for (int j = 0; j < g_simulator.cols; j++) {
                        g_simulator.current[i][j] = rand() % 2;
                    }
                }
                InvalidateRect(g_hwndEditBox, nullptr, FALSE);
            }

            // 1005: Очистка сетки
            else if (id == 1005) {
                int currentSize = g_simulator.rows;
                g_simulator.Initialize(currentSize, currentSize);
                g_simulator.SaveState();
                g_currentHistoryIndex = 0;
                InvalidateRect(g_hwndEditBox, nullptr, FALSE);
                InvalidateRect(g_hwndResultBox, nullptr, FALSE);
                SetDlgItemTextA(hwnd, 1011, "Gen: 0");
            }

            // 1012: Загрузка файла
            else if (id == 1012) {
                char filename[MAX_PATH] = "";
                OPENFILENAMEA ofn = {};
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = filename;
                ofn.nMaxFile = sizeof(filename);
                ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                if (GetOpenFileNameA(&ofn)) {
                    if (g_simulator.LoadFromFile(filename)) {
                        RECT rc;
                        GetClientRect(g_hwndEditBox, &rc);
                        g_cellSize = (rc.right - rc.left) / g_simulator.rows;
                        g_currentHistoryIndex = 0;
                        HWND hCombo = GetDlgItem(hwnd, 1001);
                        SendMessage(hCombo, CB_SETCURSEL, g_simulator.algorithm - 1, 0);
                        char sizeStr[10];
                        snprintf(sizeStr, sizeof(sizeStr), "%d", g_simulator.rows);
                        SetDlgItemTextA(hwnd, 1002, sizeStr);
                        InvalidateRect(g_hwndEditBox, nullptr, FALSE);
                        InvalidateRect(g_hwndResultBox, nullptr, FALSE);
                    }
                }
            }

            // 1013: Сохранение файла
            else if (id == 1013) {
                char filename[MAX_PATH] = "";
                OPENFILENAMEA ofn = {};
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = filename;
                ofn.nMaxFile = sizeof(filename);
                ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
                ofn.lpstrDefExt = "txt";
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

                if (GetSaveFileNameA(&ofn)) {
                    g_simulator.SaveToFile(filename);
                }
            }
            // 1014: Изменение шага для стрелок (Step jump)
            else if (id == 1014 && HIWORD(wParam) == EN_CHANGE) {
                char jumpStr[10];
                GetDlgItemTextA(hwnd, 1014, jumpStr, sizeof(jumpStr));
                int jump = atoi(jumpStr);
                if (jump <= 0) jump = 1;
                g_historyStep = jump;
            }

            // 1007: Запуск симуляции
            else if (id == 1007) {
                char stepsStr[10];
                GetDlgItemTextA(hwnd, 1006, stepsStr, sizeof(stepsStr));
                int steps = atoi(stepsStr);
                g_simulator.SaveState();
                for (int i = 0; i < steps; i++) {
                    g_simulator.Step();
                }

                g_currentHistoryIndex = 0;
                InvalidateRect(g_hwndResultBox, nullptr, FALSE);
                char genStr[20];
                snprintf(genStr, sizeof(genStr), "Gen: %d", (int)g_simulator.history.size() - 1);
                SetDlgItemTextA(hwnd, 1011, genStr);
            }

            // кнопки state
            else if (id >= 2000 && id <= 2002) {
                g_activeStateBtn = id - 2000;  // 0, 1, 2
                g_selectedState = g_activeStateBtn;
                
                // Перерисовываем все кнопки для визуального эффекта
                for (int i = 0; i < 3; i++) {
                    InvalidateRect(g_hBtnState[i], nullptr, FALSE);
                }
            }

            // 3000: Назад [<]
            else if (id == 3000) {
                if (!g_simulator.history.empty()) {
                    g_currentHistoryIndex -= g_historyStep;
                    if (g_currentHistoryIndex < 0) g_currentHistoryIndex = 0;

                    char genStr[20];
                    snprintf(genStr, sizeof(genStr), "Gen: %d", g_currentHistoryIndex);
                    SetDlgItemTextA(hwnd, 1011, genStr);
                    InvalidateRect(g_hwndResultBox, nullptr, FALSE);
                }
            }

            // 3001: Вперед [>]
            else if (id == 3001) {
                int maxIndex = (int)g_simulator.history.size() - 1;
                if (maxIndex >= 0) {
                    g_currentHistoryIndex += g_historyStep;
                    if (g_currentHistoryIndex > maxIndex) g_currentHistoryIndex = maxIndex;

                    char genStr[20];
                    snprintf(genStr, sizeof(genStr), "Gen: %d", g_currentHistoryIndex);
                    SetDlgItemTextA(hwnd, 1011, genStr);
                    InvalidateRect(g_hwndResultBox, nullptr, FALSE);
                }
            }


            break;
        }

        //обработчики цветов
        case WM_CTLCOLORDLG: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(15, 23, 42));
            SetTextColor(hdc, RGB(226, 232, 240));
            return (LRESULT)g_hBrushBg;
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(20, 30, 48));
            SetTextColor(hdc, RGB(226, 232, 240));
            return (LRESULT)g_hBrushPanel;
        }

        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(30, 41, 59));
            SetTextColor(hdc, RGB(226, 232, 240));
            return (LRESULT)g_hBrushEdit;
        }

        case WM_CTLCOLORLISTBOX: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(30, 41, 59));
            SetTextColor(hdc, RGB(96, 165, 250));
            return (LRESULT)g_hBrushEdit;
        }

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}
