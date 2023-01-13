include(../common.pri)

TEMPLATE = subdirs

SUBDIRS += \
	libQaterial \
	libQZXing \
	libTiled \
	libQtXlsxWriter \
	callofsuli-core

!wasm:!android:!ios: SUBDIRS += CuteLogger

!wasm|if($$WasmWithBox2D) {
	SUBDIRS += qml-box2d
}


