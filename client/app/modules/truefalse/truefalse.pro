TEMPLATE = lib
CONFIG += plugin

android: TARGET = truefalse_$${QT_ARCH}
else: TARGET = truefalse

include(../common.pri)

SOURCES += \
		objectiveimportertruefalse.cpp \
		moduletruefalse.cpp

HEADERS += \
	objectiveimportertruefalse.h \
	moduletruefalse.h

RESOURCES += \
	qml_truefalse.qrc
