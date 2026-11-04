# Single qmake application; all four labs are selectable in the workbench.
TEMPLATE = app
TARGET = Mini-Marmoset

SOURCES += $$quote($$PWD/main.cpp)
INCLUDEPATH += $$quote($$PWD)

QT += quick qml gui quickcontrols2
CONFIG += c++17 warn_on
CONFIG -= console app_bundle

DISTFILES += \
    $$quote($$PWD/ui/qmldir) \
    $$files($$PWD/ui/*.qml) \
    AGENTS.md README.md docs/ROADMAP.md docs/PRODUCT.md docs/DESIGN.md

QML_IMPORT_PATH += $$quote($$PWD/ui)

CONFIG(debug, debug|release) {
    LAB_BUILD_VARIANT = debug
} else {
    LAB_BUILD_VARIANT = release
}

win32-msvc: QMAKE_CXXFLAGS += /execution-charset:utf-8

DESTDIR = $$PWD/bin
LAB_BUILD_DIR = $$PWD/build/qmake/$$LAB_BUILD_VARIANT
OBJECTS_DIR = $$LAB_BUILD_DIR/obj
MOC_DIR = $$LAB_BUILD_DIR/moc
RCC_DIR = $$LAB_BUILD_DIR/rcc
UI_DIR = $$LAB_BUILD_DIR/ui
win32-msvc {
    QMAKE_LFLAGS += /PDB:$$quote($$LAB_BUILD_DIR/Mini-Marmoset.pdb)
    QMAKE_LFLAGS += /ILK:$$quote($$LAB_BUILD_DIR/Mini-Marmoset.ilk)
}

RESOURCES += $$quote($$PWD/ui/workbench.qrc)
include($$PWD/Logging/Logging.pri)
include($$PWD/Geometry/Geometry.pri)

include($$PWD/Pbr_lab/Pbr_lab.pri)
include($$PWD/Deferred_lab/Deferred_lab.pri)
include($$PWD/Ssao_lab/Ssao_lab.pri)
include($$PWD/Gpu_lab/Gpu_lab.pri)
