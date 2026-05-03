#include "RhythmEngine.h"

#include <algorithm>
#include <cmath>

void RhythmEngine::setBpm(double bpm) {
    m_bpm = std::clamp(bpm, 40.0, 300.0);
}

double RhythmEngine::beatIntervalMs() const {
    return 60000.0 / m_bpm;
}

double RhythmEngine::clapIntervalMs() const {
    return beatIntervalMs() * static_cast<double>(kBeatsPerMeasure);
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

long long RhythmEngine::nearestClapIndex(double elapsedMs) const {
    const double clapOffsetMs = beatIntervalMs() * static_cast<double>(kClapBeatOffset);
    return static_cast<long long>(std::llround((elapsedMs - clapOffsetMs) / clapIntervalMs()));
}

double RhythmEngine::nearestClapTimeMs(double elapsedMs) const {
    const double clapOffsetMs = beatIntervalMs() * static_cast<double>(kClapBeatOffset);
    return clapOffsetMs + static_cast<double>(nearestClapIndex(elapsedMs)) * clapIntervalMs();
}

double RhythmEngine::clapTimingErrorMs(double elapsedMs) const {
    return elapsedMs - nearestClapTimeMs(elapsedMs);
}

double RhythmEngine::centeredClapPhase(double elapsedMs) const {
    const double clapOffsetMs = beatIntervalMs() * static_cast<double>(kClapBeatOffset);
    const double claps = (elapsedMs - clapOffsetMs) / clapIntervalMs();
    return claps - std::floor(claps + 0.5);
}
