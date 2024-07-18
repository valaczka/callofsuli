INCLUDEPATH += \
	   $$PWD/../simple-mail/src

CONFIG += c++17
CONFIG += separate_debug_info

QT += core network

TEMPLATE = lib

android: TARGET = simplemail_$${QT_ARCH}
else: TARGET = simplemail

DESTDIR = ../

SOURCES += $$PWD/../simple-mail/src/*.cpp

HEADERS += \
	$$PWD/../simple-mail/src/SimpleMail \
	$$PWD/../simple-mail/src/*.h

DEFINES += \
	PLUGINS_PREFER_DEBUG_POSTFIX= \
	QT_NO_KEYWORDS \
	QT_NO_CAST_TO_ASCII \
	QT_NO_CAST_FROM_ASCII \
	QT_STRICT_ITERATORS \
	QT_NO_URL_CAST_FROM_STRING \
	QT_NO_CAST_FROM_BYTEARRAY \
	QT_USE_QSTRINGBUILDER \
	QT_NO_SIGNALS_SLOTS_KEYWORDS \
	QT_USE_FAST_OPERATOR_PLUS \
	QT_DISABLE_DEPRECATED_BEFORE=0x050f00
