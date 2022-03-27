QT -= gui
QT += sql websockets


include(../../version/version.pri)
include(../../common/common.pri)

CONFIG += c++11 console
CONFIG -= app_bundle

TEMPLATE = app
TARGET = callofsuli-server

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


CONFIG(debug, debug|release) {
	DEFINES += SQL_DEBUG
}


DESTDIR = ../..

QMAKE_LFLAGS += -Wl,--rpath=./,--rpath=./lib/

SOURCES += \
		abstracthandler.cpp \
		admin.cpp \
		client.cpp \
		main.cpp \
		server.cpp \
		serverdb.cpp \
		student.cpp \
		teacher.cpp \
		userinfo.cpp

HEADERS += \
	abstracthandler.h \
	admin.h \
	client.h \
	server.h \
	serverdb.h \
	student.h \
	teacher.h \
	userinfo.h

RESOURCES += \
	resources.qrc


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
