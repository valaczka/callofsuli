include(../common.pri)

TEMPLATE = subdirs

SUBDIRS += \
	libQOlm \
	libQaterial \
	libTiled \
	libQtXlsxWriter \
	libSCodes \
	qml-box2d

!wasm:!android:!ios: SUBDIRS += CuteLogger



