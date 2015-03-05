include(jpeg.pri)

TEMPLATE = lib
CONFIG += staticlib
TARGET = salv-jpeg

DESTDIR = $$OUT_PWD/..

#SOURCES += $$PWD/compiletest.cpp

include(../common/colorgcc.pri)
