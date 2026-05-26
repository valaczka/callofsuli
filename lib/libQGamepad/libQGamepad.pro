TEMPLATE = aux

QTDIR = $$dirname(QMAKESPEC)/../bin

DESTDIR = ../../

CMAKE_DIR=$$OUT_PWD/CMakeOut

!exists($$QTDIR/../modules/GamepadLegacy.json) {
	system("$$QTDIR/qt-cmake -S $$PWD/../qtgamepadlegacy -B $$CMAKE_DIR && cd $$CMAKE_DIR && make -j8 && make install")
}
