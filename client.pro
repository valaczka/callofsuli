TEMPLATE = subdirs

#CONFIG += skip_version

!skip_version {
	SUBDIRS = version
}

SUBDIRS += \
		client/Bacon2D-static \
		client/QtXlsxWriter \
		client/QZXing \
		client/app/modules \
		client/app

CONFIG += ordered

