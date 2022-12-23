include(../common.pri)

TEMPLATE = subdirs

SUBDIRS = \
	qaterial \
	libtiled \
	SortFilterProxyModel \
	QtXlsxWriter \
	QZXing \
	callofsuli-core

!wasm: SUBDIRS += CuteLogger

!wasm|if($$WasmWithBox2D) {
	SUBDIRS += qml-box2d
}

