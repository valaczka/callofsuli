include(../../common.pri)

QT = core

CONFIG += c++20 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


INCLUDEPATH += $$LibSodiumInclude
LIBS += $$LibSodiumLibs


!isEmpty(LibSodiumDefines) {
        DEFINES += $$LibSodiumDefines
}

HEADERS += $$PWD/../../lib/callofsuli-core/desktoputils.h

SOURCES += \
        main.cpp \
        $$PWD/../../lib/callofsuli-core/desktoputils.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
