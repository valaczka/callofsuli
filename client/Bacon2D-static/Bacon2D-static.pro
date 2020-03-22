# Include this file in your .pro file to statically compile the Bacon2D QML
# plugin into your project.
#
# Basic usage instructions:
#
#  #include <plugins.h>
#
#  int main(int argc, char *argv[])
#  {
#      QApplication app(argc, argv);
#
#      Plugin plugin;
#      plugin.registerTypes("Bacon2D");
#
#      ...
#  }

TEMPLATE = lib
CONFIG += static plugin

android: TARGET = bacon2d_$${QT_ARCH}
else: TARGET = bacon2d

QT += quick

CONFIG += c++17

linux:!android: QMAKE_CXXFLAGS += -Wno-deprecated-copy -Wno-deprecated-declarations
android: QMAKE_CXXFLAGS += -Wno-deprecated
win32: QMAKE_CXX += -Wno-strict-aliasing -Wno-deprecated-declarations

LIBS += -lz

INCLUDEPATH += $$PWD

INCLUDEPATH += $$PWD/qml-box2d/
INCLUDEPATH += $$PWD/tiled/

include($$PWD/qml-box2d/box2d-static.pri)
include($$PWD/tiled/libtiled/libtiled-static.pri)
include($$PWD/src/tmx/tmx.pri)
include($$PWD/src/Bacon2D-static.pri)
