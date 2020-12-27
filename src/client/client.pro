QT += sql websockets quick svg multimedia

CONFIG += c++11

include(../version/version.pro)
include(../common/common.pri)
include(../3rdparty/SortFilterProxyModel/SortFilterProxyModel.pri)
include(../3rdparty/Bacon2D-static/src/Bacon2D-static.pri)

TEMPLATE = app
TARGET = callofsuli

LIBS += -lz

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


#DEFINES += COS_SQL_DEBUG

SOURCES += \
		abstractactivity.cpp \
		cosclient.cpp \
		cosdownloader.cpp \
		cosgame.cpp \
		fontimage.cpp \
		gameactivity.cpp \
		gameblock.cpp \
		gameenemy.cpp \
		gameenemydata.cpp \
		gameenemysoldier.cpp \
		gameentity.cpp \
		gameladder.cpp \
		gamematch.cpp \
		gameobject.cpp \
		gameplayer.cpp \
		gamequestion.cpp \
		gamescene.cpp \
		gameterrain.cpp \
		main.cpp \
		mapeditor.cpp \
		servers.cpp \
		serversettings.cpp \
		sqlimage.cpp \
		teachermaps.cpp \
		tiledpaintedlayer.cpp \
		variantmapdata.cpp \
		variantmapmodel.cpp


HEADERS += \
	abstractactivity.h \
	cosclient.h \
	cosdownloader.h \
	cosgame.h \
	fontimage.h \
	gameactivity.h \
	gameblock.h \
	gameenemy.h \
	gameenemydata.h \
	gameenemysoldier.h \
	gameentity.h \
	gameladder.h \
	gamematch.h \
	gameobject.h \
	gameplayer.h \
	gamequestion.h \
	gamescene.h \
	gameterrain.h \
	mapeditor.h \
	servers.h \
	serversettings.h \
	sqlimage.h \
	teachermaps.h \
	tiledpaintedlayer.h \
	variantmapdata.h \
	variantmapmodel.h

RESOURCES += \
	qml/qml.qrc


QML_IMPORT_PATH += $$PWD/../3rdparty/Bacon2D-static/src/

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH += $$PWD/../3rdparty/Bacon2D-static/src/

QML2_IMPORT_PATH += $$PWD/../3rdparty/Bacon2D-static/src/

CONFIG(release, debug|release) {
	DEFINES += QT_NO_DEBUG_OUTPUT
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



win32 {
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
#    RC_ICONS = $$PWD/../../share/student.ico
}


android {
#	QT += androidextras

DISTFILES += \
		android/AndroidManifest.xml

	ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

	CommonRcc.path = /assets
	CommonRcc.files += $$PWD/../../share/*.cres

	INSTALLS += CommonRcc
}

ANDROID_ABIS = armeabi-v7a

