TEMPLATE = app
TARGET = callofsuli

QT += gui quick svg xml network gui-private quickcontrols2 charts websockets core5compat pdf

CONFIG += c++2a
CONFIG += separate_debug_info
CONFIG += permissions

include(../../common.pri)
include(../../version/version.pri)
include(../../lib/callofsuli-core/callofsuli-core.pri)
include(../../translations/translations.pri)


DESTDIR = ../..

QML_IMPORT_PATH += $$PWD/../qml
QML2_IMPORT_PATH += $$PWD/../qml
QMLPATHS += $$PWD/../qml


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

include(app_sign.pri)


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

	equals(QT_ARCH, armeabi-v7a)|equals(ANDROID_ABIS, armeabi-v7a): androidCodeApi = 100000
	else:equals(QT_ARCH, arm64-v8a): androidCodeApi = 200000
	else:equals(QT_ARCH, x86): androidCodeApi = 300000
	else:equals(QT_ARCH, x86_64): androidCodeApi = 400000


	QT += concurrent

	DISTFILES += \
		android/res/drawable/splashscreen.xml \
		android/src/hu/piarista/vjp/callofsuli/ClientActivity.java

	alist.input = $$PWD/../deploy/AndroidManifest.xml.in
	alist.output = $$PWD/android/AndroidManifest.xml

	versionAtLeast(QT_VERSION, 6.8) {
		ANDROID_MIN_SDK_VERSION = 28
		androidCodeApi = $$num_add($$androidCodeApi,28000)
	} else {
		ANDROID_MIN_SDK_VERSION = 26
		androidCodeApi = $$num_add($$androidCodeApi,26000)
	}

	androidCodeApi = $$num_add($$androidCodeApi,$$AndroidVersionCode)

	ANDROID_TARGET_SDK_VERSION = 34
	ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

	QMAKE_SUBSTITUTES += alist

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

	Q_ENTITLEMENTS.name = CODE_SIGN_ENTITLEMENTS
	Q_ENTITLEMENTS.value = $$PWD/../deploy/callofsuli.entitlements
	QMAKE_MAC_XCODE_SETTINGS += Q_ENTITLEMENTS

	versionAtLeast(QT_VERSION, 6.8) {
		QMAKE_IOS_DEPLOYMENT_TARGET = 16.0
	} else {
		QMAKE_IOS_DEPLOYMENT_TARGET = 14.0
	}

	LIBS += \
		-framework AudioToolBox \
		-framework AVFoundation

	QMAKE_APPLE_TARGETED_DEVICE_FAMILY = 1,2

	QMAKE_ASSET_CATALOGS += $$PWD/../deploy/Assets.xcassets

	app_launch_screen.files = $$PWD/../deploy/COSLaunchScreen.storyboard $$PWD/../../resources/internal/img/cos.png

	QMAKE_BUNDLE_DATA += app_launch_screen

	OTHER_FILES += $$PWD/../deploy/Info.plist.in

	INFO_PLIST_VERSION = "$$VERSION"
	INFO_PLIST_SHORT_VERSION = "$$VERSION"

	plist.input = $$PWD/../deploy/Info.plist.in
	plist.output = $$OUT_PWD/Info.plist
	QMAKE_SUBSTITUTES += plist

	QMAKE_INFO_PLIST = $$OUT_PWD/Info.plist


	app_privacy_declarations.files = $$PWD/../deploy/PrivacyInfo.xcprivacy
	QMAKE_BUNDLE_DATA += app_privacy_declarations

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
	actionrpggame.cpp \
	application.cpp \
	basemap.cpp \
	basemaphandler.cpp \
	campaign.cpp \
	classobject.cpp \
	client.cpp \
	clientcache.cpp \
	conquestgame.cpp \
	conquestgameadjacencysetup.cpp \
	conquestland.cpp \
	conquestlanddata.cpp \
	downloader.cpp \
	editorundostack.cpp \
	exam.cpp \
	examgame.cpp \
	examresultmodel.cpp \
	fetchmodel.cpp \
	fontimage.cpp \
	gamequestion.cpp \
	gamequestioncomponent.cpp \
	grade.cpp \
	httpconnection.cpp \
	isometricbullet.cpp \
	isometricenemy.cpp \
	isometricentity.cpp \
	isometricobject.cpp \
	isometricplayer.cpp \
	litegame.cpp \
	main.cpp \
	mapeditor.cpp \
	mapeditormap.cpp \
	mapgame.cpp \
	mapimage.cpp \
	mapplay.cpp \
	mapplaycampaign.cpp \
	mapplaydemo.cpp \
	maskedmousearea.cpp \
	offsetmodel.cpp \
	question.cpp \
	rpgarmory.cpp \
	rpgarrow.cpp \
	rpgaxe.cpp \
	rpgbroadsword.cpp \
	rpgcoin.cpp \
	rpgcontrolgroup.cpp \
	rpgcontrolgroupcontainer.cpp \
	rpgcontrolgroupdoor.cpp \
	rpgcontrolgroupoverlay.cpp \
	rpgcontrolgroupsave.cpp \
	rpgdagger.cpp \
	rpgenemybase.cpp \
	rpgfireball.cpp \
	rpgfirefog.cpp \
	rpggame.cpp \
	rpghammer.cpp \
	rpghp.cpp \
	rpgkeypickable.cpp \
	rpglightning.cpp \
	rpglongbow.cpp \
	rpglongsword.cpp \
	rpgmace.cpp \
	rpgmagestaff.cpp \
	rpgmp.cpp \
	rpgpickableobject.cpp \
	rpgplayer.cpp \
	rpgquestion.cpp \
	rpgshield.cpp \
	rpgshortbow.cpp \
	rpgtimepickable.cpp \
	rpguserwallet.cpp \
	rpgwerebear.cpp \
	rpgworldlanddata.cpp \
	scorelist.cpp \
	server.cpp \
	sound.cpp \
	stb_helper.cpp \
	studentgroup.cpp \
	studentmap.cpp \
	studentmaphandler.cpp \
	task.cpp \
	teacherexam.cpp \
	teachergroup.cpp \
	teachermap.cpp \
	teachermaphandler.cpp \
	testgame.cpp \
	tiledcontainer.cpp \
	tiledeffect.cpp \
	tiledfixpositionmotor.cpp \
	tiledgame.cpp \
	tiledgamesfx.cpp \
	tiledobject.cpp \
	tiledpathmotor.cpp \
	tiledreturnpathmotor.cpp \
	tiledscene.cpp \
	tiledspritehandler.cpp \
	tiledtransport.cpp \
	tiledvisualitem.cpp \
	tiledweapon.cpp \
	updater.cpp \
	user.cpp \
	userimporter.cpp \
	userloglist.cpp \
	websocket.cpp

