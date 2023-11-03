TEMPLATE = lib
CONFIG += plugin

android: TARGET = binding_$${QT_ARCH}
else: TARGET = binding

include(../common.pri)

SOURCES += \
	modulebinding.cpp

HEADERS += \
	modulebinding.h

RESOURCES += \
	qml_binding_qt$${QT_MAJOR_VERSION}.qrc
