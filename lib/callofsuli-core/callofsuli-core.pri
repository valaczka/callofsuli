INCLUDEPATH += $$PWD

DEFINES += \
	COS_VERSION_MAJOR=$$VER_MAJ \
	COS_VERSION_MINOR=$$VER_MIN \

!wasm {
	include($$PWD/../QDeferred/src/qdeferred.pri)
	include($$PWD/../QDeferred/src/qlambdathreadworker.pri)

	INCLUDEPATH += $$PWD/../QDeferred/src
}


INCLUDEPATH += $$PWD/../QJsonWebToken/src


android: INCLUDEPATH += $$PWD/../android_openssl/static/include


# info: utils.h includes selectableobject.h

HEADERS += \
	$$PWD/credential.h \
	$$PWD/gamemap.h \
	$$PWD/gamemapreaderiface.h \
	$$PWD/rank.h \
	$$PWD/selectableobject.h \
	$$PWD/utils.h \
	$$PWD/../QJsonWebToken/src/qjsonwebtoken.h


SOURCES += \
	$$PWD/credential.cpp \
	$$PWD/gamemap.cpp \
	$$PWD/gamemapreaderiface.cpp \
	$$PWD/rank.cpp \
	$$PWD/selectableobject.cpp \
	$$PWD/utils.cpp \
	$$PWD/../QJsonWebToken/src/qjsonwebtoken.cpp

!wasm {
	HEADERS += \
		$$PWD/database.h

	SOURCES += \
		$$PWD/database.cpp

}
