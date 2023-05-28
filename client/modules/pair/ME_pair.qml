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

	onModifiedChanged: if (objectiveEditor) objectiveEditor.modified = true

	readonly property bool isBinding: storage && (storage.module == "binding" || storage.module == "numbers")

	QFormTextField {
		id: _question
		title: qsTr("Kérdés")
		placeholderText: qsTr("Ez a kérdés fog megjelenni")
		field: "question"
		width: parent.width

		text: qsTr("Rendezd össze a párokat!")

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormSection {
		width: parent.width
		text: qsTr("Párok")
		icon.source: Qaterial.Icons.abacus
	}

	QFormBindingField {
		id: _binding
		width: parent.width

		defaultLeftData: ""
		defaultRightData: ""

		visible: !isBinding

		readOnly: false

		leftComponent: QFormBindingTextField {
			bindingField: _binding

			onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
		}

		rightComponent: QFormBindingTextField {
			bindingField: _binding

			onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
		}
	}

	QFormSpinBox {
		id: _spinCount
		field: "count"
		text: qsTr("Párok száma:")

		from: 2
		value: 4
		to: 99
		spin.editable: true

		spin.onValueModified: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormSpinBox {
		id: _spinOptions
		field: "optionsCount"
		text: qsTr("Válaszlehetőségek száma:")

		from: _spinCount.value
		value: 5
		to: 99
		spin.editable: true

		spin.onValueModified: if (objectiveEditor) objectiveEditor.previewRefresh()
	}


	QFormComboBox {
		id: _modeOrder
		text: qsTr("Párok készítése:")

		combo.width: Math.max(combo.implicitWidth, 200)

		field: "mode"

		valueRole: "value"
		textRole: "text"

		model: [
			{value: "first", text: qsTr("Bal oldaliakhoz")},
			{value: "second", text: qsTr("Jobb oldaliakhoz")},
			{value: "both", text: qsTr("Véletlenszerű oldaliakhoz")},
			{value: "shuffle", text: qsTr("Keverve")}
		]

		combo.onActivated: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	MapEditorSpinStorageCount {
		id: _countBinding
		visible: isBinding
	}




	function loadData() {
		_countBinding.value = objective.storageCount

		setItems([_question, _spinOptions, _modeOrder, _spinCount], objective.data)

		if (!isBinding && objective.data.pairs !== undefined)
			_binding.loadFromList(objective.data.pairs)
		else
			_binding.loadFromList([])
	}


	function saveData() {
		objective.data = previewData()
		objective.storageCount = _countBinding.value
	}



	function previewData() {
		let d = getItems([_question, _spinOptions, _modeOrder, _spinCount])

		if (!isBinding)
			d.pairs = _binding.saveToList()

		return d
	}
}


