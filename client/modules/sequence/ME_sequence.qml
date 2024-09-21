import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
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


	QFormTextArea {
		id: _areaItems
		title: qsTr("Elemek")
		placeholderText: qsTr("A sorozat elemei (soronként) növekvő sorrendben ")
		helperText: qsTr("Növekvő sorrendben")
		width: parent.width

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
				_areaItems.fieldData = storage.data.items.join("\n")
		}
	}


	function previewData() {
		return getItems([_title, _areaItems])
	}
}
