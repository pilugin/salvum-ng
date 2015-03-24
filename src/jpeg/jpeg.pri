HEADERS += \
    $$PWD/sharedimage.h \
    $$PWD/pjdecodr.h \
    $$PWD/pjhistory.h

SOURCES += \
    $$PWD/sharedimage.cpp \
    $$PWD/pjdecodr.cpp \
    $$PWD/pjhistory.cpp

INCLUDEPATH *= $$PWD/.. $$PWD/../../picojpeg/

QMAKE_CXXFLAGS *= -std=c++11
