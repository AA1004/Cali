#include "Renderer.h"

#include "imgui.h"

#include <algorithm>
#include <cmath>

void Renderer::resetHitMarks() {
    m_hitMarks.clear();
}

void Renderer::addHitMark(double phase) {
    m_hitMarks.push_back(HitMark {static_cast<float>(phase), 0.0f});
    while (m_hitMarks.size() > 24) {
        m_hitMarks.pop_front();
    }
}

void Renderer::drawCalibrationLane(double phase, double latestErrorMs, float laneHeight) {
    ImGui::TextUnformatted("판정 레인");

    const ImVec2 size(ImGui::GetContentRegionAvail().x, laneHeight);
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const ImVec2 end(origin.x + size.x, origin.y + size.y);
    auto* drawList = ImGui::GetWindowDrawList();

    drawList->AddRectFilled(origin, end, IM_COL32(20, 24, 32, 255), 10.0f);
    drawList->AddRect(origin, end, IM_COL32(76, 87, 104, 255), 10.0f, 0, 2.0f);

    const float centerX = origin.x + size.x * 0.5f;
    drawList->AddLine(
        ImVec2(centerX, origin.y + 14.0f),
        ImVec2(centerX, end.y - 14.0f),
        IM_COL32(255, 230, 128, 255),
        3.0f
    );

    const float markerX = origin.x + static_cast<float>((phase + 0.5) * size.x);
    const float deltaSeconds = ImGui::GetIO().DeltaTime;
    for (auto& hitMark : m_hitMarks) {
        hitMark.ageSeconds += deltaSeconds;
    }

    for (std::size_t i = 0; i < m_hitMarks.size(); ++i) {
        const float t = static_cast<float>(i + 1) / static_cast<float>(m_hitMarks.size());
        const float hitX = origin.x + (m_hitMarks[i].phase + 0.5f) * size.x;
        const int alpha = static_cast<int>(28.0f + t * 96.0f);
        const float thickness = 1.0f + t * 1.5f;
        drawList->AddLine(
            ImVec2(hitX, origin.y + 18.0f),
            ImVec2(hitX, end.y - 18.0f),
            IM_COL32(255, 136, 91, alpha),
            thickness
        );

        const float pulseProgress = std::min(m_hitMarks[i].ageSeconds / 0.22f, 1.0f);
        if (pulseProgress < 1.0f) {
            const float pulseFade = 1.0f - pulseProgress;
            const float pulseHeight = 14.0f + pulseProgress * 42.0f;
            const float pulseWidth = 4.0f + pulseProgress * 8.0f;
            const int pulseAlpha = static_cast<int>(pulseFade * 180.0f);

            drawList->AddLine(
                ImVec2(hitX, origin.y + 22.0f),
                ImVec2(hitX, origin.y + 22.0f - pulseHeight),
                IM_COL32(255, 220, 150, pulseAlpha),
                pulseWidth
            );
            drawList->AddLine(
                ImVec2(hitX, end.y - 22.0f),
                ImVec2(hitX, end.y - 22.0f + pulseHeight),
                IM_COL32(255, 220, 150, pulseAlpha),
                pulseWidth
            );
        }
    }

    drawList->AddLine(
        ImVec2(markerX, origin.y + 12.0f),
        ImVec2(markerX, end.y - 12.0f),
        IM_COL32(97, 218, 251, 255),
        4.0f
    );

    const float absError = static_cast<float>(std::abs(latestErrorMs));
    const float bandPx = std::clamp(14.0f + absError * 1.2f, 14.0f, size.x * 0.45f);
    drawList->AddRectFilled(
        ImVec2(centerX - bandPx, origin.y + 54.0f),
        ImVec2(centerX + bandPx, origin.y + 86.0f),
        IM_COL32(88, 166, 255, 42),
        6.0f
    );

    ImGui::Dummy(size);
}

void Renderer::drawStats(const Stats& stats) {
    ImGui::Text("샘플 수: %zu", stats.count());
    ImGui::Text("최근 판정: %.2f ms", stats.latest());
    ImGui::Text("평균: %.2f ms", stats.average());
    ImGui::Text("중앙값: %.2f ms", stats.median());
    ImGui::Text("표준편차: %.2f ms", stats.stddev());
    ImGui::Text("추천 오프셋: %.2f ms", stats.recommendedOffset());
}
