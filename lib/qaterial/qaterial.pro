TEMPLATE = aux

include(../../common.pri)
include(../clang.pri)

wasm: BUILDSHARED = ON
else: BUILDSHARED = ON

BUILDCONFIG = Release

wasm: BUILDPCH = OFF
else: BUILDPCH = ON


android {
	extralib.commands = echo \"Building Qaterial...\"; \
				for a in $${ANDROID_ABIS}; do \
					test -d $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}a || mkdir $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}a ; \
					cd $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}a ; \
					cmake $$PWD/Qaterial-1.4.6 \
					$${CMakeArguments} -DANDROID_ABI:STRING=$${LITERAL_DOLLAR}$${LITERAL_DOLLAR}a \
					-DQATERIAL_ICONS=\"*.svg\" \
					-DQATERIAL_ENABLE_ROBOTO=OFF -DQATERIAL_ENABLE_ROBOTOMONO=OFF -DQATERIAL_ENABLE_LATO=OFF \
					-DQATERIAL_BUILD_SHARED=$$BUILDSHARED -DQATERIAL_ENABLE_TESTS=OFF -DQATERIAL_ENABLE_TESTS=OFF \
					-DQATERIAL_ENABLE_PCH=$$BUILDPCH \
					-DQATERIAL_ENABLE_HIGHDPIFIX=OFF -DQATERIAL_ENABLE_INSTALL=ON && \
					cmake --build . --target QaterialComponents --config $$BUILDCONFIG $$CMakeProc && \
					cmake --build . --target QaterialIcons --config $$BUILDCONFIG $$CMakeProc && \
					cmake --build . --target Qaterial --config $$BUILDCONFIG $$CMakeProc ; \
					cd .. ; \
				done

} else {
	extralib.commands = echo \"Building Qaterial...\"; \
				cmake $$PWD/Qaterial-1.4.6 \
				$${CMakeArguments} \
				-DQATERIAL_ICONS=\"*.svg\" \
				-DQATERIAL_ENABLE_ROBOTO=OFF -DQATERIAL_ENABLE_ROBOTOMONO=OFF -DQATERIAL_ENABLE_LATO=OFF \
				-DQATERIAL_BUILD_SHARED=$$BUILDSHARED -DQATERIAL_ENABLE_TESTS=OFF -DQATERIAL_ENABLE_TESTS=OFF \
				-DQATERIAL_ENABLE_PCH=$$BUILDPCH \
				-DQATERIAL_ENABLE_HIGHDPIFIX=OFF -DQATERIAL_ENABLE_INSTALL=ON && \
				cmake --build . --target QaterialComponents --config $$BUILDCONFIG $$CMakeProc && \
				cmake --build . --target QaterialIcons --config $$BUILDCONFIG $$CMakeProc && \
				cmake --build . --target Qaterial --config $$BUILDCONFIG $$CMakeProc
}


extralib.depends =

extralib.target = libQaterial.so

QMAKE_EXTRA_TARGETS += extralib
PRE_TARGETDEPS = $$extralib.target


