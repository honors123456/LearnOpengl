INCLUDEPATH += $$quote($$PWD)
HEADERS += $$quote($$PWD/PbrLab.h)
SOURCES += $$quote($$PWD/PbrLab.cpp)

DISTFILES += \
    $$PWD/lightShader.frag \
    $$PWD/lightShader.vert \
    $$PWD/pbrShader.frag \
    $$PWD/pbrShader.vert
