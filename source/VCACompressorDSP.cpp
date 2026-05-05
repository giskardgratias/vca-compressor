#include "VCACompressorDSP.h"
#include <cstring>

namespace VCAComp {

static constexpr double HPF_FREQS[] = { 0.0, 60.0, 100.0, 200.0 };

// ─── Setup ───────────────────────────────────────────────────────────────────

void VCACompressorDSP::setSampleRate(double sr)
{
    mSampleRate = sr;
    // RMS integrator: ~50ms time constant gives a smooth RMS estimate
    mRMScoef = std::exp(-1.0 / (0.050 * sr));
    updateCoefficients();
    reset();
}

void VCACompressorDSP::reset()
{
    mEnvelopeL = mEnvelopeR = 0.0;
    mGainState  = 1.0;
    mRMSstateL  = mRMSstateR = 0.0;
    mGainReductionDB = 0.0f;
    mHPFL.reset();
    mHPFR.reset();
}

// ─── Parameter setters ────────────────────────────────────────────────────────

void VCACompressorDSP::setThreshold(double dBFS)
{
    mThresholdDB  = dBFS;
    mThresholdLin = dB2lin(dBFS);
}

void VCACompressorDSP::setRatio(double ratio)
{
    mRatio = (ratio >= 20.0) ? 1e6 : ratio;  // treat 20 as ∞
}

void VCACompressorDSP::setAttack(double ms)
{
    // Attack coefficient: time to reach 1/e of target
    mAttackCoef = std::exp(-1.0 / (0.001 * ms * mSampleRate));
}

void VCACompressorDSP::setRelease(double ms)
{
    mReleaseCoef = std::exp(-1.0 / (0.001 * ms * mSampleRate));
}

void VCACompressorDSP::setMakeupGain(double dB)
{
    mMakeupLin = dB2lin(dB);
}

void VCACompressorDSP::setKnee(double dBWidth)
{
    mKneeDB = dBWidth;
}

void VCACompressorDSP::setMix(double percent)
{
    mMix = percent * 0.01;
}

void VCACompressorDSP::setHPF(int slot)
{
    mHPFSlot = slot;
    double freq = (slot >= 0 && slot <= 3) ? HPF_FREQS[slot] : 0.0;
    mHPFL.setHPF(freq, mSampleRate);
    mHPFR.setHPF(freq, mSampleRate);
}

void VCACompressorDSP::setBypass(bool b)
{
    mBypass = b;
}

void VCACompressorDSP::updateCoefficients()
{
    // Re-apply to ensure coefficients match current sample rate
    setAttack (10.0);
    setRelease(100.0);
    double freq = (mHPFSlot >= 0 && mHPFSlot <= 3) ? HPF_FREQS[mHPFSlot] : 0.0;
    mHPFL.setHPF(freq, mSampleRate);
    mHPFR.setHPF(freq, mSampleRate);
}

// ─── Gain computer (static characteristic with soft knee) ────────────────────
//
//  Input  : linear detector level
//  Output : linear gain to apply (< 1.0 means reduction)
//
double VCACompressorDSP::computeGain(double detectorLinear) const
{
    const double xdB  = lin2dB(detectorLinear);   // detector in dB
    const double knee = mKneeDB * 0.5;
    const double T    = mThresholdDB;

    double gainDB = 0.0;

    if (mKneeDB > 0.0 && xdB > (T - knee) && xdB < (T + knee))
    {
        // Soft knee region — quadratic interpolation
        const double diff = (xdB - T + knee);
        gainDB = (1.0 / mRatio - 1.0) * (diff * diff) / (2.0 * mKneeDB);
    }
    else if (xdB >= (T + knee))
    {
        // Above threshold (hard or above knee)
        gainDB = T + (xdB - T) / mRatio - xdB;
    }
    // Below threshold: gainDB stays 0 (unity gain)

    return dB2lin(gainDB);
}

// ─── Main processing block ────────────────────────────────────────────────────

void VCACompressorDSP::processBlock(
    float* inL, float* inR,
    float* outL, float* outR,
    int numSamples)
{
    if (mBypass)
    {
        if (inL != outL) std::memcpy(outL, inL, sizeof(float) * numSamples);
        if (inR != outR) std::memcpy(outR, inR, sizeof(float) * numSamples);
        mGainReductionDB = 0.0f;
        return;
    }

    double peakGR = 1.0;  // track worst gain reduction for metering

    for (int i = 0; i < numSamples; ++i)
    {
        const double dryL = inL[i];
        const double dryR = inR[i];

        // ── 1. Sidechain signal (with optional HPF) ────────────────────────
        double scL = dryL;
        double scR = dryR;
        if (mHPFSlot > 0)
        {
            scL = mHPFL.process(scL);
            scR = mHPFR.process(scR);
        }

        // ── 2. RMS detector (stereo linked: max of L/R RMS) ───────────────
        mRMSstateL = mRMScoef * mRMSstateL + (1.0 - mRMScoef) * scL * scL;
        mRMSstateR = mRMScoef * mRMSstateR + (1.0 - mRMScoef) * scR * scR;
        const double rmsL   = std::sqrt(mRMSstateL);
        const double rmsR   = std::sqrt(mRMSstateR);
        const double detLev = std::max(rmsL, rmsR);

        // ── 3. Gain computer ───────────────────────────────────────────────
        const double targetGain = computeGain(detLev);

        // ── 4. Ballistics (attack / release with VCA-style branching) ──────
        //  VCA character: fast attack tracks down, slow release tracks up
        if (targetGain < mGainState)
            mGainState = mAttackCoef  * mGainState + (1.0 - mAttackCoef)  * targetGain;
        else
            mGainState = mReleaseCoef * mGainState + (1.0 - mReleaseCoef) * targetGain;

        peakGR = std::min(peakGR, mGainState);

        // ── 5. Apply gain + makeup ─────────────────────────────────────────
        const double totalGain = mGainState * mMakeupLin;

        const double wetL = dryL * totalGain;
        const double wetR = dryR * totalGain;

        // ── 6. Parallel mix (wet/dry blend) ───────────────────────────────
        outL[i] = static_cast<float>(dryL * (1.0 - mMix) + wetL * mMix);
        outR[i] = static_cast<float>(dryR * (1.0 - mMix) + wetR * mMix);
    }

    // ── 7. Update GR meter ────────────────────────────────────────────────────
    mGainReductionDB = static_cast<float>(lin2dB(peakGR));
}

} // namespace VCAComp
