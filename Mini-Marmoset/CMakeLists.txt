cmake_minimum_required(VERSION 3.20)
project(Mini-Marmoset LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/$<0:>")
set(CMAKE_PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/pdb")
set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/pdb")

find_package(Qt5 5.12 REQUIRED COMPONENTS Quick Qml Gui QuickControls2)

add_subdirectory(Geometry)
add_executable(Mini-Marmoset
    main.cpp
    ui/workbench.qrc
)
target_include_directories(Mini-Marmoset PRIVATE "${PROJECT_SOURCE_DIR}")
target_link_libraries(Mini-Marmoset PRIVATE
    Qt5::Quick Qt5::Qml Qt5::Gui Qt5::QuickControls2 MarmosetGeometry)
if(MSVC)
    target_compile_options(Mini-Marmoset PRIVATE /W4 /execution-charset:utf-8)
    set_target_properties(Mini-Marmoset PROPERTIES COMPILE_PDB_NAME "Mini-Marmoset")
else()
    target_compile_options(Mini-Marmoset PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_subdirectory(Logging)
add_subdirectory(Pbr_lab)
add_subdirectory(Deferred_lab)
add_subdirectory(Ssao_lab)
add_subdirectory(Gpu_lab)

option(BUILD_GEOMETRY_TESTS "Build CPU-only geometry and OBJ regression tests" OFF)
if(BUILD_GEOMETRY_TESTS)
    enable_testing()
    add_subdirectory(tests/Geometry)
endif()
