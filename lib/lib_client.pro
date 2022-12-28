include(../common.pri)

TEMPLATE = subdirs

SUBDIRS = \
	libQZXing \
	libQaterial \
	libTiled \
	libSortFilterProxyModel \
	libQtXlsxWriter \
	callofsuli-core

!wasm: SUBDIRS += CuteLogger

!wasm|if($$WasmWithBox2D) {
	SUBDIRS += qml-box2d
}


