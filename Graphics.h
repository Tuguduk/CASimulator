#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <windows.h>
#include <vector>

// отрисовка ячейки
void DrawCell(HDC hdc, int x, int y, int size, int state, int algorithm);

// рисование сетки
void DrawGrid(HDC hdc, const std::vector<std::vector<int>>& grid, int cellSize, int algorithm);

#endif 
