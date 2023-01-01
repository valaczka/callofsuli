TEMPLATE = lib

include($$PWD/../../version/version.pri)

android: TARGET = callofsuli-core_$${QT_ARCH}
else: TARGET = callofsuli-core

!wasm: QT += sql

CONFIG += staticlib

DESTDIR = ..

# QDeferred

include($$PWD/../QDeferred/src/qdeferred.pri)
include($$PWD/../QDeferred/src/qlambdathreadworker.pri)


INCLUDEPATH += $$PWD \
				$$PWD/../QJsonWebToken/src

DEFINES += \
	COS_VERSION_MAJOR=$$VER_MAJ \
	COS_VERSION_MINOR=$$VER_MIN \

HEADERS += \
	../QJsonWebToken/src/qjsonwebtoken.h \
	gamemap.h \
	gamemapreaderiface.h \
	websocketmessage.h

SOURCES += \
	../QJsonWebToken/src/qjsonwebtoken.cpp \
	gamemap.cpp \
	gamemapreaderiface.cpp \
	websocketmessage.cpp


!wasm {
	HEADERS += \
		database.h \

	SOURCES += \
		database.cpp \

}
