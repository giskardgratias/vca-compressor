#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "VCACompressorDSP.h"
#include "VCACompressorIDs.h"
#include <atomic>

namespace VCAComp {

// ─────────────────────────────────────────────────────────────────────────────
//  VCACompressorProcessor
//  Inherits from AudioEffect (IComponent + IAudioProcessor)
// ─────────────────────────────────────────────────────────────────────────────
class VCACompressorProcessor : public Steinberg::Vst::AudioEffect
{
public:
    VCACompressorProcessor();
    ~VCACompressorProcessor() override = default;

    // ── Factory ───────────────────────────────────────────────────────────────
    static Steinberg::FUnknown* createInstance(void* /*context*/)
    {
        return static_cast<Steinberg::Vst::IAudioProcessor*>(
            new VCACompressorProcessor());
    }

    // ── IComponent ────────────────────────────────────────────────────────────
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;
    Steinberg::tresult PLUGIN_API terminate()  override;

    // ── IAudioProcessor ───────────────────────────────────────────────────────
    Steinberg::tresult PLUGIN_API setupProcessing(
        Steinberg::Vst::ProcessSetup& setup) override;

    Steinberg::tresult PLUGIN_API process(
        Steinberg::Vst::ProcessData& data) override;

    Steinberg::tresult PLUGIN_API canProcessSampleSize(
        Steinberg::int32 symbolicSampleSize) override;

    // ── State ─────────────────────────────────────────────────────────────────
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) override;
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) override;

private:
    void applyParamChanges(Steinberg::Vst::IParameterChanges* paramChanges);
    void applyParam(Steinberg::Vst::ParamID id, double normValue);

    VCACompressorDSP mDSP;

    // Cached normalised parameter values
    struct Params {
        double threshold = 0.7;  // default: -18 dBFS normalised
        double ratio     = 0.158;
        double attack    = 0.033;
        double release   = 0.030;
        double makeup    = 0.0;
        double knee      = 0.1;
        double mix       = 1.0;
        double hpf       = 0.0;
        double bypass    = 0.0;
    } mParams;

    // GR value written by process thread, read by controller via IParameterChanges
    std::atomic<float> mGainReductionDB { 0.0f };
};

} // namespace VCAComp
