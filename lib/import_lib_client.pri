include(../common.pri)

TargetSuffix =
win32: TargetSuffix = /release

# Core

INCLUDEPATH += $$PWD/callofsuli-core

android: LIBS += -L../../lib
else: LIBS += -L../../lib$${TargetSuffix}

android: LIBS += -lcallofsuli-core_$${QT_ARCH}
else: LIBS += -lcallofsuli-core


# Core includes

INCLUDEPATH += $$PWD/QDeferred/src
INCLUDEPATH += $$PWD/QJsonWebToken/src

# Qaterial

INCLUDEPATH += \
	$$PWD/Qaterial/src \
	$$PWD/QOlm/include

QMLPATHS += $$PWD/Qaterial/qml/Qaterial

android {
	LIBS += -L../../lib/libQaterial/$${QT_ARCH}/android-build/libs/$${QT_ARCH}/ -lQaterial_$${QT_ARCH}

	if ($$QaterialBuildShared) {
		for (abi, ANDROID_ABIS): ANDROID_EXTRA_LIBS += $$OUT_PWD/../../lib/libQaterial/$${abi}/android-build/libs/$${abi}/libQaterial_$${abi}.so
	}
} else {
	LIBS += -L../../lib/libQaterial -lQaterial
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

!wasm: INCLUDEPATH += $$PWD/CuteLogger/include

android: LIBS += -lCuteLogger_$${QT_ARCH}
else:!wasm: LIBS += -lCuteLogger



# SortFilterProxyModel

include(SortFilterProxyModel/SortFilterProxyModel.pri)


# QtXlsxWriter

INCLUDEPATH += $$PWD/QtXlsxWriter/src/xlsx

android: LIBS += -lQtXlsxWriter_$${QT_ARCH}
else: LIBS += -lQtXlsxWriter
