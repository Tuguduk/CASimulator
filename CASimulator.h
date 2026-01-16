#ifndef CA_SIMULATOR_H
#define CA_SIMULATOR_H

#include <vector>
#include <string>
#include <memory>

// состояние симуляции
struct SimulationState {
    std::vector<std::vector<int>> grid;
};

class Algorithm {
public:
    virtual ~Algorithm() = default;
    
    virtual void Step(std::vector<std::vector<int>>& 
        grid, int rows, int cols) = 0;
};

class Rule30 : public Algorithm {
public:
    void Step(std::vector<std::vector<int>>& grid, int rows, int cols) override;
};


class GameOfLife : public Algorithm {
public:
    void Step(std::vector<std::vector<int>>& grid, int rows, int cols) override;

private:
    int CountAliveNeighbors(const std::vector<std::vector<int>>& grid, int r, int c, int rows, int cols) const;
};


class Traffic : public Algorithm {
public:
    void Step(std::vector<std::vector<int>>& grid, int rows, int cols) override;
};


class Epidemic : public Algorithm {
public:
    void Step(std::vector<std::vector<int>>& grid, int rows, int cols) override;

private:
    bool HasInfectedNeighbor(const std::vector<std::vector<int>>& grid, int r, int c, int rows, int cols) const;
};


class ForestFire : public Algorithm {
public:
    void Step(std::vector<std::vector<int>>& grid, int rows, int cols) override;

private:
    bool HasFireNeighbor(const std::vector<std::vector<int>>& grid, int r, int c, int rows, int cols) const;
};


class CASimulator {
public:
    std::vector<SimulationState> history; //вся история
    std::vector<std::vector<int>> current; //текущая сетка
    int rows, cols; 
    int algorithm; 

    CASimulator() : rows(0), cols(0), algorithm(0) {}

    // инициализация новой сетки
    void Initialize(int r, int c);

    // сохранить текущее состояние в историю
    void SaveState();

    // 1 шаг симуляции 
    void Step();

    bool LoadFromFile(const std::string& filename);

    bool SaveToFile(const std::string& filename);

private:
    // указатели на экземпляры алг-в
    std::unique_ptr<Rule30> rule30;
    std::unique_ptr<GameOfLife> gameOfLife;
    std::unique_ptr<Traffic> traffic;
    std::unique_ptr<Epidemic> epidemic;
    std::unique_ptr<ForestFire> forestFire;

    // инициализация алг-в
    void InitializeAlgorithms();

    // Получить текущий алгоритм по номеру
    Algorithm* GetCurrentAlgorithm();
};

#endif 
