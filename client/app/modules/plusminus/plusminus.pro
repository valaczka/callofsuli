TEMPLATE = lib
CONFIG += plugin

android: TARGET = plusminus_$${QT_ARCH}
else: TARGET = plusminus

include(../common.pri)

RESOURCES += \
	qml_plusminus.qrc

HEADERS += \
	moduleplusminus.h

SOURCES += \
	moduleplusminus.cpp
