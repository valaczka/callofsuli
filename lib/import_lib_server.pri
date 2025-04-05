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


# ENet

!wasm {
	INCLUDEPATH += $$PWD/enet/include
	android: LIBS += -lenet_$${QT_ARCH}
	else: LIBS += -lenet
}


# Sodium

INCLUDEPATH += $$LibSodiumInclude
LIBS += $$LibSodiumLibs


!isEmpty(LibSodiumDefines) {
	DEFINES += $$LibSodiumDefines
}


# Backward

if ($$BackwardCpp) {
	INCLUDEPATH += $$PWD/backward-cpp
	LIBS += -lbfd
	SOURCES += $$PWD/../client/src/backward.cpp
}
