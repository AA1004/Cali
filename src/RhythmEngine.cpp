#include "RhythmEngine.h"

#include <algorithm>
#include <cmath>

void RhythmEngine::setBpm(double bpm) {
    m_bpm = std::clamp(bpm, 40.0, 300.0);
}

double RhythmEngine::beatIntervalMs() const {
    return 60000.0 / m_bpm;
}

long long RhythmEngine::currentBeatIndex(double elapsedMs) const {
    return static_cast<long long>(std::floor(elapsedMs / beatIntervalMs()));
}

long long RhythmEngine::nearestBeatIndex(double elapsedMs) const {
    return static_cast<long long>(std::llround(elapsedMs / beatIntervalMs()));
}

double RhythmEngine::nearestBeatTimeMs(double elapsedMs) const {
    return static_cast<double>(nearestBeatIndex(elapsedMs)) * beatIntervalMs();
}

double RhythmEngine::timingErrorMs(double elapsedMs) const {
    return elapsedMs - nearestBeatTimeMs(elapsedMs);
}

double RhythmEngine::centeredPhase(double elapsedMs) const {
    const double beats = elapsedMs / beatIntervalMs();
    return beats - std::floor(beats + 0.5);
}
