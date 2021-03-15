TEMPLATE = subdirs

unix:!android: {
	SUBDIRS = version
}

SUBDIRS += client/QtXlsxWriter \
		client/app

CONFIG += ordered
