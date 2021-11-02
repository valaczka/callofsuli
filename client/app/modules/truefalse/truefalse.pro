TEMPLATE = lib
CONFIG += plugin

TARGET = truefalse_$${QT_ARCH}

include(../common.pri)

SOURCES += \
		objectiveimportertruefalse.cpp \
		moduletruefalse.cpp

HEADERS += \
	objectiveimportertruefalse.h \
	moduletruefalse.h

RESOURCES += \
	qml_truefalse.qrc
