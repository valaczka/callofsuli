TEMPLATE = lib
CONFIG += plugin

TARGET = simplechoice_$${QT_ARCH}

include(../common.pri)

SOURCES += \
	modulesimplechoice.cpp \
	objectiveimportersimplechoice.cpp

HEADERS += \
	modulesimplechoice.h \
	objectiveimportersimplechoice.h

RESOURCES += \
	qml_simplechoice.qrc
