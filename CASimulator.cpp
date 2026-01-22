#include "CASimulator.h"
#include <fstream>
#include <windows.h>
#include <cstdlib>
#include <ctime>


//функция обновления сетки
void Algorithm::UpdateGridWithRule(std::vector<std::vector<int>>& grid, 
                                   int rows, int cols,
                                   CellUpdateRule* rule) {
    std::vector<std::vector<int>> next(rows, std::vector<int>(cols));
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            CellContext ctx;
            ctx.currentState = grid[i][j];
            ctx.row = i;
            ctx.col = j;
            ctx.goodNeighbors = 0;
            ctx.badNeighbors = 0;
            ctx.totalNeighbors = 0;
            
            // Подсчитываем соседей
            for (int dr = -1; dr <= 1; dr++) {
                for (int dc = -1; dc <= 1; dc++) {
                    if (dr == 0 && dc == 0) continue;
                    
                    int nr = (i + dr + rows) % rows;
                    int nc = (j + dc + cols) % cols;
                    int neighborState = grid[nr][nc];
                    
                    ctx.totalNeighbors++;
                    
                    // Используем правило для классификации соседа
                    rule->ClassifyNeighbor(neighborState, ctx.goodNeighbors, ctx.badNeighbors);
                }
            }
            
            // Применяем правило для получения нового состояния
            next[i][j] = rule->GetNextState(ctx);
        }
    }
    
    grid = next;
}

// Реализации правил

// Rule 30
int Rule30RuleImpl::GetNextState(const CellContext& ctx) {
    int left = ctx.badNeighbors > 0 ? 1 : 0;  
    int center = ctx.currentState;
    int right = ctx.goodNeighbors > 0 ? 1 : 0; 
    
    // Rule 30: left XOR (center OR right)
    return left ^ (center | right);
}

void Rule30RuleImpl::ClassifyNeighbor(int neighborState, int& good, int& bad) {
    if (neighborState >= 1) good++;
    else bad++;
}

// Game of Life
int GameOfLifeRuleImpl::GetNextState(const CellContext& ctx) {
    if (ctx.currentState >= 1) {
        // Живая клетка выживает при 2 или 3 живых соседях
        return (ctx.goodNeighbors == 2 || ctx.goodNeighbors == 3) ? 1 : 0;
    } else {
        // Мёртвая клетка оживает при ровно 3 живых соседях
        return (ctx.goodNeighbors == 3) ? 1 : 0;
    }
}

void GameOfLifeRuleImpl::ClassifyNeighbor(int neighborState, int& good, int& bad) {
    if (neighborState >= 1) good++;  // Живая клетка
    else bad++;                       // Мёртвая клетка
}

// Traffic
int TrafficRuleImpl::GetNextState(const CellContext& ctx) {
    return ctx.currentState;
}

void TrafficRuleImpl::ClassifyNeighbor(int neighborState, int& good, int& bad) {
    if (neighborState == 1) good++; // Машина
    else bad++; // Пустое место
}

// Epidemic
int EpidemicRuleImpl::GetNextState(const CellContext& ctx) {
    if (ctx.currentState == 1) {
        // больная клетка: с 30% вероятностью выздоравливает
        return (rand() % 10 < 3) ? 2 : 1;
    } else if (ctx.currentState == 0) {
        // Здоровая клетка: заражается, если есть заражённые соседи
        return ctx.goodNeighbors > 0 ? 1 : 0;
    } else {
        // Выздоровевшая клетка (state == 2) остаётся выздоровевшей
        return 2;
    }
}

void EpidemicRuleImpl::ClassifyNeighbor(int neighborState, int& good, int& bad) {
    if (neighborState == 1) good++; // Больная клетка
    else if (neighborState == 0) bad++;  // Здоровая клетка
    // Выздоровевшие не считаем
}

// Forest Fire
int ForestFireRuleImpl::GetNextState(const CellContext& ctx) {
    if (ctx.currentState == 2) {
        // Горящее дерево сгорает → пустое место
        return 0;
    } else if (ctx.currentState == 1) {
        // Дерево загорается, если рядом есть огонь
        return ctx.goodNeighbors > 0 ? 2 : 1;
    } else {
        // Пустое место остаётся пустым
        return 0;
    }
}

void ForestFireRuleImpl::ClassifyNeighbor(int neighborState, int& good, int& bad) {
    if (neighborState == 2) good++;      // Огонь
    else if (neighborState == 1) bad++;  // Дерево
    // Пустые клетки (0) не считаем
}

// Конструкторы алгоритмов

Rule30::Rule30() : rule(std::make_unique<Rule30RuleImpl>()) {}

void Rule30::Step(std::vector<std::vector<int>>& grid, int rows, int cols) {
    UpdateGridWithRule(grid, rows, cols, rule.get());
}

GameOfLife::GameOfLife() : rule(std::make_unique<GameOfLifeRuleImpl>()) {}

void GameOfLife::Step(std::vector<std::vector<int>>& grid, int rows, int cols) {
    UpdateGridWithRule(grid, rows, cols, rule.get());
}

Traffic::Traffic() : rule(std::make_unique<TrafficRuleImpl>()) {}

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

Epidemic::Epidemic() : rule(std::make_unique<EpidemicRuleImpl>()) {}

void Epidemic::Step(std::vector<std::vector<int>>& grid, int rows, int cols) {
    UpdateGridWithRule(grid, rows, cols, rule.get());
}

ForestFire::ForestFire() : rule(std::make_unique<ForestFireRuleImpl>()) {}

void ForestFire::Step(std::vector<std::vector<int>>& grid, int rows, int cols) {
    UpdateGridWithRule(grid, rows, cols, rule.get());
}


// CASimulator методы

void CASimulator::Initialize(int r, int c) {
    rows = r;
    cols = c;
    current.assign(r, std::vector<int>(c, 0));
    history.clear();
    InitializeAlgorithms();
}

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

// Загрузка из файла
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

// Сохранение в файл
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
