TEMPLATE = subdirs

#CONFIG += skip_version

!skip_version {
	SUBDIRS = version
}

SUBDIRS += server/app

CONFIG += ordered
