QT += quick quickcontrols2 svg networkauth

CONFIG += c++17


TEMPLATE = app
TARGET = callofsuli-wasm

QMAKE_CXXFLAGS += -Wno-deprecated-copy -Wno-deprecated-declarations

LIBS += -L../Bacon2D-static -lbacon2d


INCLUDEPATH += ../Bacon2D-static/src
INCLUDEPATH += ../Bacon2D-static/src/tmx
INCLUDEPATH += ../Bacon2D-static/qml-box2d
INCLUDEPATH += ../Bacon2D-static/qml-box2d/Box2D
INCLUDEPATH += ../Bacon2D-static/tiled/
INCLUDEPATH += ../Bacon2D-static/tiled/libtiled

DEFINES += TILED_LIBRARY


QML_IMPORT_PATH += $$PWD/../Bacon2D-static/src/
QML_DESIGNER_IMPORT_PATH += $$PWD/../Bacon2D-static/src/
QML2_IMPORT_PATH += $$PWD/../Bacon2D-static/src/

//QML_IMPORT_PATH += $$PWD/modules/fillout

SOURCES += \
	main.cpp \
	myapp.cpp

RESOURCES += \
	qml.qrc

HEADERS += \
	myapp.h
