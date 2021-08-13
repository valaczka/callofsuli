TEMPLATE = lib
CONFIG += plugin

include(../common.pri)

SOURCES += \
	modulesimplechoice.cpp \
	objectiveimportersimplechoice.cpp

HEADERS += \
	modulesimplechoice.h \
	objectiveimportersimplechoice.h

RESOURCES += \
	qml_simplechoice.qrc
