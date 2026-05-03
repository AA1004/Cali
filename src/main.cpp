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
#include <array>
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
        "Rhythm Offset Calibrator",
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

    BeatSound selectedSound = BeatSound::Tick;
    long long lastTriggeredBeat = -1;
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
                    stats.addSample(rhythm.timingErrorMs(timer.elapsedMs()));
                }
            }
        }

        const double elapsedMs = timer.elapsedMs();
        const long long beatIndex = running ? rhythm.currentBeatIndex(elapsedMs) : -1;

        if (running && audioReady && beatIndex > lastTriggeredBeat) {
            for (long long index = lastTriggeredBeat + 1; index <= beatIndex; ++index) {
                audio.trigger(selectedSound);
            }
            lastTriggeredBeat = beatIndex;
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(24.0f, 24.0f), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(1072.0f, 700.0f), ImGuiCond_Once);
        ImGui::Begin("Rhythm Offset Calibrator", nullptr, ImGuiWindowFlags_NoCollapse);

        ImGui::Columns(2, nullptr, true);

        ImGui::TextUnformatted("Transport");
        float bpm = static_cast<float>(rhythm.bpm());
        if (ImGui::SliderFloat("BPM", &bpm, 40.0f, 300.0f, "%.0f")) {
            rhythm.setBpm(static_cast<double>(bpm));
            if (running) {
                timer.reset();
                timer.start();
                lastTriggeredBeat = -1;
            }
        }

        ImGui::Text("Beat interval: %.2f ms", rhythm.beatIntervalMs());

        static const std::array<BeatSound, 5> soundOptions {
            BeatSound::Kick,
            BeatSound::Snare,
            BeatSound::Clap,
            BeatSound::HiHat,
            BeatSound::Tick
        };

        if (ImGui::BeginCombo("Beat sound", toString(selectedSound))) {
            for (int i = 0; i < static_cast<int>(soundOptions.size()); ++i) {
                const bool isSelected = soundOptions[i] == selectedSound;
                if (ImGui::Selectable(toString(soundOptions[i]), isSelected)) {
                    selectedSound = soundOptions[i];
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::Button(running ? "Stop" : "Start", ImVec2(120.0f, 0.0f))) {
            if (running) {
                timer.stop();
                running = false;
            } else {
                timer.reset();
                timer.start();
                stats.clear();
                lastTriggeredBeat = -1;
                running = true;
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Reset Stats", ImVec2(120.0f, 0.0f))) {
            stats.clear();
        }

        ImGui::Spacing();
        ImGui::TextWrapped("Press Space when the moving marker crosses the center line. Negative is early, positive is late.");
        ImGui::Text("Elapsed: %.2f ms", elapsedMs);
        ImGui::Text("Nearest beat: %.2f ms", rhythm.nearestBeatTimeMs(elapsedMs));
        ImGui::Text("Audio device: %s", audioReady ? "Ready" : "Unavailable");

        ImGui::NextColumn();

        const double phase = running ? rhythm.centeredPhase(elapsedMs) : -0.5;
        renderer.drawCalibrationLane(phase, stats.latest());
        renderer.drawStats(stats);

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
