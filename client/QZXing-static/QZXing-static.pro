CONFIG += \
		  #enable_decoder_1d_barcodes \
		  #enable_decoder_qr_code \
		  #enable_decoder_data_matrix \
		  #enable_decoder_aztec \
		  #enable_decoder_pdf17 \
		  enable_encoder_qr_code \
		  staticlib \
		  qzxing_qml \
		  #qzxing_multimedia \

VERSION = 3.2

TARGET = QZXing
TEMPLATE = lib

include(../qzxing/src/QZXing-components.pri)

DEFINES -= DISABLE_LIBRARY_FEATURES


QMAKE_CXXFLAGS += -Wno-sign-compare
