include(../common.pri)

# Core

LIBS += -L../../lib



# QOlm

INCLUDEPATH += \
	$$PWD/QOlm/src \
	$$PWD/QOlm/include

android: LIBS += -lQOlm_$${QT_ARCH}
else: LIBS += -lQOlm

DEFINES += QOLM_STATIC


# Qaterial

INCLUDEPATH += \
	$$PWD/Qaterial/src

QMLPATHS += $$PWD/Qaterial/qml/Qaterial

android: LIBS += -lQaterial_$${QT_ARCH}
else: LIBS += -lQaterial

DEFINES += QATERIAL_STATIC


# Tiled

INCLUDEPATH += \
	$$PWD/tiled/src \
	$$PWD/tiled/src/libtiled \
	$$PWD/tiled/src/libtiledquick

android: LIBS += -ltiled_$${QT_ARCH}
else: LIBS += -ltiled

!wasm: LIBS += -lz


# SCodes

INCLUDEPATH += \
	$$PWD/SCodes/src/ \
	$$PWD/SCodes/src/zxing-cpp/core/src/ \
	$$PWD/SCodes/src/zxing-cpp/thirdparty/stb/

android: LIBS += -lSCodes_$${QT_ARCH}
else: LIBS += -lSCodes


# Box2D

INCLUDEPATH += $$PWD/box2d/include
INCLUDEPATH += $$PWD/box2cpp/include

android: LIBS += -lbox2d_$${QT_ARCH}
else: LIBS += -lbox2d



# CuteLogger

android|ios|wasm {
	HEADERS += $$PWD/../client/src/wasm_helper/Logger.h
	INCLUDEPATH += $$PWD/../client/src/wasm_helper
} else {
	INCLUDEPATH += $$PWD/CuteLogger/include
	LIBS += -lCuteLogger
}




# SortFilterProxyModel

include(SortFilterProxyModel/SortFilterProxyModel.pri)


# QtXlsxWriter

INCLUDEPATH += $$PWD/QtXlsxWriter/src/xlsx

android: LIBS += -lQtXlsxWriter_$${QT_ARCH}
else: LIBS += -lQtXlsxWriter




# QSingleInstance

linux|win32|mac: {
	HEADERS += \
		$$PWD/QSingleInstance/QSingleInstance/QSingleInstance \
		$$PWD/QSingleInstance/QSingleInstance/clientinstance.h \
		$$PWD/QSingleInstance/QSingleInstance/qsingleinstance.h \
		$$PWD/QSingleInstance/QSingleInstance/qsingleinstance_p.h

	SOURCES += \
		$$PWD/QSingleInstance/QSingleInstance/clientinstance.cpp \
		$$PWD/QSingleInstance/QSingleInstance/qsingleinstance.cpp \
		$$PWD/QSingleInstance/QSingleInstance/qsingleinstance_p.cpp

	INCLUDEPATH += \
		$$PWD/QSingleInstance/QSingleInstance
}


# Android OpenSSL

android: include($$PWD/android_openssl/openssl.pri)




# QSyncable

include(qsyncable.pri)



# Miniaudio

INCLUDEPATH += $$PWD/miniaudio


# Tar_to_stream

INCLUDEPATH += $$PWD/tar_to_stream

HEADERS += \
	$$PWD/tar_to_stream/tar_to_stream.h



# CSV parser

INCLUDEPATH += $$PWD/csv-parser/single_include


# STB

INCLUDEPATH += $$PWD/stb


# QSerializer

DEFINES += QS_HAS_JSON
include($$PWD/QSerializer/qserializer.pri)


# tcod

INCLUDEPATH += $$PWD/libtcod/libtcod/src/

android: LIBS += -ltcod_$${QT_ARCH}
else: LIBS += -ltcod


# ENet

!wasm {
	INCLUDEPATH += $$PWD/enet/include
	android: LIBS += -lenet_$${QT_ARCH}
	else: LIBS += -lenet
}


# Sodium

INCLUDEPATH += $$LibSodiumInclude
LIBS += $$LibSodiumLibs


!isEmpty(LibSodiumDefines) {
	DEFINES += $$LibSodiumDefines
}

