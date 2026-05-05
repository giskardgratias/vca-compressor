#include "VCACompressorController.h"
#include "version.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ustring.h"

#include <cmath>
#include <cstring>
#include <cstdio>

namespace VCAComp {

using namespace Steinberg;
using namespace Steinberg::Vst;

// ─── Helpers ─────────────────────────────────────────────────────────────────

static void utf8ToStr128(const char* src, String128 dst)
{
    UString128 ustr(dst);
    ustr.fromAscii(src);
}

// ─── Initialize ──────────────────────────────────────────────────────────────

tresult PLUGIN_API VCACompressorController::initialize(FUnknown* context)
{
    tresult result = EditController::initialize(context);
    if (result != kResultOk) return result;

    // ── Register parameters ──────────────────────────────────────────────────
    //   Each parameter: id, title, unit, minPlain, maxPlain, default(normalised)

    auto addParam = [&](ParamID id, const char* title, const char* unit,
                        double minP, double maxP, double defPlain,
                        int32  flags = ParameterInfo::kCanAutomate)
    {
        Parameter* p = new RangeParameter(
            USTRING(title), id, USTRING(unit),
            minP, maxP, defPlain, 0, flags);
        parameters.addParameter(p);
    };

    addParam(kThreshold, "Threshold",   "dB",  kMinThreshold, kMaxThreshold, kDefThreshold);
    addParam(kRatio,     "Ratio",       ":1",  kMinRatio,     kMaxRatio,     kDefRatio);
    addParam(kAttack,    "Attack",      "ms",  kMinAttack,    kMaxAttack,    kDefAttack);
    addParam(kRelease,   "Release",     "ms",  kMinRelease,   kMaxRelease,   kDefRelease);
    addParam(kMakeup,    "Makeup",      "dB",  kMinMakeup,    kMaxMakeup,    kDefMakeup);
    addParam(kKnee,      "Knee",        "dB",  kMinKnee,      kMaxKnee,      kDefKnee);
    addParam(kMix,       "Mix",         "%",   kMinMix,       kMaxMix,       kDefMix);

    // Sidechain HPF: 4-position enum (0=Off, 1=60Hz, 2=100Hz, 3=200Hz)
    {
        StringListParameter* hpf = new StringListParameter(
            USTRING("SC HPF"), kHPFFreq, USTRING(""),
            ParameterInfo::kCanAutomate | ParameterInfo::kIsList);
        hpf->appendString(USTRING("Off"));
        hpf->appendString(USTRING("60 Hz"));
        hpf->appendString(USTRING("100 Hz"));
        hpf->appendString(USTRING("200 Hz"));
        parameters.addParameter(hpf);
    }

    // Bypass
    addParam(kBypass, "Bypass", "",
             0.0, 1.0, 0.0,
             ParameterInfo::kCanAutomate | ParameterInfo::kIsBypass);

    // GR meter (output / read-only)
    {
        Parameter* gr = new RangeParameter(
            USTRING("GR"), kGRMeter, USTRING("dB"),
            -60.0, 0.0, 0.0, 0,
            ParameterInfo::kIsReadOnly | ParameterInfo::kIsHidden);
        parameters.addParameter(gr);
    }

    return kResultOk;
}

// ─── Sync state from processor ────────────────────────────────────────────────

tresult PLUGIN_API VCACompressorController::setComponentState(IBStream* state)
{
    if (!state) return kResultFalse;

    IBStreamer streamer(state, kLittleEndian);
    double v;

    auto read = [&](ParamID id) -> bool {
        if (!streamer.readDouble(v)) return false;
        setParamNormalized(id, v);
        return true;
    };

    read(kThreshold);
    read(kRatio);
    read(kAttack);
    read(kRelease);
    read(kMakeup);
    read(kKnee);
    read(kMix);
    read(kHPFFreq);
    read(kBypass);

    return kResultOk;
}

// ─── Normalised ↔ plain conversions ──────────────────────────────────────────

ParamValue PLUGIN_API VCACompressorController::normalizedParamToPlain(
    ParamID id, ParamValue norm)
{
    switch (id)
    {
        case kThreshold: return normToPlain(norm, kMinThreshold, kMaxThreshold);
        case kRatio:     return normToPlain(norm, kMinRatio,     kMaxRatio);
        case kAttack:    return normToPlain(norm, kMinAttack,    kMaxAttack);
        case kRelease:   return normToPlain(norm, kMinRelease,   kMaxRelease);
        case kMakeup:    return normToPlain(norm, kMinMakeup,    kMaxMakeup);
        case kKnee:      return normToPlain(norm, kMinKnee,      kMaxKnee);
        case kMix:       return normToPlain(norm, kMinMix,       kMaxMix);
        case kHPFFreq:   return std::round(norm * 3.0);
        case kBypass:    return norm >= 0.5 ? 1.0 : 0.0;
        default:         return EditController::normalizedParamToPlain(id, norm);
    }
}

ParamValue PLUGIN_API VCACompressorController::plainParamToNormalized(
    ParamID id, ParamValue plain)
{
    switch (id)
    {
        case kThreshold: return plainToNorm(plain, kMinThreshold, kMaxThreshold);
        case kRatio:     return plainToNorm(plain, kMinRatio,     kMaxRatio);
        case kAttack:    return plainToNorm(plain, kMinAttack,    kMaxAttack);
        case kRelease:   return plainToNorm(plain, kMinRelease,   kMaxRelease);
        case kMakeup:    return plainToNorm(plain, kMinMakeup,    kMaxMakeup);
        case kKnee:      return plainToNorm(plain, kMinKnee,      kMaxKnee);
        case kMix:       return plainToNorm(plain, kMinMix,       kMaxMix);
        case kHPFFreq:   return plain / 3.0;
        case kBypass:    return plain >= 0.5 ? 1.0 : 0.0;
        default:         return EditController::plainParamToNormalized(id, plain);
    }
}

// ─── String display ───────────────────────────────────────────────────────────

tresult PLUGIN_API VCACompressorController::getParamStringByValue(
    ParamID id, ParamValue norm, String128 str)
{
    char buf[64];
    const double plain = normalizedParamToPlain(id, norm);

    switch (id)
    {
        case kThreshold:
            std::snprintf(buf, sizeof(buf), "%.1f dB", plain);
            break;
        case kRatio:
            if (plain >= 19.9)
                std::snprintf(buf, sizeof(buf), "∞:1");
            else
                std::snprintf(buf, sizeof(buf), "%.1f:1", plain);
            break;
        case kAttack:
        case kRelease:
            if (plain >= 1000.0)
                std::snprintf(buf, sizeof(buf), "%.2f s", plain * 0.001);
            else
                std::snprintf(buf, sizeof(buf), "%.1f ms", plain);
            break;
        case kMakeup:
            std::snprintf(buf, sizeof(buf), "+%.1f dB", plain);
            break;
        case kKnee:
            std::snprintf(buf, sizeof(buf), "%.1f dB", plain);
            break;
        case kMix:
            std::snprintf(buf, sizeof(buf), "%.0f %%", plain);
            break;
        case kHPFFreq: {
            const char* labels[] = { "Off", "60 Hz", "100 Hz", "200 Hz" };
            int idx = static_cast<int>(plain);
            if (idx < 0) idx = 0;
            if (idx > 3) idx = 3;
            std::snprintf(buf, sizeof(buf), "%s", labels[idx]);
            break;
        }
        case kBypass:
            std::snprintf(buf, sizeof(buf), "%s", plain >= 0.5 ? "On" : "Off");
            break;
        case kGRMeter:
            std::snprintf(buf, sizeof(buf), "%.1f dB", plain);
            break;
        default:
            return EditController::getParamStringByValue(id, norm, str);
    }

    utf8ToStr128(buf, str);
    return kResultOk;
}

tresult PLUGIN_API VCACompressorController::getParamValueByString(
    ParamID id, TChar* str, ParamValue& out)
{
    // Allow host to type in values numerically
    char buf[64] = {};
    UString ustr(str, 128);
    ustr.toAscii(buf, sizeof(buf));
    double val = std::atof(buf);

    switch (id)
    {
        case kThreshold: out = plainParamToNormalized(id, val); return kResultOk;
        case kRatio:     out = plainParamToNormalized(id, val); return kResultOk;
        case kAttack:    out = plainParamToNormalized(id, val); return kResultOk;
        case kRelease:   out = plainParamToNormalized(id, val); return kResultOk;
        case kMakeup:    out = plainParamToNormalized(id, val); return kResultOk;
        case kKnee:      out = plainParamToNormalized(id, val); return kResultOk;
        case kMix:       out = plainParamToNormalized(id, val); return kResultOk;
        default: return EditController::getParamValueByString(id, str, out);
    }
}

} // namespace VCAComp
