#pragma once

#include <cstddef>
#include <vector>

class Stats {
public:
    void addSample(double value);
    void clear();
    [[nodiscard]] bool empty() const { return m_samples.empty(); }
    [[nodiscard]] std::size_t count() const { return m_samples.size(); }
    [[nodiscard]] double latest() const;
    [[nodiscard]] double average() const;
    [[nodiscard]] double median() const;
    [[nodiscard]] double stddev() const;
    [[nodiscard]] double recommendedOffset() const;
    [[nodiscard]] const std::vector<double>& samples() const { return m_samples; }

private:
    std::vector<double> m_samples;
};
