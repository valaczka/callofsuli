TEMPLATE = lib
CONFIG += plugin

android: TARGET = order_$${QT_ARCH}
else: TARGET = order

include(../common.pri)

SOURCES += \
	moduleorder.cpp

HEADERS += \
	moduleorder.h

RESOURCES += \
	qml_order.qrc
