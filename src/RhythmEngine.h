#pragma once

#include <cstdint>

class RhythmEngine {
public:
    static constexpr int kClapBeatOffset = 3;
    static constexpr int kBeatsPerMeasure = 4;

    void setBpm(double bpm);
    [[nodiscard]] double bpm() const { return m_bpm; }
    [[nodiscard]] double beatIntervalMs() const;
    [[nodiscard]] double clapIntervalMs() const;
    [[nodiscard]] long long currentBeatIndex(double elapsedMs) const;
    [[nodiscard]] long long nearestBeatIndex(double elapsedMs) const;
    [[nodiscard]] double nearestBeatTimeMs(double elapsedMs) const;
    [[nodiscard]] double timingErrorMs(double elapsedMs) const;
    [[nodiscard]] double centeredPhase(double elapsedMs) const;
    [[nodiscard]] long long nearestClapIndex(double elapsedMs) const;
    [[nodiscard]] double nearestClapTimeMs(double elapsedMs) const;
    [[nodiscard]] double clapTimingErrorMs(double elapsedMs) const;
    [[nodiscard]] double centeredClapPhase(double elapsedMs) const;

private:
    double m_bpm {120.0};
};
