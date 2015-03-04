HEADERS += \
    $$PWD/array.h \
    $$PWD/ipc.h \
    $$PWD/private/ipc_private.h \
    $$PWD/private/container.h \
    $$PWD/singleton.h \
    $$PWD/slotclosure.h

SOURCES += \
    $$PWD/private/slotclosure.cpp \
    $$PWD/private/ipc_private.cpp \
    $$PWD/private/synchromem.cpp

QMAKE_CXXFLAGS *= -std=c++11

INCLUDEPATH *= $$PWD/..
DEPENDPATH *= $$PWD/../core