include(common.pri)

android:if(isEmpty(AndroidNdkPath)|isEmpty(AndroidSdkPath)) {
	error(Missing AndrodiNdkPath / AndroidSdkPath)
}

TEMPLATE = subdirs

SUBDIRS += \
				version \
				client_lib \
				application


client_lib.file = lib/lib_client.pro
client_lib.makefile = Makefile
application.file = client/src/app.pro
application.makefile = Makefile

CONFIG += ordered

