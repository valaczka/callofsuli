TEMPLATE = subdirs

#CONFIG += skip_version

!skip_version:linux: {
	SUBDIRS = version
}

SUBDIRS += server/smtpclient \
	server/app

CONFIG += ordered
