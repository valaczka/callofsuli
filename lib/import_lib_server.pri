include(../common.pri)

TargetSuffix =
win32: TargetSuffix = /release

# Core

INCLUDEPATH += $$PWD/callofsuli-core

LIBS += -L../../lib$${TargetSuffix}

LIBS += -lcallofsuli-core

# Core includes

INCLUDEPATH += $$PWD/QDeferred/src
INCLUDEPATH += $$PWD/QJsonWebToken/src


# Qt5Service

LIBS += -L../../lib/QtService/lib -lQt5Service
INCLUDEPATH += $$OUT_PWD/../../lib/QtService/include
DEFINES += QT_SERVICE_LIB


# CuteLogger

INCLUDEPATH += $$PWD/CuteLogger/include

LIBS += -lCuteLogger

# QConsole

include($$PWD/QConsole/qconsole.pri)


