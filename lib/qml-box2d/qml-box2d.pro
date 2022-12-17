TEMPLATE = aux

include(../../common.pri)
include(../cmake.pri)

USEQT6 = OFF

equals(QT_VERSION, 6): USEQT6 = ON


android {
	extralib.commands = echo "Building qml-box2d..."; \
				for a in $${ANDROID_ABIS}; do \
					test -d $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}a || mkdir $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}a ; \
					cd $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}a ; \
					$${CMakePath} $$PWD/qml-box2d \
					$${CMakeArguments} -DANDROID_ABI:STRING=$${LITERAL_DOLLAR}$${LITERAL_DOLLAR}a \
					-DUSE_QT6=$$USEQT6 -DUSE_SYSTEM_BOX2D=OFF -DBUILD_EXAMPLES=OFF -DBUILD_SHARED_LIBS=$$QmlBox2DBuildShared && \
					$${CMakePath} --build . $$CMakeProc ; \
					cd .. ; \
				done

} else {
	extralib.commands = echo \"Building qml-box2d...\"; \
			$${CMakePath} $$PWD/qml-box2d \
			$${CMakeArguments} \
			-DUSE_QT6=$$USEQT6 -DUSE_SYSTEM_BOX2D=OFF -DBUILD_EXAMPLES=OFF -DBUILD_SHARED_LIBS=$$QmlBox2DBuildShared && \
			$${CMakePath} --build . $$CMakeProc
}


extralib.target = libqmlbox2d.file
extralib.depends =


QMAKE_EXTRA_TARGETS += extralib
PRE_TARGETDEPS = $$extralib.target


