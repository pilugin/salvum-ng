HEADERS += \
    $$PWD/history.h \
    $$PWD/decodr.h

SOURCES += \
    $$PWD/history.cpp

INCLUDEPATH *= $$PWD/.. $$PWD/../../picojpeg/

QMAKE_CXXFLAGS *= -std=c++11
