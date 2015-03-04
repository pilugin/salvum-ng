QT += dbus

HEADERS += $$PWD/declareDBusMetatypes.h 
SOURCES += $$PWD/declareDBusMetatypes.cpp 

# use DBUS_ADAPTOR_FILES =

INCLUDEPATH *= \
    $$OUT_PWD/.. \
    $$PWD/.. 

DEPENDPATH *= $$PWD/../common

genadaptors.input = DBUS_ADAPTOR_FILES
genadaptors.commands = qdbusxml2cpp ${QMAKE_FILE_NAME} -a ${QMAKE_FILE_BASE}Adp -i "dbus/declareDBusMetatypes.h"
genadaptors.output = ${QMAKE_FILE_BASE}Adp.h
genadaptors.variable_out = HEADERS

genadaptors_cpp.input = DBUS_ADAPTOR_FILES
genadaptors_cpp.commands = echo "dbusxml2cpp-adp-dummy:" ${QMAKE_FILE_BASE}Adp.cpp
genadaptors_cpp.output = ${QMAKE_FILE_BASE}Adp.cpp
genadaptors_cpp.variable_out = SOURCES
genadaptors_cpp.depends = ${QMAKE_FILE_BASE}Adp.h

QMAKE_CXXFLAGS += -std=c++11
QMAKE_EXTRA_COMPILERS += genadaptors genadaptors_cpp

OTHER_FILES += $$DBUS_ADAPTOR_FILES
