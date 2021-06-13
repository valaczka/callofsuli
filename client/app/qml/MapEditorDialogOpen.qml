import QtQuick 2.15
import QtQuick.Dialogs 1.3


FileDialog {
	id: dialog
	title: qsTr("Pálya megnyitása")
	folder: shortcuts.home

	nameFilters: [ qsTr("Call of Suli pálya fájlok")+ " (*.map)", qsTr("Minden fájl")+" (*)" ]

	//property int chapterId: -1

	selectMultiple: false
	selectExisting: true
	selectFolder: false

	onAccepted: {
		mapEditor.openUrl(dialog.fileUrl)
		//mapEditor.run("chapterImport", {chapterid: chapterId, filename: dialog.fileUrl})
	}
}
