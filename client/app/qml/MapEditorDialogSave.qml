import QtQuick 2.15
import QtQuick.Dialogs 1.3


FileDialog {
	id: dialog
	title: isSaveAs ? qsTr("Másolat mentése") : qsTr("Pálya mentése")
	folder: shortcuts.home

	property bool isSaveAs: false

	nameFilters: [ qsTr("Call of Suli pálya fájlok")+ " (*.map)", qsTr("Minden fájl")+" (*)" ]

	selectMultiple: false
	selectExisting: false
	selectFolder: false
	defaultSuffix: "map"

	onAccepted: if (mapEditor.loaded) {
					if (isSaveAs)
						mapEditor.saveCopyUrl(fileUrl)
					else
						mapEditor.saveUrl(fileUrl)
				}
}
