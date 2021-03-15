TEMPLATE = subdirs

unix:!android: {
	SUBDIRS = version
}

SUBDIRS += server/smtpclient \
	server/app

CONFIG += ordered
