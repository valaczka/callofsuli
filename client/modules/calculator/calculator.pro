TEMPLATE = lib
CONFIG += plugin

android: TARGET = calculator_$${QT_ARCH}
else: TARGET = calculator

include(../common.pri)

RESOURCES += \
	qml_calculator_qt$${QT_MAJOR_VERSION}.qrc

HEADERS += \
	modulecalculator.h

SOURCES += \
	modulecalculator.cpp
