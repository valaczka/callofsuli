INCLUDEPATH += $$PWD

include($$PWD/../QDeferred/src/qdeferred.pri)
include($$PWD/../QDeferred/src/qlambdathreadworker.pri)

INCLUDEPATH += $$PWD/../QDeferred/src
INCLUDEPATH += $$PWD/../QJsonWebToken/src

android: INCLUDEPATH += $$PWD/../android_openssl/static/include

# info: utils_.h includes selectableobject.h

HEADERS += \
	$$PWD/credential.h \
	$$PWD/gamemap.h \
	$$PWD/gamemapreaderiface.h \
	$$PWD/rank.h \
	$$PWD/selectableobject.h \
	$$PWD/../QJsonWebToken/src/qjsonwebtoken.h \
	$$PWD/utils_.h


SOURCES += \
	$$PWD/credential.cpp \
	$$PWD/gamemap.cpp \
	$$PWD/gamemapreaderiface.cpp \
	$$PWD/rank.cpp \
	$$PWD/selectableobject.cpp \
	$$PWD/../QJsonWebToken/src/qjsonwebtoken.cpp \
	$$PWD/utils_.cpp

android: {
	HEADERS += \
		$$PWD/mobileutils.h

	SOURCES += \
		$$PWD/mobileutils.cpp
}


ios: {
	HEADERS += \
		$$PWD/mobileutils.h

	OBJECTIVE_SOURCES += \
		$$PWD/mobileutils.mm
}

