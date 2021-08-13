TEMPLATE = subdirs

unix:!android: {
	SUBDIRS = version
}

SUBDIRS += client/QtXlsxWriter \
		client/app/modules \
		client/app

CONFIG += ordered
