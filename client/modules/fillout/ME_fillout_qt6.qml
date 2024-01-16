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

	onModifiedChanged: if (objectiveEditor) objectiveEditor.modified = true

	readonly property bool isText: storage && storage.module == "text"
	readonly property bool isSequence: storage && storage.module == "sequence"

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

		visible: !isText && !isSequence

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}


	QFormTextArea {
		id: _wrongAnswers
		title: qsTr("Helytelen válaszok")
		placeholderText: qsTr("Lehetséges egyéb helytelen válaszok (soronként)")
		width: parent.width

		field: "options"
		getData: function() { return text.split("\n") }

		visible: !isText && !isSequence

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormSpinBox {
		id: _countWords
		text: qsTr("Megjelenített szavak száma (0=mind):")
		field: "words"

		from: 0
		to: 20
		stepSize: 1

		spin.editable: true

		visible: isSequence
	}

	QFormSpinBox {
		id: _spinCount
		field: "count"
		text: qsTr("Kiegészítendő helyek száma:")

		visible: !isText

		from: 1
		value: 3
		to: isSequence && _countWords.value > 0 ? _countWords.value : 99
		spin.editable: true

		spin.onValueModified: if (objectiveEditor) objectiveEditor.previewRefresh()
	}


	QFormSpinBox {
		id: _spinOptions
		field: "optionsCount"
		text: qsTr("Válaszlehetőségek száma:")

		visible: !isText && !isSequence

		from: _spinCount.value
		value: 5
		to: 99
		spin.editable: true

		spin.onValueModified: if (objectiveEditor) objectiveEditor.previewRefresh()
	}


	MapEditorSpinStorageCount {
		id: _countBinding
		visible: isText || isSequence
	}



	function loadData() {
		let _items = isText ? [_question]
							: isSequence ? [_question, _countWords, _spinCount]
										 : [_question, _area, _spinOptions, _spinCount]

		_countBinding.value = objective.storageCount

		setItems(_items, objective.data)

		if (objective.data.options !== undefined)
			_wrongAnswers.fieldData = objective.data.options.join("\n")
	}


	function saveData() {
		objective.data = previewData()
		objective.storageCount = _countBinding.value
	}


	function previewData() {
		let _items = isText ? [_question]
							: isSequence ? [_question, _countWords, _spinCount]
										 : [_question, _area, _spinOptions, _spinCount, _wrongAnswers]
		return getItems(_items)
	}
}


