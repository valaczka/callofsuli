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

	readonly property bool isNumbers: storage && (storage.module == "sequence" || storage.module == "numbers")


	QFormTextArea {
		id: _areaItems
		title: qsTr("Elemek")
		placeholderText: qsTr("A sorozat elemei (soronként) növekvő sorrendben ")
		helperText: qsTr("Növekvő sorrendben")
		width: parent.width
		visible: !isNumbers

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormSpinBox {
		id: _spinCount
		field: "count"
		text: qsTr("Elemek száma:")

		from: 2
		value: 5
		to: 99
		spin.editable: true

		spin.onValueModified: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormComboBox {
		id: _modeOrder
		text: qsTr("Sorrend:")

		combo.width: Math.max(combo.implicitWidth, 200)

		field: "mode"

		valueRole: "value"
		textRole: "text"

		model: [
			{value: "ascending", text: qsTr("Növekvő sorrend")},
			{value: "descending", text: qsTr("Csökkenő sorrend")},
			{value: "random", text: qsTr("Véletlenszerű sorrend")}
		]

		combo.onActivated: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormTextField {
		id: _questionAsc
		title: qsTr("Kérdés (növekvő)")
		field: "questionAsc"
		placeholderText: qsTr("Ez a kérdés fog megjelenni növekvő sorrend esetén")
		width: parent.width

		visible: _modeOrder.currentValue === "ascending" || _modeOrder.currentValue === "random"

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormTextField {
		id: _questionDesc
		title: qsTr("Kérdés (csökkenő)")
		field: "questionDesc"
		placeholderText: qsTr("Ez a kérdés fog megjelenni csökkenő sorrend esetén")
		width: parent.width

		visible: _modeOrder.currentValue === "descending" || _modeOrder.currentValue === "random"

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormTextField {
		id: _textMin
		title: qsTr("Segítő jelzés (min.)")
		field: "placeholderMin"
		placeholderText: qsTr("Ezt jeleníti meg azon a helyen, ahova a legkisebb értéket kell helyezni")

		text: qsTr("legkisebb")
		width: parent.width

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormTextField {
		id: _textMax
		title: qsTr("Segítő jelzés (max.)")
		field: "placeholderMax"
		placeholderText: qsTr("Ezt jeleníti meg azon a helyen, ahova a legnagyobb értéket kell helyezni")

		text: qsTr("legnagyobb")
		width: parent.width

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}


	MapEditorSpinStorageCount {
		id: _countBinding
		visible: isNumbers
	}





	function loadData() {
		_countBinding.value = objective.storageCount
		setItems([_spinCount, _modeOrder, _questionAsc, _questionDesc, _textMax, _textMin], objective.data)

		if (!isNumbers && objective.data.items !== undefined)
			_areaItems.fieldData = objective.data.items.join("\n")
	}


	function saveData() {
		objective.data = previewData()
		objective.storageCount = _countBinding.value
	}



	function previewData() {
		let d = getItems([_spinCount, _modeOrder, _questionAsc, _questionDesc, _textMax, _textMin])

		if (!isNumbers)
			d.items = _areaItems.text.split("\n")

		return d
	}
}




