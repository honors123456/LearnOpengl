TEMPLATE = app
TARGET = Mini-Marmoset

QT += quick qml gui opengl
CONFIG += c++17
CONFIG -= console app_bundle

DESTDIR = $$PWD/bin

INCLUDEPATH += \
    $$PWD/shader \
    $$PWD/../External/include


include(common.pri)

INCLUDEPATH += $$OPENCV_INCLUDE_DIR

LIBS += -L$$OPENCV_BIN_DIR
LIBS += -L$$OPENCV_LIB_DIR

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
    shader/lightShader.frag \
    shader/skyShader.frag \
    shader/skyShader.vert

LIBS += \
    -L$$PWD/../External/lib \
    -lopengl32

DEFINES += SHADER_DIR=\\\"$$PWD/shader\\\"

CONFIG(debug, debug|release){
LIBS += -L$$OPENCV_LIB_DIR -lopencv_world440d
LIBS += -L$$OPENCV_LIB_DIR -lopencv_img_hash440d
}else{
LIBS += -L$$OPENCV_LIB_DIR -lopencv_world440
LIBS += -L$$OPENCV_LIB_DIR -lopencv_img_hash440
}
