TEMPLATE = lib
CONFIG += plugin

QT += qml quick

INCLUDEPATH += $$PWD

android: TARGET = writer_$${QT_ARCH}
else: TARGET = writer

include(../common.pri)

SOURCES += \
	modulewriter.cpp \
	writerengine.cpp

HEADERS += \
	modulewriter.h \
	writerengine.h

RESOURCES += \
	qml_writer.qrc
