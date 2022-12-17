include(../common.pri)

TEMPLATE = subdirs

SUBDIRS = \
	libtiled \
	SortFilterProxyModel \
	QtXlsxWriter \
	QZXing

!wasm: SUBDIRS += CuteLogger

!wasm|if($$WasmWithBox2D) {
	SUBDIRS += qml-box2d
}

if ($$SkipLibraryMakeIfExists) {
	!exists($$OUT_PWD/$$LibQaterialFile): SUBDIRS += qaterial
} else {
	SUBDIRS += qaterial
}

