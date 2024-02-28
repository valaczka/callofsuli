TEMPLATE = lib

android: TARGET = tiled_$${QT_ARCH}
else: TARGET = tiled

CONFIG += c++17
CONFIG += separate_debug_info

QT += quick

DESTDIR = ..

!wasm: LIBS += -lz

SOURCE_DIR = $$PWD/../tiled/src/libtiled
SOURCE_DIR_QUICK = $$PWD/../tiled/src/libtiledquick

INCLUDEPATH += $$PWD/../tiled/src/libtiled

DEFINES += \
	TILED_LIBRARY \
	TILED_LIB_DIR=\\\"$${SOURCE_DIR}\\\" \
	QT_NO_CAST_FROM_ASCII \
	QT_NO_CAST_TO_ASCII \
	QT_NO_URL_CAST_FROM_STRING \
	QT_NO_DEPRECATED_WARNINGS \
	_USE_MATH_DEFINES \
	TILED_QUICK_LIBRARY \
	QT_NO_FOREACH


HEADERS += \
	$${SOURCE_DIR}/compression.h \
	$${SOURCE_DIR}/containerhelpers.h \
	$${SOURCE_DIR}/fileformat.h \
	$${SOURCE_DIR}/filesystemwatcher.h \
	$${SOURCE_DIR}/gidmapper.h \
	$${SOURCE_DIR}/grid.h \
	$${SOURCE_DIR}/grouplayer.h \
	$${SOURCE_DIR}/hex.h \
	$${SOURCE_DIR}/hexagonalrenderer.h \
	$${SOURCE_DIR}/imagecache.h \
	$${SOURCE_DIR}/imagelayer.h \
	$${SOURCE_DIR}/imagereference.h \
	$${SOURCE_DIR}/isometricrenderer.h \
	$${SOURCE_DIR}/layer.h \
	$${SOURCE_DIR}/logginginterface.h \
	$${SOURCE_DIR}/map.h \
	$${SOURCE_DIR}/mapformat.h \
	$${SOURCE_DIR}/mapobject.h \
	$${SOURCE_DIR}/mapreader.h \
	$${SOURCE_DIR}/maprenderer.h \
	$${SOURCE_DIR}/maptovariantconverter.h \
	$${SOURCE_DIR}/mapwriter.h \
	$${SOURCE_DIR}/minimaprenderer.h \
	$${SOURCE_DIR}/object.h \
	$${SOURCE_DIR}/objectgroup.h \
	$${SOURCE_DIR}/objecttemplate.h \
	$${SOURCE_DIR}/objecttemplateformat.h \
	$${SOURCE_DIR}/objecttypes.h \
	$${SOURCE_DIR}/orthogonalrenderer.h \
	$${SOURCE_DIR}/plugin.h \
	$${SOURCE_DIR}/pluginmanager.h \
	$${SOURCE_DIR}/properties.h \
	$${SOURCE_DIR}/propertytype.h \
	$${SOURCE_DIR}/qtcompat_p.h \
	$${SOURCE_DIR}/savefile.h \
	$${SOURCE_DIR}/staggeredrenderer.h \
	$${SOURCE_DIR}/templatemanager.h \
	$${SOURCE_DIR}/tile.h \
	$${SOURCE_DIR}/tileanimationdriver.h \
	$${SOURCE_DIR}/tiled.h \
	$${SOURCE_DIR}/tiled_global.h \
	$${SOURCE_DIR}/tilelayer.h \
	$${SOURCE_DIR}/tileset.h \
	$${SOURCE_DIR}/tilesetformat.h \
	$${SOURCE_DIR}/tilesetmanager.h \
	$${SOURCE_DIR}/varianttomapconverter.h \
	$${SOURCE_DIR}/wangset.h \
	$${SOURCE_DIR}/world.h \
	$${SOURCE_DIR_QUICK}/mapitem.h \
	$${SOURCE_DIR_QUICK}/maploader.h \
	$${SOURCE_DIR_QUICK}/mapref.h \
	$${SOURCE_DIR_QUICK}/tiledquick_global.h \
	$${SOURCE_DIR_QUICK}/tilelayeritem.h \
	$${SOURCE_DIR_QUICK}/tilesnode.h

SOURCES += \
	$${SOURCE_DIR}/compression.cpp \
	$${SOURCE_DIR}/fileformat.cpp \
	$${SOURCE_DIR}/filesystemwatcher.cpp \
	$${SOURCE_DIR}/gidmapper.cpp \
	$${SOURCE_DIR}/grouplayer.cpp \
	$${SOURCE_DIR}/hex.cpp \
	$${SOURCE_DIR}/hexagonalrenderer.cpp \
	$${SOURCE_DIR}/imagecache.cpp \
	$${SOURCE_DIR}/imagelayer.cpp \
	$${SOURCE_DIR}/imagereference.cpp \
	$${SOURCE_DIR}/isometricrenderer.cpp \
	$${SOURCE_DIR}/layer.cpp \
	$${SOURCE_DIR}/logginginterface.cpp \
	$${SOURCE_DIR}/map.cpp \
	$${SOURCE_DIR}/mapformat.cpp \
	$${SOURCE_DIR}/mapobject.cpp \
	$${SOURCE_DIR}/mapreader.cpp \
	$${SOURCE_DIR}/maprenderer.cpp \
	$${SOURCE_DIR}/maptovariantconverter.cpp \
	$${SOURCE_DIR}/mapwriter.cpp \
	$${SOURCE_DIR}/minimaprenderer.cpp \
	$${SOURCE_DIR}/object.cpp \
	$${SOURCE_DIR}/objectgroup.cpp \
	$${SOURCE_DIR}/objecttemplate.cpp \
	$${SOURCE_DIR}/objecttemplateformat.cpp \
	$${SOURCE_DIR}/objecttypes.cpp \
	$${SOURCE_DIR}/orthogonalrenderer.cpp \
	$${SOURCE_DIR}/plugin.cpp \
	$${SOURCE_DIR}/pluginmanager.cpp \
	$${SOURCE_DIR}/properties.cpp \
	$${SOURCE_DIR}/propertytype.cpp \
	$${SOURCE_DIR}/savefile.cpp \
	$${SOURCE_DIR}/staggeredrenderer.cpp \
	$${SOURCE_DIR}/templatemanager.cpp \
	$${SOURCE_DIR}/tile.cpp \
	$${SOURCE_DIR}/tileanimationdriver.cpp \
	$${SOURCE_DIR}/tiled.cpp \
	$${SOURCE_DIR}/tilelayer.cpp \
	$${SOURCE_DIR}/tileset.cpp \
	$${SOURCE_DIR}/tilesetformat.cpp \
	$${SOURCE_DIR}/tilesetmanager.cpp \
	$${SOURCE_DIR}/varianttomapconverter.cpp \
	$${SOURCE_DIR}/wangset.cpp \
	$${SOURCE_DIR}/world.cpp \
	$${SOURCE_DIR_QUICK}/mapitem.cpp \
	$${SOURCE_DIR_QUICK}/maploader.cpp \
	$${SOURCE_DIR_QUICK}/tilelayeritem.cpp \
	$${SOURCE_DIR_QUICK}/tilesnode.cpp

