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
				$$PWD/../jwt-cpp/include

!wasm: INCLUDEPATH += $$PWD/../CuteLogger/include

wasm: INCLUDEPATH += $$PWD/../../client/src/wasm_helper

DEFINES += \
	COS_VERSION_MAJOR=$$VER_MAJ \
	COS_VERSION_MINOR=$$VER_MIN \

HEADERS += \
	credential.h \
	gamemap.h \
	gamemapreaderiface.h \
	utils.h \
	websocketmessage.h

SOURCES += \
	credential.cpp \
	gamemap.cpp \
	gamemapreaderiface.cpp \
	utils.cpp \
	websocketmessage.cpp


!wasm {
	HEADERS += \
		database.h \

	SOURCES += \
		database.cpp \

}
