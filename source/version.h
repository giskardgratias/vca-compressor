#pragma once

// ─── Plugin UIDs ──────────────────────────────────────────────────────────────
// Generate your own with guidgen / uuidgen before distributing commercially.

// Processor UID  (audio processing component)
#define VCA_PROCESSOR_UID \
    Steinberg::FUID(0xA1B2C3D4, 0xE5F60718, 0x92AB3CDE, 0xF0123456)

// Controller UID (edit controller / parameter host)
#define VCA_CONTROLLER_UID \
    Steinberg::FUID(0xB2C3D4E5, 0xF6071829, 0xA3BC4DEF, 0x01234567)

// ─── Version ─────────────────────────────────────────────────────────────────
#define MAJOR_VERSION_STR   "1"
#define MINOR_VERSION_STR   "0"
#define PATCH_VERSION_STR   "0"
#define BUILD_VERSION_STR   "0"

#define FULL_VERSION_STR    MAJOR_VERSION_STR "." MINOR_VERSION_STR "." PATCH_VERSION_STR

#define PLUGIN_NAME         "VCA Compressor"
#define VENDOR_NAME         "YourStudio"
#define VENDOR_URL          "https://yourstudio.com"
#define VENDOR_EMAIL        "support@yourstudio.com"
#define PLUGIN_CATEGORY     Steinberg::Vst::PlugType::kFx
