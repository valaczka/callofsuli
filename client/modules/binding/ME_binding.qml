import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS


QFormColumn {
	id: root

	width: parent.width

	property MapEditorObjectiveEditor objectiveEditor: null
	property MapEditorStorage storage: null
	property MapEditorObjective objective: null

	onModifiedChanged: if (objectiveEditor) objectiveEditor.modified = true

	QFormBindingField {
		id: _binding
		width: parent.width

		title: qsTr("Párok")

		defaultLeftData: ""
		defaultRightData: ""

		leftComponent: QFormBindingTextField {
			bindingField: _binding
		}

		rightComponent: QFormBindingTextField {
			bindingField: _binding
		}
	}


	function saveData() {
		if (!storage)
			return

		storage.data = {
			bindings: _binding.saveToList()
		}
	}

	function loadData() {
		if (storage && storage.data.bindings)
			_binding.loadFromList(storage.data.bindings)
		else
			_binding.loadFromList([])

		if (storage && storage.storageid <= 0)
			_binding.readOnly = false
	}
}
