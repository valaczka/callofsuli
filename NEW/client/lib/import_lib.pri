include(../../common.pri)

INCLUDEPATH += $$PWD/qaterial/Qaterial-1.4.6/src
LIBS += -L../lib/qaterial -lQaterial
AppRpath += --rpath=../lib/qaterial

#INCLUDEPATH += $$PWD/libtiled/
LIBS += ../lib/libtiled/libtiled.a

!wasm|if($$WasmWithBox2D) {
	INCLUDEPATH += $$PWD/qml-box2d/qml-box2d
	INCLUDEPATH += $$PWD/qml-box2d/qml-box2d/Box2D
	LIBS += \
		../lib/qml-box2d/bin/plugins/Box2D/libqmlbox2d.a \
		../lib/qml-box2d/lib/libBox2D.a
}
