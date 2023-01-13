include(../common.pri)

# Core

INCLUDEPATH += $$PWD/callofsuli-core

LIBS += -L../../lib

android: LIBS += -lcallofsuli-core_$${QT_ARCH}
else: LIBS += -lcallofsuli-core


# Core includes

INCLUDEPATH += $$PWD/QDeferred/src

# Qaterial

INCLUDEPATH += $$PWD/Qaterial/src

android: INCLUDEPATH += $$OUT_PWD/../../lib/libQaterial/$${QT_ARCH}/_deps/qolm-src/include
else: INCLUDEPATH += $$OUT_PWD/../../lib/libQaterial/_deps/qolm-src/include

QMLPATHS += $$PWD/Qaterial/qml/Qaterial

android {
	if ($$QaterialBuildShared) {
		LIBS += -L../../lib/libQaterial/$${QT_ARCH}/android-build/libs/$${QT_ARCH}/ -lQaterial_$${QT_ARCH} -lQOlm_$${QT_ARCH}

		for (abi, ANDROID_ABIS): ANDROID_EXTRA_LIBS += \
			$$OUT_PWD/../../lib/libQaterial/$${abi}/android-build/libs/$${abi}/libQaterial_$${abi}.so \
			$$OUT_PWD/../../lib/libQaterial/$${abi}/android-build/libs/$${abi}/libQOlm_$${abi}.so
	} else {
		LIBS += ../../lib/libQaterial/$${QT_ARCH}/_deps/qolm-build/libQOlm.a ../../lib/libQaterial/$${QT_ARCH}/libQaterial.a
	}
} else {
	LIBS += -L../../lib/libQaterial -lQaterial -L../../lib/libQaterial/_deps/qolm-build -lQOlm
}


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
