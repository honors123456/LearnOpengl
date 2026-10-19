cmake_minimum_required(VERSION 3.24)
project(tinyrender_test LANGUAGES C CXX)

# Keep runtime deliverables together; generator expressions prevent Debug/Release subfolders.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/$<0:>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/$<0:>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib/$<CONFIG>")
set(CMAKE_PDB_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/pdb/$<CONFIG>")
set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/pdb/$<CONFIG>")

include(FetchContent)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw
    URL https://codeload.github.com/glfw/glfw/tar.gz/refs/tags/3.4
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_Declare(imgui
    URL https://codeload.github.com/ocornut/imgui/tar.gz/refs/tags/v1.91.9b
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(glfw imgui)
find_package(OpenGL REQUIRED)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

add_executable(tinyrender_test
    src/main.cpp 
    src/ImageView.cpp 
    src/ImageView.h
    src/SoftwareRenderer.cpp
    src/SoftwareRenderer.h
    src/Shader.h
    src/ParseAssimpObj.h
    src/tgaimage.cpp 
    src/tgaimage.h
)
target_compile_features(tinyrender_test PRIVATE cxx_std_17)
target_include_directories(tinyrender_test SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/Eigen/include")
target_link_libraries(tinyrender_test PRIVATE imgui)
if(WIN32)
    # Copy runtime DLLs supplied by linked imported/shared CMake targets.
    file(GENERATE
        OUTPUT "${PROJECT_BINARY_DIR}/copy_runtime_dlls_$<CONFIG>.cmake"
        CONTENT "set(runtime_dlls \"$<TARGET_RUNTIME_DLLS:tinyrender_test>\")
foreach(dll IN LISTS runtime_dlls)
    file(COPY \"\${dll}\" DESTINATION \"$<TARGET_FILE_DIR:tinyrender_test>\")
endforeach()
"
    )
    add_custom_command(TARGET tinyrender_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -P
            "${PROJECT_BINARY_DIR}/copy_runtime_dlls_$<CONFIG>.cmake"
        VERBATIM
    )
endif()
if(MSVC)
    target_compile_options(tinyrender_test PRIVATE /W4 /utf-8)
else()
    target_compile_options(tinyrender_test PRIVATE -Wall -Wextra -Wpedantic)
endif()
