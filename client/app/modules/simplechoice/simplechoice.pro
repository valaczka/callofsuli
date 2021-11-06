TEMPLATE = lib
CONFIG += plugin

android: TARGET = simplechoice_$${QT_ARCH}
else: TARGET = simplechoice

include(../common.pri)

SOURCES += \
	modulesimplechoice.cpp \
	objectiveimportersimplechoice.cpp

HEADERS += \
	modulesimplechoice.h \
	objectiveimportersimplechoice.h

RESOURCES += \
	qml_simplechoice.qrc
