include(common.pri)

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

