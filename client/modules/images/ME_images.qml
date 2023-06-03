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

	spacing: 10

	property alias readOnly: _binding.readOnly

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

	QFormSection {
		width: parent.width
		text: qsTr("Képek")
		icon.source: Qaterial.Icons.imageMultiple
	}

	QFormBindingField {
		id: _binding
		width: parent.width

		defaultLeftData: ""
		defaultRightData: -1
		split: 0.7


		leftComponent: QFormBindingTextField {
			bindingField: _binding

			onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
		}

		rightComponent: MapEditorFormImage {
			bindingField: _binding
			editor: objectiveEditor.editor

			onModified: loadData(id)

		}
	}


	function saveData() {
		if (!storage)
			return

		storage.data = previewData()
	}

	function loadData() {
		if (storage && storage.data.images)
			_binding.loadFromList(storage.data.images)
		else
			_binding.loadFromList([])

		if (storage && storage.storageid <= 0)
			_binding.readOnly = false

		if (storage)
			setItems([_title], storage.data)
	}


	function previewData() {
		let d = getItems([_title])
		d.images = _binding.saveToList()

		return d
	}
}






