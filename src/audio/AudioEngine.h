#pragma once

#include "GenerativeSynth.h"

#include <atomic>
#include <mutex>
#include <thread>

struct SimulationUI; // defined in SimulationUI.h

namespace audio {

class AudioEngine
{
public:
    AudioEngine();
    ~AudioEngine();

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    bool start();
    void stop();

    void syncFromUi(const SimulationUI& ui);

    bool isRunning() const { return running_.load(); }
    bool deviceReady() const { return deviceReady_.load(); }

private:
    void workerLoop();

    static SynthParams paramsFromUi(const SimulationUI& ui);

    std::atomic<bool> running_{false};
    std::atomic<bool> deviceReady_{false};
    std::thread worker_;

    mutable std::mutex paramsMutex_;
    SynthParams params_;

    GenerativeSynth synth_{44100.0};
};

} // namespace audio
