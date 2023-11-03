TEMPLATE = lib
CONFIG += plugin

android: TARGET = pair_$${QT_ARCH}
else: TARGET = pair

include(../common.pri)

SOURCES += \
	modulepair.cpp

HEADERS += \
	modulepair.h

RESOURCES += \
	qml_pair_qt$${QT_MAJOR_VERSION}.qrc
