#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "VCACompressorIDs.h"

namespace VCAComp {

// ─────────────────────────────────────────────────────────────────────────────
//  VCACompressorController
//  Manages all automatable parameters, presets, and the generic editor UI
// ─────────────────────────────────────────────────────────────────────────────
class VCACompressorController : public Steinberg::Vst::EditController
{
public:
    VCACompressorController()  = default;
    ~VCACompressorController() = default;

    // ── Factory ───────────────────────────────────────────────────────────────
    static Steinberg::FUnknown* createInstance(void* /*context*/)
    {
        return static_cast<Steinberg::Vst::IEditController*>(
            new VCACompressorController());
    }

    // ── IPluginBase ───────────────────────────────────────────────────────────
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;

    // ── IEditController ───────────────────────────────────────────────────────
    Steinberg::tresult PLUGIN_API setComponentState(Steinberg::IBStream* state) override;

    Steinberg::Vst::ParamValue PLUGIN_API
        normalizedParamToPlain(Steinberg::Vst::ParamID id,
                               Steinberg::Vst::ParamValue norm) override;

    Steinberg::Vst::ParamValue PLUGIN_API
        plainParamToNormalized(Steinberg::Vst::ParamID id,
                               Steinberg::Vst::ParamValue plain) override;

    Steinberg::tresult PLUGIN_API
        getParamStringByValue(Steinberg::Vst::ParamID id,
                              Steinberg::Vst::ParamValue norm,
                              Steinberg::Vst::String128 str) override;

    Steinberg::tresult PLUGIN_API
        getParamValueByString(Steinberg::Vst::ParamID id,
                              Steinberg::Vst::TChar* str,
                              Steinberg::Vst::ParamValue& out) override;
};

} // namespace VCAComp
