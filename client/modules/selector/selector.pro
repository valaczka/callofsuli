TEMPLATE = lib
CONFIG += plugin

android: TARGET = selector_$${QT_ARCH}
else: TARGET = selector

include(../common.pri)

SOURCES += \
		moduleselector.cpp

HEADERS += \
	moduleselector.h

RESOURCES += \
	qml_selector.qrc
