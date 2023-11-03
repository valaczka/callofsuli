TEMPLATE = lib
CONFIG += plugin

QT += qml quick

INCLUDEPATH += $$PWD

android: TARGET = fillout_$${QT_ARCH}
else: TARGET = fillout

include(../common.pri)

SOURCES += \
	fillouthighlighter.cpp \
	modulefillout.cpp

HEADERS += \
	fillouthighlighter.h \
	modulefillout.h

RESOURCES += \
	qml_fillout_qt$${QT_MAJOR_VERSION}.qrc
