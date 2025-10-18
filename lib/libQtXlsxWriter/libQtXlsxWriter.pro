include(../../common.pri)

include(../QXlsx/QXlsx/QXlsx.pri)

CONFIG += separate_debug_info

android: TARGET = QXlsx_$${QT_ARCH}
else: TARGET = QXlsx

TEMPLATE = lib

win32 {
	CONFIG += dll
}

DESTDIR = ../

