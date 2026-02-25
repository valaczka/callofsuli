TEMPLATE = lib
CONFIG += plugin

android: TARGET = mergebinding_$${QT_ARCH}
else: TARGET = mergebinding

include(../common.pri)

SOURCES += \
	modulemergebinding.cpp

HEADERS += \
	modulemergebinding.h

RESOURCES += \
	qml_mergebinding.qrc
