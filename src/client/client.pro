QT += sql websockets quick multimedia
CONFIG += skip_version

CONFIG += c++11

include(../version/version.pro)
include(../common/common.pri)
include(../3rdparty/SortFilterProxyModel/SortFilterProxyModel.pri)


TEMPLATE = app
TARGET = callofsuli

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
		abstractactivity.cpp \
		abstractdbactivity.cpp \
		adminusers.cpp \
		cosclient.cpp \
		fontimage.cpp \
		main.cpp \
		map.cpp \
		servers.cpp \
		sqlimage.cpp \
		teachermaps.cpp

HEADERS += \
	abstractactivity.h \
	abstractdbactivity.h \
	adminusers.h \
	cosclient.h \
	fontimage.h \
	map.h \
	servers.h \
	sqlimage.h \
	teachermaps.h

RESOURCES += \
	qml/qml.qrc \
	sql.qrc


CONFIG(release, debug|release) {
	DEFINES += QT_NO_DEBUG_OUTPUT
}

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
