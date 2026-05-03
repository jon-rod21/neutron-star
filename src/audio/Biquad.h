#pragma once

#include <cmath>

// Direct-form-I biquad.
struct Biquad
{
    float b0 = 1.f, b1 = 0.f, b2 = 0.f;
    float a1 = 0.f, a2 = 0.f;
    float x1 = 0.f, x2 = 0.f;
    float y1 = 0.f, y2 = 0.f;

    void reset()
    {
        x1 = x2 = y1 = y2 = 0.f;
    }

    void setLowPass(float sampleRate, float freqHz, float Q)
    {
        const float w0 = 2.f * 3.14159265f * freqHz / sampleRate;
        const float cosw0 = std::cos(w0);
        const float sinw0 = std::sin(w0);
        const float alpha = sinw0 / (2.f * Q);

        const float b0_ = (1.f - cosw0) * 0.5f;
        const float b1_ = 1.f - cosw0;
        const float b2_ = (1.f - cosw0) * 0.5f;
        const float a0 = 1.f + alpha;
        const float a1_ = -2.f * cosw0;
        const float a2_ = 1.f - alpha;

        b0 = b0_ / a0;
        b1 = b1_ / a0;
        b2 = b2_ / a0;
        a1 = a1_ / a0;
        a2 = a2_ / a0;
    }

    // Peaking EQ (RBJ), gainDb bell at freqHz.
    void setPeaking(float sampleRate, float freqHz, float gainDb, float Q)
    {
        const float A = std::pow(10.f, gainDb / 40.f);
        const float w0 = 2.f * 3.14159265f * freqHz / sampleRate;
        const float cosw0 = std::cos(w0);
        const float sinw0 = std::sin(w0);
        const float alpha = sinw0 / (2.f * Q);

        const float b0_ = 1.f + alpha * A;
        const float b1_ = -2.f * cosw0;
        const float b2_ = 1.f - alpha * A;
        const float a0 = 1.f + alpha / A;
        const float a1_ = -2.f * cosw0;
        const float a2_ = 1.f - alpha / A;

        b0 = b0_ / a0;
        b1 = b1_ / a0;
        b2 = b2_ / a0;
        a1 = a1_ / a0;
        a2 = a2_ / a0;
    }

    inline float process(float x)
    {
        const float y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        x2 = x1;
        x1 = x;
        y2 = y1;
        y1 = y;
        return y;
    }
};
