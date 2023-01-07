TEMPLATE = aux

include(../../common.pri)
include(../cmake.pri)

BUILDCONFIG = Release

wasm: BUILDPCH = OFF
else: BUILDPCH = ON

wasm|win32: BUILDUNITY = OFF
else: BUILDUNITY = ON

if ($$QaterialBuildShared): BuildShared = ON
else: BuildShared = OFF

android {
	extralib.commands = echo \"Building Qaterial...\"; \
				for a in $${ANDROID_ABIS}; do \
					test -d $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}a || mkdir $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}a ; \
					cd $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}a ; \
					$${CMakePath} $$PWD/../Qaterial \
					$${CMakeArguments} -DANDROID_ABI:STRING=$${LITERAL_DOLLAR}$${LITERAL_DOLLAR}a \
					-DQATERIAL_ICONS=\"*.svg\" \
					-DQATERIAL_ENABLE_ROBOTO=OFF -DQATERIAL_ENABLE_ROBOTOMONO=OFF -DQATERIAL_ENABLE_LATO=OFF \
					-DQATERIAL_BUILD_SHARED=$$BuildShared -DQATERIAL_ENABLE_TESTS=OFF -DQATERIAL_ENABLE_TESTS=OFF \
					-DQATERIAL_ENABLE_PCH=$$BUILDPCH -DQATERIAL_ENABLE_UNITY_BUILD=$$BUILDUNITY \
					-DQATERIAL_ENABLE_HIGHDPIFIX=OFF -DQATERIAL_ENABLE_INSTALL=OFF && \
					$${CMakePath} --build . --target QaterialComponents --config $$BUILDCONFIG $$CMakeProc && \
					$${CMakePath} --build . --target QaterialIcons --config $$BUILDCONFIG $$CMakeProc && \
					$${CMakePath} --build . --target Qaterial --config $$BUILDCONFIG $$CMakeProc ; \
					cd .. ; \
				done

} else {
	extralib.commands = echo \"Building Qaterial...\"; \
			$${CMakePath} $$PWD/../Qaterial \
			$${CMakeArguments} \
			-DQATERIAL_ICONS=\"*.svg\" \
			-DQATERIAL_ENABLE_ROBOTO=OFF -DQATERIAL_ENABLE_ROBOTOMONO=OFF -DQATERIAL_ENABLE_LATO=OFF \
			-DQATERIAL_BUILD_SHARED=$$BuildShared -DQATERIAL_ENABLE_TESTS=OFF -DQATERIAL_ENABLE_TESTS=OFF \
			-DQATERIAL_ENABLE_PCH=$$BUILDPCH -DQATERIAL_ENABLE_UNITY_BUILD=$$BUILDUNITY \
			-DQATERIAL_ENABLE_HIGHDPIFIX=OFF -DQATERIAL_ENABLE_INSTALL=OFF && \
			$${CMakePath} --build . --target QaterialComponents --config $$BUILDCONFIG $$CMakeProc && \
			$${CMakePath} --build . --target QaterialIcons --config $$BUILDCONFIG $$CMakeProc && \
			$${CMakePath} --build . --target Qaterial --config $$BUILDCONFIG $$CMakeProc

}


extralib.target = libqaterial.file
extralib.depends =



QMAKE_EXTRA_TARGETS += extralib
PRE_TARGETDEPS = $$extralib.target


