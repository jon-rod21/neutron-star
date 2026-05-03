#include "AudioEngine.h"
#include "SimulationUI.h"

#if defined(__APPLE__)
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

namespace audio {
namespace {

#if defined(__APPLE__) && defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

constexpr int kSampleRate = 44100;
constexpr int kFramesPerBuffer = 512;
constexpr int kNumBuffers = 4;

inline void floatToS16(const float* src, short* dst, int n)
{
    for (int i = 0; i < n; ++i)
    {
        float x = src[i];
        x = std::max(-1.f, std::min(1.f, x));
        dst[i] = static_cast<short>(x * 32767.f);
    }
}

} // namespace

AudioEngine::AudioEngine() = default;

AudioEngine::~AudioEngine()
{
    stop();
}

SynthParams AudioEngine::paramsFromUi(const SimulationUI& ui)
{
    SynthParams p;
    p.enabled = ui.audioEnabled;
    p.masterGain = std::clamp(ui.masterVolume, 0.f, 1.f);
    p.beamSpeed = ui.beamSpeed;
    p.beamIntensity = ui.beamIntensity;
    p.beamRadius = ui.beamRadius;
    p.starRadius = ui.starRadius;
    p.starMass = ui.starMassSolar;
    p.rotationPeriod = ui.rotationPeriod;
    p.emissionStrength = ui.emissionStrength;
    p.lensStrength = ui.lensStrength;
    const float denom = std::max(0.05f, ui.starRadius);
    p.lensCompactness = (ui.starMassSolar / 1.4f) / denom;
    return p;
}

void AudioEngine::syncFromUi(const SimulationUI& ui)
{
    const SynthParams copy = paramsFromUi(ui);
    std::lock_guard<std::mutex> lock(paramsMutex_);
    params_ = copy;
}

bool AudioEngine::start()
{
    if (worker_.joinable())
        return deviceReady_.load();

    running_.store(true);
    worker_ = std::thread([this] { workerLoop(); });
    return true;
}

void AudioEngine::stop()
{
    running_.store(false);
    if (worker_.joinable())
        worker_.join();
    deviceReady_.store(false);
}

void AudioEngine::workerLoop()
{
    ALCdevice* device = alcOpenDevice(nullptr);
    if (!device)
    {
        std::cerr << "[audio] alcOpenDevice failed\n";
        running_.store(false);
        return;
    }

    ALCcontext* context = alcCreateContext(device, nullptr);
    if (!context || !alcMakeContextCurrent(context))
    {
        std::cerr << "[audio] OpenAL context failed\n";
        alcCloseDevice(device);
        running_.store(false);
        return;
    }

    ALuint source = 0;
    ALuint buffers[kNumBuffers]{};
    alGenSources(1, &source);
    alGenBuffers(kNumBuffers, buffers);

    if (alGetError() != AL_NO_ERROR)
    {
        std::cerr << "[audio] OpenAL gen buffers/source failed\n";
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(context);
        alcCloseDevice(device);
        running_.store(false);
        return;
    }

    synth_ = GenerativeSynth(static_cast<double>(kSampleRate));

    std::vector<float> floatBlock(static_cast<size_t>(kFramesPerBuffer));
    std::vector<short> pcm(static_cast<size_t>(kFramesPerBuffer));

    auto fillBuffer = [&](ALuint bufName) {
        SynthParams local{};
        {
            std::lock_guard<std::mutex> lock(paramsMutex_);
            local = params_;
        }
        synth_.process(floatBlock.data(), kFramesPerBuffer, local);
        floatToS16(floatBlock.data(), pcm.data(), kFramesPerBuffer);
        alBufferData(bufName, AL_FORMAT_MONO16, pcm.data(),
                     static_cast<ALsizei>(pcm.size() * sizeof(short)), kSampleRate);
    };

    for (int i = 0; i < kNumBuffers; ++i)
    {
        fillBuffer(buffers[i]);
        alSourceQueueBuffers(source, 1, &buffers[i]);
    }

    alSourcef(source, AL_GAIN, 1.f);
    alSourcePlay(source);

    deviceReady_.store(true);

    while (running_.load())
    {
        ALint processed = 0;
        alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);

        while (processed-- > 0)
        {
            ALuint buf = 0;
            alSourceUnqueueBuffers(source, 1, &buf);
            fillBuffer(buf);
            alSourceQueueBuffers(source, 1, &buf);
        }

        ALint state = AL_STOPPED;
        alGetSourcei(source, AL_SOURCE_STATE, &state);
        if (state != AL_PLAYING && running_.load())
            alSourcePlay(source);

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    alSourceStop(source);
    alDeleteSources(1, &source);
    alDeleteBuffers(kNumBuffers, buffers);

    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);

    deviceReady_.store(false);
}

#if defined(__APPLE__) && defined(__clang__)
#pragma clang diagnostic pop
#endif

} // namespace audio
