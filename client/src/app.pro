TEMPLATE = app
TARGET = callofsuli

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


!wasm {
	DEFINES += WITH_BOX2D
	SOURCES += desktopapplication.cpp
	HEADERS += desktopapplication.h
}

wasm:include(app_wasm.pri)


SOURCES += \
	application.cpp \
	main.cpp

RESOURCES += \
	../qml/qml.qrc \
	../qml/QaterialHelper.qrc


HEADERS += \
	../../version/version.h \
	application.h
