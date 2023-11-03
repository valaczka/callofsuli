TEMPLATE = lib
CONFIG += plugin

android: TARGET = multichoice_$${QT_ARCH}
else: TARGET = multichoice

include(../common.pri)

SOURCES += \
	modulemultichoice.cpp

HEADERS += \
	modulemultichoice.h

RESOURCES += \
	qml_multichoice_qt$${QT_MAJOR_VERSION}.qrc
