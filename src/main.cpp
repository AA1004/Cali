#include "AudioEngine.h"
#include "Renderer.h"
#include "RhythmEngine.h"
#include "Stats.h"
#include "Timer.h"

#include "SDL.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"

#include <algorithm>
#include <cstdio>

namespace {
constexpr int kWindowWidth = 1120;
constexpr int kWindowHeight = 760;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) != 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "리듬 오프셋 캘리브레이터",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        kWindowWidth,
        kWindowHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (window == nullptr) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* sdlRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (sdlRenderer == nullptr) {
        std::fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(window, sdlRenderer);
    ImGui_ImplSDLRenderer2_Init(sdlRenderer);

    RhythmEngine rhythm;
    Timer timer;
    Stats stats;
    AudioEngine audio;
    Renderer renderer;

    rhythm.setBpm(120.0);
    const bool audioReady = audio.initialize();

    long long lastTriggeredBeat = -1;
    float judgementOffsetMs = 0.0f;
    float kickVolume = 0.95f;
    float hiHatVolume = 0.28f;
    bool running = false;
    bool quit = false;

    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            ImGui_ImplSDL2_ProcessEvent(&event);

            if (event.type == SDL_QUIT) {
                quit = true;
            }

            if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                }

                if (event.key.keysym.sym == SDLK_SPACE && running) {
                    const double hitElapsedMs = timer.elapsedMs();
                    const double adjustedHitMs = hitElapsedMs - static_cast<double>(judgementOffsetMs);
                    stats.addSample(rhythm.clapTimingErrorMs(adjustedHitMs));
                    renderer.addHitMark(rhythm.centeredClapPhase(adjustedHitMs));
                }
            }
        }

        const double elapsedMs = timer.elapsedMs();
        const long long beatIndex = running ? rhythm.currentBeatIndex(elapsedMs) : -1;

        if (running && audioReady && beatIndex > lastTriggeredBeat) {
            for (long long index = lastTriggeredBeat + 1; index <= beatIndex; ++index) {
                audio.trigger(BeatSound::HiHat, hiHatVolume);

                if (index % 4 == 3) {
                    audio.trigger(BeatSound::Clap, 0.95f);
                } else {
                    audio.trigger(BeatSound::Kick, kickVolume);
                }
            }
            lastTriggeredBeat = beatIndex;
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(24.0f, 24.0f), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(1072.0f, 700.0f), ImGuiCond_Once);
        ImGui::Begin("리듬 오프셋 캘리브레이터", nullptr, ImGuiWindowFlags_NoCollapse);

        const double adjustedElapsedMs = elapsedMs - static_cast<double>(judgementOffsetMs);
        const double phase = running ? rhythm.centeredClapPhase(adjustedElapsedMs) : -0.5;

        ImGui::BeginChild("MainLane", ImVec2(0.0f, 360.0f), true);
        renderer.drawCalibrationLane(phase, stats.latest(), 320.0f);
        ImGui::TextWrapped("클랩 박자에만 맞춰 Space를 누르세요. 움직이는 선이 중앙 판정선에 닿을 때 입력하면 됩니다.");
        ImGui::EndChild();

        ImGui::Spacing();
        ImGui::Columns(2, nullptr, true);

        ImGui::BeginChild("Controls", ImVec2(0.0f, 0.0f), false);
        ImGui::TextUnformatted("재생 설정");
        float bpm = static_cast<float>(rhythm.bpm());
        if (ImGui::SliderFloat("BPM", &bpm, 40.0f, 300.0f, "%.0f")) {
            rhythm.setBpm(static_cast<double>(bpm));
            if (running) {
                timer.reset();
                timer.start();
                lastTriggeredBeat = -1;
                renderer.resetHitMarks();
            }
        }

        ImGui::SliderFloat("판정선 오프셋 (ms)", &judgementOffsetMs, -180.0f, 180.0f, "%.0f ms");
        ImGui::SliderFloat("킥 볼륨", &kickVolume, 0.0f, 1.5f, "%.2f");
        ImGui::SliderFloat("하이햇 볼륨", &hiHatVolume, 0.0f, 1.0f, "%.2f");

        if (ImGui::Button(running ? "정지" : "시작", ImVec2(120.0f, 0.0f))) {
            if (running) {
                timer.stop();
                running = false;
            } else {
                timer.reset();
                timer.start();
                stats.clear();
                lastTriggeredBeat = -1;
                renderer.resetHitMarks();
                running = true;
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("기록 초기화", ImVec2(120.0f, 0.0f))) {
            stats.clear();
            renderer.resetHitMarks();
        }

        ImGui::Spacing();
        ImGui::Text("비트 간격: %.2f ms", rhythm.beatIntervalMs());
        ImGui::Text("클랩 간격: %.2f ms", rhythm.clapIntervalMs());
        ImGui::TextUnformatted("패턴: 킥 / 킥 / 킥 / 클랩 + 하이햇");
        ImGui::Text("경과 시간: %.2f ms", elapsedMs);
        ImGui::Text("목표 클랩: %.2f ms", rhythm.nearestClapTimeMs(adjustedElapsedMs) + judgementOffsetMs);
        ImGui::Text("오디오 장치: %s", audioReady ? "준비됨" : "사용 불가");
        ImGui::EndChild();

        ImGui::NextColumn();
        ImGui::BeginChild("Stats", ImVec2(0.0f, 0.0f), false);
        ImGui::TextUnformatted("통계");
        renderer.drawStats(stats);
        ImGui::Spacing();
        ImGui::TextWrapped("오프셋이 양수면 중앙 판정선보다 늦게 치는 편이고, 음수면 더 일찍 치는 편입니다.");
        ImGui::EndChild();

        ImGui::Columns(1);
        ImGui::End();

        ImGui::Render();
        SDL_SetRenderDrawColor(sdlRenderer, 11, 18, 26, 255);
        SDL_RenderClear(sdlRenderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), sdlRenderer);
        SDL_RenderPresent(sdlRenderer);
    }

    audio.shutdown();
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
