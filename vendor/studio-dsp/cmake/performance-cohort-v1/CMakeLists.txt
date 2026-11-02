cmake_minimum_required(VERSION 3.16)

# Audited header-only boundary for the first real-time Performance cohort.
# It exposes selected core modules without configuring PieceRenderer, demos,
# the Studio app or the legacy root target.
set(RASGO_SYNTH_PERFORMANCE_COHORT_VERSION 1.0.0)
set(_cohort_root "${CMAKE_CURRENT_LIST_DIR}/../../src/dsp")

set(_cohort_files
    "AcidBasslineVoice.hpp|a608924a1b4cfb0dd724e5dabbe968a1b4c7346a3ccc95959986dd4f59eb9229"
    "DrumVoices.hpp|8d663ca3cdce6ce2e09c9cca2eba83850e14ac812e79bfda86b5bf1f7090271f"
    "GendynOscillator.hpp|228c3de2e707fd40f5a75e93753fb4630e8a099a69fd77db144a39f7c46f1b06"
    "PolyrhythmDivider.hpp|e7b7f9adaaa83fc3e58960361a3a6c1737f4dfb328bb440d4f9391928431adff"
    "Rungler.hpp|2ab0d6f79e6ea24ff70e32364be52d686637894d87f1e185bd59817c814f4dac"
    "BuchlaRandomSource.hpp|065f4e761c8e8788edc79a0027d87e0fbe73c1a5052008e33866266a57d05d0f"
    "Oscillator.hpp|28348a5341620fb2c5d54e3dd29cff7c67919437349cdc427a153606bd7b0885"
    "PolivoksFilter.hpp|5b7731b44101fb711e54dc845e4eed3fcc65e30815d921e4cb494e6d50dba523"
    "ColoredNoise.hpp|c0a9e5335fa5aaa15b2b9caf37cd6d57c34e36bf5adbeed91737487668d5cd00"
    "SallenKeyFilter.hpp|a5da0138067c47520f3ba6f35113abf501020556e6f926d5c1baa9119a1cd955")

foreach(_entry IN LISTS _cohort_files)
    string(REPLACE "|" ";" _parts "${_entry}")
    list(GET _parts 0 _name)
    list(GET _parts 1 _expected)
    set(_path "${_cohort_root}/${_name}")
    if(NOT EXISTS "${_path}")
        message(FATAL_ERROR "Performance cohort v1 cannot find ${_name}")
    endif()
    file(SHA256 "${_path}" _actual)
    if(NOT _actual STREQUAL _expected)
        message(FATAL_ERROR
            "${_name} differs from audited Performance cohort v${RASGO_SYNTH_PERFORMANCE_COHORT_VERSION}: "
            "expected ${_expected}, got ${_actual}")
    endif()
endforeach()

add_library(rasgo_synth_performance_cohort_v1 INTERFACE)
add_library(rasgo::performance_cohort_v1 ALIAS rasgo_synth_performance_cohort_v1)
target_compile_features(rasgo_synth_performance_cohort_v1 INTERFACE cxx_std_17)
target_include_directories(rasgo_synth_performance_cohort_v1 INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/../../src")
target_compile_definitions(rasgo_synth_performance_cohort_v1 INTERFACE
    RASGO_SYNTH_PERFORMANCE_COHORT_VERSION=10000)
