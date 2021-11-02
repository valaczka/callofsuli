TEMPLATE = lib
CONFIG += plugin

TARGET = plusminus_$${QT_ARCH}

include(../common.pri)

RESOURCES += \
	qml_plusminus.qrc

HEADERS += \
	moduleplusminus.h

SOURCES += \
	moduleplusminus.cpp
