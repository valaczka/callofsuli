TEMPLATE = lib
CONFIG += plugin

android: TARGET = plusminus_$${QT_ARCH}
else: TARGET = plusminus

include(../common.pri)

RESOURCES += \
	qml_plusminus_qt$${QT_MAJOR_VERSION}.qrc

HEADERS += \
	moduleplusminus.h

SOURCES += \
	moduleplusminus.cpp
