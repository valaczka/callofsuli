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


QMAKE_CXXFLAGS -= -Wextra
QMAKE_CXXFLAGS += -Wno-extra

win32 {
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
	RC_ICONS = $$PWD/../../resources/internal/img/cos.ico
	RC_LANG = 0x040E
	QMAKE_TARGET_COPYRIGHT = Valaczka Janos Pal
	QMAKE_TARGET_DESCRIPTION = Call of Suli server
}


SOURCES += \
	abstracthandler.cpp \
	adminhandler.cpp \
	authhandler.cpp \
	client.cpp \
	databasemain.cpp \
	googleoauth2authenticator.cpp \
	main.cpp \
	oauth2authenticator.cpp \
	oauth2codeflow.cpp \
	oauth2replyhandler.cpp \
	serverhandler.cpp \
	serverservice.cpp \
	serversettings.cpp \
	websocketserver.cpp

RESOURCES += \
	server.qrc


HEADERS += \
	../../version/version.h \
	abstracthandler.h \
	adminhandler.h \
	authhandler.h \
	client.h \
	databasemain.h \
	googleoauth2authenticator.h \
	oauth2authenticator.h \
	oauth2codeflow.h \
	oauth2replyhandler.h \
	serverhandler.h \
	serverservice.h \
	serversettings.h \
	websocketserver.h


