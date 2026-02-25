TEMPLATE = lib
CONFIG += plugin

android: TARGET = mergeblock_$${QT_ARCH}
else: TARGET = mergeblock

include(../common.pri)

SOURCES += \
	modulemergeblock.cpp

HEADERS += \
	modulemergeblock.h

RESOURCES += \
	qml_mergeblock.qrc
