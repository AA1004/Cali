#pragma once

#include "RhythmEngine.h"
#include "Stats.h"

struct CalibrationUiState {
    double bpm {120.0};
    bool running {false};
};

class Renderer {
public:
    void drawCalibrationLane(double phase, double latestErrorMs);
    void drawStats(const Stats& stats);
};
