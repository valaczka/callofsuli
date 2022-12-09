TEMPLATE = aux

include(../../../common.pri)

QTDIR = $$dirname(QMAKE_QMAKE)/..

BUILDSHARED = OFF
USEQT6 = OFF

nproc = $$system("nproc")

PROC = -j$${nproc}

if ($$CMakeVerboseOutput) {
	PROC = -j$${nproc} --verbose
}


extralib.target = libqmlbox2d.a

extralib.commands = echo "Building qml-box2d..."; \
				CMAKE_PREFIX_PATH="$$QTDIR" \
				cmake $$PWD/qml-box2d \
				-D CMAKE_CXX_COMPILER=$$QMAKE_CXX \
				-DUSE_QT6=$$USEQT6 -DUSE_SYSTEM_BOX2D=OFF -DBUILD_EXAMPLES=OFF -DBUILD_SHARED_LIBS=$$BUILDSHARED && \
				cmake --build . $$PROC

extralib.depends =


QMAKE_EXTRA_TARGETS += extralib
PRE_TARGETDEPS = $$extralib.target


