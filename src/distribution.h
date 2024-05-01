#pragma once

#include <random>
#include <vector>

class TDices {
public:
    enum class ERollType {
        Straight,
        Advantage,
        Disadvantage,
    };

    TDices(int random_seed);

    int Roll(int dice);
    int Roll(int dice, ERollType type);

private:
    std::mt19937 rng;
};

struct TDice {
    int size;
};

class TDistribution {
public:
    TDistribution(const TDistribution& other) = default;
    TDistribution(TDistribution&& other) = default;

    TDistribution& operator =(const TDistribution& other) = default;
    TDistribution& operator =(TDistribution&& other) = default;

    TDistribution(int value);
    TDistribution(TDice dice);

    TDistribution operator +(const TDistribution& other) const;
    TDistribution operator -(const TDistribution& other) const;
    TDistribution operator *(int count) const;
    TDistribution operator /(int count) const;

    TDistribution& operator +=(const TDistribution& other);
    TDistribution& operator -=(const TDistribution& other);
    TDistribution& operator *=(int count);
    TDistribution& operator /=(int count);

    int GetValue(std::mt19937& RandGen) const;

private:
    TDistribution(std::vector<std::pair<double, int>>&& valueByProbability);
    bool Check() const;

    std::vector<std::pair<double, int>> valueByProbability;
};
