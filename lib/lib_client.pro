include(../common.pri)

TEMPLATE = subdirs

SUBDIRS = \
	libtiled \
	SortFilterProxyModel \
	QtXlsxWriter \
	QZXing

!wasm: SUBDIRS += CuteLogger


if ($$SkipLibraryMakeIfExists) {
	!exists($$OUT_PWD/$$LibQaterialFile): SUBDIRS += qaterial

	!wasm|if($$WasmWithBox2D) {
		!exists($$OUT_PWD/$$LibQmlBox2DFile): SUBDIRS += qml-box2d
	}

} else {
	SUBDIRS += qaterial

	!wasm|if($$WasmWithBox2D) {
		SUBDIRS += qml-box2d
	}
}

