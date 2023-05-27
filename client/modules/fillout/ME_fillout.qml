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

	onModifiedChanged: if (objectiveEditor) objectiveEditor.modified = true

	spacing: 10

	FilloutHighlighter {
		document: _area.textArea.textDocument
		wordForeground: Qaterial.Colors.black
		wordBackground: Qaterial.Colors.amber700
	}


	QFormTextField {
		id: _question
		title: qsTr("Kérdés")
		placeholderText: qsTr("Ez a kérdés fog megjelenni")
		field: "question"
		width: parent.width
		text: qsTr("Egészítsd ki a szöveget!")

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormTextArea {
		id: _area
		title: qsTr("Szöveg")
		field: "text"
		placeholderText: qsTr("Ide kell írni a szöveget, amiből a hiányzó szavakat ki kell egészíteniük. A lehetséges pótolandó szavakat vagy kifejezéseket két százalékjel (%) közé kell tenni. (Pl: A %hiányzó% szó, vagy a %hiányzó kifejezések%.)\nAmennyiben %-jelet szeretnénk megjeleníteni ezt kell írni helyette: \\%")
		width: parent.width
		helperText: qsTr("A lehetséges pótolandó szavakat vagy kifejezéseket két százalékjel (%) közé kell tenni")

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}


	QFormTextArea {
		id: _wrongAnswers
		title: qsTr("Helytelen válaszok")
		placeholderText: qsTr("Lehetséges egyéb helytelen válaszok (soronként)")
		width: parent.width

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormSpinBox {
		id: _spinCount
		field: "count"
		text: qsTr("Kiegészítendő helyek száma:")

		from: 1
		value: 3
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



	function loadData() {
		setItems([_question, _area, _spinOptions, _spinCount], objective.data)
		if (objective.data.options !== undefined)
			_wrongAnswers.fieldData = objective.data.options.join("\n")
	}


	function saveData() {
		objective.data = previewData()
		//objective.storageCount = _countBinding.value
	}



	function previewData() {
		let d = getItems([_question, _area, _spinOptions, _spinCount])

		d.options = _wrongAnswers.text.split("\n")

		return d
	}
}


