#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "AudioEngine.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr double kSampleRate = 48000.0;
constexpr double kPi = 3.14159265358979323846;
}

const char* toString(BeatSound sound) {
    switch (sound) {
    case BeatSound::Kick:
        return "Kick";
    case BeatSound::Snare:
        return "Snare";
    case BeatSound::Clap:
        return "Clap";
    case BeatSound::HiHat:
        return "Hi-Hat";
    case BeatSound::Tick:
        return "Tick";
    }

    return "Unknown";
}

bool AudioEngine::initialize() {
    if (m_initialized) {
        return true;
    }

    auto* device = new ma_device {};
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = ma_format_f32;
    config.playback.channels = 2;
    config.sampleRate = static_cast<ma_uint32>(kSampleRate);
    config.dataCallback = &AudioEngine::dataCallback;
    config.pUserData = this;

    if (ma_device_init(nullptr, &config, device) != MA_SUCCESS) {
        delete device;
        return false;
    }

    if (ma_device_start(device) != MA_SUCCESS) {
        ma_device_uninit(device);
        delete device;
        return false;
    }

    m_deviceHandle = device;
    m_initialized = true;
    return true;
}

void AudioEngine::shutdown() {
    if (!m_initialized) {
        return;
    }

    auto* device = m_deviceHandle;
    ma_device_uninit(device);
    delete device;
    m_deviceHandle = nullptr;
    m_initialized = false;
}

void AudioEngine::trigger(BeatSound sound) {
    std::scoped_lock lock(m_mutex);

    for (auto& voice : m_voices) {
        if (!voice.active) {
            voice.sound = sound;
            voice.ageSeconds = 0.0;
            voice.active = true;
            voice.noiseState = voice.noiseState * 1664525u + 1013904223u;
            return;
        }
    }

    m_voices.front() = Voice {sound, 0.0, true, 0xCAFEBABEu};
}

void AudioEngine::dataCallback(ma_device* device, void* output, const void* input, ma_uint32 frameCount) {
    (void)input;
    auto* self = static_cast<AudioEngine*>(device->pUserData);
    self->render(static_cast<float*>(output), frameCount);
}

void AudioEngine::render(float* output, unsigned int frameCount) {
    std::scoped_lock lock(m_mutex);
    const float dt = static_cast<float>(1.0 / kSampleRate);

    for (unsigned int frame = 0; frame < frameCount; ++frame) {
        float mixed = 0.0f;

        for (auto& voice : m_voices) {
            if (voice.active) {
                mixed += sampleVoice(voice, dt);
            }
        }

        mixed = std::clamp(mixed, -0.9f, 0.9f);
        output[frame * 2] = mixed;
        output[frame * 2 + 1] = mixed;
    }
}

float AudioEngine::sampleVoice(Voice& voice, float dt) {
    const double t = voice.ageSeconds;
    float sample = 0.0f;
    bool finished = false;

    switch (voice.sound) {
    case BeatSound::Kick: {
        const double env = std::exp(-t * 10.0);
        const double freq = 120.0 - t * 70.0;
        sample = static_cast<float>(std::sin(2.0 * kPi * freq * t) * env * 0.9);
        finished = t > 0.35;
        break;
    }
    case BeatSound::Snare: {
        const double env = std::exp(-t * 18.0);
        sample = nextNoise(voice) * static_cast<float>(env * 0.75);
        finished = t > 0.22;
        break;
    }
    case BeatSound::Clap: {
        const double env1 = std::exp(-std::max(0.0, t - 0.00) * 30.0);
        const double env2 = std::exp(-std::max(0.0, t - 0.02) * 35.0);
        const double env3 = std::exp(-std::max(0.0, t - 0.04) * 40.0);
        sample = nextNoise(voice) * static_cast<float>((env1 + env2 + env3) * 0.25);
        finished = t > 0.18;
        break;
    }
    case BeatSound::HiHat: {
        const double env = std::exp(-t * 45.0);
        sample = nextNoise(voice) * static_cast<float>(env * 0.4);
        finished = t > 0.10;
        break;
    }
    case BeatSound::Tick: {
        const double env = std::exp(-t * 55.0);
        sample = static_cast<float>(std::sin(2.0 * kPi * 1800.0 * t) * env * 0.35);
        finished = t > 0.08;
        break;
    }
    }

    voice.ageSeconds += dt;
    if (finished) {
        voice.active = false;
    }

    return sample;
}

float AudioEngine::nextNoise(Voice& voice) {
    voice.noiseState = voice.noiseState * 1664525u + 1013904223u;
    const float normalized = static_cast<float>((voice.noiseState >> 8) & 0x00FFFFFFu) / 8388607.5f;
    return normalized - 1.0f;
}
