TEMPLATE = lib

TARGET = tiled

CONFIG += c++17 staticlib

DEFINES += \
	TILED_LIBRARY \
	QT_NO_CAST_FROM_ASCII \
	QT_NO_CAST_TO_ASCII \
	QT_NO_URL_CAST_FROM_STRING \
	QT_NO_DEPRECATED_WARNINGS \
	_USE_MATH_DEFINES

HEADERS += \
	libtiled-1.9.2/compression.h \
	libtiled-1.9.2/containerhelpers.h \
	libtiled-1.9.2/fileformat.h \
	libtiled-1.9.2/filesystemwatcher.h \
	libtiled-1.9.2/gidmapper.h \
	libtiled-1.9.2/grid.h \
	libtiled-1.9.2/grouplayer.h \
	libtiled-1.9.2/hex.h \
	libtiled-1.9.2/hexagonalrenderer.h \
	libtiled-1.9.2/imagecache.h \
	libtiled-1.9.2/imagelayer.h \
	libtiled-1.9.2/imagereference.h \
	libtiled-1.9.2/isometricrenderer.h \
	libtiled-1.9.2/layer.h \
	libtiled-1.9.2/logginginterface.h \
	libtiled-1.9.2/map.h \
	libtiled-1.9.2/mapformat.h \
	libtiled-1.9.2/mapobject.h \
	libtiled-1.9.2/mapreader.h \
	libtiled-1.9.2/maprenderer.h \
	libtiled-1.9.2/maptovariantconverter.h \
	libtiled-1.9.2/mapwriter.h \
	libtiled-1.9.2/minimaprenderer.h \
	libtiled-1.9.2/object.h \
	libtiled-1.9.2/objectgroup.h \
	libtiled-1.9.2/objecttemplate.h \
	libtiled-1.9.2/objecttemplateformat.h \
	libtiled-1.9.2/objecttypes.h \
	libtiled-1.9.2/orthogonalrenderer.h \
	libtiled-1.9.2/plugin.h \
	libtiled-1.9.2/pluginmanager.h \
	libtiled-1.9.2/properties.h \
	libtiled-1.9.2/propertytype.h \
	libtiled-1.9.2/qtcompat_p.h \
	libtiled-1.9.2/savefile.h \
	libtiled-1.9.2/staggeredrenderer.h \
	libtiled-1.9.2/templatemanager.h \
	libtiled-1.9.2/tile.h \
	libtiled-1.9.2/tileanimationdriver.h \
	libtiled-1.9.2/tiled.h \
	libtiled-1.9.2/tiled_global.h \
	libtiled-1.9.2/tilelayer.h \
	libtiled-1.9.2/tileset.h \
	libtiled-1.9.2/tilesetformat.h \
	libtiled-1.9.2/tilesetmanager.h \
	libtiled-1.9.2/varianttomapconverter.h \
	libtiled-1.9.2/wangset.h \
	libtiled-1.9.2/worldmanager.h

SOURCES += \
	libtiled-1.9.2/compression.cpp \
	libtiled-1.9.2/fileformat.cpp \
	libtiled-1.9.2/filesystemwatcher.cpp \
	libtiled-1.9.2/gidmapper.cpp \
	libtiled-1.9.2/grouplayer.cpp \
	libtiled-1.9.2/hex.cpp \
	libtiled-1.9.2/hexagonalrenderer.cpp \
	libtiled-1.9.2/imagecache.cpp \
	libtiled-1.9.2/imagelayer.cpp \
	libtiled-1.9.2/imagereference.cpp \
	libtiled-1.9.2/isometricrenderer.cpp \
	libtiled-1.9.2/layer.cpp \
	libtiled-1.9.2/logginginterface.cpp \
	libtiled-1.9.2/map.cpp \
	libtiled-1.9.2/mapformat.cpp \
	libtiled-1.9.2/mapobject.cpp \
	libtiled-1.9.2/mapreader.cpp \
	libtiled-1.9.2/maprenderer.cpp \
	libtiled-1.9.2/maptovariantconverter.cpp \
	libtiled-1.9.2/mapwriter.cpp \
	libtiled-1.9.2/minimaprenderer.cpp \
	libtiled-1.9.2/object.cpp \
	libtiled-1.9.2/objectgroup.cpp \
	libtiled-1.9.2/objecttemplate.cpp \
	libtiled-1.9.2/objecttemplateformat.cpp \
	libtiled-1.9.2/objecttypes.cpp \
	libtiled-1.9.2/orthogonalrenderer.cpp \
	libtiled-1.9.2/plugin.cpp \
	libtiled-1.9.2/pluginmanager.cpp \
	libtiled-1.9.2/properties.cpp \
	libtiled-1.9.2/propertytype.cpp \
	libtiled-1.9.2/savefile.cpp \
	libtiled-1.9.2/staggeredrenderer.cpp \
	libtiled-1.9.2/templatemanager.cpp \
	libtiled-1.9.2/tile.cpp \
	libtiled-1.9.2/tileanimationdriver.cpp \
	libtiled-1.9.2/tiled.cpp \
	libtiled-1.9.2/tilelayer.cpp \
	libtiled-1.9.2/tileset.cpp \
	libtiled-1.9.2/tilesetformat.cpp \
	libtiled-1.9.2/tilesetmanager.cpp \
	libtiled-1.9.2/varianttomapconverter.cpp \
	libtiled-1.9.2/wangset.cpp \
	libtiled-1.9.2/worldmanager.cpp
