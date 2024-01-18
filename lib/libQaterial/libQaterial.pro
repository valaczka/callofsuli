INCLUDEPATH += \
	$$PWD/../Qaterial/src \
	$$PWD/../QOlm/include

CONFIG += c++17 static
CONFIG += separate_debug_info

TEMPLATE = lib

android: TARGET = Qaterial_$${QT_ARCH}
else: TARGET = Qaterial


DESTDIR = ../

RESOURCES += \
	$$PWD/QaterialIcons.qrc

lessThan(QT_MAJOR_VERSION, 6): RESOURCES += $$PWD/Qaterial.qrc
else: RESOURCES += $$PWD/Qaterial-Qt6.qrc


DEFINES += \
	QOLM_STATIC \
	QATERIAL_STATIC \
	QATERIAL_VERSION_MAJOR=1 \
	QATERIAL_VERSION_MINOR=4 \
	QATERIAL_VERSION_PATCH=7 \
	QATERIAL_VERSION_TAG_HEX=0x0000000



QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-unused-lambda-capture -Wno-deprecated-declarations


# Qrc
lessThan(QT_MAJOR_VERSION, 6) {
	!exists(Qaterial.qrc): {
		lines = <RCC>
		lines += "<qresource prefix=\"/Qaterial\">"

		flist = $$files($$PWD/../Qaterial/qml/Qaterial/*)

		for (file, flist): lines += "	<file alias=\"$$basename(file)\">$$relative_path($$file)</file>"

		lines += </qresource>
		lines += </RCC>

		message(Create Qaterial.qrc)
		write_file(Qaterial.qrc, lines)
	}
} else {
	!exists(Qaterial-Qt6.qrc): {
		lines = <RCC>
		lines += "<qresource prefix=\"/Qaterial\">"

		flist = $$files($$PWD/../Qaterial/qml/Qaterial-Qt6/*)

		for (file, flist): lines += "	<file alias=\"$$basename(file)\">$$relative_path($$file)</file>"

		lines += </qresource>
		lines += </RCC>

		message(Create Qaterial-Qt6.qrc)
		write_file(Qaterial-Qt6.qrc, lines)
	}
}


# Icons

lessThan(QT_MAJOR_VERSION, 6): ICONS_QML = $$PWD/../Qaterial/qml/Qaterial/Icons.qml
else: ICONS_QML = $$PWD/../Qaterial/qml/Qaterial-Qt6/Icons.qml

!exists(QaterialIcons.qrc)|!exists($$ICONS_QML): {

	lines = <RCC>
	lines += "<qresource prefix=\"/Qaterial/Icons\">"


	qmlLines = "// File auto generated"
	qmlLines += "// Generated at $${_DATE_}"
	qmlLines += "pragma Singleton"
	qmlLines += "import QtQuick 2.12"
	qmlLines += "QtObject"
	qmlLines += "{"


	flist = $$files($$PWD/../MaterialDesignSvgo/svg/*)

	restrictedNames = console delete export function import null package switch

	for (file, flist) {
		baseName = $$basename(file)
		iconBaseName = $$replace(baseName, .svg,)

		iconNameParts = $$split(iconBaseName, -)

		partNum = $$size(iconNameParts)

		qmlName =

		greaterThan(partNum, 1): {
			for (part, iconNameParts) {
				isEmpty(qmlName): qmlName = $$part
				else: qmlName += $$upper($$str_member($$part, 0, 0))$$str_member($$part, 1, -1)
			}
		} else {
			qmlName = $$iconBaseName
		}


		contains(restrictedNames, $$qmlName): qmlName = $${qmlName}_

		qmlLines += "	readonly property string $$join(qmlName,,,): \"qrc:/Qaterial/Icons/$$baseName\";"

		lines += "	<file alias=\"$$basename(file)\">$$relative_path($$file)</file>"
	}

	lines += </qresource>
	lines += </RCC>

	qmlLines += "}"

	message(Create QaterialIcons.qrc and $$ICONS_QML)
	write_file(QaterialIcons.qrc, lines)
	write_file($$ICONS_QML, qmlLines)
}


# Sources

HEADERS += \
	$$PWD/../Qaterial/src/Qaterial/Details/Export.hpp \
	$$PWD/../Qaterial/src/Qaterial/Details/Property.hpp \
	$$PWD/../Qaterial/src/Qaterial/Details/Utils.hpp \
	$$PWD/../Qaterial/src/Qaterial/Details/Version.hpp \
	$$PWD/../Qaterial/src/Qaterial/Display/IconDescription.hpp \
	$$PWD/../Qaterial/src/Qaterial/Display/IconLabelImpl.hpp \
	$$PWD/../Qaterial/src/Qaterial/Display/IconLabelPositioner.hpp \
	$$PWD/../Qaterial/src/Qaterial/IO/Clipboard.hpp \
	$$PWD/../Qaterial/src/Qaterial/IO/FolderTreeModel.hpp \
	$$PWD/../Qaterial/src/Qaterial/IO/TextFile.hpp \
	$$PWD/../Qaterial/src/Qaterial/Layout/Layout.hpp \
	$$PWD/../Qaterial/src/Qaterial/Navigation/StepperElement.hpp \
	$$PWD/../Qaterial/src/Qaterial/Navigation/TreeElement.hpp \
	$$PWD/../Qaterial/src/Qaterial/Pch/Pch.hpp \
	$$PWD/../Qaterial/src/Qaterial/Qaterial.hpp \
	$$PWD/../Qaterial/src/Qaterial/Theme/ColorTheme.hpp \
	$$PWD/../Qaterial/src/Qaterial/Theme/TextTheme.hpp \
	$$PWD/../Qaterial/src/Qaterial/Theme/Theme.hpp

SOURCES += \
	$$PWD/../Qaterial/src/Qaterial/Details/Utils.cpp \
	$$PWD/../Qaterial/src/Qaterial/Details/Version.cpp \
	$$PWD/../Qaterial/src/Qaterial/Display/IconDescription.cpp \
	$$PWD/../Qaterial/src/Qaterial/Display/IconLabelImpl.cpp \
	$$PWD/../Qaterial/src/Qaterial/Display/IconLabelPositioner.cpp \
	$$PWD/../Qaterial/src/Qaterial/IO/Clipboard.cpp \
	$$PWD/../Qaterial/src/Qaterial/IO/FolderTreeModel.cpp \
	$$PWD/../Qaterial/src/Qaterial/IO/TextFile.cpp \
	$$PWD/../Qaterial/src/Qaterial/Layout/Layout.cpp \
	$$PWD/../Qaterial/src/Qaterial/Navigation/StepperElement.cpp \
	$$PWD/../Qaterial/src/Qaterial/Navigation/TreeElement.cpp \
	$$PWD/../Qaterial/src/Qaterial/Theme/ColorTheme.cpp \
	$$PWD/../Qaterial/src/Qaterial/Theme/TextTheme.cpp \
	$$PWD/../Qaterial/src/Qaterial/Theme/Theme.cpp
