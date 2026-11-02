cmake_minimum_required(VERSION 3.16)

# Audited v2 boundary: breath and voice bodies proposed for Performance.
set(RASGO_SYNTH_PERFORMANCE_COHORT_V2_VERSION 2.0.0)
set(_cohort_root "${CMAKE_CURRENT_LIST_DIR}/../../src/dsp")
set(_cohort_files
    "Envelope.hpp|ca1c9300ae0bd8727ee5c18c3a0054d67072cdfa82333513c33b7ea810693014"
    "Oscillator.hpp|28348a5341620fb2c5d54e3dd29cff7c67919437349cdc427a153606bd7b0885"
    "LfoBank.hpp|324b737587c71f1bc9892cf9b92b4db5d1a7bed043b413809ac7ecde456ca5d6"
    "MultibandFilter.hpp|af1fd0f7c1cc308a3fc603d8e079b91e5eb3171dbc3b79c55f32a511e6eaff76"
    "FmFluteVoice.hpp|9f8247fc5db3b6535fb57cee5c04664d065e21338526da0c24224fdf7123386c"
    "ThereminVoice.hpp|ca5b5a97d855bc758111db38f625bd836d901e7992145766ba6c4185c998f7fd"
    "ScatVoice.hpp|e8921369409cdb46a7690e9adf239d3ed5d3298c3815f45a5b9e55ce2bd0cf36"
    "ChoirEnsemble.hpp|e6a99c8ecb8fc5bf2b57851a0e77eb764b4a7957ae6a4e24d9e451dad581dba3"
    "VosimOscillator.hpp|16fcd7e0de8402db33e82d588cddb2deb83ad2ecb5800429b0c9870be7735407")
foreach(_entry IN LISTS _cohort_files)
    string(REPLACE "|" ";" _parts "${_entry}")
    list(GET _parts 0 _name)
    list(GET _parts 1 _expected)
    file(SHA256 "${_cohort_root}/${_name}" _actual)
    if(NOT _actual STREQUAL _expected)
        message(FATAL_ERROR "Performance cohort v2 audit mismatch: ${_name}")
    endif()
endforeach()
add_library(rasgo_synth_performance_cohort_v2 INTERFACE)
add_library(rasgo::performance_cohort_v2 ALIAS rasgo_synth_performance_cohort_v2)
target_compile_features(rasgo_synth_performance_cohort_v2 INTERFACE cxx_std_17)
target_include_directories(rasgo_synth_performance_cohort_v2 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/../../src")
target_compile_definitions(rasgo_synth_performance_cohort_v2 INTERFACE RASGO_SYNTH_PERFORMANCE_COHORT_V2_VERSION=20000)
