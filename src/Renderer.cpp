#include "Renderer.h"

#include "imgui.h"

#include <algorithm>
#include <cmath>

void Renderer::drawCalibrationLane(double phase, double latestErrorMs) {
    ImGui::TextUnformatted("Calibration Lane");

    const ImVec2 size(ImGui::GetContentRegionAvail().x, 140.0f);
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
    drawList->AddCircleFilled(ImVec2(markerX, origin.y + size.y * 0.5f), 12.0f, IM_COL32(97, 218, 251, 255));

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
    ImGui::Text("Samples: %zu", stats.count());
    ImGui::Text("Latest: %.2f ms", stats.latest());
    ImGui::Text("Average: %.2f ms", stats.average());
    ImGui::Text("Median: %.2f ms", stats.median());
    ImGui::Text("Stddev: %.2f ms", stats.stddev());
    ImGui::Text("Recommended offset: %.2f ms", stats.recommendedOffset());
}
