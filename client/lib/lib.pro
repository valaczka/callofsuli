include(../../common.pri)

TEMPLATE = subdirs

SUBDIRS = \
	qaterial \
	libtiled


!wasm|if($$WasmWithBox2D) {
	SUBDIRS += qml-box2d
}

