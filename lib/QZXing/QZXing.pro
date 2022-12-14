include(../../common.pri)

CONFIG += \
	enable_encoder_qr_code \
	qzxing_qml \

!wasm: CONFIG += \
	enable_decoder_1d_barcodes \
	enable_decoder_qr_code \
	enable_decoder_data_matrix \
	#enable_decoder_aztec \
	#enable_decoder_pdf17 \
	qzxing_multimedia \

win32: CONFIG += dll
win32: CONFIG += c++11

android: TARGET = QZXing_$${QT_ARCH}
else: TARGET = QZXing

TEMPLATE = lib

include(QZXing-components.pri)

QMAKE_CXXFLAGS += -Wno-sign-compare

#win32: target.path = $${OUT_PWD}/../../build
#else: target.path = $${OUT_PWD}/../../build/lib

DESTDIR = ../

INSTALLS -= headers target

INSTALLS += target
