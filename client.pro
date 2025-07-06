include(common.pri)

ios: {
	!versionAtLeast(QT_VERSION, 6.7): error(Qt 6.7 required)
} else {
	!versionAtLeast(QT_VERSION, 6.9): error(Qt 6.9 required)
}

android {
	_NDK_PATH=$$(ANDROID_NDK_ROOT)

	_NDK_VERSION=$$str_member($$_NDK_PATH, -13, -1)
	_NDK_REQUIRED = 27.2.12479018

	!isEqual(_NDK_VERSION, $$_NDK_REQUIRED): error(Invalid NDK: $$_NDK_VERSION)
}


TEMPLATE = subdirs

client_lib.file = lib/lib_client.pro
client_lib.makefile = Makefile
application.file = client/src/app.pro
application.makefile = Makefile
modules.file = client/modules/modules.pro
modules.makefile = Makefile

SUBDIRS += \
		version \
		client_lib \
		modules \
		application

CONFIG(release,debug|release):linux|win32:!android:!ios {
	if($$CreateBundle):	SUBDIRS += bundle-client
} else:wasm: {
	SUBDIRS += bundle-client
	bundle-client.file = bundle-client/bundle-client-wasm.pro
}


CONFIG += ordered

