TEMPLATE = lib

QT += qml

CONFIG += staticlib

android: TARGET = SortFilterProxyModel_$${QT_ARCH}
else: TARGET = SortFilterProxyModel

SOURCE_DIR = $$PWD/../SortFilterProxyModel

INCLUDEPATH += $${SOURCE_DIR}

DESTDIR = ..

HEADERS += $${SOURCE_DIR}/qqmlsortfilterproxymodel.h \
	$${SOURCE_DIR}/filters/filter.h \
	$${SOURCE_DIR}/filters/filtercontainer.h \
	$${SOURCE_DIR}/filters/rolefilter.h \
	$${SOURCE_DIR}/filters/valuefilter.h \
	$${SOURCE_DIR}/filters/indexfilter.h \
	$${SOURCE_DIR}/filters/regexpfilter.h \
	$${SOURCE_DIR}/filters/rangefilter.h \
	$${SOURCE_DIR}/filters/expressionfilter.h \
	$${SOURCE_DIR}/filters/filtercontainerfilter.h \
	$${SOURCE_DIR}/filters/anyoffilter.h \
	$${SOURCE_DIR}/filters/alloffilter.h \
	$${SOURCE_DIR}/sorters/sorter.h \
	$${SOURCE_DIR}/sorters/sortercontainer.h \
	$${SOURCE_DIR}/sorters/rolesorter.h \
	$${SOURCE_DIR}/sorters/stringsorter.h \
	$${SOURCE_DIR}/sorters/expressionsorter.h \
	$${SOURCE_DIR}/proxyroles/proxyrole.h \
	$${SOURCE_DIR}/proxyroles/proxyrolecontainer.h \
	$${SOURCE_DIR}/proxyroles/joinrole.h \
	$${SOURCE_DIR}/proxyroles/switchrole.h \
	$${SOURCE_DIR}/proxyroles/expressionrole.h \
	$${SOURCE_DIR}/proxyroles/singlerole.h \
	$${SOURCE_DIR}/proxyroles/regexprole.h \
	$${SOURCE_DIR}/sorters/filtersorter.h \
	$${SOURCE_DIR}/proxyroles/filterrole.h

SOURCES += $${SOURCE_DIR}/qqmlsortfilterproxymodel.cpp \
	$${SOURCE_DIR}/filters/filter.cpp \
	$${SOURCE_DIR}/filters/filtercontainer.cpp \
	$${SOURCE_DIR}/filters/rolefilter.cpp \
	$${SOURCE_DIR}/filters/valuefilter.cpp \
	$${SOURCE_DIR}/filters/indexfilter.cpp \
	$${SOURCE_DIR}/filters/regexpfilter.cpp \
	$${SOURCE_DIR}/filters/rangefilter.cpp \
	$${SOURCE_DIR}/filters/expressionfilter.cpp \
	$${SOURCE_DIR}/filters/filtercontainerfilter.cpp \
	$${SOURCE_DIR}/filters/anyoffilter.cpp \
	$${SOURCE_DIR}/filters/alloffilter.cpp \
	$${SOURCE_DIR}/filters/filtersqmltypes.cpp \
	$${SOURCE_DIR}/sorters/sorter.cpp \
	$${SOURCE_DIR}/sorters/sortercontainer.cpp \
	$${SOURCE_DIR}/sorters/rolesorter.cpp \
	$${SOURCE_DIR}/sorters/stringsorter.cpp \
	$${SOURCE_DIR}/sorters/expressionsorter.cpp \
	$${SOURCE_DIR}/sorters/sortersqmltypes.cpp \
	$${SOURCE_DIR}/proxyroles/proxyrole.cpp \
	$${SOURCE_DIR}/proxyroles/proxyrolecontainer.cpp \
	$${SOURCE_DIR}/proxyroles/joinrole.cpp \
	$${SOURCE_DIR}/proxyroles/switchrole.cpp \
	$${SOURCE_DIR}/proxyroles/expressionrole.cpp \
	$${SOURCE_DIR}/proxyroles/proxyrolesqmltypes.cpp \
	$${SOURCE_DIR}/proxyroles/singlerole.cpp \
	$${SOURCE_DIR}/proxyroles/regexprole.cpp \
	$${SOURCE_DIR}/sorters/filtersorter.cpp \
	$${SOURCE_DIR}/proxyroles/filterrole.cpp
