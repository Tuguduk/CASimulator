#include "CASimulator.h"
#include <windows.h>
#include <fstream>
#include <cstdlib>

// инициализация сим-ра
void CASimulator::Initialize(int r, int c) {
    rows = r;
    cols = c;
    current.assign(r, std::vector<int>(c, 0));
    history.clear();
    InitializeAlgorithms();
}

// инициализация алг-ма
void CASimulator::InitializeAlgorithms() {
    rule30 = std::make_unique<Rule30>();
    gameOfLife = std::make_unique<GameOfLife>();
    traffic = std::make_unique<Traffic>();
    epidemic = std::make_unique<Epidemic>();
    forestFire = std::make_unique<ForestFire>();
}


Algorithm* CASimulator::GetCurrentAlgorithm() {
    switch (algorithm) {
    case 1: return rule30.get();
    case 2: return gameOfLife.get();
    case 3: return traffic.get();
    case 4: return epidemic.get();
    case 5: return forestFire.get();
    default: return nullptr;
    }
}


void CASimulator::SaveState() {
    SimulationState state;
    state.grid = current;
    history.push_back(state);
}

void CASimulator::Step() {
    Algorithm* algo = GetCurrentAlgorithm();
    if (algo != nullptr) {
        algo->Step(current, rows, cols);
        SaveState();
    }
}

void Rule30::Step(std::vector<std::vector<int>>& grid, int rows, int cols) {
    std::vector<std::vector<int>> next(rows, std::vector<int>(cols));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int left = grid[i][(j - 1 + cols) % cols];
            int center = grid[i][j];
            int right = grid[i][(j + 1) % cols];
            next[i][j] = left ^ (center | right);
        }
    }
    grid = next;
}

int GameOfLife::CountAliveNeighbors(const std::vector<std::vector<int>>& grid, int r, int c, int rows, int cols) const {
    int count = 0;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            int nr = (r + dr + rows) % rows;
            int nc = (c + dc + cols) % cols;
            if (grid[nr][nc] >= 1) count++;
        }
    }
    return count;
}

void GameOfLife::Step(std::vector<std::vector<int>>& grid, int rows, int cols) {
    std::vector<std::vector<int>> next(rows, std::vector<int>(cols));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int neighbors = CountAliveNeighbors(grid, i, j, rows, cols);
            if (grid[i][j] >= 1) {
                next[i][j] = (neighbors == 2 || neighbors == 3) ? 1 : 0;
            } else {
                next[i][j] = (neighbors == 3) ? 1 : 0;
            }
        }
    }
    grid = next;
}



void Traffic::Step(std::vector<std::vector<int>>& grid, int rows, int cols) {
    std::vector<std::vector<int>> next(rows, std::vector<int>(cols, 0));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (grid[i][j] == 1) {
                int nextJ = (j + 1) % cols;
                if (grid[i][nextJ] == 0) {
                    next[i][nextJ] = 1;
                } else {
                    next[i][j] = 1;
                }
            }
        }
    }
    grid = next;
}


bool Epidemic::HasInfectedNeighbor(const std::vector<std::vector<int>>& grid, int r, int c, int rows, int cols) const {
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            int nr = (r + dr + rows) % rows;
            int nc = (c + dc + cols) % cols;
            if (grid[nr][nc] == 1) return true;
        }
    }
    return false;
}

void Epidemic::Step(std::vector<std::vector<int>>& grid, int rows, int cols) {
    std::vector<std::vector<int>> next(rows, std::vector<int>(cols));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int state = grid[i][j];
            if (state == 1) {
                next[i][j] = (rand() % 10 < 3) ? 2 : 1;
            } else if (state == 0) {
                next[i][j] = HasInfectedNeighbor(grid, i, j, rows, cols) ? 1 : 0;
            } else {
                next[i][j] = 2;
            }
        }
    }
    grid = next;
}


bool ForestFire::HasFireNeighbor(const std::vector<std::vector<int>>& grid, int r, int c, int rows, int cols) const {
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            int nr = (r + dr + rows) % rows;
            int nc = (c + dc + cols) % cols;
            if (grid[nr][nc] == 2) return true;
        }
    }
    return false;
}

void ForestFire::Step(std::vector<std::vector<int>>& grid, int rows, int cols) {
    std::vector<std::vector<int>> next(rows, std::vector<int>(cols));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int state = grid[i][j];
            if (state == 2) {
                next[i][j] = 0;
            } else if (state == 1) {
                next[i][j] = HasFireNeighbor(grid, i, j, rows, cols) ? 2 : 1;
            } else {
                next[i][j] = 0;
            }
        }
    }
    grid = next;
}

//загрузка из файла
bool CASimulator::LoadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        MessageBoxA(nullptr, "Cannot open file", "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    std::string line;
    int size = 20, alg = 1;
    std::vector<std::string> gridLines;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        if (line.find("SIZE=") == 0) {
            size = atoi(line.c_str() + 5);
            if (size <= 0 || size > 50) size = 20;
        }
        else if (line.find("ALGORITHM=") == 0) {
            alg = atoi(line.c_str() + 10);
            if (alg < 1 || alg > 5) alg = 1;
        }
        else if (line.find("GRID=") == 0) {
            gridLines.push_back(line.substr(5));
        }
    }

    file.close();

    Initialize(size, size);
    algorithm = alg;

    for (int i = 0; i < (int)gridLines.size() && i < size; i++) {
        const std::string& row = gridLines[i];
        for (int j = 0; j < (int)row.length() && j < size; j++) {
            current[i][j] = row[j] - '0';
        }
    }

    return true;
}


//сохранение в файл
bool CASimulator::SaveToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        MessageBoxA(nullptr, "Cannot create file", "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    file << "# Cellular Automata Configuration\n";
    file << "# Generated output\n\n";

    file << "SIZE=" << rows << "\n";
    file << "ALGORITHM=" << algorithm << "\n";

    for (int i = 0; i < rows; i++) {
        file << "GRID=";
        for (int j = 0; j < cols; j++) {
            file << current[i][j];
        }
        file << "\n";
    }

    file.close();
    MessageBoxA(nullptr, "File saved successfully!", "Success", MB_OK | MB_ICONINFORMATION);
    return true;
}
