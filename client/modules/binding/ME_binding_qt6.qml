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
		trailingContent: Qaterial.TextFieldButtonContainer
		{
			Qaterial.TextFieldClearButton {  }
		}
	}

	QFormSection {
		width: parent.width
		text: qsTr("Párok")
		icon.source: Qaterial.Icons.cards
	}

	QFormBindingField {
		id: _binding
		width: parent.width

		defaultLeftData: ""
		defaultRightData: ""

		leftComponent: QFormBindingTextField {
			bindingField: _binding

			onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
		}

		rightComponent: QFormBindingTextField {
			bindingField: _binding

			onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
		}
	}


	function saveData() {
		if (!storage)
			return

		storage.data = previewData()
	}

	function loadData() {
		if (storage && storage.data.bindings)
			_binding.loadFromList(storage.data.bindings)
		else
			_binding.loadFromList([])

		if (storage && storage.storageid <= 0)
			_binding.readOnly = false

		if (storage)
			setItems([_title], storage.data)
	}


	function previewData() {
		let d = getItems([_title])
		d.bindings = _binding.saveToList()

		return d
	}
}