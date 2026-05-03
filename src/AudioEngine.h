#pragma once

#include <array>
#include <cstdint>
#include <mutex>
#include <vector>

#include "miniaudio.h"

enum class BeatSound {
    Kick = 0,
    Snare,
    Clap,
    HiHat,
    Tick
};

const char* toString(BeatSound sound);

class AudioEngine {
public:
    bool initialize();
    void shutdown();
    void trigger(BeatSound sound);
    [[nodiscard]] bool initialized() const { return m_initialized; }

private:
    struct Voice {
        BeatSound sound {BeatSound::Tick};
        double ageSeconds {0.0};
        bool active {false};
        uint32_t noiseState {0x12345678u};
    };

    static void dataCallback(ma_device* device, void* output, const void* input, ma_uint32 frameCount);
    void render(float* output, unsigned int frameCount);
    float sampleVoice(Voice& voice, float dt);
    float nextNoise(Voice& voice);

    bool m_initialized {false};
    std::mutex m_mutex;
    std::array<Voice, 32> m_voices {};
    ma_device* m_deviceHandle {nullptr};
};
