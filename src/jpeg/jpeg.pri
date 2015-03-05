HEADERS += \
    $$PWD/sharedimage.h

SOURCES += \
    $$PWD/sharedimage.cpp

INCLUDEPATH *= $$PWD/.. $$PWD/../../picojpeg/

QMAKE_CXXFLAGS *= -std=c++11
