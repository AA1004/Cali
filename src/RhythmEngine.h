#pragma once

#include <cstdint>

class RhythmEngine {
public:
    void setBpm(double bpm);
    [[nodiscard]] double bpm() const { return m_bpm; }
    [[nodiscard]] double beatIntervalMs() const;
    [[nodiscard]] long long currentBeatIndex(double elapsedMs) const;
    [[nodiscard]] long long nearestBeatIndex(double elapsedMs) const;
    [[nodiscard]] double nearestBeatTimeMs(double elapsedMs) const;
    [[nodiscard]] double timingErrorMs(double elapsedMs) const;
    [[nodiscard]] double centeredPhase(double elapsedMs) const;

private:
    double m_bpm {120.0};
};
