include(../../common.pri)

SOURCE_DIR = $$PWD/../QtXlsxWriter/src/xlsx


INCLUDEPATH += $${SOURCE_DIR}
DEPENDPATH += $${SOURCE_DIR}

QT += core gui gui-private

CONFIG += separate_debug_info
CONFIG += build_xlsx_lib

android: TARGET = QtXlsxWriter_$${QT_ARCH}
else: TARGET = QtXlsxWriter

DEFINES += XLSX_NO_LIB

TEMPLATE = lib

win32 {
	CONFIG += c++11
	CONFIG += dll
}

linux:!android: QMAKE_CXXFLAGS += -Wno-deprecated-copy -Wno-deprecated-declarations
android: QMAKE_CXXFLAGS += -Wno-deprecated
win32: QMAKE_CXX += -Wno-strict-aliasing -Wno-deprecated-declarations

DESTDIR = ../

HEADERS += $${SOURCE_DIR}/xlsxdocpropscore_p.h \
	$${SOURCE_DIR}/xlsxdocpropsapp_p.h \
	$${SOURCE_DIR}/xlsxrelationships_p.h \
	$${SOURCE_DIR}/xlsxutility_p.h \
	$${SOURCE_DIR}/xlsxsharedstrings_p.h \
	$${SOURCE_DIR}/xlsxcontenttypes_p.h \
	$${SOURCE_DIR}/xlsxtheme_p.h \
	$${SOURCE_DIR}/xlsxformat.h \
	$${SOURCE_DIR}/xlsxworkbook.h \
	$${SOURCE_DIR}/xlsxstyles_p.h \
	$${SOURCE_DIR}/xlsxabstractsheet.h \
	$${SOURCE_DIR}/xlsxabstractsheet_p.h \
	$${SOURCE_DIR}/xlsxworksheet.h \
	$${SOURCE_DIR}/xlsxworksheet_p.h \
	$${SOURCE_DIR}/xlsxchartsheet.h \
	$${SOURCE_DIR}/xlsxchartsheet_p.h \
	$${SOURCE_DIR}/xlsxzipwriter_p.h \
	$${SOURCE_DIR}/xlsxworkbook_p.h \
	$${SOURCE_DIR}/xlsxformat_p.h \
	$${SOURCE_DIR}/xlsxglobal.h \
	$${SOURCE_DIR}/xlsxdrawing_p.h \
	$${SOURCE_DIR}/xlsxzipreader_p.h \
	$${SOURCE_DIR}/xlsxdocument.h \
	$${SOURCE_DIR}/xlsxdocument_p.h \
	$${SOURCE_DIR}/xlsxcell.h \
	$${SOURCE_DIR}/xlsxcell_p.h \
	$${SOURCE_DIR}/xlsxdatavalidation.h \
	$${SOURCE_DIR}/xlsxdatavalidation_p.h \
	$${SOURCE_DIR}/xlsxcellreference.h \
	$${SOURCE_DIR}/xlsxcellrange.h \
	$${SOURCE_DIR}/xlsxrichstring_p.h \
	$${SOURCE_DIR}/xlsxrichstring.h \
	$${SOURCE_DIR}/xlsxconditionalformatting.h \
	$${SOURCE_DIR}/xlsxconditionalformatting_p.h \
	$${SOURCE_DIR}/xlsxcolor_p.h \
	$${SOURCE_DIR}/xlsxnumformatparser_p.h \
	$${SOURCE_DIR}/xlsxdrawinganchor_p.h \
	$${SOURCE_DIR}/xlsxmediafile_p.h \
	$${SOURCE_DIR}/xlsxabstractooxmlfile.h \
	$${SOURCE_DIR}/xlsxabstractooxmlfile_p.h \
	$${SOURCE_DIR}/xlsxchart.h \
	$${SOURCE_DIR}/xlsxchart_p.h \
	$${SOURCE_DIR}/xlsxsimpleooxmlfile_p.h \
	$${SOURCE_DIR}/xlsxcellformula.h \
	$${SOURCE_DIR}/xlsxcellformula_p.h

SOURCES += $${SOURCE_DIR}/xlsxdocpropscore.cpp \
	$${SOURCE_DIR}/xlsxdocpropsapp.cpp \
	$${SOURCE_DIR}/xlsxrelationships.cpp \
	$${SOURCE_DIR}/xlsxutility.cpp \
	$${SOURCE_DIR}/xlsxsharedstrings.cpp \
	$${SOURCE_DIR}/xlsxcontenttypes.cpp \
	$${SOURCE_DIR}/xlsxtheme.cpp \
	$${SOURCE_DIR}/xlsxformat.cpp \
	$${SOURCE_DIR}/xlsxstyles.cpp \
	$${SOURCE_DIR}/xlsxworkbook.cpp \
	$${SOURCE_DIR}/xlsxabstractsheet.cpp \
	$${SOURCE_DIR}/xlsxworksheet.cpp \
	$${SOURCE_DIR}/xlsxchartsheet.cpp \
	$${SOURCE_DIR}/xlsxzipwriter.cpp \
	$${SOURCE_DIR}/xlsxdrawing.cpp \
	$${SOURCE_DIR}/xlsxzipreader.cpp \
	$${SOURCE_DIR}/xlsxdocument.cpp \
	$${SOURCE_DIR}/xlsxcell.cpp \
	$${SOURCE_DIR}/xlsxdatavalidation.cpp \
	$${SOURCE_DIR}/xlsxcellreference.cpp \
	$${SOURCE_DIR}/xlsxcellrange.cpp \
	$${SOURCE_DIR}/xlsxrichstring.cpp \
	$${SOURCE_DIR}/xlsxconditionalformatting.cpp \
	$${SOURCE_DIR}/xlsxcolor.cpp \
	$${SOURCE_DIR}/xlsxnumformatparser.cpp \
	$${SOURCE_DIR}/xlsxdrawinganchor.cpp \
	$${SOURCE_DIR}/xlsxmediafile.cpp \
	$${SOURCE_DIR}/xlsxabstractooxmlfile.cpp \
	$${SOURCE_DIR}/xlsxchart.cpp \
	$${SOURCE_DIR}/xlsxsimpleooxmlfile.cpp \
	$${SOURCE_DIR}/xlsxcellformula.cpp

