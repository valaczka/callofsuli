include(../common.pri)

TEMPLATE = subdirs

SUBDIRS = libQaterial

exists($${QaterialLibFilePath}) {
	SUBDIRS += \
		libQZXing \
		libTiled \
		libQtXlsxWriter \
		callofsuli-core

	!wasm: SUBDIRS += CuteLogger

	!wasm|if($$WasmWithBox2D) {
		SUBDIRS += qml-box2d
	}

}

