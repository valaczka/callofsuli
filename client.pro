include(common.pri)

TEMPLATE = subdirs

client_lib.file = lib/lib_client.pro
client_lib.makefile = Makefile
application.file = client/src/app.pro
application.makefile = Makefile


SUBDIRS += \
			version \
			client_lib \
			application


linux|win32|mac:!android:!ios {
	if($$CreateBundle):	SUBDIRS += bundle
}

CONFIG += ordered

