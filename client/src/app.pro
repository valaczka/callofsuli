TEMPLATE = app
TARGET = callofsuli

QT += gui quick svg xml network gui-private quickcontrols2 charts multimedia websockets

CONFIG += c++20
CONFIG += separate_debug_info
CONFIG += permissions

include(../../common.pri)
include(../../version/version.pri)
include(../../lib/callofsuli-core/callofsuli-core.pri)
include(../../translations/translations.pri)


DESTDIR = ../..

lessThan(QT_MAJOR_VERSION, 6) {
	QML_IMPORT_PATH += $$PWD/../qml
	QMLPATHS += $$PWD/../qml
} else {
	QML_IMPORT_PATH += $$PWD/../qml-Qt6
	QMLPATHS += $$PWD/../qml-Qt6
}

DEFINES += CLIENT_UTILS

include(../../lib/import_lib_client.pri)

!android:if(linux|win32){
	QMAKE_LFLAGS += \
		"-Wl,--rpath,'$${LITERAL_DOLLAR}$${LITERAL_DOLLAR}ORIGIN'" \
		"-Wl,--rpath,'$${LITERAL_DOLLAR}$${LITERAL_DOLLAR}ORIGIN/lib'"
}

!wasm {
	SOURCES += \
		mobileapplication.cpp \
		standaloneclient.cpp

	HEADERS += \
		mobileapplication.h \
		standaloneclient.h

	!android:!ios {
		SOURCES += desktopapplication.cpp
		HEADERS += desktopapplication.h
	}
}

wasm:include(app_wasm.pri)


##### Modules

include(../modules/modules.pri)

LIBS += -L../modules

for(m, MODULES_LIST) {
		android: LIBS += -l$${m}_$${QT_ARCH}
		else: LIBS += -l$${m}
}


##### Assets

