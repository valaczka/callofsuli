include(../common.pri)

TEMPLATE = subdirs

SUBDIRS = \
	QtService \
	callofsuli-core \
	CuteLogger

QtService.file = QtService/qtservice.pro

CONFIG += ordered

