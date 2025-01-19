TEMPLATE = lib
CONFIG += plugin

android: TARGET = binary_$${QT_ARCH}
else: TARGET = binary

include(../common.pri)

SOURCES += \
		modulebinary.cpp

HEADERS += \
	modulebinary.h

RESOURCES += \
	qml_binary.qrc
