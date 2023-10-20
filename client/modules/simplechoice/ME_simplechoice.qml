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
	readonly property bool isImages: storage && storage.module == "images"
	readonly property bool isBlock: storage && storage.module == "block"


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

	QFormComboBox {
		id: _modeImages
		text: qsTr("Kérdések készítése:")

		combo.width: Math.min(parent.width-spacing-label.width, Math.max(combo.implicitWidth, 300*Qaterial.Style.pixelSizeRatio))

		field: "mode"

		valueRole: "value"
		textRole: "text"

		model: [
			{value: "image", text: qsTr("Képhez (szövegekből választhat)")},
			{value: "text", text: qsTr("Szöveghez (képekből választhat)")}
		]

		visible: isImages

		combo.onActivated: if (objectiveEditor) objectiveEditor.previewRefresh()
	}


	QFormTextField {
		id: _question
		title: qsTr("Kérdés")
		placeholderText: qsTr("Ez a kérdés fog megjelenni")
		helperText: isBinding ? qsTr("A \%1 jelöli a generált elem helyét") : ""
		field: "question"
		width: parent.width
		visible: !isImages && !isBlock

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormTextField {
		id: _questionII
		title: qsTr("Kérdés")
		placeholderText: qsTr("Ez a kérdés fog megjelenni")
		field: "question"
		width: parent.width
		visible: isImages && _modeImages.currentValue === "image"
		text: qsTr("Mi látható a képen?")

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormTextField {
		id: _questionIT
		title: qsTr("Kérdés")
		placeholderText: qsTr("Ez a kérdés fog megjelenni")
		helperText: qsTr("A \%1 jelöli a generált elem helyét")
		field: "question"
		width: parent.width
		visible: isImages && _modeImages.currentValue === "text"

		text: qsTr("Melyik képen látható: %1?")

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormTextField {
		id: _questionBlock
		title: qsTr("Kérdés")
		placeholderText: qsTr("Ez a kérdés fog megjelenni")
		helperText: qsTr("A \%1 jelöli az elemnek, a \%2 a blokk nevének a helyét")
		field: "question"
		width: parent.width
		visible: isBlock

		text: qsTr("Minek a része: %1?")

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormTextField {
		id: _correctAnswer
		title: qsTr("Helyes válasz")
		placeholderText: qsTr("Ez lesz az egyetlen helyes válasz")
		field: "correct"
		width: parent.width
		visible: !isBinding && !isImages && !isBlock

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormTextArea {
		id: _answers
		title: qsTr("Helytelen válaszok")
		placeholderText: qsTr("Lehetséges helytelen válaszok (soronként)")
		width: parent.width
		visible: !isBinding && !isImages && !isBlock

		field: "answers"
		getData: function() { return text.split("\n") }

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormTextField {
		id: _answerImage
		title: qsTr("Válaszok")
		placeholderText: qsTr("A válaszok formátuma")
		helperText: qsTr("A \%1 jelöli a generált elem helyét")
		field: "answers"
		width: parent.width

		visible: isImages && _modeImages.currentValue === "image"

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	MapEditorSpinStorageCount {
		id: _countBinding
		visible: isBinding || isImages || isBlock
	}




	function loadData() {
		let _items = isBinding ? [_question, _modeBinding] :
								 isImages ? (objective.data.mode === "image" ?
												 [_modeImages, _questionII, _answerImage] :
												 [_modeImages, _questionIT]) :
											isBlock ? [_questionBlock] :
													  [_question, _correctAnswer]

		_countBinding.value = objective.storageCount
		setItems(_items, objective.data)
		if (!isBinding && !isImages && !isBlock && objective.data.answers !== undefined)
			_answers.fieldData = objective.data.answers.join("\n")
	}


	function saveData() {
		objective.data = previewData()
		objective.storageCount = _countBinding.value
	}



	function previewData() {
		let _items = isBinding ? [_question, _modeBinding] :
								 isImages ? (_modeImages.currentValue === "image" ?
												 [_modeImages, _questionII, _answerImage] :
												 [_modeImages, _questionIT]) :
											isBlock ? [_questionBlock] :
													  [_question, _correctAnswer, _answers]

		return getItems(_items)
	}
}



