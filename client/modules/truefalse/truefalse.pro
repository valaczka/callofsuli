TEMPLATE = lib
CONFIG += plugin

android: TARGET = truefalse_$${QT_ARCH}
else: TARGET = truefalse

include(../common.pri)

SOURCES += \
		moduletruefalse.cpp

HEADERS += \
	moduletruefalse.h

RESOURCES += \
	qml_truefalse.qrc
