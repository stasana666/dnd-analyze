#pragma once

#include <random>
#include <vector>

struct TDice {
    int size;
};

class TDistribution {
public:
    TDistribution(const TDistribution& other) = default;
    TDistribution(TDistribution&& other) = default;

    // Для случаев, когда в веротностью p1 происходит один из исходов dist1, с p2 - dist2, и и.д.
    TDistribution(const std::vector<std::pair<double, TDistribution>>& distributions);

    TDistribution& operator =(const TDistribution& other) = default;
    TDistribution& operator =(TDistribution&& other) = default;

    TDistribution(int value);
    TDistribution(TDice dice);

    void Print() const;

    TDistribution operator +(const TDistribution& other) const;
    TDistribution operator -(const TDistribution& other) const;
    TDistribution operator *(int count) const;
    TDistribution operator /(int count) const;

    double operator <(double value) const;

    TDistribution& operator +=(const TDistribution& other);
    TDistribution& operator -=(const TDistribution& other);
    TDistribution& operator *=(int count);
    TDistribution& operator /=(int count);

    int GetValue(std::mt19937& RandGen) const;
    double GetEstimate() const;

    void DelBelowZero();
    void MakeMaxValue(int value);

private:
    TDistribution(std::vector<std::pair<double, int>>&& valueByProbability);
    void Check() const;

    std::vector<std::pair<double, int>> valueByProbability;
};
