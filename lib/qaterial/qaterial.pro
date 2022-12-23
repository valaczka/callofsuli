TEMPLATE = aux

include(../../common.pri)
include(../cmake.pri)

BUILDCONFIG = Release

wasm: BUILDPCH = OFF
else: BUILDPCH = ON


android {
	extralib.commands = echo \"Building Qaterial...\"; \
				for a in $${ANDROID_ABIS}; do \
					test -d $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}a || mkdir $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}a ; \
					cd $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}a ; \
					$${CMakePath} $$PWD/Qaterial-1.4.6 \
					$${CMakeArguments} -DANDROID_ABI:STRING=$${LITERAL_DOLLAR}$${LITERAL_DOLLAR}a \
					-DQATERIAL_ICONS=\"*.svg\" \
					-DQATERIAL_ENABLE_ROBOTO=OFF -DQATERIAL_ENABLE_ROBOTOMONO=OFF -DQATERIAL_ENABLE_LATO=OFF \
					-DQATERIAL_BUILD_SHARED=$$QaterialBuildShared -DQATERIAL_ENABLE_TESTS=OFF -DQATERIAL_ENABLE_TESTS=OFF \
					-DQATERIAL_ENABLE_PCH=$$BUILDPCH \
					-DQATERIAL_ENABLE_HIGHDPIFIX=OFF -DQATERIAL_ENABLE_INSTALL=ON && \
					$${CMakePath} --build . --target QaterialComponents --config $$BUILDCONFIG $$CMakeProc && \
					$${CMakePath} --build . --target QaterialIcons --config $$BUILDCONFIG $$CMakeProc && \
					$${CMakePath} --build . --target Qaterial --config $$BUILDCONFIG $$CMakeProc ; \
					cd .. ; \
				done

} else {
	extralib.commands = echo \"Building Qaterial...\"; \
			$${CMakePath} $$PWD/Qaterial-1.4.6 \
			$${CMakeArguments} \
			-DQATERIAL_ICONS=\"*.svg\" \
			-DQATERIAL_ENABLE_ROBOTO=OFF -DQATERIAL_ENABLE_ROBOTOMONO=OFF -DQATERIAL_ENABLE_LATO=OFF \
			-DQATERIAL_BUILD_SHARED=$$QaterialBuildShared -DQATERIAL_ENABLE_TESTS=OFF -DQATERIAL_ENABLE_TESTS=OFF \
			-DQATERIAL_ENABLE_PCH=$$BUILDPCH \
			-DQATERIAL_ENABLE_HIGHDPIFIX=OFF -DQATERIAL_ENABLE_INSTALL=ON && \
			$${CMakePath} --build . --target QaterialComponents --config $$BUILDCONFIG $$CMakeProc && \
			$${CMakePath} --build . --target QaterialIcons --config $$BUILDCONFIG $$CMakeProc && \
			$${CMakePath} --build . --target Qaterial --config $$BUILDCONFIG $$CMakeProc

}


extralib.target = libqaterial.file
extralib.depends =



QMAKE_EXTRA_TARGETS += extralib
PRE_TARGETDEPS = $$extralib.target


