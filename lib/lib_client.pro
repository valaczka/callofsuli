include(../common.pri)

TEMPLATE = subdirs

SUBDIRS += \
	libQOlm \
	libQaterial \
	libQZXing \
	libTiled \
	libQtXlsxWriter

!wasm:!android:!ios: SUBDIRS += CuteLogger

!wasm|if($$WasmWithBox2D) {
	SUBDIRS += qml-box2d
}


