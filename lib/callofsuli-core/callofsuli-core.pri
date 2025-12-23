include(../../common.pri)

INCLUDEPATH += $$PWD

!wasm: {
	include($$PWD/../QDeferred/src/qdeferred.pri)
	include($$PWD/../QDeferred/src/qlambdathreadworker.pri)

	INCLUDEPATH += $$PWD/../QDeferred/src
}

android: INCLUDEPATH += $$PWD/../android_openssl/static/include

# info: utils_.h includes selectableobject.h

HEADERS += \
	$$PWD/commonsettings.h \
	$$PWD/credential.h \
	$$PWD/gamemap.h \
	$$PWD/gamemapreaderiface.h \
	$$PWD/rank.h \
	$$PWD/rpgconfig.h \
	$$PWD/rpgstream.h \
	$$PWD/selectableobject.h \
	$$PWD/udpbitstream.hpp \
	$$PWD/udphelper.h \
	$$PWD/utils_.h


SOURCES += \
	$$PWD/credential.cpp \
	$$PWD/gamemap.cpp \
	$$PWD/gamemapreaderiface.cpp \
	$$PWD/rank.cpp \
	$$PWD/rpgconfig.cpp \
	$$PWD/rpgstream.cpp \
	$$PWD/selectableobject.cpp \
	$$PWD/utils_.cpp

android: {
	HEADERS += \
		$$PWD/mobileutils.h

	SOURCES += \
		$$PWD/mobileutils.cpp
}


ios: {
	HEADERS += \
		$$PWD/mobileutils.h

	OBJECTIVE_SOURCES += \
		$$PWD/mobileutils.mm
}



!isEmpty(FtxuiPath) {
	DEFINES += WITH_FTXUI

	INCLUDEPATH += $${FtxuiPath}/include
	LIBS += -L$${FtxuiPath}/lib -lftxui-component -lftxui-dom -lftxui-screen

	HEADERS += \
		$$PWD/ftxterminal.hpp
}
