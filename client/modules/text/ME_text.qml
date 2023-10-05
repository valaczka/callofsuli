import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS


QFormColumn {
	id: root

	width: parent.width

	property Item objectiveEditor: null
	property MapEditorStorage storage: null
	property MapEditorObjective objective: null

	property bool readOnly: true

	onModifiedChanged: if (objectiveEditor) objectiveEditor.modified = true

	QFormTextField {
		id: _title
		title: qsTr("Név")
		width: parent.width

		field: "name"

		enabled: !root.readOnly

		placeholderText: qsTr("Adatbank elnevezése")
		leadingIconSource: Qaterial.Icons.renameBox
		trailingContent: Qaterial.TextFieldButtonContainer
		{
			Qaterial.TextFieldClearButton {  }
		}
	}


	FilloutHighlighter {
		document: _area.textArea.textDocument
		wordForeground: Qaterial.Colors.black
		wordBackground: Qaterial.Colors.amber700
	}


	QFormTextArea {
		id: _area
		title: qsTr("Szövegek")

		placeholderText: qsTr("Ide kell írni a szöveget, amiből a hiányzó szavakat ki kell egészíteniük. A lehetséges pótolandó szavakat vagy kifejezéseket két százalékjel (%) közé kell tenni. (Pl: A %hiányzó% szó, vagy a %hiányzó kifejezések%.)\nAmennyiben %-jelet szeretnénk megjeleníteni ezt kell írni helyette: \\%")
		width: parent.width
		helperText: qsTr("A lehetséges pótolandó szavakat vagy kifejezéseket két százalékjel (%) közé kell tenni")

		enabled: !root.readOnly

		field: "items"
		getData: function() { return text.split("\n") }

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}



	function saveData() {
		if (!storage)
			return

		storage.data = previewData()
	}

	function loadData() {
		if (storage && storage.storageid <= 0)
			readOnly = false

		if (storage) {
			setItems([_title], storage.data)
			if (storage.data.items !== undefined)
				_area.fieldData = storage.data.items.join("\n")
		}
	}


	function previewData() {
		return getItems([_title, _area])
	}
}
