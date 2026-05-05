//─────────────────────────────────────────────────────────────────────────────
//  entry.cpp  —  VST3 plugin entry point
//  Registers Processor and Controller in the module factory.
//─────────────────────────────────────────────────────────────────────────────

#include "public.sdk/source/main/pluginfactory.h"
#include "VCACompressorProcessor.h"
#include "VCACompressorController.h"
#include "version.h"

using namespace VCAComp;
using namespace Steinberg;
using namespace Steinberg::Vst;

//─── Module-level init / exit (optional platform hooks) ──────────────────────

bool InitModule()   { return true; }
bool DeinitModule() { return true; }

//─── Factory definition ───────────────────────────────────────────────────────

BEGIN_FACTORY_DEF(VENDOR_NAME, VENDOR_URL, VENDOR_EMAIL)

    //── 1. Audio Processor ────────────────────────────────────────────────────
    DEF_CLASS2(
        INLINE_UID_FROM_FUID(VCA_PROCESSOR_UID),
        PClassInfo::kManyInstances,
        kVstAudioEffectClass,
        PLUGIN_NAME,
        Vst::kDistributable,
        PLUGIN_CATEGORY,
        FULL_VERSION_STR,
        kVstVersionString,
        VCACompressorProcessor::createInstance
    )

    //── 2. Edit Controller ────────────────────────────────────────────────────
    DEF_CLASS2(
        INLINE_UID_FROM_FUID(VCA_CONTROLLER_UID),
        PClassInfo::kManyInstances,
        kVstComponentControllerClass,
        PLUGIN_NAME " Controller",
        0,
        "",
        FULL_VERSION_STR,
        kVstVersionString,
        VCACompressorController::createInstance
    )

END_FACTORY
