#include "Timer.h"

void Timer::start() {
    if (m_running) {
        return;
    }

    m_startedAt = clock::now();
    m_running = true;
}

void Timer::stop() {
    if (!m_running) {
        return;
    }

    m_accumulatedMs = elapsedMs();
    m_running = false;
}

void Timer::reset() {
    m_accumulatedMs = 0.0;
    m_startedAt = clock::now();
}

double Timer::elapsedMs() const {
    if (!m_running) {
        return m_accumulatedMs;
    }

    const auto delta = clock::now() - m_startedAt;
    return m_accumulatedMs + std::chrono::duration<double, std::milli>(delta).count();
}
