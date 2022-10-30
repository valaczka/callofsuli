TEMPLATE = lib
CONFIG += plugin

android: TARGET = images_$${QT_ARCH}
else: TARGET = images

include(../common.pri)

SOURCES += \
	moduleimages.cpp

HEADERS += \
	moduleimages.h

RESOURCES += \
	qml_images.qrc
