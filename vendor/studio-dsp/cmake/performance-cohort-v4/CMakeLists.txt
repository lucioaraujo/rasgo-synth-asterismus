cmake_minimum_required(VERSION 3.16)
set(RASGO_SYNTH_PERFORMANCE_COHORT_V4_VERSION 4.0.0)
set(_cohort_root "${CMAKE_CURRENT_LIST_DIR}/../../src/dsp")
set(_cohort_files
    "DejaVuVoice.hpp|e37fca4fe259674806e95aa064a1a8baefe9382a7b0f1a5ac71df5e93ebe5a60"
    "AtropeloVoice.hpp|5de4ba9e7b8b89179eecee2e555150790839387fad6177ba2247dc4e9c4d1fc0"
    "GaloVoice.hpp|84aea1857c227b0022c2a53d3f0a607e7071bc95b41b53f79e273f7e770c414a"
    "Oscillator.hpp|28348a5341620fb2c5d54e3dd29cff7c67919437349cdc427a153606bd7b0885"
    "MoogFilter.hpp|c470cba056d14e2922b6ccec5c17b60dd5f099b22a684f8cd83602fd9d599f18"
    "MultibandFilter.hpp|af1fd0f7c1cc308a3fc603d8e079b91e5eb3171dbc3b79c55f32a511e6eaff76"
    "ModalResonator.hpp|5e87d761f3f9b07ee337234b4e402fb6571e2dba44ce09df4f1593649628b996"
    "BreathExciter.hpp|794e9eaf88e4a8848ac0c1b176c2593fc60b488b8f99da6d2d36cc4320dedf48")
foreach(_entry IN LISTS _cohort_files)
    string(REPLACE "|" ";" _parts "${_entry}")
    list(GET _parts 0 _name)
    list(GET _parts 1 _expected)
    file(SHA256 "${_cohort_root}/${_name}" _actual)
    if(NOT _actual STREQUAL _expected)
        message(FATAL_ERROR "Performance cohort v4 audit mismatch: ${_name}")
    endif()
endforeach()
add_library(rasgo_synth_performance_cohort_v4 INTERFACE)
add_library(rasgo::performance_cohort_v4 ALIAS rasgo_synth_performance_cohort_v4)
target_compile_features(rasgo_synth_performance_cohort_v4 INTERFACE cxx_std_17)
target_include_directories(rasgo_synth_performance_cohort_v4 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/../../src")
target_compile_definitions(rasgo_synth_performance_cohort_v4 INTERFACE RASGO_SYNTH_PERFORMANCE_COHORT_V4_VERSION=40000)
