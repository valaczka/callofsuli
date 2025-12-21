TEMPLATE = lib

android: TARGET = cppbinarystream_$${QT_ARCH}
else: TARGET = cppbinarystream

CONFIG += c++20 static
CONFIG += separate_debug_info
CONFIG -= qt


SOURCEDIR = $$PWD/../CppBinaryStream
DESTDIR = ..


SOURCES = $$SOURCEDIR/src/*.cpp
HEADERS = $$SOURCEDIR/include/BMLib/*.hpp
HEADERS += $$SOURCEDIR/include/BMLib/exceptions/*.hpp

INCLUDEPATH += $$SOURCEDIR/include
