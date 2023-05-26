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

	readonly property bool isBinding: storage && (storage.module == "binding" || storage.module == "numbers")
	readonly property bool isImages: storage && storage.module == "images"


	QFormComboBox {
		id: _modeBinding
		text: qsTr("Kérdések készítése:")

		visible: isBinding

		combo.width: Math.max(combo.implicitWidth, 200)

		field: "mode"

		valueRole: "value"
		textRole: "text"

		model: [
			{value: "left", text: qsTr("Bal oldaliakhoz")},
			{value: "right", text: qsTr("Jobb oldaliakhoz")}
		]
	}

	QFormComboBox {
		id: _modeImages
		text: qsTr("Kérdések készítése:")

		combo.width: Math.max(combo.implicitWidth, 300)

		field: "mode"

		valueRole: "value"
		textRole: "text"

		model: [
			{value: "image", text: qsTr("Képhez (szövegekből választhat)")},
			{value: "text", text: qsTr("Szöveghez (képekből választhat)")}
		]

		visible: isImages

		//onActivated: previewImg.refresh()
	}


	QFormTextField {
		id: _question
		title: qsTr("Kérdés")
		placeholderText: qsTr("Ez a kérdés fog megjelenni")
		helperText: isBinding ? qsTr("A \%1 jelöli a generált elem helyét") : ""
		field: "question"
		width: parent.width
		visible: !isImages
	}

	QFormTextField {
		id: _questionII
		title: qsTr("Kérdés")
		placeholderText: qsTr("Ez a kérdés fog megjelenni")
		field: "question"
		width: parent.width
		visible: isImages && _modeImages.currentValue === "image"
		text: qsTr("Mi látható a képen?")
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
	}

	QFormTextField {
		id: _correctAnswer
		title: qsTr("Helyes válasz")
		placeholderText: qsTr("Ez lesz az egyetlen helyes válasz")
		field: "correct"
		width: parent.width
		visible: !isBinding && !isImages
	}

	QFormTextArea {
		id: _answers
		title: qsTr("Helytelen válaszok")
		placeholderText: qsTr("Lehetséges helytelen válaszok (soronként)")
		width: parent.width
		visible: !isBinding && !isImages
	}

	QFormTextField {
		id: _answerImage
		title: qsTr("Válaszok")
		placeholderText: qsTr("A válaszok formátuma")
		helperText: qsTr("A \%1 jelöli a generált elem helyét")
		field: "answers"
		width: parent.width

		visible: isImages && _modeImages.currentValue === "image"

		//onTextModified: previewImg.refresh()
	}

	MapEditorSpinStorageCount {
		id: _countBinding
		visible: isBinding || isImages
	}




	function loadData() {
		let _items = isBinding ? [_question, _modeBinding] :
								 isImages ? (objective.data.mode === "image" ?
												 [_modeImages, _questionII, _answerImage] :
												 [_modeImages, _questionIT]) :
											[_question, _correctAnswer]

		_countBinding.value = objective.storageCount
		setItems(_items, objective.data)
		if (!isBinding && !isImages && objective.data.answers !== undefined)
			_answers.fieldData = objective.data.answers.join("\n")
	}

	function saveData() {
		let _items = isBinding ? [_question, _modeBinding] :
								 isImages ? (_modeImages.currentValue === "image" ?
												 [_modeImages, _questionII, _answerImage] :
												 [_modeImages, _questionIT]) :
											[_question, _correctAnswer]

		let d = getItems(_items)

		if (!isBinding && !isImages)
			d.answers = _answers.text.split("\n")

		objective.data = d
		objective.storageCount = _countBinding.value
	}


	/*	MapEditorObjectivePreview {
				id: previewImg

				refreshFunc: function() { return mapEditor.objectiveGeneratePreview("simplechoice", getData(), storageModule, storageData) }

				Connections {
					target: ldr
					function onStorageDataChanged() {
						previewImg.refresh()
					}
				}
			}


			function getData() {
				moduleData = JS.getSqlFields([comboModeImg,
											  comboModeImg.currentValue === "image" ?
												  textQuestionImgImg :
												  textQuestionImgText,
											  textAnswerImg])

				return moduleData
			}
		}*/



}