RESOURCES += \
	../qml/qml.qrc \
	../qml/QaterialHelper.qrc

HEADERS += \
	../../version/version.h \
	abstractgame.h \
	abstractlevelgame.h \
	abstracttiledmotor.h \
	actionrpggame.h \
	application.h \
	basemap.h \
	basemaphandler.h \
	campaign.h \
	classobject.h \
	client.h \
	clientcache.h \
	conquestgame.h \
	conquestgameadjacencysetup.h \
	conquestland.h \
	conquestlanddata.h \
	downloader.h \
	editorundostack.h \
	exam.h \
	examgame.h \
	examresultmodel.h \
	fetchmodel.h \
	fontimage.h \
	gamequestion.h \
	gamequestioncomponent.h \
	grade.h \
	httpconnection.h \
	isometricbullet.h \
	isometricbullet_p.h \
	isometricenemy.h \
	isometricentity.h \
	isometricobject.h \
	isometricobjectiface.h \
	isometricplayer.h \
	litegame.h \
	mapeditor.h \
	mapeditormap.h \
	mapgame.h \
	mapimage.h \
	mapplay.h \
	mapplaycampaign.h \
	mapplaydemo.h \
	maskedmousearea.h \
	offsetmodel.h \
	question.h \
	rpgarmory.h \
	rpgarrow.h \
	rpgaxe.h \
	rpgbroadsword.h \
	rpgcoin.h \
	rpgcontrolgroup.h \
	rpgcontrolgroupcontainer.h \
	rpgcontrolgroupdoor.h \
	rpgcontrolgroupoverlay.h \
	rpgcontrolgroupsave.h \
	rpgcontrolgroupstate.h \
	rpgdagger.h \
	rpgenemybase.h \
	rpgenemyiface.h \
	rpgfireball.h \
	rpgfirefog.h \
	rpggame.h \
	rpghammer.h \
	rpghp.h \
	rpgkeypickable.h \
	rpglightning.h \
	rpglongbow.h \
	rpglongsword.h \
	rpgmace.h \
	rpgmagestaff.h \
	rpgmp.h \
	rpgpickableobject.h \
	rpgplayer.h \
	rpgquestion.h \
	rpgshield.h \
	rpgshortbow.h \
	rpgtimepickable.h \
	rpguserwallet.h \
	rpgwerebear.h \
	rpgworldlanddata.h \
	rpgworldlanddata_p.h \
	scorelist.h \
	server.h \
	sound.h \
	studentgroup.h \
	studentmap.h \
	studentmaphandler.h \
	task.h \
	teacherexam.h \
	teachergroup.h \
	teachermap.h \
	teachermaphandler.h \
	testgame.h \
	tiledcontainer.h \
	tiledeffect.h \
	tiledfixpositionmotor.h \
	tiledgame.h \
	tiledgamesfx.h \
	tiledobject.h \
	tiledobjectspritedef.h \
	tiledpathmotor.h \
	tiledpickableiface.h \
	tiledreturnpathmotor.h \
	tiledscene.h \
	tiledspritehandler.h \
	tiledtransport.h \
	tiledvisualitem.h \
	tiledweapon.h \
	updater.h \
	user.h \
	userimporter.h \
	userloglist.h \
	websocket.h

DISTFILES += \
	translation.pri

