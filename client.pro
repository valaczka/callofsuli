TEMPLATE = subdirs

#CONFIG += skip_version

unix:!android: {
	SUBDIRS = version
}

SUBDIRS += client/QtXlsxWriter \
		client/QZXing \
		client/app/modules \
		client/app

CONFIG += ordered

