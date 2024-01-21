include(../common.pri)

TargetSuffix =
win32: TargetSuffix = /release

# Core

LIBS += -L../../lib$${TargetSuffix}

LIBS += -lcrypto

# CuteLogger

INCLUDEPATH += $$PWD/CuteLogger/include

LIBS += -lCuteLogger

# QConsole

include($$PWD/QConsole/qconsole.pri)
