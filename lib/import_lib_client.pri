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

INCLUDEPATH += $$PWD/tiled/src

android: LIBS += -ltiled_$${QT_ARCH}
else: LIBS += -ltiled

!wasm: LIBS += -lz


# QZXing

INCLUDEPATH += $$PWD/qzxing/src

android: LIBS += -lQZXing_$${QT_ARCH}
else: LIBS += -lQZXing

INCLUDEPATH += \
	$$PWD/SCodes/src/ \
	$$PWD/SCodes/src/zxing-cpp/core/src/ \
	$$PWD/SCodes/src/zxing-cpp/thirdparty/stb/

android: LIBS += -lSCodes_$${QT_ARCH}
else: LIBS += -lSCodes


# QML-Box2D

INCLUDEPATH += $$PWD/qml-box2d/
INCLUDEPATH += $$PWD/qml-box2d/Box2D

android: LIBS += -lqml-box2d_$${QT_ARCH}
else: LIBS += -lqml-box2d



# CuteLogger

android|ios|wasm {
	HEADERS += $$PWD/../client/src/wasm_helper/Logger.h
	INCLUDEPATH += $$PWD//../client/src/wasm_helper
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
