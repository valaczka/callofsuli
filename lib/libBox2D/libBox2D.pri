DEFINES += BOX2D_ENABLE_SIMD

win32 {
	win32-g++ {
		DEFINES += BOX2D_AVX2
	}
}

linux:!android {
	DEFINES += BOX2D_AVX2
}
