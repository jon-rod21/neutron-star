#pragma once

#include "Biquad.h"
#include "SimpleReverb.h"

#include <cstddef>

namespace audio {

struct SynthParams
{
    bool enabled = true;
    float masterGain = 0.65f;

    float beamSpeed = 2.f;
    float beamIntensity = 5.f;
    float beamRadius = 0.2f;

    float starRadius = 1.f;
    float starMass = 1.4f;
    float rotationPeriod = 5.f;
    float emissionStrength = 1.f;

    float lensStrength = 0.25f;
    float lensCompactness = 1.f;
};

class GenerativeSynth
{
public:
    explicit GenerativeSynth(double sampleRate);

    void process(float* monoOut, int numFrames, const SynthParams& p);

private:
    double sr_;

    static constexpr int kBeamOsc = 5;
    static constexpr int kStarOsc = 3;

    float beamPhase_[kBeamOsc]{};
    float starPhase_[kStarOsc]{};

    float beamPhaseLfo_ = 0.f;
    float rotationLfo_ = 0.f;

    float eqFreqSm_ = 900.f;

    Biquad eq_;
    Biquad lpMaster_;
    Biquad starVoiceLp_;
    SimpleReverb reverb_;
};

} // namespace audio
