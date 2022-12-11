include(../common.pri)

QTDIR = $$dirname(QMAKE_QMAKE)/..

nproc = $$system("nproc")

CMakeProc = -j$${nproc}

if ($$CMakeVerboseOutput) {
	CMakeProc = -j$${nproc} --verbose
}

CMakeArguments = \
	-DCMAKE_PREFIX_PATH=$${QTDIR} \
	-DCMAKE_FIND_ROOT_PATH:PATH=${QTDIR}

android {
	CMakeArguments += \
		-DANDROID_SDK:PATH=$${AndroidSdkPath} \
		-DANDROID_NDK:PATH=$${AndroidNdkPath} \
		-DCMAKE_C_COMPILER:FILEPATH=$${AndroidNdkPath}/toolchains/llvm/prebuilt/linux-x86_64/bin/clang \
		-DCMAKE_TOOLCHAIN_FILE:FILEPATH=$${AndroidNdkPath}/build/cmake/android.toolchain.cmake
} else {
	CMakeArguments += \
		-DCMAKE_CXX_COMPILER=\"$$QMAKE_CXX\" \
		-DCMAKE_LINKER=\"$$QMAKE_LINK\"
}



