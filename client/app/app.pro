QT += sql websockets quick svg multimedia

CONFIG += c++11

include(../../version/version.pri)
include(../../common/common.pri)
include(../SortFilterProxyModel/SortFilterProxyModel.pri)
include(../Bacon2D-static/src/Bacon2D-static.pri)
android: include(../android_openssl/openssl.pri)
!android: include(../QSingleInstance/QSingleInstance.pri)
unix:!android: include(graphviz.pri)


TEMPLATE = app
TARGET = callofsuli

LIBS += -lz

INCLUDEPATH += ../QtXlsxWriter \
				../QZXing

DESTDIR = ../..

INSTALL_DIR = $${OUT_PWD}/$${DESTDIR}

win32: LIBS += -L../QtXlsxWriter/release -lqtxlsx -L../QZXing/release -lQZXing
else:android: LIBS += -L../QtXlsxWriter -lqtxlsx_$${QT_ARCH} -L../QZXing -lQZXing_$${QT_ARCH}
else: LIBS += -L../QtXlsxWriter -lqtxlsx -L../QZXing -lQZXing

!android: QMAKE_LFLAGS += -Wl,--rpath=../lib,--rpath=../QtXlsxWriter,--rpath=../QZXing

unix:!android: QMAKE_CXXFLAGS += -Wno-deprecated-copy
win32: QMAKE_CXX += -Wno-strict-aliasing



include(modules/modules.pri)

equals(MODULE_BUILD, static) {
	LIBS += -L./modules

	for(m, MODULES_LIST) {
		android: LIBS += -l$${m}_$${QT_ARCH}
		else: LIBS += -l$${m}
	}
}



QML_IMPORT_PATH += $$PWD/../Bacon2D-static/src/
QML_DESIGNER_IMPORT_PATH += $$PWD/../Bacon2D-static/src/
QML2_IMPORT_PATH += $$PWD/../Bacon2D-static/src/


win32 {
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
	RC_ICONS = $$PWD/../../resources/internal/img/cos96.ico
}


android {
	QT += androidextras

DISTFILES += \
		android/AndroidManifest.xml \
		android/build.gradle \
		android/res/drawable/splashscreen.xml \
		android/src/hu/piarista/vjp/callofsuli/ClientActivity.java

	ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

	ANDROID_ABIS = armeabi-v7a

	!CONFIG(debug, debug|release): ANDROID_ABIS += arm64-v8a
}


win32: target.path = $${INSTALL_DIR}/build
else: !android: target.path = $${INSTALL_DIR}/build/bin

CommonRcc.files += $$PWD/../../share/*.cres

android: CommonRcc.path = /assets
else: CommonRcc.path = $${INSTALL_DIR}/build/share

!isEmpty(target.path): INSTALLS += target
!isEmpty(CommonRcc.path): INSTALLS += CommonRcc


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
		fontimage.cpp \
		gameactivity.cpp \
		gameblock.cpp \
		gameenemy.cpp \
		gameenemydata.cpp \
		gameenemysoldier.cpp \
		gameentity.cpp \
		gameladder.cpp \
		gamemapmodel.cpp \
		gamematch.cpp \
		gameobject.cpp \
		gamepickable.cpp \
		gameplayer.cpp \
		gamequestion.cpp \
		gamescene.cpp \
		gameterrain.cpp \
		main.cpp \
		mapeditor.cpp \
		profile.cpp \
		qrimage.cpp \
		question.cpp \
		scores.cpp \
		servers.cpp \
		serversettings.cpp \
		sqlimage.cpp \
		studentmaps.cpp \
		teachergroups.cpp \
		teachermaps.cpp \
		tiledpaintedlayer.cpp \
		variantmapdata.cpp \
		variantmapmodel.cpp


HEADERS += \
	abstractactivity.h \
	abstractobjectiveimporter.h \
	androidshareutils.h \
	chapterimporter.h \
	cosclient.h \
	cosdownloader.h \
	cosgame.h \
	cossound.h \
	fontimage.h \
	gameactivity.h \
	gameblock.h \
	gameenemy.h \
	gameenemydata.h \
	gameenemysoldier.h \
	gameentity.h \
	gameladder.h \
	gamemapmodel.h \
	gamematch.h \
	gameobject.h \
	gamepickable.h \
	gameplayer.h \
	gamequestion.h \
	gamescene.h \
	gameterrain.h \
	mapeditor.h \
	modules/interfaces.h \
	profile.h \
	qrimage.h \
	question.h \
	scores.h \
	servers.h \
	serversettings.h \
	sqlimage.h \
	studentmaps.h \
	teachergroups.h \
	teachermaps.h \
	tiledpaintedlayer.h \
	variantmapdata.h \
	variantmapmodel.h

RESOURCES += \
	qml/qml.qrc

DISTFILES += \
	innosetup.pri

