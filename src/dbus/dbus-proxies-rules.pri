QT += dbus

HEADERS += $$PWD/declareDBusMetatypes.h
SOURCES += $$PWD/declareDBusMetatypes.cpp

#use DBUS_PROXY_FILES =

INCLUDEPATH *= \
    $$OUT_PWD/.. \
    $$PWD/..

DEPENDPATH *= $$PWD/../common    

genproxies.input = DBUS_PROXY_FILES
genproxies.commands = qdbusxml2cpp ${QMAKE_FILE_NAME} -p ${QMAKE_FILE_BASE} -i "dbus/declareDBusMetatypes.h"
genproxies.output = ${QMAKE_FILE_BASE}.h
genproxies.variable_out = HEADERS

genproxies_cpp.input = DBUS_PROXY_FILES
genproxies_cpp.commands = echo "dbusxml2cpp-dummy:" ${QMAKE_FILE_BASE}.cpp
genproxies_cpp.output = ${QMAKE_FILE_BASE}.cpp
genproxies_cpp.variable_out = SOURCES
genproxies_cpp.depends = ${QMAKE_FILE_BASE}.h

QMAKE_CXXFLAGS += -std=c++11
QMAKE_EXTRA_COMPILERS += genproxies genproxies_cpp

OTHER_FILES += $$DBUS_PROXY_FILES
