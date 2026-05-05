#pragma once
#include <cmath>
#include <algorithm>
#include <array>

namespace VCAComp {

// ─────────────────────────────────────────────────────────────────────────────
//  Biquad filter (for sidechain HPF)
// ─────────────────────────────────────────────────────────────────────────────
struct Biquad
{
    double b0=1, b1=0, b2=0, a1=0, a2=0;
    double z1=0, z2=0;

    void reset() { z1 = z2 = 0.0; }

    // Design a 2nd-order Butterworth HPF
    void setHPF(double freqHz, double sampleRate)
    {
        if (freqHz <= 0.0) { b0=1; b1=b2=a1=a2=0; return; }
        const double w0  = 2.0 * M_PI * freqHz / sampleRate;
        const double cosw = std::cos(w0);
        const double sinw = std::sin(w0);
        const double Q   = 0.7071;
        const double alpha = sinw / (2.0 * Q);
        const double a0  = 1.0 + alpha;
        b0 = ( (1.0 + cosw) * 0.5 ) / a0;
        b1 = (-(1.0 + cosw)      ) / a0;
        b2 = ( (1.0 + cosw) * 0.5) / a0;
        a1 = (-2.0 * cosw        ) / a0;
        a2 = ( 1.0 - alpha       ) / a0;
    }

    inline double process(double x)
    {
        const double y = b0*x + z1;
        z1 = b1*x - a1*y + z2;
        z2 = b2*x - a2*y;
        return y;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  VCA Compressor DSP — stereo, feed-forward, RMS detector
// ─────────────────────────────────────────────────────────────────────────────
class VCACompressorDSP
{
public:
    // ── Construction ──────────────────────────────────────────────────────────
    VCACompressorDSP() = default;

    // ── Setup ─────────────────────────────────────────────────────────────────
    void setSampleRate(double sr);
    void reset();

    // ── Parameter setters (call from process thread, thread-safe via atomics) ─
    void setThreshold (double dBFS);          // -60 … 0
    void setRatio     (double ratio);          // 1 … 20
    void setAttack    (double ms);             // 0.1 … 300
    void setRelease   (double ms);             // 10 … 3000
    void setMakeupGain(double dB);             // 0 … 30
    void setKnee      (double dBWidth);        // 0 … 20
    void setMix       (double percent);        // 0 … 100
    void setHPF       (int slot);              // 0=off 1=60 2=100 3=200 Hz
    void setBypass    (bool b);

    // ── Process a stereo block ────────────────────────────────────────────────
    //  in/out : non-interleaved float arrays (L and R pointers)
    void processBlock(float* inL, float* inR,
                      float* outL, float* outR,
                      int numSamples);

    // ── Metering (call from UI thread) ────────────────────────────────────────
    float getGainReductionDB() const { return mGainReductionDB; }

private:
    // ── Internal helpers ──────────────────────────────────────────────────────
    double computeGain(double detectorLinear) const;   // static gain from detector level
    void   updateCoefficients();

    // ── State ─────────────────────────────────────────────────────────────────
    double mSampleRate   = 44100.0;

    // Parameters (plain units)
    double mThresholdLin = 1.0;    // linear threshold
    double mThresholdDB  = 0.0;
    double mRatio        = 4.0;
    double mAttackCoef   = 0.0;    // per-sample coefficient
    double mReleaseCoef  = 0.0;
    double mMakeupLin    = 1.0;
    double mKneeDB       = 2.0;
    double mMix          = 1.0;    // 0…1
    bool   mBypass       = false;

    // Envelope detector state (shared L+R for linked stereo)
    double mEnvelopeL = 0.0;
    double mEnvelopeR = 0.0;
    double mGainState = 1.0;       // current applied gain (smoothed)

    // RMS integration (100ms window approximation via leaky integrator)
    double mRMScoef   = 0.0;
    double mRMSstateL = 0.0;
    double mRMSstateR = 0.0;

    // Sidechain HPF (stereo)
    Biquad mHPFL, mHPFR;
    int    mHPFSlot = 0;

    // Metering (atomic-ish — updated in process, read in UI)
    float  mGainReductionDB = 0.0f;

    static constexpr double kDbFloor = -120.0;

    inline double lin2dB(double lin) const
    {
        return (lin < 1e-10) ? kDbFloor : 20.0 * std::log10(lin);
    }
    inline double dB2lin(double db) const
    {
        return std::pow(10.0, db * 0.05);
    }
};

} // namespace VCAComp
