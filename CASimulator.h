#ifndef CA_SIMULATOR_H
#define CA_SIMULATOR_H

#include <vector>
#include <memory>
#include <string>

// Состояние симуляции
struct SimulationState {
    std::vector<std::vector<int>> grid;
};

// Структура для информации о клетке и её соседях
struct CellContext {
    int currentState; // текущее состояние клетки
    int goodNeighbors;  // количество "хороших" соседей
    int badNeighbors;  // количество "плохих" соседей
    int totalNeighbors;  // всего соседей
    int row, col;  // координаты клетки
};

// Базовый класс для правил обновления клеток
class CellUpdateRule {
public:
    virtual ~CellUpdateRule() = default;
    
    // Возвращает новое состояние клетки
    virtual int GetNextState(const CellContext& ctx) = 0;
    
    // Классифицирует соседа как "хорошего" или "плохого"
    virtual void ClassifyNeighbor(int neighborState, int& good, int& bad) = 0;
};

// Базовый класс алгоритма
class Algorithm {
public:
    virtual ~Algorithm() = default;
    virtual void Step(std::vector<std::vector<int>>& grid, int rows, int cols) = 0;
    
protected:
    // Универсальная функция обновления сетки с правилом
    void UpdateGridWithRule(std::vector<std::vector<int>>& grid, 
                           int rows, int cols,
                           CellUpdateRule* rule);
};

//Реализации правил

class Rule30RuleImpl : public CellUpdateRule {
public:
    int GetNextState(const CellContext& ctx) override;
    void ClassifyNeighbor(int neighborState, int& good, int& bad) override;
};

class GameOfLifeRuleImpl : public CellUpdateRule {
public:
    int GetNextState(const CellContext& ctx) override;
    void ClassifyNeighbor(int neighborState, int& good, int& bad) override;
};

class TrafficRuleImpl : public CellUpdateRule {
public:
    int GetNextState(const CellContext& ctx) override;
    void ClassifyNeighbor(int neighborState, int& good, int& bad) override;
};

class EpidemicRuleImpl : public CellUpdateRule {
public:
    int GetNextState(const CellContext& ctx) override;
    void ClassifyNeighbor(int neighborState, int& good, int& bad) override;
};

class ForestFireRuleImpl : public CellUpdateRule {
public:
    int GetNextState(const CellContext& ctx) override;
    void ClassifyNeighbor(int neighborState, int& good, int& bad) override;
};

// Классы алгоритмов

class Rule30 : public Algorithm {
public:
    Rule30();
    void Step(std::vector<std::vector<int>>& grid, int rows, int cols) override;
private:
    std::unique_ptr<Rule30RuleImpl> rule;
};

class GameOfLife : public Algorithm {
public:
    GameOfLife();
    void Step(std::vector<std::vector<int>>& grid, int rows, int cols) override;
private:
    std::unique_ptr<GameOfLifeRuleImpl> rule;
};

class Traffic : public Algorithm {
public:
    Traffic();
    void Step(std::vector<std::vector<int>>& grid, int rows, int cols) override;
private:
    std::unique_ptr<TrafficRuleImpl> rule;
};

class Epidemic : public Algorithm {
public:
    Epidemic();
    void Step(std::vector<std::vector<int>>& grid, int rows, int cols) override;
private:
    std::unique_ptr<EpidemicRuleImpl> rule;
};

class ForestFire : public Algorithm {
public:
    ForestFire();
    void Step(std::vector<std::vector<int>>& grid, int rows, int cols) override;
private:
    std::unique_ptr<ForestFireRuleImpl> rule;
};

//Симулятор 

class CASimulator {
public:
    std::vector<SimulationState> history; // вся история
    std::vector<std::vector<int>> current; // текущая сетка
    int rows, cols;
    int algorithm;

    CASimulator() : rows(0), cols(0), algorithm(0) {}

    // Инициализация новой сетки
    void Initialize(int r, int c);

    // Сохранить текущее состояние в историю
    void SaveState();

    // 1 шаг симуляции
    void Step();

    bool LoadFromFile(const std::string& filename);
    bool SaveToFile(const std::string& filename);

private:
    // Указатели на экземпляры алгоритмов
    std::unique_ptr<Rule30> rule30;
    std::unique_ptr<GameOfLife> gameOfLife;
    std::unique_ptr<Traffic> traffic;
    std::unique_ptr<Epidemic> epidemic;
    std::unique_ptr<ForestFire> forestFire;

    // Инициализация алгоритмов
    void InitializeAlgorithms();

    // Получить текущий алгоритм по номеру
    Algorithm* GetCurrentAlgorithm();
};

#endif
