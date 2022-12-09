DEPENDPATH += .
DEFINES += STATIC_PLUGIN_BOX2D

RESOURCES += \
	$$PWD/Bacon2D-static.qrc

HEADERS += \
	$$PWD/entity.h \
	$$PWD/enums.h \
	$$PWD/scene.h \
	$$PWD/game.h \
	$$PWD/plugins.h \
	#$$PWD/spritesheet.h \
	#$$PWD/sprite.h \
	#$$PWD/spriteanimation.h \
	$$PWD/animationtransition.h \
	$$PWD/animationchangeevent.h \
	$$PWD/bacon2dlayer.h \
	$$PWD/bacon2dimagelayer.h \
	$$PWD/viewport.h \
	$$PWD/behavior.h \
	$$PWD/scriptbehavior.h \
	$$PWD/scrollbehavior.h \
	$$PWD/scrollbehaviorimpl.h \
	$$PWD/imagelayerscrollbehavior.h \
	$$PWD/layerscrollbehavior.h \
	$$PWD/settings.h \
	$$PWD/tiledobject.h \
	$$PWD/tiledlayer.h \
	$$PWD/tiledscene.h \

SOURCES += \
	$$PWD/entity.cpp \
	$$PWD/enums.cpp \
	$$PWD/scene.cpp \
	$$PWD/game.cpp \
	$$PWD/plugins.cpp \
	#$$PWD/spritesheet.cpp \
	#$$PWD/sprite.cpp \
	#$$PWD/spriteanimation.cpp \
	$$PWD/animationtransition.cpp \
	$$PWD/bacon2dlayer.cpp \
	$$PWD/bacon2dimagelayer.cpp \
	$$PWD/viewport.cpp \
	$$PWD/behavior.cpp \
	$$PWD/scriptbehavior.cpp \
	$$PWD/scrollbehavior.cpp \
	$$PWD/imagelayerscrollbehavior.cpp \
	$$PWD/layerscrollbehavior.cpp \
	$$PWD/settings.cpp \
	$$PWD/tiledscene.cpp \
	$$PWD/tiledlayer.cpp \
	$$PWD/tiledobject.cpp
