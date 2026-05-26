include(../common.pri)

TEMPLATE = subdirs

SUBDIRS += \
	libtcod \
	libQOlm \
	libQaterial \
	libTiled \
	libQtXlsxWriter \
	libSCodes \
	libChipmunk2D \
	libCppBinaryStream

if ($$WithGamepad): SUBDIRS += libQGamepad

!wasm: SUBDIRS += libENet
!wasm:!android:!ios: SUBDIRS += CuteLogger



