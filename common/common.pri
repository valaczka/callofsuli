include(CuteLogger/CuteLogger.pri)

INCLUDEPATH += $$PWD

SOURCES += \
			$$PWD/cosdb.cpp \
			$$PWD/cosmessage.cpp \
			$$PWD/gamemap.cpp \
			$$PWD/gamemapchapter.cpp \
			$$PWD/gamemapinventory.cpp \
			$$PWD/gamemapmission.cpp \
			$$PWD/gamemapmissionlevel.cpp \
			$$PWD/gamemapnew.cpp \
			$$PWD/gamemapobjective.cpp \
			$$PWD/gamemapstorage.cpp

HEADERS += \
			$$PWD/cosdb.h \
			$$PWD/cosmessage.h \
			$$PWD/gamemap.h \
			$$PWD/gamemapchapter.h \
			$$PWD/gamemapinventory.h \
			$$PWD/gamemapmission.h \
			$$PWD/gamemapmissionlevel.h \
			$$PWD/gamemapnew.h \
			$$PWD/gamemapobjective.h \
			$$PWD/gamemapstorage.h

DISTFILES += \
	$$PWD/license.txt


