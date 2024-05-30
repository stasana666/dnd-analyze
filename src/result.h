#pragma once

#include <array>

class TResult {
public:
    static constexpr int kMaxPlayer = 5;

    TResult(double prob, int teamSz);

    TResult operator +(TResult other) const;
    TResult operator *(double prob) const;

    TResult& operator +=(const TResult& other);

    bool operator <(const TResult& other) const;
    bool operator >(const TResult& other) const;

    double probWin;
    std::array<double, kMaxPlayer> alivePlayers;
};
