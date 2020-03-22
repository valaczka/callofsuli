QT += core network

TARGET = smtpclient

TEMPLATE = lib
DEFINES += SMTP_BUILD
win32:CONFIG += dll

QMAKE_CXXFLAGS += -Wno-deprecated-declarations

DESTDIR = ../..

SOURCES += \
	$$PWD/emailaddress.cpp \
	$$PWD/mimeattachment.cpp \
	$$PWD/mimefile.cpp \
	$$PWD/mimehtml.cpp \
	$$PWD/mimeinlinefile.cpp \
	$$PWD/mimemessage.cpp \
	$$PWD/mimepart.cpp \
	$$PWD/mimetext.cpp \
	$$PWD/smtpclient.cpp \
	$$PWD/quotedprintable.cpp \
	$$PWD/mimemultipart.cpp \
	$$PWD/mimecontentformatter.cpp \

HEADERS  += \
	$$PWD/emailaddress.h \
	$$PWD/mimeattachment.h \
	$$PWD/mimefile.h \
	$$PWD/mimehtml.h \
	$$PWD/mimeinlinefile.h \
	$$PWD/mimemessage.h \
	$$PWD/mimepart.h \
	$$PWD/mimetext.h \
	$$PWD/smtpclient.h \
	$$PWD/SmtpMime \
	$$PWD/quotedprintable.h \
	$$PWD/mimemultipart.h \
	$$PWD/mimecontentformatter.h \
	$$PWD/smtpexports.h
