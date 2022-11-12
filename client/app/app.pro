QT += sql websockets quick svg multimedia network networkauth gui-private webview

CONFIG += c++17

include(../../version/version.pri)
include(../../common/common.pri)
include(../SortFilterProxyModel/SortFilterProxyModel.pri)
include(../QObjectListModel/qobjectlistmodel.pri)
android: include(../android_openssl/openssl.pri)
linux|win32|mac: include(../QSingleInstance/QSingleInstance.pri)


TEMPLATE = app
TARGET = callofsuli
win32: CONFIG += windows

INCLUDEPATH += ../QtXlsxWriter \
				../QZXing \

INCLUDEPATH += ../Bacon2D-static/src
INCLUDEPATH += ../Bacon2D-static/src/tmx
INCLUDEPATH += ../Bacon2D-static/qml-box2d
INCLUDEPATH += ../Bacon2D-static/qml-box2d/Box2D
INCLUDEPATH += ../Bacon2D-static/tiled/
INCLUDEPATH += ../Bacon2D-static/tiled/libtiled

DEFINES += TILED_LIBRARY

linux|win32|mac: DESTDIR = ../..

INSTALL_DIR = $${OUT_PWD}/$${DESTDIR}

win32: LIBS += -L../QtXlsxWriter/release -lqtxlsx -L../QZXing/release -lQZXing -L../Bacon2D-static/release -lbacon2d
else:android: LIBS += -L../QtXlsxWriter -lqtxlsx_$${QT_ARCH} -L../QZXing -lQZXing_$${QT_ARCH} -L../Bacon2D-static -lbacon2d_$${QT_ARCH}
else: LIBS += -L../QtXlsxWriter -lqtxlsx -L../QZXing -lQZXing -L../Bacon2D-static -lbacon2d

LIBS += -lz

linux|win32|mac:!ios:!android: QMAKE_LFLAGS += -Wl,--rpath=../lib,--rpath=../QtXlsxWriter,--rpath=../QZXing,--rpath=../Bacon2D-static



linux:!android: QMAKE_CXXFLAGS += -Wno-deprecated-copy -Wno-deprecated-declarations
android: QMAKE_CXXFLAGS += -Wno-deprecated
win32: QMAKE_CXX += -Wno-strict-aliasing -Wno-deprecated-declarations




include(modules/modules.pri)

LIBS += -L./modules

for(m, MODULES_LIST) {
	android: LIBS += -l$${m}_$${QT_ARCH}
	else: LIBS += -l$${m}
}




QML_IMPORT_PATH += $$PWD/../Bacon2D-static/src/
QML_DESIGNER_IMPORT_PATH += $$PWD/../Bacon2D-static/src/
QML2_IMPORT_PATH += $$PWD/../Bacon2D-static/src/

QML_IMPORT_PATH += $$PWD/modules/fillout

win32 {
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
	RC_ICONS = $$PWD/../../resources/internal/img/cos.ico
	RC_LANG = 0x040E
	QMAKE_TARGET_COPYRIGHT = Valaczka Janos Pal
	QMAKE_TARGET_DESCRIPTION = Call of Suli kliens
}


android {
	QT += androidextras

DISTFILES += \
		android/build.gradle \
		android/res/drawable/splashscreen.xml \
		android/src/hu/piarista/vjp/callofsuli/ClientActivity.java


	INFO_PLIST_VERSION = "$$VERSION"

	alist.input = $$PWD/../deploy/AndroidManifest.xml.in
	alist.output = $$PWD/android/AndroidManifest.xml
	QMAKE_SUBSTITUTES += alist


	ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

	ANDROID_ABIS = armeabi-v7a

	!CONFIG(debug, debug|release): ANDROID_ABIS += arm64-v8a

	for (abi, ANDROID_ABIS): ANDROID_EXTRA_LIBS += \
		$$OUT_PWD/../QtXlsxWriter/libqtxlsx_$${abi}.so \
		$$OUT_PWD/../QZXing/libQZXing_$${abi}.so
}


win32: target.path = $${INSTALL_DIR}/build
else:linux:!android: target.path = $${INSTALL_DIR}/build/bin

CommonRcc.files += $$files($$PWD/../../share/*.cres)

android: CommonRcc.path = /assets
else:ios: CommonRcc.path = share
else: CommonRcc.path = $${INSTALL_DIR}/build/share

!isEmpty(target.path): INSTALLS += target
!isEmpty(CommonRcc.path): INSTALLS += CommonRcc

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

	QMAKE_INFO_PLIST = $$OUT_PWD/../Info.plist


}

win32 {
	license.path = $${INSTALL_DIR}/build
	license.files = $$PWD/../../LICENSE

	INSTALLS += license

	include(innosetup.pri)
}



#CONFIG(debug, debug|release) {
#	DEFINES += SQL_DEBUG
#}

SOURCES += \
		abstractactivity.cpp \
		abstractobjectiveimporter.cpp \
		androidshareutils.cpp \
		chapterimporter.cpp \
		cosclient.cpp \
		cosdownloader.cpp \
		cosgame.cpp \
		cossound.cpp \
		editoraction.cpp \
		editorundostack.cpp \
		fontimage.cpp \
		gameactivity.cpp \
		gameblock.cpp \
		gameenemy.cpp \
		gameenemydata.cpp \
		gameenemysniper.cpp \
		gameenemysoldier.cpp \
		gameentity.cpp \
		gameladder.cpp \
		gamemapeditor.cpp \
		gamematch.cpp \
		gameobject.cpp \
		gamepickable.cpp \
		gameplayer.cpp \
		gamequestion.cpp \
		gamescene.cpp \
		gameterrain.cpp \
		googleoauth2.cpp \
		main.cpp \
		mapeditor.cpp \
		mapeditoraction.cpp \
		mapimage.cpp \
		maplistobject.cpp \
		objectlistmodel.cpp \
		objectlistmodelobject.cpp \
		profile.cpp \
		profilescoreobject.cpp \
		qrimage.cpp \
		question.cpp \
		serverobject.cpp \
		servers.cpp \
		serversettings.cpp \
		sqlimage.cpp \
		studentmaps.cpp \
		teachergroups.cpp \
		teachermaps.cpp \
		tiledpaintedlayer.cpp \
		userlistobject.cpp


HEADERS += \
	abstractactivity.h \
	abstractobjectiveimporter.h \
	androidshareutils.h \
	chapterimporter.h \
	cosclient.h \
	cosdownloader.h \
	cosgame.h \
	cossound.h \
	editoraction.h \
	editorundostack.h \
	fontimage.h \
	gameactivity.h \
	gameblock.h \
	gameenemy.h \
	gameenemydata.h \
	gameenemysniper.h \
	gameenemysoldier.h \
	gameentity.h \
	gameladder.h \
	gamemapeditor.h \
	gamematch.h \
	gameobject.h \
	gamepickable.h \
	gameplayer.h \
	gamequestion.h \
	gamescene.h \
	gameterrain.h \
	googleoauth2.h \
	mapeditor.h \
	mapeditoraction.h \
	mapimage.h \
	maplistobject.h \
	modules/interfaces.h \
	objectlistmodel.h \
	objectlistmodelobject.h \
	profile.h \
	profilescoreobject.h \
	qrimage.h \
	question.h \
	serverobject.h \
	servers.h \
	serversettings.h \
	sqlimage.h \
	studentmaps.h \
	teachergroups.h \
	teachermaps.h \
	tiledpaintedlayer.h \
	userlistobject.h

RESOURCES += \
	qml/qml.qrc