CommonRcc.files += $$files($$PWD/../../share/*.cres)

android: CommonRcc.path = /assets
ios: CommonRcc.path = share
macx: CommonRcc.path = Contents/Resources

!isEmpty(CommonRcc.path)	INSTALLS += CommonRcc



################ PLATFORM SPECIFIC SETTINGS ######################x

win32 {
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
	RC_ICONS = $$PWD/../../resources/internal/img/cos.ico
	RC_LANG = 0x040E
	QMAKE_TARGET_COPYRIGHT = Valaczka Janos Pal
	QMAKE_TARGET_DESCRIPTION = Call of Suli
}


android {
	androidCodeApi = 0

	lessThan(QT_MAJOR_VERSION, 6) {
		QT += androidextras

		DISTFILES += \
			android/res/drawable/splashscreen.xml \
			android/src/hu/piarista/vjp/callofsuli/ClientActivity.java

		alist.input = $$PWD/../deploy/AndroidManifest.xml.in
		alist.output = $$PWD/android/AndroidManifest.xml

		ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
	} else {
		QT += concurrent

		DISTFILES += \
			android-Qt6/res/drawable/splashscreen.xml \
			android-Qt6/src/hu/piarista/vjp/callofsuli/ClientActivity.java

		alist.input = $$PWD/../deploy/Qt6/AndroidManifest.xml.in
		alist.output = $$PWD/android-Qt6/AndroidManifest.xml

		ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android-Qt6

		equals(QT_ARCH, armeabi-v7a): androidCodeApi = 100000
		else:equals(QT_ARCH, arm64-v8a): androidCodeApi = 200000
		else:equals(QT_ARCH, x86): androidCodeApi = 300000
		else:equals(QT_ARCH, x86_64): androidCodeApi = 400000

		androidCodeApi = $$num_add($$androidCodeApi,26000)
	}

	androidCodeApi = $$num_add($$androidCodeApi,$$AndroidVersionCode)


	QMAKE_SUBSTITUTES += alist

	ANDROID_TARGET_SDK_VERSION = 33

	ANDROID_VERSION_CODE = $${androidCodeApi}
	ANDROID_VERSION_NAME = $$VERSION

	QTDIR = $$dirname(QMAKE_QMAKE)/../qml

}



ios {
	QMAKE_BUNDLE_DATA += CommonRcc

	QMAKE_TARGET_BUNDLE_PREFIX = hu.piarista.vjp

	Q_ENABLE_BITCODE.name = ENABLE_BITCODE
	Q_ENABLE_BITCODE.value = NO
	QMAKE_MAC_XCODE_SETTINGS += Q_ENABLE_BITCODE

	lessThan(QT_MAJOR_VERSION, 6): QMAKE_IOS_DEPLOYMENT_TARGET = 12.0
	else: QMAKE_IOS_DEPLOYMENT_TARGET = 14.0

	QMAKE_APPLE_TARGETED_DEVICE_FAMILY = 1,2

	QMAKE_ASSET_CATALOGS += $$PWD/../deploy/Assets.xcassets

	app_launch_screen.files = $$PWD/../deploy/COSLaunchScreen.storyboard $$PWD/../../resources/internal/img/cos.png

	QMAKE_BUNDLE_DATA += app_launch_screen

	OTHER_FILES += $$PWD/../deploy/Info.plist.in

	INFO_PLIST_VERSION = "$$VERSION"
	#INFO_PLIST_SHORT_VERSION = "$${VER_MAJ}.$${VER_MIN}"
	INFO_PLIST_SHORT_VERSION = "$$VERSION"

	plist.input = $$PWD/../deploy/Info.plist.in
	plist.output = $$OUT_PWD/Info.plist
	QMAKE_SUBSTITUTES += plist

	QMAKE_INFO_PLIST = $$OUT_PWD/Info.plist

	SOURCES += sound_helper.mm
}



macx {
	CONFIG += app_bundle

	QMAKE_BUNDLE_DATA += CommonRcc
	ICON = $$PWD/../deploy/CallOfSuli.icns
	QMAKE_TARGET_BUNDLE_PREFIX = hu.piarista.vjp

	SOURCES += sound_helper.mm
}


######## SOURCES ###############



SOURCES += \
	abstractgame.cpp \
	abstractlevelgame.cpp \
	actiongame.cpp \
	application.cpp \
	basemap.cpp \
	basemaphandler.cpp \
	campaign.cpp \
	classobject.cpp \
	client.cpp \
	clientcache.cpp \
	editorundostack.cpp \
	examgame.cpp \
	fetchmodel.cpp \
	fontimage.cpp \
	gameenemy.cpp \
	gameenemysniper.cpp \
	gameenemysoldier.cpp \
	gameentity.cpp \
	gameladder.cpp \
	gameobject.cpp \
	gamepickable.cpp \
	gameplayer.cpp \
	gameplayermulti.cpp \
	gameplayerposition.cpp \
	gamequestion.cpp \
	gamequestioncomponent.cpp \
	gamescene.cpp \
	gameterrain.cpp \
	gameterrainmap.cpp \
	grade.cpp \
	httpconnection.cpp \
	litegame.cpp \
	main.cpp \
	mapeditor.cpp \
	mapeditormap.cpp \
	mapgame.cpp \
	mapimage.cpp \
	mapplay.cpp \
	mapplaycampaign.cpp \
	mapplaydemo.cpp \
	multiplayergame.cpp \
	offsetmodel.cpp \
	qrimage.cpp \
	question.cpp \
	scorelist.cpp \
	server.cpp \
	sound.cpp \
	studentgroup.cpp \
	studentmap.cpp \
	studentmaphandler.cpp \
	task.cpp \
	teachergroup.cpp \
	teachermap.cpp \
	teachermaphandler.cpp \
	testgame.cpp \
	updater.cpp \
	user.cpp \
	userimporter.cpp \
	userloglist.cpp \
	websocket.cpp

lessThan(QT_MAJOR_VERSION, 6): {
	RESOURCES += \
		../qml/qml.qrc \
		../qml/QaterialHelper.qrc
} else {
	RESOURCES += \
		../qml-Qt6/qml.qrc \
		../qml-Qt6/QaterialHelper.qrc
}


HEADERS += \
	../../version/version.h \
	abstractgame.h \
	abstractlevelgame.h \
	actiongame.h \
	application.h \
	basemap.h \
	basemaphandler.h \
	campaign.h \
	classobject.h \
	client.h \
	clientcache.h \
	editorundostack.h \
	examgame.h \
	fetchmodel.h \
	fontimage.h \
	gameenemy.h \
	gameenemysniper.h \
	gameenemysoldier.h \
	gameentity.h \
	gameladder.h \
	gameobject.h \
	gamepickable.h \
	gameplayer.h \
	gameplayermulti.h \
	gameplayerposition.h \
	gamequestion.h \
	gamequestioncomponent.h \
	gamescene.h \
	gameterrain.h \
	gameterrainmap.h \
	grade.h \
	httpconnection.h \
	litegame.h \
	mapeditor.h \
	mapeditormap.h \
	mapgame.h \
	mapimage.h \
	mapplay.h \
	mapplaycampaign.h \
	mapplaydemo.h \
	multiplayergame.h \
	offsetmodel.h \
	qrimage.h \
	question.h \
	scorelist.h \
	server.h \
	sound.h \
	studentgroup.h \
	studentmap.h \
	studentmaphandler.h \
	task.h \
	teachergroup.h \
	teachermap.h \
	teachermaphandler.h \
	testgame.h \
	updater.h \
	user.h \
	userimporter.h \
	userloglist.h \
	websocket.h

DISTFILES += \
	translation.pri


