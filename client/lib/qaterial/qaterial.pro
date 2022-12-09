TEMPLATE = aux

include(../../../common.pri)

QTDIR = $$dirname(QMAKE_QMAKE)/..

BUILDSHARED = ON
BUILDCONFIG = Release

wasm: BUILDPCH = OFF
else: BUILDPCH = ON

nproc = $$system("nproc")

PROC = -j$${nproc}

if ($$CMakeVerboseOutput) {
	PROC = -j$${nproc} --verbose
}


extralib.commands = echo "Building Qaterial..."; \
				CMAKE_PREFIX_PATH="$$QTDIR" \
				cmake $$PWD/Qaterial-1.4.6 \
				-D CMAKE_CXX_COMPILER=$$QMAKE_CXX \
				-DQATERIAL_ICONS="*.svg" \
				-DQATERIAL_ENABLE_ROBOTO=OFF -DQATERIAL_ENABLE_ROBOTOMONO=OFF -DQATERIAL_ENABLE_LATO=OFF \
				-DQATERIAL_BUILD_SHARED=$$BUILDSHARED -DQATERIAL_ENABLE_TESTS=OFF -DQATERIAL_ENABLE_TESTS=OFF \
				-DQATERIAL_ENABLE_PCH=$$BUILDPCH \
				-DQATERIAL_ENABLE_HIGHDPIFIX=OFF -DQATERIAL_ENABLE_INSTALL=ON && \
				cmake --build . --target QaterialComponents --config $$BUILDCONFIG $$PROC && \
				cmake --build . --target QaterialIcons --config $$BUILDCONFIG $$PROC && \
				cmake --build . --target Qaterial --config $$BUILDCONFIG $$PROC

extralib.depends =

win32: extralib.target = libQaterial.dll
else: extralib.target = libQaterial.so

QMAKE_EXTRA_TARGETS += extralib
PRE_TARGETDEPS = $$extralib.target


