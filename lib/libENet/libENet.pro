TEMPLATE = lib

CONFIG += c++20 static
CONFIG += separate_debug_info
CONFIG -= qt qml

android: TARGET = enet_$${QT_ARCH}
else: TARGET = enet


QTDIR = $$dirname(QMAKESPEC)/../bin

DESTDIR = ../

CMAKE_DIR=$$OUT_PWD/CMakeOut

!exists($$CMAKE_DIR/defines.pri) {
	system("$$QTDIR/qt-cmake -S $$PWD/../enet -B $$CMAKE_DIR && cat $$CMAKE_DIR/CMakeCache.txt | grep -e "^HAS_"  | sed s/:INTERNAL// >$$CMAKE_DIR/defines.pri")
}

infile($$CMAKE_DIR/defines.pri, HAS_FCNTL, 1):				DEFINES += HAS_FCNTL=1
infile($$CMAKE_DIR/defines.pri, HAS_GETADDRINFO, 1):		DEFINES += HAS_GETADDRINFO=1
infile($$CMAKE_DIR/defines.pri, HAS_GETHOSTBYADDR_R, 1):	DEFINES += HAS_GETHOSTBYADDR_R=1
infile($$CMAKE_DIR/defines.pri, HAS_GETHOSTBYNAME_R, 1):	DEFINES += HAS_GETHOSTBYNAME_R=1
infile($$CMAKE_DIR/defines.pri, HAS_GETNAMEINFO, 1):		DEFINES += HAS_GETNAMEINFO=1
infile($$CMAKE_DIR/defines.pri, HAS_INET_NTOP, 1):			DEFINES += HAS_INET_NTOP=1
infile($$CMAKE_DIR/defines.pri, HAS_INET_PTON, 1):			DEFINES += HAS_INET_PTON=1
infile($$CMAKE_DIR/defines.pri, HAS_MSGHDR_FLAGS, 1):		DEFINES += HAS_MSGHDR_FLAGS=1
infile($$CMAKE_DIR/defines.pri, HAS_POLL, 1):				DEFINES += HAS_POLL=1
infile($$CMAKE_DIR/defines.pri, HAS_SOCKLEN_T, 4):			DEFINES += HAS_SOCKLEN_T=1


# Sources

INCLUDEPATH += \
	$$PWD/../enet/include/


HEADERS += \
	$$files($$PWD/../enet/include/enet/*.h)

SOURCES += \
	$$files($$PWD/../enet/*.c)

