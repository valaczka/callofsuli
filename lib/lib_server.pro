include(../common.pri)

TEMPLATE = subdirs

SUBDIRS = \
	QtService \

	CuteLogger

QtService.file = QtService/qtservice.pro

CONFIG += ordered

