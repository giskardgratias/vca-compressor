#pragma once
#include "pluginterfaces/vst/vsttypes.h"

namespace VCAComp {

// ─── Parameter IDs ────────────────────────────────────────────────────────────
enum ParamID : Steinberg::Vst::ParamID
{
    kThreshold  = 0,   // dBFS  : -60 … 0
    kRatio      = 1,   // ratio :  1  … 20  (∞ beyond 20)
    kAttack     = 2,   // ms    :  0.1… 300
    kRelease    = 3,   // ms    : 10  … 3000
    kMakeup     = 4,   // dB    :  0  … +30
    kKnee       = 5,   // dB    :  0  … 20
    kMix        = 6,   // %     :  0  … 100  (parallel compression)
    kHPFFreq    = 7,   // enum  :  0=off 1=60Hz 2=100Hz 3=200Hz
    kBypass     = 8,   // bool
    kGRMeter    = 9,   // read-only: gain-reduction meter (output from processor)
};

// ─── Default values (plain units) ─────────────────────────────────────────────
static constexpr double kDefThreshold  = -18.0;  // dBFS
static constexpr double kDefRatio      =   4.0;  // :1
static constexpr double kDefAttack     =  10.0;  // ms
static constexpr double kDefRelease    = 100.0;  // ms
static constexpr double kDefMakeup     =   0.0;  // dB
static constexpr double kDefKnee       =   2.0;  // dB
static constexpr double kDefMix        = 100.0;  // %
static constexpr double kDefHPFFreq    =   0.0;  // off

// ─── Ranges ────────────────────────────────────────────────────────────────────
static constexpr double kMinThreshold  = -60.0;
static constexpr double kMaxThreshold  =   0.0;

static constexpr double kMinRatio      =   1.0;
static constexpr double kMaxRatio      =  20.0;

static constexpr double kMinAttack     =   0.1;
static constexpr double kMaxAttack     = 300.0;

static constexpr double kMinRelease    =  10.0;
static constexpr double kMaxRelease    = 3000.0;

static constexpr double kMinMakeup     =   0.0;
static constexpr double kMaxMakeup     =  30.0;

static constexpr double kMinKnee       =   0.0;
static constexpr double kMaxKnee       =  20.0;

static constexpr double kMinMix        =   0.0;
static constexpr double kMaxMix        = 100.0;

// ─── Helper : normalized ↔ plain ──────────────────────────────────────────────
inline double normToPlain(double norm, double lo, double hi)
{
    return lo + norm * (hi - lo);
}
inline double plainToNorm(double plain, double lo, double hi)
{
    return (plain - lo) / (hi - lo);
}

} // namespace VCAComp
