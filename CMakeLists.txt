cmake_minimum_required(VERSION 3.15)
project(VCACompressor VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Path to your local VST3 SDK ──────────────────────────────────────────────
# Set this variable before running cmake:
#   cmake -DVST3_SDK_ROOT=/path/to/vst3sdk ..
if(NOT DEFINED VST3_SDK_ROOT)
    set(VST3_SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../vst3sdk"
        CACHE PATH "Path to the Steinberg VST3 SDK root")
endif()

if(NOT EXISTS "${VST3_SDK_ROOT}/CMakeLists.txt")
    message(FATAL_ERROR
        "VST3 SDK not found at '${VST3_SDK_ROOT}'.\n"
        "Download it from https://github.com/steinbergmedia/vst3sdk\n"
        "and pass -DVST3_SDK_ROOT=<path> to cmake.")
endif()

# ── VST3 SDK ─────────────────────────────────────────────────────────────────
set(SMTG_ADD_VSTGUI      OFF CACHE BOOL "" FORCE)  # No GUI (parameter panel only)
set(SMTG_CREATE_BUNDLE_FOR_WINDOWS ON CACHE BOOL "" FORCE)
add_subdirectory(${VST3_SDK_ROOT} ${CMAKE_BINARY_DIR}/vst3sdk)

# ── Plugin sources ────────────────────────────────────────────────────────────
set(PLUGIN_SOURCES
    source/entry.cpp
    source/VCACompressorProcessor.cpp
    source/VCACompressorController.cpp
    source/VCACompressorDSP.cpp
)

smtg_add_vst3plugin(${PROJECT_NAME}
    ${PLUGIN_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    source
    ${VST3_SDK_ROOT}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    sdk
)

smtg_target_setup_as_vst3_plugin(${PROJECT_NAME})

# ── Output name ───────────────────────────────────────────────────────────────
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "VCA Compressor"
)
