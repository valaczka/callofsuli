TEMPLATE = aux

include(../../common.pri)
include(../clang.pri)

wasm: BUILDSHARED = OFF
else: BUILDSHARED = ON

USEQT6 = OFF

wasm: extralib.target = libqmlbox2d.a
else: extralib.target = libqmlbox2d.so

android {
	extralib.commands = echo "Building qml-box2d..."; \
				for a in $${ANDROID_ABIS}; do \
					test -d $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}a || mkdir $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}a ; \
					cd $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}a ; \
					cmake $$PWD/qml-box2d \
					$${CMakeArguments} -DANDROID_ABI:STRING=$${LITERAL_DOLLAR}$${LITERAL_DOLLAR}a \
					-DUSE_QT6=$$USEQT6 -DUSE_SYSTEM_BOX2D=OFF -DBUILD_EXAMPLES=OFF -DBUILD_SHARED_LIBS=$$BUILDSHARED && \
					cmake --build . $$CMakeProc ; \
					cd .. ; \
				done

} else {
	extralib.commands = echo \"Building qml-box2d...\"; \
				cmake $$PWD/qml-box2d \
				$${CMakeArguments} \
				-DUSE_QT6=$$USEQT6 -DUSE_SYSTEM_BOX2D=OFF -DBUILD_EXAMPLES=OFF -DBUILD_SHARED_LIBS=$$BUILDSHARED && \
				cmake --build . $$CMakeProc
}



extralib.depends =


QMAKE_EXTRA_TARGETS += extralib
PRE_TARGETDEPS = $$extralib.target


