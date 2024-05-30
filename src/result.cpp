#include "result.h"
#include <stdexcept>
#include <iostream>

TResult::TResult(double prob, int teamSz)
    : probWin(prob)
{
    for (int i = 0; i < kMaxPlayer; ++i) {
        alivePlayers[i] = 0;
    }
    if (teamSz == 0) {
        if (prob > 1e-3) {
            throw std::logic_error("prob win greater zero, but all players is dead");
        }
        alivePlayers[teamSz] = (1 - prob);
    } else if (teamSz != -1) {
        alivePlayers[teamSz] = prob;
    }
}

TResult TResult::operator +(TResult other) const
{
    other.probWin += probWin;
    for (int i = 0; i < kMaxPlayer; ++i) {
        other.alivePlayers[i] += alivePlayers[i];
    }
    return other;
}

TResult TResult::operator *(double prob) const
{
    TResult res(*this);
    res.probWin *= prob;
    for (int i = 0; i < kMaxPlayer; ++i) {
        res.alivePlayers[i] *= prob;
    }
    return res;
}

TResult& TResult::operator +=(const TResult& other)
{
    probWin += other.probWin;
    for (int i = 0; i < kMaxPlayer; ++i) {
        alivePlayers[i] += other.alivePlayers[i];
    }
    return *this;
}

bool TResult::operator <(const TResult& other) const
{
    return probWin < other.probWin;
}

bool TResult::operator >(const TResult& other) const
{
    return probWin > other.probWin;
}
