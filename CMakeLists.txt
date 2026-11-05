cmake_minimum_required(VERSION 3.16)

# O padrão multiplataforma do RASGO
# (RASGO_DOCUMENTATION/PADRAO_DISTRIBUICAO_MULTIPLATAFORMA.md), antes de
# qualquer alvo (30/10, docs/PUBLICACAO.md):
# - Windows: runtime do Visual C++ estático (/MT), dentro do .exe — sem ele
#   o app não abre numa máquina sem o Redistributable;
# - macOS: Universal 2 (x86_64 e arm64) e alvo de implantação explícito,
#   antes do project(). APPLE só existe depois do project(), por isso o
#   teste é pelo sistema hospedeiro. 10.15: o JUCE 9 pede 10.11, e o código
#   não usa std::filesystem nem from_chars/to_chars (verificado em 30/10).
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "macOS mínimo" FORCE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Universal 2" FORCE)
endif()
# A versão do Asterismus vive só aqui: o app, o SOBRE, o rodapé e os pacotes a
# leem daqui (4/11/2026; antes estava repetida à mão no código).
project(RasgoSynthPerformanceContracts VERSION 0.2.0 LANGUAGES C CXX)

include(CTest)

add_library(rasgo_synth_performance_contracts INTERFACE)
target_include_directories(rasgo_synth_performance_contracts
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(rasgo_synth_performance_contracts INTERFACE cxx_std_17)

option(RASGO_PERFORMANCE_BUILD_APP "Build the independent JUCE host" OFF)
set(RASGO_PERFORMANCE_JUCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/JUCE"
    CACHE PATH "Performance-owned JUCE checkout (not the Studio checkout)")
set(RASGO_PERFORMANCE_MODAL_MATTER_PACKAGE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/studio-dsp/cmake/modal-matter" CACHE PATH
    "Optional audited Rasgo modal-matter CMake package directory")
set(RASGO_PERFORMANCE_MODULE_COHORT_PACKAGE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/studio-dsp/cmake/performance-cohort-v1" CACHE PATH
    "Optional audited Rasgo Performance module-cohort CMake package directory")
set(RASGO_PERFORMANCE_MODULE_COHORT_V2_PACKAGE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/studio-dsp/cmake/performance-cohort-v2" CACHE PATH
    "Optional audited Rasgo Performance breath/voice cohort CMake package directory")
set(RASGO_PERFORMANCE_MODULE_COHORT_V3_PACKAGE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/studio-dsp/cmake/performance-cohort-v3" CACHE PATH
    "Optional audited Rasgo Performance digital cohort CMake package directory")
set(RASGO_PERFORMANCE_MODULE_COHORT_V4_PACKAGE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/studio-dsp/cmake/performance-cohort-v4" CACHE PATH
    "Optional audited Rasgo Performance memory/voice cohort CMake package directory")
set(RASGO_PERFORMANCE_FRACTURE_V5_PACKAGE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/studio-dsp/cmake/performance-fracture-v5" CACHE PATH
    "Optional audited Rasgo RasgoFracture v5 CMake package directory")

if(RASGO_PERFORMANCE_MODAL_MATTER_PACKAGE)
    if(NOT EXISTS "${RASGO_PERFORMANCE_MODAL_MATTER_PACKAGE}/CMakeLists.txt")
        message(FATAL_ERROR
            "RASGO_PERFORMANCE_MODAL_MATTER_PACKAGE must contain the audited modal-matter package")
    endif()
    add_subdirectory("${RASGO_PERFORMANCE_MODAL_MATTER_PACKAGE}"
                     "${CMAKE_BINARY_DIR}/rasgo-modal-matter" EXCLUDE_FROM_ALL)
    target_link_libraries(rasgo_synth_performance_contracts
        INTERFACE rasgo::modal_matter_v1)
    target_compile_definitions(rasgo_synth_performance_contracts
        INTERFACE RASGO_PERFORMANCE_WITH_MODAL_CORE=1)
endif()

if(RASGO_PERFORMANCE_MODULE_COHORT_V2_PACKAGE)
    if(NOT EXISTS "${RASGO_PERFORMANCE_MODULE_COHORT_V2_PACKAGE}/CMakeLists.txt")
        message(FATAL_ERROR "RASGO_PERFORMANCE_MODULE_COHORT_V2_PACKAGE must contain the audited cohort")
    endif()
    add_subdirectory("${RASGO_PERFORMANCE_MODULE_COHORT_V2_PACKAGE}"
                     "${CMAKE_BINARY_DIR}/rasgo-performance-cohort-v2" EXCLUDE_FROM_ALL)
    target_link_libraries(rasgo_synth_performance_contracts
        INTERFACE rasgo::performance_cohort_v2)
    target_compile_definitions(rasgo_synth_performance_contracts
        INTERFACE RASGO_PERFORMANCE_WITH_MODULE_COHORT_V2=1)
endif()

if(RASGO_PERFORMANCE_MODULE_COHORT_V3_PACKAGE)
    if(NOT EXISTS "${RASGO_PERFORMANCE_MODULE_COHORT_V3_PACKAGE}/CMakeLists.txt")
        message(FATAL_ERROR "RASGO_PERFORMANCE_MODULE_COHORT_V3_PACKAGE must contain the audited cohort")
    endif()
    add_subdirectory("${RASGO_PERFORMANCE_MODULE_COHORT_V3_PACKAGE}"
                     "${CMAKE_BINARY_DIR}/rasgo-performance-cohort-v3" EXCLUDE_FROM_ALL)
    target_link_libraries(rasgo_synth_performance_contracts
        INTERFACE rasgo::performance_cohort_v3)
    target_compile_definitions(rasgo_synth_performance_contracts
        INTERFACE RASGO_PERFORMANCE_WITH_MODULE_COHORT_V3=1)
endif()

if(RASGO_PERFORMANCE_MODULE_COHORT_V4_PACKAGE)
    if(NOT EXISTS "${RASGO_PERFORMANCE_MODULE_COHORT_V4_PACKAGE}/CMakeLists.txt")
        message(FATAL_ERROR "RASGO_PERFORMANCE_MODULE_COHORT_V4_PACKAGE must contain the audited cohort")
    endif()
    add_subdirectory("${RASGO_PERFORMANCE_MODULE_COHORT_V4_PACKAGE}"
                     "${CMAKE_BINARY_DIR}/rasgo-performance-cohort-v4" EXCLUDE_FROM_ALL)
    target_link_libraries(rasgo_synth_performance_contracts
        INTERFACE rasgo::performance_cohort_v4)
    target_compile_definitions(rasgo_synth_performance_contracts
        INTERFACE RASGO_PERFORMANCE_WITH_MODULE_COHORT_V4=1)
endif()

if(RASGO_PERFORMANCE_MODULE_COHORT_PACKAGE)
    if(NOT EXISTS "${RASGO_PERFORMANCE_MODULE_COHORT_PACKAGE}/CMakeLists.txt")
        message(FATAL_ERROR
            "RASGO_PERFORMANCE_MODULE_COHORT_PACKAGE must contain the audited Performance cohort package")
    endif()
    add_subdirectory("${RASGO_PERFORMANCE_MODULE_COHORT_PACKAGE}"
                     "${CMAKE_BINARY_DIR}/rasgo-performance-cohort" EXCLUDE_FROM_ALL)
    target_link_libraries(rasgo_synth_performance_contracts
        INTERFACE rasgo::performance_cohort_v1)
    target_compile_definitions(rasgo_synth_performance_contracts
        INTERFACE RASGO_PERFORMANCE_WITH_MODULE_COHORT_V1=1)
endif()

if(RASGO_PERFORMANCE_FRACTURE_V5_PACKAGE)
    if(NOT EXISTS "${RASGO_PERFORMANCE_FRACTURE_V5_PACKAGE}/CMakeLists.txt")
        message(FATAL_ERROR "RASGO_PERFORMANCE_FRACTURE_V5_PACKAGE must contain the audited fracture package")
    endif()
    add_subdirectory("${RASGO_PERFORMANCE_FRACTURE_V5_PACKAGE}"
                     "${CMAKE_BINARY_DIR}/rasgo-performance-fracture-v5" EXCLUDE_FROM_ALL)
    target_link_libraries(rasgo_synth_performance_contracts
        INTERFACE rasgo::performance_fracture_v5)
endif()

if(RASGO_PERFORMANCE_BUILD_APP)
    if(NOT EXISTS "${RASGO_PERFORMANCE_JUCE_ROOT}/CMakeLists.txt")
        message(FATAL_ERROR
            "RASGO_PERFORMANCE_BUILD_APP requires an independent JUCE checkout at "
            "RASGO_PERFORMANCE_JUCE_ROOT. Do not point it to Rasgo Synth Studio.")
    endif()
    add_subdirectory("${RASGO_PERFORMANCE_JUCE_ROOT}" "${CMAKE_BINARY_DIR}/performance-juce" EXCLUDE_FROM_ALL)
    add_subdirectory(src/app)
endif()

if(BUILD_TESTING)
    add_executable(rasgo_synth_performance_audio_measurement_tests
        tests/test_performance_audio_measurement.cpp)
    target_link_libraries(rasgo_synth_performance_audio_measurement_tests
        PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_audio_measurement_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_audio_measurement_tests
             COMMAND rasgo_synth_performance_audio_measurement_tests)

    add_executable(rasgo_synth_performance_contract_tests
        tests/test_performance_contracts.cpp)
    target_link_libraries(rasgo_synth_performance_contract_tests
        PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_contract_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_contract_tests
             COMMAND rasgo_synth_performance_contract_tests)

    add_executable(rasgo_synth_performance_relational_phenomena_tests
        tests/test_performance_relational_phenomena.cpp)
    target_link_libraries(rasgo_synth_performance_relational_phenomena_tests
        PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_relational_phenomena_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_relational_phenomena_tests
             COMMAND rasgo_synth_performance_relational_phenomena_tests)

    add_executable(rasgo_synth_performance_engine_tests
        tests/test_performance_engine.cpp)
    target_link_libraries(rasgo_synth_performance_engine_tests
        PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_engine_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_engine_tests
             COMMAND rasgo_synth_performance_engine_tests)

    # Eight stems are exercised only in an explicit calibration build. The
    # production target remains capped at four even on a four-core CPU.
    add_executable(rasgo_synth_performance_expanded_engine_tests
        tests/test_performance_engine.cpp)
    target_link_libraries(rasgo_synth_performance_expanded_engine_tests
        PRIVATE rasgo_synth_performance_contracts)
    target_compile_definitions(rasgo_synth_performance_expanded_engine_tests
        PRIVATE RASGO_PERFORMANCE_TEST_SAFE_BODY_LIMIT=8)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_expanded_engine_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_expanded_engine_tests
             COMMAND rasgo_synth_performance_expanded_engine_tests)

    add_executable(rasgo_synth_performance_morceau_lifecycle_tests
        tests/test_performance_morceau_lifecycle.cpp)
    target_link_libraries(rasgo_synth_performance_morceau_lifecycle_tests
        PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_morceau_lifecycle_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_morceau_lifecycle_tests
             COMMAND rasgo_synth_performance_morceau_lifecycle_tests)

    # Offline-only proof for a future fifth stem. The production host keeps
    # its measured safety cap of four; this target may never be linked into it.
    add_executable(rasgo_synth_performance_modal_fifth_stem_tests
        tests/test_modal_fifth_stem.cpp)
    target_link_libraries(rasgo_synth_performance_modal_fifth_stem_tests
        PRIVATE rasgo_synth_performance_contracts)
    target_compile_definitions(rasgo_synth_performance_modal_fifth_stem_tests
        PRIVATE RASGO_PERFORMANCE_TEST_SAFE_BODY_LIMIT=5)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_modal_fifth_stem_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_modal_fifth_stem_tests
             COMMAND rasgo_synth_performance_modal_fifth_stem_tests)

    add_executable(rasgo_synth_performance_stability_tests
        tests/test_performance_stability.cpp)
    target_link_libraries(rasgo_synth_performance_stability_tests
        PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_stability_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_stability_tests
             COMMAND rasgo_synth_performance_stability_tests)
    set_tests_properties(rasgo_synth_performance_stability_tests PROPERTIES LABELS stress)

    add_executable(rasgo_synth_performance_modal_fifth_stability_tests
        tests/test_performance_stability.cpp)
    target_link_libraries(rasgo_synth_performance_modal_fifth_stability_tests
        PRIVATE rasgo_synth_performance_contracts)
    target_compile_definitions(rasgo_synth_performance_modal_fifth_stability_tests
        PRIVATE RASGO_PERFORMANCE_TEST_SAFE_BODY_LIMIT=5)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_modal_fifth_stability_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_modal_fifth_stability_tests
             COMMAND rasgo_synth_performance_modal_fifth_stability_tests modal-five)
    set_tests_properties(rasgo_synth_performance_modal_fifth_stability_tests PROPERTIES LABELS stress)
    add_test(NAME rasgo_synth_performance_modal_fifth_smoke_tests
             COMMAND rasgo_synth_performance_modal_fifth_stability_tests modal-five-smoke)
    add_test(NAME rasgo_synth_performance_galo_stability_tests
             COMMAND rasgo_synth_performance_stability_tests galo)
    set_tests_properties(rasgo_synth_performance_galo_stability_tests PROPERTIES LABELS stress)
    add_test(NAME rasgo_synth_performance_expanded_eight_stability_tests
             COMMAND rasgo_synth_performance_stability_tests expanded-eight)
    set_tests_properties(rasgo_synth_performance_expanded_eight_stability_tests PROPERTIES LABELS stress)

    add_executable(rasgo_synth_performance_modal_adapter_tests
        tests/test_modal_matter_adapter.cpp)
    target_link_libraries(rasgo_synth_performance_modal_adapter_tests
        PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_modal_adapter_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_modal_adapter_tests
             COMMAND rasgo_synth_performance_modal_adapter_tests)

    add_executable(rasgo_synth_performance_fracture_adapter_tests
        tests/test_rasgo_fracture_adapter.cpp)
    target_link_libraries(rasgo_synth_performance_fracture_adapter_tests
        PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_fracture_adapter_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_fracture_adapter_tests
             COMMAND rasgo_synth_performance_fracture_adapter_tests)


    add_executable(rasgo_synth_performance_sky_tests
        tests/test_performance_sky.cpp)
    target_link_libraries(rasgo_synth_performance_sky_tests
        PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_sky_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_sky_tests
             COMMAND rasgo_synth_performance_sky_tests)

    add_executable(rasgo_synth_performance_sky_director_tests
        tests/test_performance_sky_director.cpp)
    target_link_libraries(rasgo_synth_performance_sky_director_tests
        PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_sky_director_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_sky_director_tests
             COMMAND rasgo_synth_performance_sky_director_tests)

    add_executable(rasgo_synth_performance_module_library_tests
        tests/test_performance_module_library.cpp)
    target_link_libraries(rasgo_synth_performance_module_library_tests
        PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_module_library_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_module_library_tests
             COMMAND rasgo_synth_performance_module_library_tests)

    add_executable(rasgo_synth_performance_tuba_breath_tests tests/test_tuba_breath_voice.cpp)
    target_link_libraries(rasgo_synth_performance_tuba_breath_tests PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_tuba_breath_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_tuba_breath_tests COMMAND rasgo_synth_performance_tuba_breath_tests)

    add_executable(rasgo_synth_performance_bluff_signal_tests tests/test_performance_bluff_signal.cpp)
    target_link_libraries(rasgo_synth_performance_bluff_signal_tests PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_bluff_signal_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_bluff_signal_tests COMMAND rasgo_synth_performance_bluff_signal_tests)

    add_executable(rasgo_synth_performance_harmony_stars_tests tests/test_performance_harmony_stars.cpp)
    target_link_libraries(rasgo_synth_performance_harmony_stars_tests PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_harmony_stars_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_harmony_stars_tests COMMAND rasgo_synth_performance_harmony_stars_tests)

    add_executable(rasgo_synth_performance_sky_pan_tests tests/test_performance_sky_pan.cpp)
    target_link_libraries(rasgo_synth_performance_sky_pan_tests PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_sky_pan_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_sky_pan_tests COMMAND rasgo_synth_performance_sky_pan_tests)

    add_executable(rasgo_synth_performance_concept_stars_tests tests/test_performance_concept_stars.cpp)
    target_link_libraries(rasgo_synth_performance_concept_stars_tests PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_concept_stars_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_concept_stars_tests COMMAND rasgo_synth_performance_concept_stars_tests)

    add_executable(rasgo_synth_performance_memory_tests tests/test_performance_memory.cpp)
    target_link_libraries(rasgo_synth_performance_memory_tests PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_memory_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_memory_tests COMMAND rasgo_synth_performance_memory_tests)

    add_executable(rasgo_synth_performance_album_tests tests/test_performance_album.cpp)
    target_link_libraries(rasgo_synth_performance_album_tests PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_album_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_album_tests COMMAND rasgo_synth_performance_album_tests)

    add_executable(rasgo_synth_performance_album_master_tests tests/test_performance_album_master.cpp)
    target_link_libraries(rasgo_synth_performance_album_master_tests PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_album_master_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_album_master_tests COMMAND rasgo_synth_performance_album_master_tests)

    # GERAR O ÁLBUM (5/11): uma faixa renderizada fora do tempo real, 8 corpos.
    add_executable(rasgo_synth_performance_offline_render_tests tests/test_performance_offline_render.cpp)
    target_link_libraries(rasgo_synth_performance_offline_render_tests PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_offline_render_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_offline_render_tests COMMAND rasgo_synth_performance_offline_render_tests)

    add_executable(rasgo_synth_performance_session_document_tests
        tests/test_performance_session_document.cpp)
    target_link_libraries(rasgo_synth_performance_session_document_tests
        PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_session_document_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_session_document_tests
             COMMAND rasgo_synth_performance_session_document_tests)

    add_executable(rasgo_synth_performance_studio_territory_importer_tests
        tests/test_studio_territory_importer.cpp)
    target_link_libraries(rasgo_synth_performance_studio_territory_importer_tests
        PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_studio_territory_importer_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_studio_territory_importer_tests
             COMMAND rasgo_synth_performance_studio_territory_importer_tests)

    add_executable(rasgo_synth_performance_session_controller_tests
        tests/test_performance_session_controller.cpp)
    target_link_libraries(rasgo_synth_performance_session_controller_tests
        PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_session_controller_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_session_controller_tests
             COMMAND rasgo_synth_performance_session_controller_tests)

    add_executable(rasgo_synth_performance_calibrated_fifth_controller_tests
        tests/test_performance_session_controller.cpp)
    target_link_libraries(rasgo_synth_performance_calibrated_fifth_controller_tests
        PRIVATE rasgo_synth_performance_contracts)
    target_compile_definitions(rasgo_synth_performance_calibrated_fifth_controller_tests
        PRIVATE RASGO_PERFORMANCE_TEST_SAFE_BODY_LIMIT=5)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_calibrated_fifth_controller_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_calibrated_fifth_controller_tests
             COMMAND rasgo_synth_performance_calibrated_fifth_controller_tests)

    add_executable(rasgo_synth_performance_territory_factory_tests
        tests/test_performance_territory_factory.cpp)
    target_link_libraries(rasgo_synth_performance_territory_factory_tests
        PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_territory_factory_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_territory_factory_tests
             COMMAND rasgo_synth_performance_territory_factory_tests)

    add_executable(rasgo_synth_performance_stem_distribution_tests
        tests/test_performance_stem_distribution.cpp)
    target_link_libraries(rasgo_synth_performance_stem_distribution_tests
        PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_stem_distribution_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_stem_distribution_tests
             COMMAND rasgo_synth_performance_stem_distribution_tests)

    add_executable(rasgo_synth_performance_internal_stem_layers_tests
                   tests/test_performance_internal_stem_layers.cpp)
    target_link_libraries(rasgo_synth_performance_internal_stem_layers_tests
        PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_internal_stem_layers_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_internal_stem_layers_tests
             COMMAND rasgo_synth_performance_internal_stem_layers_tests)



    add_executable(rasgo_synth_performance_layout_tests
        tests/test_performance_layout.cpp)
    target_link_libraries(rasgo_synth_performance_layout_tests
        PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_layout_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_layout_tests
             COMMAND rasgo_synth_performance_layout_tests)

    add_executable(rasgo_synth_performance_seed_trajectory_tests
        tests/test_performance_seed_trajectory.cpp)
    target_link_libraries(rasgo_synth_performance_seed_trajectory_tests
        PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_seed_trajectory_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_seed_trajectory_tests
             COMMAND rasgo_synth_performance_seed_trajectory_tests)

    add_executable(rasgo_synth_performance_spatial_game_tests
        tests/test_performance_spatial_game.cpp)
    target_link_libraries(rasgo_synth_performance_spatial_game_tests
        PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_spatial_game_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_spatial_game_tests
             COMMAND rasgo_synth_performance_spatial_game_tests)

    add_executable(rasgo_synth_performance_cumulative_desk_tests
        tests/test_performance_cumulative_desk.cpp)
    target_link_libraries(rasgo_synth_performance_cumulative_desk_tests
        PRIVATE rasgo_synth_performance_contracts)
    if(NOT MSVC)
        target_compile_options(rasgo_synth_performance_cumulative_desk_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_synth_performance_cumulative_desk_tests
             COMMAND rasgo_synth_performance_cumulative_desk_tests)

    add_test(NAME rasgo_synth_performance_runtime_artifacts
             COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/check_runtime_artifacts.sh)
endif()
