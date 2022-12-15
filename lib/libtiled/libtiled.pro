TEMPLATE = lib

android: TARGET = tiled_$${QT_ARCH}
else: TARGET = tiled

CONFIG += c++17 staticlib

LIBS += -lz

DEFINES += \
	TILED_LIBRARY \
	QT_NO_CAST_FROM_ASCII \
	QT_NO_CAST_TO_ASCII \
	QT_NO_URL_CAST_FROM_STRING \
	QT_NO_DEPRECATED_WARNINGS \
	_USE_MATH_DEFINES

HEADERS += \
	libtiled/compression.h \
	libtiled/containerhelpers.h \
	libtiled/fileformat.h \
	libtiled/filesystemwatcher.h \
	libtiled/gidmapper.h \
	libtiled/grid.h \
	libtiled/grouplayer.h \
	libtiled/hex.h \
	libtiled/hexagonalrenderer.h \
	libtiled/imagecache.h \
	libtiled/imagelayer.h \
	libtiled/imagereference.h \
	libtiled/isometricrenderer.h \
	libtiled/layer.h \
	libtiled/logginginterface.h \
	libtiled/map.h \
	libtiled/mapformat.h \
	libtiled/mapobject.h \
	libtiled/mapreader.h \
	libtiled/maprenderer.h \
	libtiled/maptovariantconverter.h \
	libtiled/mapwriter.h \
	libtiled/minimaprenderer.h \
	libtiled/object.h \
	libtiled/objectgroup.h \
	libtiled/objecttemplate.h \
	libtiled/objecttemplateformat.h \
	libtiled/objecttypes.h \
	libtiled/orthogonalrenderer.h \
	libtiled/plugin.h \
	libtiled/pluginmanager.h \
	libtiled/properties.h \
	libtiled/propertytype.h \
	libtiled/qtcompat_p.h \
	libtiled/savefile.h \
	libtiled/staggeredrenderer.h \
	libtiled/templatemanager.h \
	libtiled/tile.h \
	libtiled/tileanimationdriver.h \
	libtiled/tiled.h \
	libtiled/tiled_global.h \
	libtiled/tilelayer.h \
	libtiled/tileset.h \
	libtiled/tilesetformat.h \
	libtiled/tilesetmanager.h \
	libtiled/varianttomapconverter.h \
	libtiled/wangset.h \
	libtiled/worldmanager.h

SOURCES += \
	libtiled/compression.cpp \
	libtiled/fileformat.cpp \
	libtiled/filesystemwatcher.cpp \
	libtiled/gidmapper.cpp \
	libtiled/grouplayer.cpp \
	libtiled/hex.cpp \
	libtiled/hexagonalrenderer.cpp \
	libtiled/imagecache.cpp \
	libtiled/imagelayer.cpp \
	libtiled/imagereference.cpp \
	libtiled/isometricrenderer.cpp \
	libtiled/layer.cpp \
	libtiled/logginginterface.cpp \
	libtiled/map.cpp \
	libtiled/mapformat.cpp \
	libtiled/mapobject.cpp \
	libtiled/mapreader.cpp \
	libtiled/maprenderer.cpp \
	libtiled/maptovariantconverter.cpp \
	libtiled/mapwriter.cpp \
	libtiled/minimaprenderer.cpp \
	libtiled/object.cpp \
	libtiled/objectgroup.cpp \
	libtiled/objecttemplate.cpp \
	libtiled/objecttemplateformat.cpp \
	libtiled/objecttypes.cpp \
	libtiled/orthogonalrenderer.cpp \
	libtiled/plugin.cpp \
	libtiled/pluginmanager.cpp \
	libtiled/properties.cpp \
	libtiled/propertytype.cpp \
	libtiled/savefile.cpp \
	libtiled/staggeredrenderer.cpp \
	libtiled/templatemanager.cpp \
	libtiled/tile.cpp \
	libtiled/tileanimationdriver.cpp \
	libtiled/tiled.cpp \
	libtiled/tilelayer.cpp \
	libtiled/tileset.cpp \
	libtiled/tilesetformat.cpp \
	libtiled/tilesetmanager.cpp \
	libtiled/varianttomapconverter.cpp \
	libtiled/wangset.cpp \
	libtiled/worldmanager.cpp

