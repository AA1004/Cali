#include "Stats.h"

#include <algorithm>
#include <cmath>
#include <numeric>

void Stats::addSample(double value) {
    m_samples.push_back(value);
}

void Stats::clear() {
    m_samples.clear();
}

double Stats::latest() const {
    return m_samples.empty() ? 0.0 : m_samples.back();
}

double Stats::average() const {
    if (m_samples.empty()) {
        return 0.0;
    }

    const double sum = std::accumulate(m_samples.begin(), m_samples.end(), 0.0);
    return sum / static_cast<double>(m_samples.size());
}

double Stats::median() const {
    if (m_samples.empty()) {
        return 0.0;
    }

    std::vector<double> sorted = m_samples;
    std::sort(sorted.begin(), sorted.end());
    const std::size_t middle = sorted.size() / 2;

    if (sorted.size() % 2 == 0) {
        return (sorted[middle - 1] + sorted[middle]) * 0.5;
    }

    return sorted[middle];
}

double Stats::stddev() const {
    if (m_samples.size() < 2) {
        return 0.0;
    }

    const double mean = average();
    double sumSquares = 0.0;

    for (const double sample : m_samples) {
        const double delta = sample - mean;
        sumSquares += delta * delta;
    }

    return std::sqrt(sumSquares / static_cast<double>(m_samples.size()));
}

double Stats::recommendedOffset() const {
    return -average();
}
