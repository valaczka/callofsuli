TEMPLATE = app
TARGET = callofsuliTest

QT += gui quick svg #sql websockets quick svg multimedia network networkauth gui-private webview

CONFIG += c++17

include(../../common.pri)
include(../../version/version.pri)

AppRpath =
include(../lib/import_lib.pri)
FullAppRpath = $$join(AppRpath,",","-Wl,")

!isEmpty(FullAppRpath):!wasm  {
	QMAKE_LFLAGS += $$FullAppRpath
}


!wasm: DEFINES += WITH_BOX2D

wasm:include(app_wasm.pri)


SOURCES += \
	application.cpp \
	desktopapplication.cpp \
	main.cpp \
	onlineapplication.cpp

RESOURCES += \
	../qml/qml.qrc

HEADERS += \
	../../version/version.h \
	application.h \
	desktopapplication.h \
	onlineapplication.h
