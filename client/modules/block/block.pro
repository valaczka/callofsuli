TEMPLATE = lib
CONFIG += plugin

android: TARGET = block_$${QT_ARCH}
else: TARGET = block

include(../common.pri)

SOURCES += \
	moduleblock.cpp

HEADERS += \
	moduleblock.h

RESOURCES += \
	qml_block_qt$${QT_MAJOR_VERSION}.qrc
