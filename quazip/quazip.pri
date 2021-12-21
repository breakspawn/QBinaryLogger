INCLUDEPATH+=$$PWD
DEPENDPATH+=$$PWD/quazip

LIBS += -lz
include($$PWD/quazip/quazip.pri)
