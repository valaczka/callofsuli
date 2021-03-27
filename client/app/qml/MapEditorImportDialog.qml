import QtQuick 2.15
import QtQuick.Dialogs 1.3


FileDialog {
	id: importDialog
	title: qsTr("Importálás")
	folder: shortcuts.home

	nameFilters: [ qsTr("Excel fájlok")+ " (*.xlsx)" ]

	property int chapterId: -1

	selectMultiple: false
	selectExisting: true
	selectFolder: false

	onAccepted: {
		mapEditor.run("chapterImport", {chapterid: chapterId, filename: importDialog.fileUrl})
	}
}
