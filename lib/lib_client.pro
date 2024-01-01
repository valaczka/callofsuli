include(../common.pri)

TEMPLATE = subdirs

SUBDIRS += \
	libQOlm \
	libQaterial \
	libQZXing \
	libTiled \
	libQtXlsxWriter \
	libSCodes \
	qml-box2d

!wasm:!android:!ios: SUBDIRS += CuteLogger



