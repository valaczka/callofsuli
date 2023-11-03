TEMPLATE = lib
CONFIG += plugin

android: TARGET = sequence_$${QT_ARCH}
else: TARGET = sequence

include(../common.pri)

SOURCES += \
	modulesequence.cpp

HEADERS += \
	modulesequence.h

RESOURCES += \
	qml_sequence_qt$${QT_MAJOR_VERSION}.qrc
