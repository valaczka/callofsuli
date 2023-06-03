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

	readonly property bool isBinding: storage && storage.module == "binding"
	readonly property bool isNumbers: storage && storage.module == "numbers"


	QFormTextField {
		id: _questionNone
		title: qsTr("Állítás")
		placeholderText: qsTr("Ez az állítás fog megjelenni")
		field: "question"
		width: parent.width
		visible: !isBinding && !isNumbers

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}


	QFormComboBox {
		id: _correct
		text: qsTr("Az állítás igazságtartalma:")

		visible: !isBinding && !isNumbers

		combo.width: Math.min(parent.width-spacing-label.width, Math.max(combo.implicitWidth, 200*Qaterial.Style.pixelSizeRatio))

		field: "correct"

		valueRole: "value"
		textRole: "text"

		model: [
			{value: false, text: qsTr("Hamis")},
			{value: true, text: qsTr("Igaz")}
		]

		combo.onActivated: if (objectiveEditor) objectiveEditor.previewRefresh()
	}





	QFormComboBox {
		id: _modeBinding
		text: qsTr("Kérdések készítése:")

		visible: isBinding

		combo.width: Math.min(parent.width-spacing-label.width, Math.max(combo.implicitWidth, 300*Qaterial.Style.pixelSizeRatio))

		field: "mode"

		valueRole: "value"
		textRole: "text"

		model: [
			{value: "left", text: qsTr("Bal oldal: mind igaz, jobb: mind hamis")},
			{value: "right", text: qsTr("Jobb oldal: mind igaz, bal: mind hamis")},
			{value: "generateLeft", text: qsTr("Készítés a bal oldaliakhoz")},
			{value: "generateRight", text: qsTr("Készítés a jobb oldaliakhoz")}
		]

		combo.onActivated: if (objectiveEditor) objectiveEditor.previewRefresh()
	}



	QFormComboBox {
		id: _modeNumbers
		text: qsTr("Kérdések készítése:")

		visible: isNumbers

		combo.width: Math.min(parent.width-spacing-label.width, Math.max(combo.implicitWidth, 200*Qaterial.Style.pixelSizeRatio))

		field: "mode"

		valueRole: "value"
		textRole: "text"

		model: [
			{value: "generateLeft", text: qsTr("Bal oldaliakhoz")},
			{value: "generateRight", text: qsTr("Jobb oldaliakhoz")}
		]

		combo.onActivated: if (objectiveEditor) objectiveEditor.previewRefresh()
	}


	QFormTextField {
		id: _questionBinding
		title: qsTr("Kérdés")
		placeholderText: qsTr("Ez a kérdés fog megjelenni")
		helperText: qsTr("A \%1 jelöli a kérdés oldali, a \%2 a válasz oldali elem helyét")
		field: "question"
		width: parent.width
		visible: isNumbers || (isBinding && (_modeBinding.currentValue === "generateLeft" || _modeBinding.currentValue === "generateRight"))

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}





	MapEditorSpinStorageCount {
		id: _countBinding
		visible: isBinding || isNumbers
	}




	function loadData() {
		let _items = isBinding ? [_modeBinding, _questionBinding] :
								 isNumbers ? [_modeNumbers, _questionBinding] :
											 [_questionNone, _correct]


		_countBinding.value = objective.storageCount
		setItems(_items, objective.data)
	}


	function saveData() {
		objective.data = previewData()
		objective.storageCount = _countBinding.value
	}



	function previewData() {
		let _items = isBinding ? [_modeBinding, _questionBinding] :
								 isNumbers ? [_modeNumbers, _questionBinding] :
											 [_questionNone, _correct]

		return getItems(_items)
	}
}





