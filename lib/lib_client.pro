include(../common.pri)

TEMPLATE = subdirs

SUBDIRS += \
	libtcod \
	libQOlm \
	libQaterial \
	libTiled \
	libQtXlsxWriter \
	libSCodes \
	qml-box2d

!wasm: SUBDIRS += libENet
!wasm:!android:!ios: SUBDIRS += CuteLogger



