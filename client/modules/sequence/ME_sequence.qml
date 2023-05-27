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
		trailingContent: Qaterial.TextFieldClearButton { visible: _title.length; textField: _title }
	}


	QFormTextArea {
		id: _areaItems
		title: qsTr("Elemek")
		placeholderText: qsTr("A sorozat elemei (soronként) növekvő sorrendben ")
		helperText: qsTr("Növekvő sorrendben")
		width: parent.width

		enabled: !root.readOnly

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
		let d = getItems([_title])
		d.items = _areaItems.text.split("\n")

		return d
	}
}
