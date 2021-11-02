CONFIG += \
		  #enable_decoder_1d_barcodes \
		  #enable_decoder_qr_code \
		  #enable_decoder_data_matrix \
		  #enable_decoder_aztec \
		  #enable_decoder_pdf17 \
		  enable_encoder_qr_code \
		  qzxing_qml \
		  #qzxing_multimedia \
		  #staticlib \

win32 {
	CONFIG += dll
}

VERSION = 3.2

TARGET = QZXing_$${QT_ARCH}

TEMPLATE = lib

DEFINES -= DISABLE_LIBRARY_FEATURES

include(QZXing-components.pri)

QMAKE_CXXFLAGS += -Wno-sign-compare
