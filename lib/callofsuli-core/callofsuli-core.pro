TEMPLATE = lib

include($$PWD/../../version/version.pri)

android: TARGET = callofsuli-core_$${QT_ARCH}
else: TARGET = callofsuli-core

QT += networkauth core-private

!wasm: QT += sql

CONFIG += staticlib

DESTDIR = ..

# QDeferred

include($$PWD/../QDeferred/src/qdeferred.pri)
include($$PWD/../QDeferred/src/qlambdathreadworker.pri)

android: INCLUDEPATH += $$PWD/../android_openssl/static/include


INCLUDEPATH += $$PWD \
				$$PWD/../jwt-cpp/include


wasm|ios|android: INCLUDEPATH += $$PWD/../../client/src/wasm_helper
else: INCLUDEPATH += $$PWD/../CuteLogger/include

DEFINES += \
	COS_VERSION_MAJOR=$$VER_MAJ \
	COS_VERSION_MINOR=$$VER_MIN \

HEADERS += \
	credential.h \
	gamemap.h \
	gamemapreaderiface.h \
	utils.h \
	websocketmessage.h

!wasm: HEADERS += \
	googleoauth2authenticator.h \
	oauth2authenticator.h \
	oauth2codeflow.h \
	oauth2replyhandler.h

SOURCES += \
	credential.cpp \
	gamemap.cpp \
	gamemapreaderiface.cpp \
	utils.cpp \
	websocketmessage.cpp

!wasm: SOURCES += \
	googleoauth2authenticator.cpp \
	oauth2authenticator.cpp \
	oauth2codeflow.cpp \
	oauth2replyhandler.cpp


!wasm {
	HEADERS += \
		database.h \

	SOURCES += \
		database.cpp \

}
