TEMPLATE = lib
CONFIG += plugin

android: TARGET = doublechoice_$${QT_ARCH}
else: TARGET = doublechoice

include(../common.pri)

SOURCES += \
	moduledoublechoice.cpp

HEADERS += \
	moduledoublechoice.h

RESOURCES += \
	qml_doublechoice.qrc
