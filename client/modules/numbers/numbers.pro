TEMPLATE = lib
CONFIG += plugin

android: TARGET = numbers_$${QT_ARCH}
else: TARGET = numbers

include(../common.pri)

SOURCES += \
	modulenumbers.cpp

HEADERS += \
	modulenumbers.h

RESOURCES += \
	qml_numbers_qt$${QT_MAJOR_VERSION}.qrc
