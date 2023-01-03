TEMPLATE = app
TARGET = callofsuli-server

QT += network sql websockets networkauth

CONFIG += c++17 console
CONFIG -= app_bundle

include(../../common.pri)
include(../../version/version.pri)

DESTDIR = ../..

!android:if(linux|win32) {
	AppRpath += --rpath=. --rpath=../lib --rpath=./lib --rpath=./lib/QtService/lib
} else {
	AppRpath =
}

include(../../lib/import_lib_server.pri)

!isEmpty(AppRpath) {
	FullAppRpath = $$join(AppRpath,",","-Wl,")
	QMAKE_LFLAGS += $$FullAppRpath
}


win32 {
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
	RC_ICONS = $$PWD/../../resources/internal/img/cos.ico
	RC_LANG = 0x040E
	QMAKE_TARGET_COPYRIGHT = Valaczka Janos Pal
	QMAKE_TARGET_DESCRIPTION = Call of Suli server
}


SOURCES += \
	client.cpp \
	databasemain.cpp \
	googleoauth2authenticator.cpp \
	main.cpp \
	oauth2authenticator.cpp \
	oauth2codeflow.cpp \
	serverservice.cpp \
	serversettings.cpp \
	websocketserver.cpp

RESOURCES += \
	server.qrc


HEADERS += \
	../../version/version.h \
	client.h \
	databasemain.h \
	googleoauth2authenticator.h \
	oauth2authenticator.h \
	oauth2codeflow.h \
	serverservice.h \
	serversettings.h \
	websocketserver.h


