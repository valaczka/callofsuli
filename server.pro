include(common.pri)

TEMPLATE = subdirs

server_lib.file = lib/lib_server.pro
server_lib.makefile = Makefile
application.file = server/src/app.pro
application.makefile = Makefile


SUBDIRS += \
			version \
			server_lib \
			application


linux|win32|mac:!android:!ios {
	if($$CreateBundle):	SUBDIRS += bundle
}

CONFIG += ordered

