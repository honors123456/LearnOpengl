isEmpty(MARMOSET_SPDLOG_DIR): MARMOSET_SPDLOG_DIR = D:/spdlog
!exists($$MARMOSET_SPDLOG_DIR/include/spdlog/spdlog.h): error("spdlog headers not found: $$MARMOSET_SPDLOG_DIR")
INCLUDEPATH += $$quote($$PWD) $$quote($$MARMOSET_SPDLOG_DIR/include)
win32: DEFINES += SPDLOG_WCHAR_FILENAMES
HEADERS += $$quote($$PWD/Logger.h)
SOURCES += $$quote($$PWD/Logger.cpp)
