#include "distribution.h"

#include <algorithm>
#include <iostream>
#include <map>

TDistribution::TDistribution(int value)
{
    valueByProbability.emplace_back(1., value);
    Check();
}

TDistribution::TDistribution(TDice dice)
{
    double prob = 1. / dice.size;
    for (int i = 1; i <= dice.size; ++i) {
        valueByProbability.emplace_back(prob, i);
    }
    Check();
}

TDistribution::TDistribution(std::vector<std::pair<double, int>>&& valueByProbability)
    : valueByProbability(valueByProbability)
{
    Check();
}

TDistribution::TDistribution(const std::vector<std::pair<double, TDistribution>>& distributions)
{
    std::map<int, double> resultMap;
    for (auto& [prob, distribution] : distributions) {
        for (auto [p, v] : distribution.valueByProbability) {
            resultMap[v] += p * prob;
        }
    }
    for (auto [value, prob] : resultMap) {
        valueByProbability.emplace_back(prob, value);
    }
    Check();
}

TDistribution TDistribution::operator +(const TDistribution& other) const
{
    std::map<int, double> resultMap;
    for (auto [prob_1, value_1] : valueByProbability) {
        for (auto [prob_2, value_2] : other.valueByProbability) {
            resultMap[value_1 + value_2] += prob_1 * prob_2;
        }
    }
    std::vector<std::pair<double, int>> resultVec;
    for (auto [value, prob] : resultMap) {
        resultVec.emplace_back(prob, value);
    }
    return TDistribution(std::move(resultVec));
}

void TDistribution::Print() const
{
    std::cerr << "[" << std::endl;
    for (auto [prob, value] : valueByProbability) {
        std::cerr << "    { " << prob << ", " << value << " }" << std::endl;
    }
    std::cerr << "]" << std::endl;
}

TDistribution TDistribution::operator -(const TDistribution& other) const
{
    std::map<int, double> resultMap;
    for (auto [prob_1, value_1] : valueByProbability) {
        for (auto [prob_2, value_2] : other.valueByProbability) {
            resultMap[value_1 - value_2] += prob_1 * prob_2;
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
    TDistribution result(0);
    TDistribution pw = *this;
    while (count > 0) {
        if (count & 1) {
            result += pw;
        }
        pw += pw;
        count >>= 1;
    }
    return result;
}

TDistribution TDistribution::operator /(int count) const
{
    std::map<int, double> resultMap;
    for (auto [prob, value] : valueByProbability) {
            resultMap[value / count] += prob;
    }
    std::vector<std::pair<double, int>> resultVec;
    for (auto [value, prob] : resultMap) {
        resultVec.emplace_back(prob, value);
    }
    return TDistribution(std::move(resultVec));
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
    throw std::runtime_error("not supported: TDistribution::operator *=");
    return *this;
}

TDistribution& TDistribution::operator /=(int count)
{
    throw std::runtime_error("not supported: TDistribution::operator /=");
    return *this;
}

double TDistribution::operator <(double value) const
{
    double res = 0;
    for (auto [p, v] : valueByProbability) {
        res += p * (v < value);
    }
    return res;
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

double TDistribution::GetEstimate() const
{
    double res = 0;
    for (auto [prob, value] : valueByProbability) {
        res += prob * value;
    }
    return res;
}

void TDistribution::DelBelowZero()
{
    auto it = std::remove_if(valueByProbability.begin(), valueByProbability.end(), [](std::pair<double, int> x) {
        return x.second <= 0;
    });
    valueByProbability.erase(it, valueByProbability.end());
    double sum = 0;
    for (auto [prob, _] : valueByProbability) {
        sum += prob;
    }
    if (sum < 1e-4) {
        throw std::logic_error("sum of probs less 1e-4");
    }
    for (auto& [prob, _] : valueByProbability) {
        prob /= sum;
    }
}

void TDistribution::MakeMaxValue(int value)
{
    auto it = std::remove_if(valueByProbability.begin(), valueByProbability.end(), [](std::pair<double, int> x) {
        return x.second <= 0;
    });
    valueByProbability.erase(it, valueByProbability.end());
    double sum = 0;
    for (auto [prob, _] : valueByProbability) {
        sum += prob;
    }
    for (auto& [prob, v] : valueByProbability) {
        if (v == value) {
            prob += (1 - sum);
            return;
        }
    }
    if (sum < 1. - 1e-4) {
        valueByProbability.emplace_back((1 - sum), value);
    }
}

void TDistribution::Check() const
{
    double sum = 0;
    for (auto& [prob, value] : valueByProbability) {
        sum += prob;
    }
    if (std::fabs(sum - 1.) > 1e-4) {
        throw std::logic_error("sum of probs != 1");
    }
}
