#pragma once

#include <chrono>

class Timer {
public:
    void start();
    void stop();
    void reset();
    [[nodiscard]] double elapsedMs() const;
    [[nodiscard]] bool running() const { return m_running; }

private:
    using clock = std::chrono::steady_clock;

    clock::time_point m_startedAt {};
    double m_accumulatedMs {0.0};
    bool m_running {false};
};
