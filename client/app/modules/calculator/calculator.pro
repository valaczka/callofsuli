TEMPLATE = lib
CONFIG += plugin

TARGET = calculator_$${QT_ARCH}

include(../common.pri)

RESOURCES += \
	qml_calculator.qrc

HEADERS += \
	modulecalculator.h

SOURCES += \
	modulecalculator.cpp
