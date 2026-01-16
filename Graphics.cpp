#include "Graphics.h"

//отрисовка 1ой ячейки
void DrawCell(HDC hdc, int x, int y, int size, int state, int algorithm) {
    COLORREF color = RGB(255, 255, 255);

    // Выбор цвета в зависимости от алгоритма и состояния
    if (algorithm == 1) {
        // Правило 30: чёрный и белый
        color = state ? RGB(0, 0, 0) : RGB(255, 255, 255);
    } else if (algorithm == 2) {
        // Игра Жизни: чёрный и белый
        color = state ? RGB(0, 0, 0) : RGB(255, 255, 255);
    } else if (algorithm == 3) {
        // Трафик: красный и серый
        color = state ? RGB(200, 0, 0) : RGB(200, 200, 200);
    } else if (algorithm == 4) {
        // Эпидемия: серый (здоровый), красный (инфицированный), зелёный (выздоровел)
        if (state == 0) color = RGB(200, 200, 200);
        else if (state == 1) color = RGB(255, 0, 0);
        else color = RGB(0, 200, 0);
    } else if (algorithm == 5) {
        // Лесной пожар: серый (пусто), зелёный (лес), оранжевый (пожар)
        if (state == 0) color = RGB(200, 200, 200);
        else if (state == 1) color = RGB(34, 139, 34); 
        else color = RGB(255, 100, 0);  
    }

    // заливка прямоугольника
    HBRUSH hBrush = CreateSolidBrush(color);
    RECT rc = {x, y, x + size, y + size};
    FillRect(hdc, &rc, hBrush);
    DeleteObject(hBrush);

    // границы ячейки 
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    MoveToEx(hdc, x, y, nullptr);
    LineTo(hdc, x + size, y);
    LineTo(hdc, x + size, y + size);
    LineTo(hdc, x, y + size);
    LineTo(hdc, x, y);

    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

    //рисование сетки
    void DrawGrid(HDC hdc, const std::vector<std::vector<int>>& grid, int cellSize, int algorithm) {
        if (grid.empty()) return;

    // Заполнение фона белым цветом
    HBRUSH hBgBrush = CreateSolidBrush(RGB(255, 255, 255));
    RECT rcBg = {0, 0, (int)grid[0].size() * cellSize, (int)grid.size() * cellSize};
    FillRect(hdc, &rcBg, hBgBrush);
    DeleteObject(hBgBrush);

    // Рисование каждой ячейки
    for (int i = 0; i < (int)grid.size(); i++) {
        for (int j = 0; j < (int)grid[i].size(); j++) {
            DrawCell(hdc, j * cellSize, i * cellSize, cellSize, grid[i][j], algorithm);
        }
    }
}

