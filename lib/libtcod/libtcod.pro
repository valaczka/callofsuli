TEMPLATE = lib

android: TARGET = tcod_$${QT_ARCH}
else: TARGET = tcod

CONFIG += c++2a separate_debug_info
CONFIG -= qt

win32 {
	DEFINES += LIBTCOD_EXPORTS
} else {
	CONFIG += staticlib
	DEFINES += LIBTCOD_STATIC
}

DESTDIR = ..

HEADERS += \
	$$PWD/libtcod/src/libtcod/bresenham.h \
	$$PWD/libtcod/src/libtcod/bresenham.hpp \
	$$PWD/libtcod/src/libtcod/config.h \
	$$PWD/libtcod/src/libtcod/error.h \
	$$PWD/libtcod/src/libtcod/error.hpp \
	$$PWD/libtcod/src/libtcod/fov.h \
	$$PWD/libtcod/src/libtcod/fov.hpp \
	$$PWD/libtcod/src/libtcod/fov_types.h \
	$$PWD/libtcod/src/libtcod/libtcod_int.h \
	$$PWD/libtcod/src/libtcod/list.h \
	$$PWD/libtcod/src/libtcod/list.hpp \
	$$PWD/libtcod/src/libtcod/logging.h \
	$$PWD/libtcod/src/libtcod/path.hpp \
	$$PWD/libtcod/src/libtcod/path.h \
	$$PWD/libtcod/src/libtcod/portability.h \
	$$PWD/libtcod/src/libtcod/utility.h \
	$$PWD/libtcod/src/libtcod/version.h

SOURCES += \
	$$PWD/libtcod/src/libtcod/bresenham.cpp \
	$$PWD/libtcod/src/libtcod/bresenham_c.c \
	$$PWD/libtcod/src/libtcod/error.c \
	$$PWD/libtcod/src/libtcod/fov.cpp \
	$$PWD/libtcod/src/libtcod/fov_c.c \
	$$PWD/libtcod/src/libtcod/fov_circular_raycasting.c \
	$$PWD/libtcod/src/libtcod/fov_diamond_raycasting.c \
	$$PWD/libtcod/src/libtcod/fov_permissive2.c \
	$$PWD/libtcod/src/libtcod/fov_recursive_shadowcasting.c \
	$$PWD/libtcod/src/libtcod/fov_restrictive.c \
	$$PWD/libtcod/src/libtcod/fov_symmetric_shadowcast.c \
	$$PWD/libtcod/src/libtcod/list_c.c \
	$$PWD/libtcod/src/libtcod/logging.c \
	$$PWD/libtcod/src/libtcod/path.cpp \
	$$PWD/libtcod/src/libtcod/path_c.c
