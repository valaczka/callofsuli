TEMPLATE = lib
CONFIG += plugin

android: TARGET = simplechoice_$${QT_ARCH}
else: TARGET = simplechoice

include(../common.pri)

SOURCES += \
	modulesimplechoice.cpp

HEADERS += \
	modulesimplechoice.h

RESOURCES += \
	qml_simplechoice_qt$${QT_MAJOR_VERSION}.qrc
