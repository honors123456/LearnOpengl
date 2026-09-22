# ---------------------------------------------------------------------------
# Mini-Marmoset.pro
#
# qmake project file so this sample can be opened and built in Qt Creator,
# in addition to the existing CMake setup (Mini-Marmoset/CMakeLists.txt).
#
# It mirrors CMakeLists.txt on purpose:
#   * C++17, MSVC /W4 /utf-8 (source files contain UTF-8 Chinese comments)
#   * sources  : src/main.cpp, shader/shader.cpp, camera/Camera.cpp,
#                ../External/src/glad.c
#   * includes : shader, camera, ../External/include
#   * libs     : glfw3 (static, ../External/lib), opengl32, gdi32
#   * output   : Mini-Marmoset/bin (same folder as the CMake build)
#
# The sample does not use Qt, so the Qt modules are switched off. Any desktop
# kit works, e.g. "Desktop Qt 5.12.12 MSVC2017 64bit".
# ---------------------------------------------------------------------------

TEMPLATE = app
TARGET   = Mini-Marmoset

QT -= core gui
CONFIG += console c++17
CONFIG -= app_bundle

# Same output folder as the CMake build, so both builds produce one exe.
DESTDIR = $$PWD/bin

INCLUDEPATH += \
    $$PWD/shader \
    $$PWD/camera \
    $$PWD/../External/include

SOURCES += \
    src/main.cpp \
    shader/shader.cpp \
    camera/Camera.cpp \
    $$PWD/../External/src/glad.c

HEADERS += \
    shader/shader.h \
    camera/Camera.h

# Keep the GLSL files visible in the Qt Creator project tree.
DISTFILES += \
    shader/lightShader.vert \
    shader/lightShader.frag \
    shader/pbrShader.frag \
    shader/pbrShader.vert

# GLFW is a static library, so the Win32 libs it uses internally must be
# linked too: user32/gdi32 for window handling, shell32 for drag & drop.
LIBS += \
    -L$$PWD/../External/lib \
    -lglfw3 \
    -lopengl32 \
    -luser32 \
    -lgdi32 \
    -lshell32

# /utf-8 is required because the sources are UTF-8 encoded and MSVC would
# otherwise read them with the local code page. WARN_ON is overwritten (not
# appended) so the CMake warning level actually wins over qmake's default /W3.
msvc {
    QMAKE_CFLAGS          += /utf-8
    QMAKE_CXXFLAGS        += /utf-8
    QMAKE_CFLAGS_WARN_ON   = /W4
    QMAKE_CXXFLAGS_WARN_ON = /W4
}

# Kept in sync with the SHADER_DIR define of the CMake build. The quotes are
# needed so the macro expands to a string literal on the compiler command line.
DEFINES += SHADER_DIR=\\\"$$PWD/shader\\\"
