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

	spacing: 10

	onModifiedChanged: if (objectiveEditor) objectiveEditor.modified = true

	readonly property bool isBinding: storage && (storage.module == "binding" ||
												  storage.module == "mergebinding" ||
												  storage.module == "numbers")
	readonly property bool isMergeBinding: storage && (storage.module == "mergebinding" ||
													   storage.module == "mergeblock")
	readonly property bool isImages: storage && storage.module == "images"
	readonly property bool isBlock: storage && (storage.module == "block" ||
												storage.module == "mergeblock")
	readonly property bool isSequence: storage && storage.module == "sequence"


	QFormSectionSelector {
		id: _sectionSelector

		form: root
		field: "sections"
		visible: isMergeBinding
		storage: root.storage
	}

	QFormComboBox {
		id: _modeBinding
		text: qsTr("Kérdések készítése:")

		visible: isBinding

		combo.width: Math.min(parent.width-spacing-label.width, Math.max(combo.implicitWidth, 200*Qaterial.Style.pixelSizeRatio))

		field: "mode"

		valueRole: "value"
		textRole: "text"

		model: ListModel {
			ListElement {value: "left"; text: qsTr("Bal oldaliakhoz")}
			ListElement {value: "right"; text: qsTr("Jobb oldaliakhoz")}
		}

		combo.onActivated: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormComboBox {
		id: _modeImages
		text: qsTr("Kérdések készítése:")

		combo.width: Math.min(parent.width-spacing-label.width, Math.max(combo.implicitWidth, 300*Qaterial.Style.pixelSizeRatio))

		field: "mode"

		valueRole: "value"
		textRole: "text"

		model: ListModel {
			ListElement {value: "image"; text: qsTr("Képhez (szövegekből választhat)")}
			ListElement {value: "text"; text: qsTr("Szöveghez (képekből választhat)")}
		}

		visible: isImages

		combo.onActivated: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormComboBox {
		id: _modeBlock
		text: qsTr("Kérdések készítése:")

		visible: isBlock

		combo.width: Math.min(parent.width-spacing-label.width, Math.max(combo.implicitWidth, 200*Qaterial.Style.pixelSizeRatio))

		field: "mode"

		valueRole: "value"
		textRole: "text"

		model: ListModel {
			ListElement {value: "contains"; text: qsTr("Halmazba tartozás")}
			ListElement {value: "exclude"; text: qsTr("Halmazból kizárás")}
			ListElement {value: "simple"; text: qsTr("Egyszerű választás")}
			ListElement {value: "quiz"; text: qsTr("Kvíz-mód")}
		}

		combo.onActivated: if (objectiveEditor) objectiveEditor.previewRefresh()
	}


	QFormTextField {
		id: _question
		title: qsTr("Kérdés")
		placeholderText: qsTr("Ez a kérdés fog megjelenni")
		helperText: isBinding ? qsTr("A \%1 jelöli a generált elem helyét") : ""
		field: "question"
		width: parent.width
		visible: !isImages && !isBlock && !isSequence

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
		helperText: qsTr("A \%1 jelöli az elemnek a helyét")
		field: "question"
		width: parent.width
		visible: isBlock

		text: _modeBlock.currentValue === "contains" ?
				  qsTr("Minek a része: %1?") :
				  _modeBlock.currentValue === "exclude" ?
					  qsTr("Melyik nem része: %1?") :
					  ""

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormTextField {
		id: _correctAnswer
		title: qsTr("Helyes válasz")
		placeholderText: qsTr("Ez lesz az egyetlen helyes válasz")
		field: "correct"
		width: parent.width
		visible: !isBinding && !isImages && !isBlock && !isSequence

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormTextArea {
		id: _answers
		title: qsTr("Helytelen válaszok")
		placeholderText: qsTr("Lehetséges helytelen válaszok (soronként)")
		width: parent.width
		visible: !isBinding && !isImages && !isBlock && !isSequence

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


	QFormTextField {
		id: _questionSqMin
		title: qsTr("Kérdés a legkisebb elemre")
		placeholderText: qsTr("Ez a kérdés fog megjelenni")
		field: "questionMin"
		width: parent.width
		visible: isSequence
		text: qsTr("Melyik a legkisebb?")

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormTextField {
		id: _questionSqMax
		title: qsTr("Kérdés a legnagyobb elemre")
		placeholderText: qsTr("Ez a kérdés fog megjelenni")
		field: "questionMax"
		width: parent.width
		visible: isSequence
		text: qsTr("Melyik a legnagyobb?")

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}


	QFormCheckButton {
		id: _checkMonospace
		field: "monospace"
		text: qsTr("Válaszlehetőségek monospace betűtípussal")
	}

	QFormSpinBox {
		id: _spinOptions
		field: "maxOptions"
		text: qsTr("Válaszlehetőségek száma:")

		from: 2
		to: 6
		stepSize: 1
		value: 4

		spin.editable: true

		visible: !isImages
	}


	MapEditorSpinStorageCount {
		id: _countBinding
		visible: isBinding || isImages || isBlock || isSequence
	}




	function loadData() {
		let _items = isBinding ? [_question, _modeBinding, _spinOptions, _checkMonospace] :
								 isImages ? (objective.data.mode === "image" ?
												 [_modeImages, _questionII, _answerImage, _checkMonospace] :
												 [_modeImages, _questionIT]) :
											isBlock ? [_modeBlock, _questionBlock, _spinOptions, _checkMonospace] :
													  isSequence ? [_questionSqMin, _questionSqMax, _spinOptions, _checkMonospace ] :
																   [_question, _correctAnswer, _spinOptions, _checkMonospace]

		if (isMergeBinding)
			_items.push(_sectionSelector)

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
		let _items = isBinding ? [_question, _modeBinding, _spinOptions, _checkMonospace] :
								 isImages ? (_modeImages.currentValue === "image" ?
												 [_modeImages, _questionII, _answerImage, _checkMonospace] :
												 [_modeImages, _questionIT]) :
											isBlock ? [_modeBlock, _questionBlock, _spinOptions, _checkMonospace] :
													  isSequence ? [_questionSqMin, _questionSqMax, _spinOptions, _checkMonospace ] :
																   [_question, _correctAnswer, _answers, _spinOptions, _checkMonospace]

		if (isMergeBinding)
			_items.push(_sectionSelector)

		return getItems(_items)
	}
}

