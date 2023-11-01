!greaterThan(QT_MAJOR_VERSION, 5): error(Qt6 required)

TEMPLATE = app
TARGET = callofsuli-server

QT += network sql networkauth httpserver websockets

CONFIG += c++17 console
CONFIG -= app_bundle
CONFIG += separate_debug_info

include(../../common.pri)
include(../../version/version.pri)
include(../../lib/callofsuli-core/callofsuli-core.pri)

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
	abstractapi.cpp \
	adminapi.cpp \
	authapi.cpp \
	databasemain.cpp \
	generalapi.cpp \
	googleoauth2authenticator.cpp \
	handler.cpp \
	main.cpp \
	microsoftoauth2authenticator.cpp \
	oauth2authenticator.cpp \
	oauth2codeflow.cpp \
	serverservice.cpp \
	serversettings.cpp \
	teacherapi.cpp \
	userapi.cpp \
	webserver.cpp

RESOURCES += \
	html.qrc \
	server.qrc


HEADERS += \
	../../version/version.h \
	abstractapi.h \
	adminapi.h \
	authapi.h \
	databasemain.h \
	generalapi.h \
	googleoauth2authenticator.h \
	handler.h \
	microsoftoauth2authenticator.h \
	oauth2authenticator.h \
	oauth2codeflow.h \
	serverservice.h \
	serversettings.h \
	teacherapi.h \
	userapi.h \
	webserver.h

