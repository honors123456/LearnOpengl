TEMPLATE = app
TARGET = Mini-Marmoset

QT += quick qml gui opengl
CONFIG += c++17
CONFIG -= console app_bundle

DESTDIR = $$PWD/bin

INCLUDEPATH += \
    $$PWD/shader \
    $$PWD/../External/include

SOURCES += \
    src/main.cpp \
    src/MarmosetItem.cpp \
    shader/shader.cpp \
    $$PWD/../External/src/glad.c

HEADERS += \
    src/MarmosetItem.h \
    shader/shader.h

RESOURCES += $$PWD/qml.qrc

DISTFILES += \
    shader/pbrShader.vert \
    shader/pbrShader.frag \
    shader/lightShader.vert \
    shader/lightShader.frag

LIBS += \
    -L$$PWD/../External/lib \
    -lopengl32

DEFINES += SHADER_DIR=\\\"$$PWD/shader\\\"
