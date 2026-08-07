# 指定当前工程需要的最低 CMake 版本。
cmake_minimum_required(VERSION 3.20)

# 定义工程名称，并声明当前工程会使用 C 和 C++。
project(Stencil-Testing LANGUAGES C CXX)

# 指定 C++ 标准为 C++17，并要求编译器必须支持该标准。
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 计算常用目录：
# PROJECT_ROOT_DIR 是当前 CMakeLists.txt 的上一级目录。
# EXTERNAL_DIR 是第三方库目录。
# OUTPUT_DIR 是最终程序和库文件的输出目录。
set(PROJECT_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
set(EXTERNAL_DIR "${PROJECT_ROOT_DIR}/External")
set(OUTPUT_DIR "${CMAKE_CURRENT_LIST_DIR}/bin")
set(OPENCV_DIR "E:/opencv/build")

# 转成绝对路径，避免相对路径写法不同导致比较失败。

# 检查当前构建目录是否是 CMakeLists.txt 同级的 build 目录。
# CMakeLists.txt 本身不能主动决定 -B 目录，所以这里用校验来约束。
# 设置单配置生成器的输出目录，例如 Ninja 或 Makefiles。
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")

# 设置多配置生成器的输出目录，例如 Visual Studio。
# 这样 Debug、Release 等配置都会输出到同级 bin 目录。
foreach(CONFIG_TYPE IN LISTS CMAKE_CONFIGURATION_TYPES)
    string(TOUPPER "${CONFIG_TYPE}" CONFIG_TYPE_UPPER)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${OUTPUT_DIR}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${OUTPUT_DIR}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${OUTPUT_DIR}")
endforeach()

# 收集当前示例程序的源文件。
set(SOURCES
    src/main.cpp
    shader/shader.cpp
    "${EXTERNAL_DIR}/src/glad.c"
    camera/camera.cpp
)

# 生成可执行程序，目标名使用 project() 里定义的 Triangle。
add_executable(${PROJECT_NAME} ${SOURCES})

# 添加头文件搜索路径，让代码可以 include glad、GLFW 等第三方头文件。
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/shader"
    "${CMAKE_CURRENT_LIST_DIR}/camera"
    "${EXTERNAL_DIR}/include"
    "${OPENCV_DIR}/include"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    SHADER_DIR="${CMAKE_CURRENT_LIST_DIR}/shader"
    TEXTURE_FILE1="${PROJECT_ROOT_DIR}/container2.png"
    TEXTURE_FILE2="${PROJECT_ROOT_DIR}/container2_specular.png"
    TEXTURE_FILE3="${PROJECT_ROOT_DIR}/matrix.jpg"
)

# 添加库文件搜索路径，让链接器能找到 External/lib 里的 glfw3.lib。
target_link_directories(${PROJECT_NAME} PRIVATE
    "${EXTERNAL_DIR}/lib"
    "${OPENCV_DIR}/x64/vc16/lib"
)

# 链接 GLFW 静态库和 Windows 系统 OpenGL 库。
target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw3
    opengl32
    $<$<CONFIG:Debug>:opencv_img_hash440d>
    $<$<CONFIG:Release>:opencv_img_hash440>
    $<$<CONFIG:Debug>:opencv_world440d>
    $<$<CONFIG:Release>:opencv_world440>
)

# 根据编译器类型开启常用警告选项。
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /utf-8)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
