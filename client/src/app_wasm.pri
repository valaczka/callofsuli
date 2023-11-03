
#########################
### WASM LIMITATIONS
#########################

# Url not avaliable (CORS)
# Proxy roles not available (too much recursion)

#########################

SOURCES += \
	onlineapplication.cpp \
	onlineclient.cpp

HEADERS += \
	onlineapplication.h \
	onlineclient.h


if($$WasmWithBox2D): DEFINES += WITH_BOX2D


DESTDIR = ../html

lessThan(QT_MAJOR_VERSION, 6) {
	QMAKE_LFLAGS += -s TOTAL_MEMORY=32MB

	QTDIR = $$dirname(QMAKE_QMAKE)/../qml

	SOURCES += \
		wasm_static_plugins.cpp

	LIBS += \
		$$QTDIR/Qt/labs/platform/libqtlabsplatformplugin.a \
		$$QTDIR/QtGraphicalEffects/libqtgraphicaleffectsplugin.a \
		$$QTDIR/QtGraphicalEffects/private/libqtgraphicaleffectsprivate.a \
		$$QTDIR/QtQml/Models.2/libmodelsplugin.a \
		$$QTDIR/QtQml/libqmlplugin.a \
		$$QTDIR/QtQuick/Controls.2/Fusion/libqtquickcontrols2fusionstyleplugin.a \
		$$QTDIR/QtQuick/Controls.2/Imagine/libqtquickcontrols2imaginestyleplugin.a \
		$$QTDIR/QtQuick/Controls.2/Material/libqtquickcontrols2materialstyleplugin.a \
		$$QTDIR/QtQuick/Controls.2/Universal/libqtquickcontrols2universalstyleplugin.a \
		$$QTDIR/QtQuick/Controls.2/libqtquickcontrols2plugin.a \
		$$QTDIR/QtQuick/Layouts/libqquicklayoutsplugin.a \
		$$QTDIR/QtQuick/Templates.2/libqtquicktemplates2plugin.a \
		$$QTDIR/QtQuick/Window.2/libwindowplugin.a \
		$$QTDIR/Qt/labs/folderlistmodel/libqmlfolderlistmodelplugin.a \
		$$QTDIR/Qt/labs/calendar/libqtlabscalendarplugin.a \
		$$QTDIR/Qt/labs/qmlmodels/liblabsmodelsplugin.a \
		$$QTDIR/Qt/labs/settings/libqmlsettingsplugin.a

	LIBS += -s EXIT_RUNTIME=0
}



