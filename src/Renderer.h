#pragma once

#include "RhythmEngine.h"
#include "Stats.h"

#include <deque>

struct CalibrationUiState {
    double bpm {120.0};
    bool running {false};
};

class Renderer {
public:
    void resetHitMarks();
    void addHitMark(double phase);
    void drawCalibrationLane(double phase, double latestErrorMs, float laneHeight);
    void drawStats(const Stats& stats);

private:
    struct HitMark {
        float phase {0.0f};
        float ageSeconds {0.0f};
    };

    std::deque<HitMark> m_hitMarks;
};
