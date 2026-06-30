CONFIG += c++2a static
CONFIG += separate_debug_info

QT += gui quick

android: QT += core-private

# Multimedia support
with_multimedia = 0

TEMPLATE = lib

android: TARGET = SCodes_$${QT_ARCH}
else: TARGET = SCodes

DESTDIR = ../

include($$PWD/../SCodes/src/SCodes.pri)
wasm: QT -= concurrent
