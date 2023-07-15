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
	readonly property bool isImages: storage && storage.module == "images"


	QFormComboBox {
		id: _modeBinding
		text: qsTr("Kérdések készítése:")

		visible: isBinding

		combo.width: Math.min(parent.width-spacing-label.width, Math.max(combo.implicitWidth, 200*Qaterial.Style.pixelSizeRatio))

		field: "mode"

		valueRole: "value"
		textRole: "text"

		model: [
			{value: "left", text: qsTr("Bal oldaliakhoz")},
			{value: "right", text: qsTr("Jobb oldaliakhoz")}
		]

		combo.onActivated: if (objectiveEditor) objectiveEditor.previewRefresh()
	}


	QFormTextField {
		id: _question
		title: qsTr("Kérdés")
		placeholderText: qsTr("Ez a kérdés fog megjelenni")
		helperText: isBinding ? qsTr("A \%1 jelöli a generált elem helyét") : ""
		field: "question"
		width: parent.width
		visible: !isImages

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormTextField {
		id: _questionII
		title: qsTr("Kérdés")
		placeholderText: qsTr("Ez a kérdés fog megjelenni")
		field: "question"
		width: parent.width
		visible: isImages
		text: qsTr("Mi látható a képen?")

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}


	QFormTextField {
		id: _correctAnswer
		title: qsTr("Helyes válasz")
		placeholderText: qsTr("Ez lesz a helyes válasz")
		field: "correct"
		width: parent.width
		visible: !isBinding && !isImages

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}


	MapEditorSpinStorageCount {
		id: _countBinding
		visible: isBinding || isImages
	}




	function loadData() {
		let _items = isBinding ? [_question, _modeBinding] :
								 isImages ? [_questionII] :
											[_question, _correctAnswer]

		_countBinding.value = objective.storageCount
		setItems(_items, objective.data)
	}


	function saveData() {
		objective.data = previewData()
		objective.storageCount = _countBinding.value
	}



	function previewData() {
		let _items = isBinding ? [_question, _modeBinding] :
								 isImages ? [_questionII, _answerImage] :
											[_question, _correctAnswer]

		return getItems(_items)
	}
}



