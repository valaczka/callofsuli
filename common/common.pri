include(CuteLogger/CuteLogger.pri)

INCLUDEPATH += $$PWD

SOURCES += \
			$$PWD/cosdb.cpp \
			$$PWD/cosmessage.cpp \
			$$PWD/gamemap.cpp \
			$$PWD/gamemapreaderiface.cpp

HEADERS += \
			$$PWD/cosdb.h \
			$$PWD/cosmessage.h \
			$$PWD/gamemap.h \
			$$PWD/gamemapreaderiface.h

DISTFILES += \
	$$PWD/license.txt


