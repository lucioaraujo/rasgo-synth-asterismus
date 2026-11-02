cmake_minimum_required(VERSION 3.16)

# Audited boundary for the digital Performance cohort. The host consumes only
# these fixed-state DSP headers; it never configures PieceRenderer or Studio.
set(RASGO_SYNTH_PERFORMANCE_COHORT_V3_VERSION 3.0.0)
set(_cohort_root "${CMAKE_CURRENT_LIST_DIR}/../../src/dsp")
set(_cohort_files
    "WalshOscillator.hpp|3161082f18ca18373e7d6cff5f12938035c0552caa2fac9e2c6a9e34948263f6"
    "HardSyncVoice.hpp|4e2b6e7b5a0b5f0d6c297a9800618093e2e444f1fbad214d34c216f43e6dddba"
    "AtariPunkConsole.hpp|fa54ea364e64a425e45409267bf3f6638e1d29738aebf71129972c9d80cc7d48"
    "Oscillator.hpp|28348a5341620fb2c5d54e3dd29cff7c67919437349cdc427a153606bd7b0885")

foreach(_entry IN LISTS _cohort_files)
    string(REPLACE "|" ";" _parts "${_entry}")
    list(GET _parts 0 _name)
    list(GET _parts 1 _expected)
    file(SHA256 "${_cohort_root}/${_name}" _actual)
    if(NOT _actual STREQUAL _expected)
        message(FATAL_ERROR "Performance cohort v3 audit mismatch: ${_name}")
    endif()
endforeach()

add_library(rasgo_synth_performance_cohort_v3 INTERFACE)
add_library(rasgo::performance_cohort_v3 ALIAS rasgo_synth_performance_cohort_v3)
target_compile_features(rasgo_synth_performance_cohort_v3 INTERFACE cxx_std_17)
target_include_directories(rasgo_synth_performance_cohort_v3 INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/../../src")
target_compile_definitions(rasgo_synth_performance_cohort_v3 INTERFACE
    RASGO_SYNTH_PERFORMANCE_COHORT_V3_VERSION=30000)
