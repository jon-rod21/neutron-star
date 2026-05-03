#pragma once

#include <vector>

namespace audio {

// Lightweight Schroeder-style mono reverb (feedback comb bank + all-pass chain).
class SimpleReverb
{
public:
    explicit SimpleReverb(double sampleRate);
    float tick(float x);

private:
    class Comb
    {
    public:
        Comb(int delaySamples, float feedback);
        float process(float x);

    private:
        float feedback_;
        std::vector<float> line_;
        size_t pos_ = 0;
    };

    class Allpass
    {
    public:
        Allpass(int delaySamples, float coeff);
        float process(float x);

    private:
        float coeff_;
        std::vector<float> line_;
        size_t pos_ = 0;
    };

    std::vector<Comb> combs_;
    std::vector<Allpass> allpass_;
};

} // namespace audio
