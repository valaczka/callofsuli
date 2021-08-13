TEMPLATE = lib
CONFIG += plugin

include(../common.pri)

SOURCES += \
		objectiveimportertruefalse.cpp \
		moduletruefalse.cpp

HEADERS += \
	objectiveimportertruefalse.h \
	moduletruefalse.h

RESOURCES += \
	qml_truefalse.qrc
