#INCLUDEPATH += \
#	$$PWD/../QOlm/include

CONFIG += c++17 static
CONFIG += separate_debug_info

QT += gui quick

TEMPLATE = lib

android: TARGET = SCodes_$${QT_ARCH}
else: TARGET = SCodes

DESTDIR = ../

include($$PWD/../SCodes/src/SCodes.pri)
