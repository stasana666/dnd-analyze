#include "distribution.h"

#include <map>

TDistribution::TDistribution(int value)
{
    valueByProbability.emplace_back(1., value);
}

TDistribution::TDistribution(TDice dice)
{
    double prob = 1. / dice.size;
}

TDistribution::TDistribution(std::vector<std::pair<double, int>>&& valueByProbability)
    : valueByProbability(valueByProbability) {}

TDistribution TDistribution::operator +(const TDistribution& other) const
{
    std::map<int, double> resultMap;
    for (auto [prob_1, value_1] : valueByProbability) {
        for (auto [prob_2, value_2] : other.valueByProbability) {
            resultMap[value_1 + value_2] += value_1 * value_2;
        }
    }
    std::vector<std::pair<double, int>> resultVec;
    for (auto [value, prob] : resultMap) {
        resultVec.emplace_back(prob, value);
    }
    return TDistribution(std::move(resultVec));
}

TDistribution TDistribution::operator -(const TDistribution& other) const
{
    std::map<int, double> resultMap;
    for (auto [prob_1, value_1] : valueByProbability) {
        for (auto [prob_2, value_2] : other.valueByProbability) {
            resultMap[value_1 - value_2] -= value_1 * value_2;
        }
    }
    std::vector<std::pair<double, int>> resultVec;
    for (auto [value, prob] : resultMap) {
        resultVec.emplace_back(prob, value);
    }
    return TDistribution(std::move(resultVec));
}

TDistribution TDistribution::operator *(int count) const
{
    // TODO
}

TDistribution TDistribution::operator /(int count) const
{
    // TODO
}

TDistribution& TDistribution::operator +=(const TDistribution& other)
{
    (*this) = this->operator +(other);
    return *this;
}

TDistribution& TDistribution::operator -=(const TDistribution& other)
{
    (*this) = this->operator -(other);
    return *this;
}

TDistribution& TDistribution::operator *=(int count)
{
    return *this;
}

TDistribution& TDistribution::operator /=(int count)
{
    return *this;
}

int TDistribution::GetValue(std::mt19937& RandGen) const
{
    static std::uniform_real_distribution<> rngDis(0., 1.);
    double doubleIndex = rngDis(RandGen);
    int index = 0;
    while (index < valueByProbability.size() && doubleIndex > valueByProbability[index].first) {
        doubleIndex -= valueByProbability[index].first;
        ++index;
    }
    return valueByProbability[index].second;
}

bool TDistribution::Check() const
{
    double sum = 0;
    for (auto& [prob, value] : valueByProbability) {
        sum += prob;
    }
    return sum == 1.;
}
