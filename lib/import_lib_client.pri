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


# QML-Box2D

!wasm|if($$WasmWithBox2D) {
	INCLUDEPATH += $$PWD/qml-box2d/
	INCLUDEPATH += $$PWD/qml-box2d/Box2D

	android: LIBS += -lqml-box2d_$${QT_ARCH}
	else: LIBS += -lqml-box2d

}



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



# Android OpenSSL

android: include($$PWD/android_openssl/openssl.pri)





# QSyncable

include(qsyncable.pri)
