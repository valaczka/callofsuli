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

	QFormTextField {
		id: _question
		title: qsTr("Kérdés")
		placeholderText: qsTr("Ez a kérdés fog megjelenni")
		field: "question"
		width: parent.width

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormTextArea {
		id: _correctAnswers
		title: qsTr("Helyes válaszok")
		placeholderText: qsTr("Lehetséges helyes válaszok (soronként)")
		width: parent.width

		field: "corrects"
		getData: function() { return text.split("\n") }

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormTextArea {
		id: _wrongAnswers
		title: qsTr("Helytelen válaszok")
		placeholderText: qsTr("Lehetséges helytelen válaszok (soronként)")
		width: parent.width

		field: "answers"
		getData: function() { return text.split("\n") }

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormSpinBox {
		id: _spinMin
		field: "correctMin"
		text: qsTr("Min. helyes válasz")

		from: 2
		value: 2
		to: 99
		spin.editable: true

		spin.onValueModified: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormSpinBox {
		id: _spinMax
		field: "correctMax"
		text: qsTr("Max. helyes válasz")

		from: _spinMin.value
		value: 4
		to: 99
		spin.editable: true

		spin.onValueModified: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormSpinBox {
		id: _spinCount
		field: "count"
		text: qsTr("Max. lehetőség")

		from: _spinMax.value+1
		value: 5
		to: 99
		spin.editable: true

		spin.onValueModified: if (objectiveEditor) objectiveEditor.previewRefresh()
	}





	function loadData() {
		setItems([_question, _spinMin, _spinMax, _spinCount], objective.data)
		if (objective.data.answers !== undefined)
			_wrongAnswers.fieldData = objective.data.answers.join("\n")
		if (objective.data.corrects !== undefined)
			_correctAnswers.fieldData = objective.data.corrects.join("\n")
	}


	function saveData() {
		objective.data = previewData()
		//objective.storageCount = _countBinding.value
	}



	function previewData() {
		return getItems([_question, _spinMin, _spinMax, _spinCount, _wrongAnswers, _correctAnswers])
	}
}



