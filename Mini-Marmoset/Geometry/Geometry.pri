isEmpty(MARMOSET_GLM_INCLUDE_DIR) {
    MARMOSET_GLM_INCLUDE_DIR = $$clean_path($$PWD/../../External/include)
}
INCLUDEPATH += $$quote($$PWD/..) $$quote($$MARMOSET_GLM_INCLUDE_DIR)
HEADERS += $$quote($$PWD/MeshGenerator.h) $$quote($$PWD/ObjLoader.h)
