TEMPLATE = lib

include(../../version/version.pri)

android: TARGET = callofsuli-core_$${QT_ARCH}
else: TARGET = callofsuli-core

android: CONFIG += staticlib

DESTDIR = ..

INCLUDEPATH += $$PWD

DEFINES += \
	COS_VERSION_MAJOR=$$VER_MAJ \
	COS_VERSION_MINOR=$$VER_MIN \

HEADERS += \
	QJsonWebToken/qjsonwebtoken.h \
	gamemap.h \
	gamemapreaderiface.h \
	websocketmessage.h

SOURCES += \
	QJsonWebToken/qjsonwebtoken.cpp \
	gamemap.cpp \
	gamemapreaderiface.cpp \
	websocketmessage.cpp

