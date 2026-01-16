#ifndef UI_H
#define UI_H

#include <windows.h>

// Макросы для получения X и Y из LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

// Глобальные переменные интерфейса
extern HWND g_hwndEditBox;      // Окно редактора (слева)
extern HWND g_hwndResultBox;    // Окно результатов (справа)
extern int g_cellSize;          // Размер ячейки в пикселях
extern int g_selectedState;     // Выбранное состояние для рисования
extern int g_currentHistoryIndex; // Текущий индекс в истории

// Callback функции для обработки сообщений Windows
LRESULT CALLBACK EditBoxProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ResultBoxProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif 
