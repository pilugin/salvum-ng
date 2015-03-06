HEADERS += \
    $$PWD/sharedimage.h \
    $$PWD/pjdecodr.h

SOURCES += \
    $$PWD/sharedimage.cpp \
    $$PWD/pjdecodr.cpp

INCLUDEPATH *= $$PWD/.. $$PWD/../../picojpeg/

QMAKE_CXXFLAGS *= -std=c++11
