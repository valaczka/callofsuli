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

# QSerializer

DEFINES += QS_HAS_JSON
include($$PWD/QSerializer/qserializer.pri)

# SmtpClient

INCLUDEPATH += $$PWD/simple-mail/src
LIBS += -lsimplemail
