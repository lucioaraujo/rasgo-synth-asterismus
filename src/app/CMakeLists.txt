# O ícone embutido no executável (Windows, macOS) entra quando o autor
# aprovar um ícone em packaging/icon/asterismus-1024.png (a proposta fica em
# packaging/icon/proposta/ até lá).
set(ASTERISMUS_ICON_ARGS)
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../packaging/icon/asterismus-1024.png")
    set(ASTERISMUS_ICON_ARGS ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/../../packaging/icon/asterismus-1024.png")
endif()
juce_add_gui_app(RasgoSynthAsterismusApp
    PRODUCT_NAME "Rasgo Synth Asterismus"
    COMPANY_NAME "RASGO"
    BUNDLE_ID "org.rasgo.synthperformance"
    VERSION "${PROJECT_VERSION}"
    ${ASTERISMUS_ICON_ARGS})

target_sources(RasgoSynthAsterismusApp PRIVATE PerformanceApp.cpp)
target_link_libraries(RasgoSynthAsterismusApp PRIVATE
    rasgo_synth_performance_contracts
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_gui_basics)
target_compile_definitions(RasgoSynthAsterismusApp PRIVATE
    JUCE_STRING_UTF_TYPE=8
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_USE_XINPUT=0
    RASGO_ASTERISMUS_VERSION="${PROJECT_VERSION}")

# Textos do painel são UTF-8 (incluindo português e os separadores ·/→).
# Fixar a codificação impede que o compilador a deduza do locale da máquina.
if (MSVC)
    set_source_files_properties(PerformanceApp.cpp PROPERTIES COMPILE_OPTIONS "/utf-8")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set_source_files_properties(PerformanceApp.cpp PROPERTIES
        COMPILE_OPTIONS "-finput-charset=UTF-8;-fexec-charset=UTF-8")
endif()

juce_generate_juce_header(RasgoSynthAsterismusApp)

# Identidade visual autocontida: o binário recebe sua própria cópia dos
# recursos licenciados, sem depender da árvore do Rasgo Synth Studio.
add_custom_command(TARGET RasgoSynthAsterismusApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
        "$<TARGET_FILE_DIR:RasgoSynthAsterismusApp>/assets")

# Instalação e pacotes (30/10, padrão multiplataforma RASGO; o molde é o do
# Trioio, que já publica). O app procura assets/ ao lado do executável e,
# no Linux, em ../share/rasgo-synth-asterismus/assets (PerformanceApp.cpp).
set(ASTERISMUS_ICON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../packaging/icon")
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    install(PROGRAMS "$<TARGET_FILE:RasgoSynthAsterismusApp>" DESTINATION bin RENAME rasgo-synth-asterismus)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../assets/" DESTINATION share/rasgo-synth-asterismus/assets)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../../packaging/linux/rasgo-synth-asterismus.desktop"
        DESTINATION share/applications)
    foreach(size 16 32 48 64 128 256 512)
        if (EXISTS "${ASTERISMUS_ICON_DIR}/asterismus-${size}.png")
            install(FILES "${ASTERISMUS_ICON_DIR}/asterismus-${size}.png"
                DESTINATION share/icons/hicolor/${size}x${size}/apps RENAME rasgo-synth-asterismus.png)
        endif()
    endforeach()
elseif (WIN32)
    install(TARGETS RasgoSynthAsterismusApp RUNTIME DESTINATION .)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../assets" DESTINATION .)
elseif (APPLE)
    install(TARGETS RasgoSynthAsterismusApp BUNDLE DESTINATION .)
endif()

# Licença, avisos de terceiros e créditos viajam com o programa (GPLv3, e o
# JUCE pela via AGPLv3: o código-fonte acompanha a publicação).
set(ASTERISMUS_DOCS
    "${CMAKE_CURRENT_SOURCE_DIR}/../../LICENSE"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../THIRD_PARTY_NOTICES.md"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../CREDITS.md"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../DEPENDENCIES.md")
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    install(FILES ${ASTERISMUS_DOCS} DESTINATION share/doc/rasgo-synth-asterismus)
elseif (WIN32)
    install(FILES ${ASTERISMUS_DOCS} DESTINATION .)
elseif (APPLE)
    install(FILES ${ASTERISMUS_DOCS} DESTINATION .)
endif()

set(CPACK_PACKAGE_NAME "rasgo-synth-asterismus")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "RASGO")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Rasgo Synth Asterismus: a generative sky of stars that relate, dance and play")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../../LICENSE")
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
    set(CPACK_GENERATOR "DEB;TGZ")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Lúcio Araújo <rasgo.instruments@gmail.com>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "sound")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    # O que o JUCE abre por dlopen não aparece no shlibdeps (lição do
    # Antitotem e do Trioio).
    set(CPACK_DEBIAN_PACKAGE_DEPENDS
        "libx11-6, libxext6, libxss1, libasound2t64 | libasound2, libfreetype6, libglib2.0-0t64 | libglib2.0-0")
elseif (WIN32)
    # NSIS: atalho no Menu Iniciar só com CPACK_PACKAGE_EXECUTABLES; .exe na
    # raiz; pasta e registro sem a versão (lições do Antitotem, Navalha 2,
    # Rasgo Modular e Trioio).
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_PACKAGE_NAME "Rasgo Synth Asterismus")
    set(CPACK_NSIS_DISPLAY_NAME "Rasgo Synth Asterismus")
    set(CPACK_NSIS_CONTACT "rasgo.instruments@gmail.com")
    set(CPACK_NSIS_MODIFY_PATH OFF)
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "Rasgo Synth Asterismus")
    set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "RasgoSynthAsterismus")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_PACKAGE_EXECUTABLES "Rasgo Synth Asterismus" "Rasgo Synth Asterismus")
    set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
    set(CPACK_CREATE_DESKTOP_LINKS "Rasgo Synth Asterismus")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "Rasgo Synth Asterismus.exe")
    set(CPACK_NSIS_MUI_FINISHPAGE_RUN "Rasgo Synth Asterismus.exe")
    if (EXISTS "${ASTERISMUS_ICON_DIR}/asterismus.ico")
        set(CPACK_NSIS_MUI_ICON "${ASTERISMUS_ICON_DIR}/asterismus.ico")
        set(CPACK_NSIS_MUI_UNIICON "${ASTERISMUS_ICON_DIR}/asterismus.ico")
    endif()
elseif (APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "Rasgo Synth Asterismus")
endif()
include(CPack)
