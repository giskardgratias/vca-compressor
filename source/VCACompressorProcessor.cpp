#include "VCACompressorProcessor.h"
#include "VCACompressorController.h"
#include "VCACompressorIDs.h"
#include "version.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/base/ibstream.h"

#include <algorithm>
#include <cstring>

namespace VCAComp {

using namespace Steinberg;
using namespace Steinberg::Vst;

// ─────────────────────────────────────────────────────────────────────────────

VCACompressorProcessor::VCACompressorProcessor()
{
    setControllerClass(VCA_CONTROLLER_UID);
}

// ─── IComponent ──────────────────────────────────────────────────────────────

tresult PLUGIN_API VCACompressorProcessor::initialize(FUnknown* context)
{
    tresult result = AudioEffect::initialize(context);
    if (result != kResultOk) return result;

    // Stereo in / stereo out
    addAudioInput (STR16("Stereo In"),  SpeakerArr::kStereo);
    addAudioOutput(STR16("Stereo Out"), SpeakerArr::kStereo);

    return kResultOk;
}

tresult PLUGIN_API VCACompressorProcessor::terminate()
{
    return AudioEffect::terminate();
}

// ─── IAudioProcessor ─────────────────────────────────────────────────────────

tresult PLUGIN_API VCACompressorProcessor::canProcessSampleSize(int32 symbolicSampleSize)
{
    return (symbolicSampleSize == kSample32 || symbolicSampleSize == kSample64)
           ? kResultTrue : kResultFalse;
}

tresult PLUGIN_API VCACompressorProcessor::setupProcessing(ProcessSetup& setup)
{
    mDSP.setSampleRate(setup.sampleRate);
    return AudioEffect::setupProcessing(setup);
}

// ─── Param application ────────────────────────────────────────────────────────

void VCACompressorProcessor::applyParam(ParamID id, double norm)
{
    using namespace VCAComp;

    switch (id)
    {
        case kThreshold:
            mParams.threshold = norm;
            mDSP.setThreshold(normToPlain(norm, kMinThreshold, kMaxThreshold));
            break;
        case kRatio:
            mParams.ratio = norm;
            mDSP.setRatio(normToPlain(norm, kMinRatio, kMaxRatio));
            break;
        case kAttack:
            mParams.attack = norm;
            mDSP.setAttack(normToPlain(norm, kMinAttack, kMaxAttack));
            break;
        case kRelease:
            mParams.release = norm;
            mDSP.setRelease(normToPlain(norm, kMinRelease, kMaxRelease));
            break;
        case kMakeup:
            mParams.makeup = norm;
            mDSP.setMakeupGain(normToPlain(norm, kMinMakeup, kMaxMakeup));
            break;
        case kKnee:
            mParams.knee = norm;
            mDSP.setKnee(normToPlain(norm, kMinKnee, kMaxKnee));
            break;
        case kMix:
            mParams.mix = norm;
            mDSP.setMix(normToPlain(norm, kMinMix, kMaxMix));
            break;
        case kHPFFreq:
            mParams.hpf = norm;
            mDSP.setHPF(static_cast<int>(norm * 3.0 + 0.5));
            break;
        case kBypass:
            mParams.bypass = norm;
            mDSP.setBypass(norm >= 0.5);
            break;
        default: break;
    }
}

void VCACompressorProcessor::applyParamChanges(IParameterChanges* paramChanges)
{
    if (!paramChanges) return;

    const int32 numQueues = paramChanges->getParameterCount();
    for (int32 q = 0; q < numQueues; ++q)
    {
        IParamValueQueue* queue = paramChanges->getParameterData(q);
        if (!queue) continue;

        const ParamID id = queue->getParameterId();
        int32  numPts    = queue->getPointCount();
        int32  sampleOffset;
        ParamValue value;

        // Take the last value in the queue (end-of-block)
        if (queue->getPoint(numPts - 1, sampleOffset, value) == kResultOk)
            applyParam(id, value);
    }
}

// ─── Main process ─────────────────────────────────────────────────────────────

tresult PLUGIN_API VCACompressorProcessor::process(ProcessData& data)
{
    // ── Handle parameter changes ──────────────────────────────────────────────
    applyParamChanges(data.inputParameterChanges);

    // ── Handle audio ──────────────────────────────────────────────────────────
    if (data.numSamples > 0 &&
        data.numInputs  > 0 && data.inputs[0].numChannels  >= 2 &&
        data.numOutputs > 0 && data.outputs[0].numChannels >= 2)
    {
        float* inL  = static_cast<float*>(data.inputs[0].channelBuffers32[0]);
        float* inR  = static_cast<float*>(data.inputs[0].channelBuffers32[1]);
        float* outL = static_cast<float*>(data.outputs[0].channelBuffers32[0]);
        float* outR = static_cast<float*>(data.outputs[0].channelBuffers32[1]);

        mDSP.processBlock(inL, inR, outL, outR, data.numSamples);

        mGainReductionDB.store(mDSP.getGainReductionDB());

        // ── Output GR meter to host ───────────────────────────────────────────
        if (data.outputParameterChanges)
        {
            int32 index = 0;
            IParamValueQueue* grQueue =
                data.outputParameterChanges->addParameterData(kGRMeter, index);

            if (grQueue)
            {
                // Normalise GR: 0 = no reduction (-0 dB), 1 = max (-60 dB)
                const float   grDB   = mGainReductionDB.load();
                const double  grNorm = std::clamp(-grDB / 60.0, 0.0, 1.0);
                grQueue->addPoint(0, grNorm, index);
            }
        }
    }
    else
    {
        // Silence pass-through when no valid audio
        if (data.numOutputs > 0 && data.outputs[0].numChannels >= 2)
        {
            std::memset(data.outputs[0].channelBuffers32[0], 0,
                        sizeof(float) * data.numSamples);
            std::memset(data.outputs[0].channelBuffers32[1], 0,
                        sizeof(float) * data.numSamples);
        }
    }

    return kResultOk;
}

// ─── State serialisation ──────────────────────────────────────────────────────

tresult PLUGIN_API VCACompressorProcessor::getState(IBStream* state)
{
    IBStreamer streamer(state, kLittleEndian);
    streamer.writeDouble(mParams.threshold);
    streamer.writeDouble(mParams.ratio);
    streamer.writeDouble(mParams.attack);
    streamer.writeDouble(mParams.release);
    streamer.writeDouble(mParams.makeup);
    streamer.writeDouble(mParams.knee);
    streamer.writeDouble(mParams.mix);
    streamer.writeDouble(mParams.hpf);
    streamer.writeDouble(mParams.bypass);
    return kResultOk;
}

tresult PLUGIN_API VCACompressorProcessor::setState(IBStream* state)
{
    IBStreamer streamer(state, kLittleEndian);
    double v;
    if (streamer.readDouble(v)) applyParam(kThreshold, v);
    if (streamer.readDouble(v)) applyParam(kRatio,     v);
    if (streamer.readDouble(v)) applyParam(kAttack,    v);
    if (streamer.readDouble(v)) applyParam(kRelease,   v);
    if (streamer.readDouble(v)) applyParam(kMakeup,    v);
    if (streamer.readDouble(v)) applyParam(kKnee,      v);
    if (streamer.readDouble(v)) applyParam(kMix,       v);
    if (streamer.readDouble(v)) applyParam(kHPFFreq,   v);
    if (streamer.readDouble(v)) applyParam(kBypass,    v);
    return kResultOk;
}

} // namespace VCAComp
