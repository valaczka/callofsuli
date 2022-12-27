TEMPLATE = lib

android: TARGET = CuteLogger_$${QT_ARCH}
else: TARGET = CuteLogger

#!win32: CONFIG += staticlib

DESTDIR = ..

DEFINES += CUTELOGGER_LIBRARY

INCLUDEPATH += $$PWD/include

SOURCES += $$PWD/src/Logger.cpp \
		   $$PWD/src/AbstractAppender.cpp \
		   $$PWD/src/AbstractStringAppender.cpp \
		   $$PWD/src/ConsoleAppender.cpp \
		   $$PWD/src/FileAppender.cpp \
		   $$PWD/src/RollingFileAppender.cpp \
		   $$PWD/src/ColorConsoleAppender.cpp

HEADERS += $$PWD/include/Logger.h \
		   $$PWD/include/CuteLogger_global.h \
		   $$PWD/include/AbstractAppender.h \
		   $$PWD/include/AbstractStringAppender.h \
		   $$PWD/include/ConsoleAppender.h \
		   $$PWD/include/FileAppender.h \
		   $$PWD/include/RollingFileAppender.h \
		   $$PWD/include/ColorConsoleAppender.h

win32 {
	SOURCES += $$PWD/src/OutputDebugAppender.cpp
	HEADERS += $$PWD/include/OutputDebugAppender.h
}

android {
	SOURCES += $$PWD/src/AndroidAppender.cpp
	HEADERS += $$PWD/include/AndroidAppender.h
}

