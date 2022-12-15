TEMPLATE = app
TARGET = callofsuli

QT += gui quick svg xml network #sql websockets quick svg multimedia network networkauth gui-private webview

CONFIG += c++17

include(../../common.pri)
include(../../version/version.pri)

android {
	isEmpty(AndroidNdkPath): error(AndroidNdkPath empty)
	isEmpty(AndroidSdkPath): error(AndroidSdkPath empty)
}


DESTDIR = ../..

AppRpath =


include(../../lib/import_lib_client.pri)

FullAppRpath = $$join(AppRpath,",","-Wl,")

!isEmpty(FullAppRpath):!wasm  {
	QMAKE_LFLAGS += $$FullAppRpath
}


!wasm {
	DEFINES += WITH_BOX2D
	SOURCES += desktopapplication.cpp
	HEADERS += desktopapplication.h
}

wasm:include(app_wasm.pri)



CommonRcc.files += $$files($$PWD/../../share/*.cres)

android: CommonRcc.path = /assets
ios: CommonRcc.path = share


!isEmpty(CommonRcc.path)	INSTALLS += CommonRcc


win32 {
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
	RC_ICONS = $$PWD/../../resources/internal/img/cos.ico
	RC_LANG = 0x040E
	QMAKE_TARGET_COPYRIGHT = Valaczka Janos Pal
	QMAKE_TARGET_DESCRIPTION = Call of Suli
}


android {
	QT += androidextras

	DISTFILES += \
		android/build.gradle \
		android/res/drawable/splashscreen.xml \
		android/src/hu/piarista/vjp/callofsuli/ClientActivity.java

	alist.input = $$PWD/../deploy/AndroidManifest.xml.in
	alist.output = $$PWD/android/AndroidManifest.xml
	QMAKE_SUBSTITUTES += alist

	ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

	ANDROID_VERSION_CODE = 32
	ANDROID_VERSION_NAME = $$VERSION

	ANDROID_ABIS = armeabi-v7a

	!CONFIG(debug, debug|release): ANDROID_ABIS += arm64-v8a

	QTDIR = $$dirname(QMAKE_QMAKE)/../qml
}



ios {
	QMAKE_BUNDLE_DATA += CommonRcc

	QMAKE_TARGET_BUNDLE_PREFIX = hu.piarista.vjp

	Q_ENABLE_BITCODE.name = ENABLE_BITCODE
	Q_ENABLE_BITCODE.value = NO
	QMAKE_MAC_XCODE_SETTINGS += Q_ENABLE_BITCODE

	QMAKE_ASSET_CATALOGS += $$PWD/../deploy/Assets.xcassets

	app_launch_screen.files = $$PWD/../deploy/COSLaunchScreen.storyboard $$PWD/../../resources/internal/img/cos.png

	QMAKE_BUNDLE_DATA += app_launch_screen

	OTHER_FILES += $$PWD/../deploy/Info.plist.in

	INFO_PLIST_VERSION = "$$VERSION"

	plist.input = $$PWD/../deploy/Info.plist.in
	plist.output = $$OUT_PWD/../Info.plist
	QMAKE_SUBSTITUTES += plist

	QMAKE_INFO_PLIST = $$DESTDIR/../Info.plist


}




SOURCES += \
	abstractgame.cpp \
	application.cpp \
	client.cpp \
	main.cpp \
	utils.cpp

RESOURCES += \
	../qml/qml.qrc \
	../qml/QaterialHelper.qrc


HEADERS += \
	../../version/version.h \
	abstractgame.h \
	application.h \
	client.h \
	utils.h
