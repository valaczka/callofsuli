wasm {
	QMAKE_LFLAGS += -s TOTAL_MEMORY=32MB

	SOURCES += \
		wasm_static_plugins.cpp \
		onlineapplication.cpp \
		onlineclient.cpp

	HEADERS += \
		onlineapplication.h \
		onlineclient.h

	if($$WasmWithBox2D): DEFINES += WITH_BOX2D

	QTDIR = $$dirname(QMAKE_QMAKE)/../qml

	DESTDIR = ../html

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
		$$QTDIR/Qt/labs/settings/libqmlsettingsplugin.a

	WasmRccFiles = $$files($$PWD/../../share/*.cres)
	WasmRccFiles += $$PWD/../deploy/wasm_resources.json

	WasmRcc.commands = $(COPY_FILE) $$shell_path($$WasmRccFiles) $$shell_path($$OUT_PWD/$$DESTDIR)

	QMAKE_EXTRA_TARGETS += WasmRcc

	POST_TARGETDEPS += WasmRcc

	LIBS += -s EXIT_RUNTIME=0



}
