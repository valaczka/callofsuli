INCLUDEPATH += \
	$$PWD/../QOlm/include

CONFIG += c++17 static
CONFIG += separate_debug_info

TEMPLATE = lib

android: TARGET = QOlm_$${QT_ARCH}
else: TARGET = QOlm

DESTDIR = ../

DEFINES += QOLM_STATIC


QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-unused-lambda-capture -Wno-deprecated-declarations



# Sources

HEADERS += \
	$$PWD/../QOlm/include/QOlm/Details/Export.hpp \
	$$PWD/../QOlm/include/QOlm/Details/QOlmBase.hpp \
	$$PWD/../QOlm/include/QOlm/QOlm.hpp

SOURCES += \
	$$PWD/../QOlm/src/QOlmBase.cpp
