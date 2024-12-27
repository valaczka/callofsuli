TEMPLATE = lib

android: TARGET = box2d_$${QT_ARCH}
else: TARGET = box2d

CONFIG += c++17 separate_debug_info
CONFIG -= qt


VERSION = 3.1.0


SOURCEDIR = $$PWD/../box2d/src
DESTDIR = ..


SOURCES = $$SOURCEDIR/*.c
HEADERS = $$SOURCEDIR/*.h

HEADERS += \
	$$SOURCEDIR/../include/box2d/base.h \
	$$SOURCEDIR/../include/box2d/box2d.h \
	$$SOURCEDIR/../include/box2d/collision.h \
	$$SOURCEDIR/../include/box2d/id.h \
	$$SOURCEDIR/../include/box2d/math_functions.h \
	$$SOURCEDIR/../include/box2d/types.h \

INCLUDEPATH += $$SOURCEDIR/../include

#QMAKE_CFLAGS += -fvisibility=hidden

include(libBox2D.pri)

win32 {
	win32-g++ {
		QMAKE_CFLAGS += -mavx2 -ffp-contract=off
	}
}

wasm {
	QMAKE_CFLAGS += -msimd128 -msse2
}

linux:!android {
	QMAKE_CFLAGS += -mavx2 -ffp-contract=off
}

ios {
	QMAKE_CFLAGS += -fsanitize=address -fsanitize-address-use-after-scope -fsanitize=undefined -ffp-contract=off
	LIBS += -fsanitize=address -fsanitize-address-use-after-scope -fsanitize=undefined
}

