include(core.pri)

TEMPLATE = lib
CONFIG += staticlib
TARGET = salv-core

DESTDIR = $$OUT_PWD/..

SOURCES += $$PWD/compiletest.cpp

include(../common/colorgcc.pri)
