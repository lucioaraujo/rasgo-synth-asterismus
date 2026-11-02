cmake_minimum_required(VERSION 3.16)

# Public, header-only boundary for the single audited DSP body currently
# authorised for Rasgo Synth Performance. This subpackage intentionally does
# not configure the Studio app, demos, tests or the legacy core root target.
set(RASGO_SYNTH_MODAL_MATTER_VERSION 1.0.0)
set(RASGO_SYNTH_MODAL_MATTER_HEADER
    "${CMAKE_CURRENT_LIST_DIR}/../../src/dsp/ModalResonator.hpp")
set(RASGO_SYNTH_MODAL_MATTER_SHA256
    "5e87d761f3f9b07ee337234b4e402fb6571e2dba44ce09df4f1593649628b996")

if(NOT EXISTS "${RASGO_SYNTH_MODAL_MATTER_HEADER}")
    message(FATAL_ERROR "ModalMatter package cannot find ModalResonator.hpp")
endif()

file(SHA256 "${RASGO_SYNTH_MODAL_MATTER_HEADER}" modal_matter_actual_sha256)
if(NOT modal_matter_actual_sha256 STREQUAL RASGO_SYNTH_MODAL_MATTER_SHA256)
    message(FATAL_ERROR
        "ModalResonator.hpp differs from audited package v${RASGO_SYNTH_MODAL_MATTER_VERSION}: "
        "expected ${RASGO_SYNTH_MODAL_MATTER_SHA256}, got ${modal_matter_actual_sha256}")
endif()

add_library(rasgo_synth_modal_matter_v1 INTERFACE)
add_library(rasgo::modal_matter_v1 ALIAS rasgo_synth_modal_matter_v1)
target_compile_features(rasgo_synth_modal_matter_v1 INTERFACE cxx_std_17)
target_include_directories(rasgo_synth_modal_matter_v1 INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/../../src")
target_compile_definitions(rasgo_synth_modal_matter_v1 INTERFACE
    RASGO_SYNTH_MODAL_MATTER_VERSION=10000
    RASGO_SYNTH_MODAL_MATTER_SHA256="${RASGO_SYNTH_MODAL_MATTER_SHA256}")
