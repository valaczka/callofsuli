include(common.pri)

TEMPLATE = subdirs

client_lib.file = lib/lib_client.pro
client_lib.makefile = Makefile
application.file = client/src/app.pro
application.makefile = Makefile
modules.file = client/modules/modules.pro
modules.makefile = Makefile

message(Search for $${QaterialLibFilePath})

exists($${QaterialLibFilePath}): message(Qaterial library exists, building other subdirs)
else: message(Qaterial libray doesn\'t exists)

SUBDIRS += client_lib

exists($${QaterialLibFilePath}) {
	SUBDIRS += \
			version \
			modules \
			application

	linux|win32|mac:!android:!ios {
		if($$CreateBundle):	SUBDIRS += bundle-client
	}

}

CONFIG += ordered

