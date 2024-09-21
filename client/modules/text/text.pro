TEMPLATE = lib
CONFIG += plugin

android: TARGET = text_$${QT_ARCH}
else: TARGET = text

include(../common.pri)

SOURCES += \
	moduletext.cpp

HEADERS += \
	moduletext.h

RESOURCES += \
	qml_text.qrc
