#include "GenerativeSynth.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace audio {
namespace {

constexpr float kTwoPi = 6.2831853f;

// Human hearing range; used to clamp every audible-frequency parameter.
constexpr float kAudibleMinHz = 20.f;
constexpr float kAudibleMaxHz = 20000.f;

inline float clampf(float x, float lo, float hi)
{
    return std::max(lo, std::min(hi, x));
}

inline float clampHz(float hz)
{
    return clampf(hz, kAudibleMinHz, kAudibleMaxHz);
}

inline float sawTick(float& phase01, float hz, double sr)
{
    const float inc = static_cast<float>(hz / sr);
    phase01 += inc;
    phase01 -= std::floor(phase01);
    return 2.f * phase01 - 1.f;
}

inline float centsToRatio(float cents)
{
    return std::pow(2.f, cents / 1200.f);
}

inline float softClip(float x)
{
    return x / (1.f + std::fabs(x));
}

} // namespace

GenerativeSynth::GenerativeSynth(double sampleRate) : sr_(sampleRate), reverb_(sampleRate) {}

void GenerativeSynth::process(float* monoOut, int numFrames, const SynthParams& p)
{
    if (!monoOut || numFrames <= 0)
        return;

    if (!p.enabled)
    {
        std::memset(monoOut, 0, static_cast<size_t>(numFrames) * sizeof(float));
        return;
    }

    const float refRadius = 1.0f;
    // Star bass: pitch inversely related to radius, never below human hearing floor.
    float starHz = 48.f * refRadius / std::max(0.3f, p.starRadius);
    starHz = clampHz(clampf(starHz, kAudibleMinHz, 220.f));

    const float beamRootHz = clampHz(clampf(95.f + p.beamSpeed * 48.f, 60.f, 420.f));
    const float detuneBeam = 5.f + clampf(p.beamIntensity, 0.f, 20.f) * 1.6f;
    // Detune of star voice now scales with mass (heavier = a touch wider Reese).
    const float massNorm = clampf(p.starMass / 1.4f, 0.3f, 2.5f);
    const float detuneStar = 6.f + massNorm * 5.f;

    const float lpCutoff =
        std::exp(std::log(250.f) + (1.f - clampf(p.lensStrength, 0.f, 1.f)) * std::log(18000.f / 250.f));
    lpMaster_.setLowPass(static_cast<float>(sr_), clampHz(lpCutoff), 0.707f);

    const float compact = clampf(p.lensCompactness, 0.2f, 6.f);
    const float eqGainDb = clampf(-6.f + (compact - 1.f) * 4.f, -14.f, 10.f);
    eqFreqSm_ += 0.12f * ((400.f + compact * 180.f) - eqFreqSm_);
    eq_.setPeaking(static_cast<float>(sr_), clampHz(eqFreqSm_), eqGainDb, 1.15f);

    // Emission still controls star-voice brightness via its low-pass.
    const float voiceLpHz = clampHz(clampf(520.f + p.emissionStrength * 420.f, 180.f, 6500.f));
    starVoiceLp_.setLowPass(static_cast<float>(sr_), voiceLpHz, 0.65f);

    // Emission also drives a tanh waveshaper on the bass for more grit as it grows.
    const float emissionDrive = 1.f + clampf(p.emissionStrength, 0.f, 5.f) * 0.9f;
    const float driveNorm = std::tanh(emissionDrive); // for level compensation

    const float wetAmt = clampf(0.06f + p.lensStrength * 0.42f + compact * 0.03f, 0.f, 0.85f);

    const float rotOmega = kTwoPi / std::max(0.25f, p.rotationPeriod);
    const float beamLfoOmega = kTwoPi * (0.12f + p.beamSpeed * 0.22f);

    // Audibly perceptible vibrato (~80 cents at full swing) tied to rotation period.
    constexpr float kVibratoDepth = 0.048f;

    for (int n = 0; n < numFrames; ++n)
    {
        beamPhaseLfo_ += beamLfoOmega / static_cast<float>(sr_);
        if (beamPhaseLfo_ > kTwoPi)
            beamPhaseLfo_ -= kTwoPi;

        rotationLfo_ += rotOmega / static_cast<float>(sr_);
        if (rotationLfo_ > kTwoPi)
            rotationLfo_ -= kTwoPi;

        const float phaseWalk = 0.09f * std::sin(beamPhaseLfo_);

        float beamSum = 0.f;
        for (int i = 0; i < kBeamOsc; ++i)
        {
            const float spread = static_cast<float>(i - (kBeamOsc / 2));
            const float hz = clampHz(beamRootHz * centsToRatio(spread * detuneBeam + phaseWalk * 17.f));
            beamSum += sawTick(beamPhase_[i], hz, sr_);
        }
        beamSum /= static_cast<float>(kBeamOsc);

        const float beamEnv =
            (0.06f + 0.045f * clampf(p.beamIntensity, 0.f, 20.f)) *
            (0.55f + 0.45f * clampf(p.beamRadius / 0.2f, 0.2f, 3.f));
        const float phaseShimmer = 1.f + 0.12f * std::sin(beamPhaseLfo_ * 1.37f + phaseWalk);
        // Supersaw level trimmed by 20% to better balance against the bass.
        beamSum *= beamEnv * phaseShimmer * 0.8f;

        const float vibrato = 1.f + kVibratoDepth * std::sin(rotationLfo_);

        float starSum = 0.f;
        for (int i = 0; i < kStarOsc; ++i)
        {
            const float spread = static_cast<float>(i - (kStarOsc / 2));
            const float hz = clampHz(starHz * vibrato * centsToRatio(spread * detuneStar));
            starSum += sawTick(starPhase_[i], hz, sr_);
        }
        starSum /= static_cast<float>(kStarOsc);
        starSum = starVoiceLp_.process(starSum);

        // Emission-driven distortion (tanh waveshaper, level-compensated).
        starSum = std::tanh(starSum * emissionDrive) / driveNorm;

        // Bass level now comes from star mass, not emission strength.
        const float starGain = (0.05f + 0.10f * massNorm) * (0.85f + 0.15f * compact);

        float mix = beamSum + starSum * starGain;
        mix = eq_.process(mix);
        mix = lpMaster_.process(mix);

        const float wet = reverb_.tick(mix);
        mix = (1.f - wetAmt) * mix + wetAmt * wet;

        mix *= p.masterGain;
        mix = softClip(mix);
        monoOut[n] = mix * 0.92f;
    }
}

} // namespace audio
