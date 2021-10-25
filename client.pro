TEMPLATE = subdirs

unix:!android: {
	SUBDIRS = version
}

SUBDIRS += client/QtXlsxWriter \
		client/QZXing-static \
		client/app/modules \
		client/app

CONFIG += ordered
