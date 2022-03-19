import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS



Loader {
	id: ldr
	width: parent.width

	property MapEditor mapEditor: null

	property var moduleData: ({})
	property var storageData: ({})
	property string storageModule: ""
	property int storageCount: 0

	Component {
		id: cmpNone

		QGridLayout {
			watchModification: false

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

		}

	}


	Component {
		id: cmpBinding

		Column {


			QGridLayout {
				watchModification: false

				QGridText {
					field: comboMode
					text: qsTr("Kérdések készítése:")
				}

				QGridComboBox {
					id: comboMode
					sqlField: "mode"

					valueRole: "value"
					textRole: "text"

					model: [
						{value: "left", text: qsTr("Bal oldaliakhoz")},
						{value: "right", text: qsTr("Jobb oldaliakhoz")}
					]

					onActivated: textPreview.refresh()
				}



				QGridLabel { field: textQuestion2 }

				QGridTextField {
					id: textQuestion2
					fieldName: qsTr("Kérdés")
					sqlField: "question"
					placeholderText: qsTr("Ez a kérdés fog megjelenni. \%1 az összerendelésből kiválasztott tételre cserélődik ki.")

					onTextModified: textPreview.refresh()
				}


				QGridText { text: qsTr("Feladatok száma:") }

				QGridSpinBox {
					id: spinCount
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

					JS.setSqlFields([comboMode, textQuestion2], moduleData)
					spinCount.setData(storageCount)

				}

			}

			QCollapsible {
				id: collapsiblePreview

				title: qsTr("Előnézet")
				collapsed: true
				visible: mapEditor

				onCollapsedChanged: textPreview.refresh()

				QLabel {
					id: textPreview
					width: parent.width
					wrapMode: Text.Wrap
					textFormat: Text.MarkdownText
					leftPadding: 20
					rightPadding: 20

					Connections {
						target: ldr
						function onStorageDataChanged() {
							textPreview.refresh()
						}
					}

					function refresh() {
						if (collapsiblePreview.collapsed)
							return

						var d = mapEditor.objectiveGeneratePreview("simplechoice", getData(), storageModule, storageData)
						text = d.text
					}
				}
			}

			function getData() {
				moduleData = JS.getSqlFields([comboMode, textQuestion2])

				return moduleData
			}
		}
	}

	Component.onCompleted: {
		if (storageModule == "binding")
			ldr.sourceComponent = cmpBinding
		else
			ldr.sourceComponent = cmpNone
	}


	function getData() {
		if (ldr.status == Loader.Ready)
			return ldr.item.getData()

		return {}
	}

	function setStorageData(data) {
		storageData = data
	}

}



