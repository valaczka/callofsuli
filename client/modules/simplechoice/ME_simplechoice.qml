import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS


Loader {
	id: root
	width: parent.width

	property MapEditorObjectiveEditor objectiveEditor: null
	property MapEditorStorage storage: null
	property MapEditorObjective objective: null

	sourceComponent: {
		if (!storage)
			return cmpNone

		/*if (storage.module == "binding" || storage.module == "numbers")
			return cmpBinding
		else if (storage.module == "images")
			return cmpImages
		else
			return cmpNone
*/

		return cmpBinding
	}



	Component {
		id: cmpNone

		QFormColumn {
			onModifiedChanged: if (objectiveEditor) objectiveEditor.modified = true

			QFormComboBox {
				id: _modeNone
				text: qsTr("Kérdések készítése:")

				field: "mode"

				valueRole: "value"
				textRole: "text"

				model: [
					{value: "left", text: qsTr("Bal oldaliakhoz")},
					{value: "right", text: qsTr("Jobb oldaliakhoz")}
				]
			}

			QFormTextField {
				id: _questionNone
				title: qsTr("Kérdés")
				placeholderText: qsTr("Ez a kérdés fog megjelenni")
				helperText: qsTr("A \%1 jelöli a generált elem helyét")
				field: "question"
				width: parent.width
			}

			MapEditorSpinStorageCount {
				id: _countNone
			}

			Qaterial.LabelBody1 {
				text: "Hello"
			}

			QFormBindingField {
				width: parent.width

				leftComponent: Qaterial.LabelCaption { }
				rightComponent: Qaterial.LabelHeadline6 { }

			}

			readonly property var _items: [_modeNone, _questionNone]

			function loadData() {
				setItems(_items, objective.data)
				_countNone.value = objective.storageCount
			}

			function saveData() {
				objective.storageCount = _countNone.value
				objective.data = getItems(_items)
			}
		}


		/*QGridLayout {
			id: layout

			watchModification: true
			onModifiedChanged: if (layout.modified)
								   ldr.modified()

			QGridLabel { field: textQuestion }

			QGridTextField {
				id: textQuestion
				fieldName: qsTr("Kérdés")
				sqlField: "question"
				placeholderText: qsTr("Ez a kérdés fog megjelenni")
			}

			QGridLabel { field: textCorrectAnswer }

			QGridTextField {
				id: textCorrectAnswer
				fieldName: qsTr("Helyes válasz")
				sqlField: "correct"
				placeholderText: qsTr("Ez lesz a helyes válasz")
			}

			QGridLabel {
				field: areaAnswers
			}

			QGridTextArea {
				id: areaAnswers
				fieldName: qsTr("Helytelen válaszok")
				placeholderText: qsTr("Lehetséges helytelen válaszok (soronként)")
				minimumHeight: CosStyle.baseHeight*2
			}




			Component.onCompleted: {
				if (!moduleData)
					return

				JS.setSqlFields([textQuestion, textCorrectAnswer], moduleData)
				areaAnswers.setData(moduleData.answers.join("\n"))
			}


			function getData() {
				var d = JS.getSqlFields([textQuestion, textCorrectAnswer])
				d.answers = areaAnswers.text.split("\n")

				moduleData = d
				return moduleData
			}

		}*/

	}


	Component {
		id: cmpBinding

		QFormColumn {
			onModifiedChanged: if (objectiveEditor) objectiveEditor.modified = true

			QFormComboBox {
				id: _modeBinding
				text: qsTr("Kérdések készítése:")

				field: "mode"

				valueRole: "value"
				textRole: "text"

				model: [
					{value: "left", text: qsTr("Bal oldaliakhoz")},
					{value: "right", text: qsTr("Jobb oldaliakhoz nagyon hosszú lehetőség, hogy lássuk")}
				]
			}

			QFormTextField {
				id: _questionBinding
				title: qsTr("Kérdés")
				placeholderText: qsTr("Ez a kérdés fog megjelenni")
				helperText: qsTr("A \%1 jelöli a generált elem helyét")
				field: "question"
				width: parent.width
			}

			MapEditorSpinStorageCount {
				id: _countBinding
			}


			readonly property var _items: [_modeBinding, _questionBinding]

			function loadData() {
				setItems(_items, objective.data)
				_countBinding.value = objective.storageCount
			}

			function saveData() {
				objective.storageCount = _countBinding.value
				objective.data = getItems(_items)
			}
		}

	}

	/*
	Component {
		id: cmpImages

		Column {

			QGridLayout {
				id: layout3
				watchModification: true
				onModifiedChanged: if (layout3.modified)
									   ldr.modified()

				QGridText {
					field: comboModeImg
					text: qsTr("Kérdések készítése:")
				}

				QGridComboBox {
					id: comboModeImg
					sqlField: "mode"

					valueRole: "value"
					textRole: "text"

					model: [
						{value: "image", text: qsTr("Képhez (szövegekből választhat)")},
						{value: "text", text: qsTr("Szöveghez (képekből választhat)")}
					]

					onActivated: previewImg.refresh()
				}



				QGridLabel {
					field: textQuestionImgImg
					visible: textQuestionImgImg.visible
				}

				QGridTextField {
					id: textQuestionImgImg
					fieldName: qsTr("Kérdés")
					sqlField: "question"
					placeholderText: qsTr("Ez a kérdés fog megjelenni.")
					text: qsTr("Mit látsz a képen?")

					visible: comboModeImg.currentValue === "image"

					onTextModified: previewImg.refresh()
				}

				QGridLabel {
					field: textQuestionImgText
					visible: textQuestionImgText.visible
				}

				QGridTextField {
					id: textQuestionImgText
					fieldName: qsTr("Kérdés")
					sqlField: "question"
					placeholderText: qsTr("Ez a kérdés fog megjelenni. \%1 az összerendelésből kiválasztott tételre cserélődik ki.")
					text: qsTr("Melyik képen látható: %1?")

					visible: comboModeImg.currentValue === "text"

					onTextModified: previewImg.refresh()
				}

				QGridLabel {
					field: textAnswerImg
					visible: textAnswerImg.visible
				}

				QGridTextField {
					id: textAnswerImg
					fieldName: qsTr("Válaszok")
					sqlField: "answers"
					placeholderText: qsTr("A válaszok formátuma. \%1 az összerendelésből kiválasztott tételre cserélődik ki.")

					visible: comboModeImg.currentValue === "image"
					onTextModified: previewImg.refresh()
				}


				QGridText {
					text: qsTr("Feladatok száma:")
					field: spinCountImg
				}

				QGridSpinBox {
					id: spinCountImg
					from: 1
					to: 99
					editable: true

					onValueModified: {
						storageCount = value
					}
				}


				Component.onCompleted: {
					if (!moduleData)
						return

					JS.setSqlFields([comboModeImg, textQuestionImgImg, textQuestionImgText, textAnswerImg], moduleData)
					spinCountImg.setData(storageCount)

				}

			}

			MapEditorObjectivePreview {
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
		}
	}
*/

	function saveData() {
		if (status == Loader.Ready)
			item.saveData()
	}

	function loadData() {
		if (status == Loader.Ready)
			item.loadData()
	}
}



