cmake_minimum_required(VERSION 3.22)

project(TherapistDocumentationTool_Production VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Test)
find_package(Qt6 6.5 OPTIONAL_COMPONENTS Quick3D)

qt_standard_project_setup()

set(CORE_SOURCES
    src/core/ClinicalTypes.h
    src/core/HybridSearchIndex.h
    src/core/HybridSearchIndex.cpp
    src/core/ClinicalEngine.h
    src/core/ClinicalEngine.cpp
    src/core/SessionCadenceTracker.h
    src/core/SessionCadenceTracker.cpp
    src/core/UpdateChecker.h
    src/core/UpdateChecker.cpp
)

set(APP_RESOURCES "")
if(WIN32)
    enable_language(RC)
    set(APP_RESOURCES src/app.rc)
endif()

qt_add_executable(appTherapistDocumentationTool_Production
    WIN32
    src/main.cpp
    ${CORE_SOURCES}
    ${APP_RESOURCES}
)

set_source_files_properties(../data/clinical_banks.json PROPERTIES
    QT_RESOURCE_ALIAS clinical_banks.json
)

qt_add_resources(appTherapistDocumentationTool_Production "clinical_data"
    PREFIX "/data"
    FILES
        ../data/clinical_banks.json
)

qt_add_qml_module(appTherapistDocumentationTool_Production
    URI ProductionApp
    VERSION 1.0
    QML_FILES
        src/ui/MainScene.qml
    RESOURCES
        src/app_icon.png
)

target_link_libraries(appTherapistDocumentationTool_Production PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

if(Qt6Quick3D_FOUND)
    target_link_libraries(appTherapistDocumentationTool_Production PRIVATE Qt6::Quick3D)
endif()

if(MINGW)
    target_link_options(appTherapistDocumentationTool_Production PRIVATE
        -Wl,--dynamicbase
        -Wl,--nxcompat
        -Wl,--high-entropy-va
    )
endif()

# Automated Verification & Telemetry Suite (RT-001 / RT-008)
enable_testing()

qt_add_executable(tst_production_suite
    tests/tst_production_suite.cpp
    ${CORE_SOURCES}
)

qt_add_resources(tst_production_suite "test_clinical_data"
    PREFIX "/data"
    FILES
        ../data/clinical_banks.json
)

target_compile_definitions(tst_production_suite PRIVATE
    PRODUCTION_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(tst_production_suite PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Test
)

add_test(NAME ProductionSuiteTest COMMAND tst_production_suite)
