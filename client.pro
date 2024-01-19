include(common.pri)

android {
	_NDK_PATH=$$(ANDROID_NDK_ROOT)
	_NDK_VERSION=$$str_member($$_NDK_PATH, -12, -1)

	lessThan(QT_MAJOR_VERSION, 6): _NDK_REQUIRED = 21.4.7075529
	else: _NDK_REQUIRED = 25.1.8937393

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

