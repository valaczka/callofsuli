include(../common.pri)

TargetSuffix =
win32: TargetSuffix = /release


# Qaterial

INCLUDEPATH += $$PWD/qaterial/Qaterial-1.4.6/src

QMLPATHS += $$PWD/qaterial/Qaterial-1.4.6/qml/Qaterial

AppRpath += --rpath=. --rpath=../lib --rpath=./lib

android {
	LIBS += -L../../lib/qaterial/$${QT_ARCH}/android-build/libs/$${QT_ARCH}/ -lQaterial_$${QT_ARCH}
	for (abi, ANDROID_ABIS): ANDROID_EXTRA_LIBS += $$OUT_PWD/../../lib/qaterial/$${abi}/android-build/libs/$${abi}/libQaterial_$${abi}.so
} else:wasm {
	LIBS += -L../../lib/qaterial -lQaterial
} else {
	LIBS += -L../../lib -lQaterial
}


# Tiled

INCLUDEPATH += $$PWD/libtiled

android: LIBS += -L../../lib/libtiled -ltiled_$${QT_ARCH}
else: LIBS += -L../../lib/libtiled$${TargetSuffix} -ltiled

LIBS += -lz


# QZXing

INCLUDEPATH += $$PWD/QZXing

android: LIBS += -L../../lib -lQZXing_$${QT_ARCH}
else: LIBS += -L../../lib$${TargetSuffix} -lQZXing


# QML-Box2D

!wasm|if($$WasmWithBox2D) {
	INCLUDEPATH += $$PWD/qml-box2d/qml-box2d
	INCLUDEPATH += $$PWD/qml-box2d/qml-box2d/Box2D

	android {
		LIBS += -L../../lib/qml-box2d/$${QT_ARCH}/bin/plugins/Box2D -lqmlbox2d_$${QT_ARCH}
		for (abi, ANDROID_ABIS): ANDROID_EXTRA_LIBS += $$OUT_PWD/../../lib/qml-box2d/$${abi}/bin/plugins/Box2D/libqmlbox2d_$${abi}.so
	} else:wasm {
		LIBS += \
				../../lib/qml-box2d/bin/plugins/Box2D/libqmlbox2d.a \
				../../lib/qml-box2d/lib/libBox2D.a
	} else {
		LIBS += -L../../lib -lqmlbox2d
	}
}



# CuteLogger

!wasm: INCLUDEPATH += $$PWD/CuteLogger/include

android: LIBS += -L../../lib -lCuteLogger_$${QT_ARCH}
else:!wasm: LIBS += -L../../lib$${TargetSuffix} -lCuteLogger



# SortFilterProxyModel

INCLUDEPATH += $$PWD/SortFilterProxyModel

android: LIBS += -L../../lib/SortFilterProxyModel -lSortFilterProxyModel_$${QT_ARCH}
else: LIBS += -L../../lib/SortFilterProxyModel$${TargetSuffix} -lSortFilterProxyModel


# QtXlsxWriter

INCLUDEPATH += $$PWD/QtXlsxWriter

android: LIBS += -L../../lib -lQtXlsxWriter_$${QT_ARCH}
else: LIBS += -L../../lib$${TargetSuffix} -lQtXlsxWriter
