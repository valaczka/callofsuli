TEMPLATE = lib

android: TARGET = chipmunk2d_$${QT_ARCH}
else: TARGET = chipmunk2d

CONFIG += c++17 separate_debug_info
CONFIG -= qt


#VERSION = 7.0.3


SOURCEDIR = $$PWD/../Chipmunk2D
DESTDIR = ..

DEFINES += NDEBUG

SOURCES = $$SOURCEDIR/src/*.c
HEADERS = $$SOURCEDIR/include/chipmunk/*.h

INCLUDEPATH += $$SOURCEDIR/include
