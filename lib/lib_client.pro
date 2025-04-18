include(../common.pri)

TEMPLATE = subdirs

SUBDIRS += \
	libtcod \
	libQOlm \
	libQaterial \
	libTiled \
	libQtXlsxWriter \
	libSCodes \
	libChipmunk2D

!wasm: SUBDIRS += libENet
!wasm:!android:!ios: SUBDIRS += CuteLogger



