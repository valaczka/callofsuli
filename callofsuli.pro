TEMPLATE = subdirs

SUBDIRS = \
	src/3rdparty/Bacon2D \
	src/client

unix:!android: {
	SUBDIRS += src/server
}

CONFIG += ordered
