TEMPLATE = subdirs

#CONFIG += skip_version

!skip_version:unix:!android: {
	SUBDIRS = version
}

SUBDIRS += server/smtpclient \
	server/app

CONFIG += ordered
