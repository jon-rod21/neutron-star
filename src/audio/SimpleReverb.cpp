#include "SimpleReverb.h"

#include <algorithm>

namespace audio {

SimpleReverb::Comb::Comb(int delaySamples, float feedback) : feedback_(feedback), line_(std::max(1, delaySamples), 0.f) {}

float SimpleReverb::Comb::process(float x)
{
    const float out = line_[pos_];
    line_[pos_] = x + out * feedback_;
    pos_ = (pos_ + 1) % line_.size();
    return out;
}

SimpleReverb::Allpass::Allpass(int delaySamples, float coeff) : coeff_(coeff), line_(std::max(1, delaySamples), 0.f) {}

float SimpleReverb::Allpass::process(float x)
{
    const float delayed = line_[pos_];
    const float y = delayed + coeff_ * x;
    line_[pos_] = x - coeff_ * delayed;
    pos_ = (pos_ + 1) % line_.size();
    return y;
}

SimpleReverb::SimpleReverb(double sampleRate)
{
    const double scale = sampleRate / 44100.0;
    const int c0 = std::max(2, static_cast<int>(1557 * scale));
    const int c1 = std::max(2, static_cast<int>(1617 * scale));
    const int c2 = std::max(2, static_cast<int>(1491 * scale));
    combs_.emplace_back(c0, 0.805f);
    combs_.emplace_back(c1, 0.827f);
    combs_.emplace_back(c2, 0.783f);
    const int a0 = std::max(1, static_cast<int>(225 * scale));
    const int a1 = std::max(1, static_cast<int>(556 * scale));
    allpass_.emplace_back(a0, 0.5f);
    allpass_.emplace_back(a1, 0.5f);
}

float SimpleReverb::tick(float x)
{
    float s = 0.f;
    for (auto& c : combs_)
        s += c.process(x);
    s *= (1.f / 3.f);
    for (auto& a : allpass_)
        s = a.process(s);
    return s;
}

} // namespace audio
