import QtQuick 2.15
import QtQuick.Controls 2.15
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

	signal modified()

	Component {
		id: cmpNone

		QGridLayout {
			id: layout

			watchModification: true
			onModifiedChanged: if (layout.modified)
								   ldr.modified()


			QGridLabel { field: textQuestion }

			QGridTextField {
				id: textQuestion
				fieldName: qsTr("Állítás")
				sqlField: "question"
				placeholderText: qsTr("Ez az állítás fog megjelenni")
			}

			QGridCheckBox {
				id: chTrue
				text: qsTr("Az állítás IGAZ")
				sqlField: "correct"
			}



			Component.onCompleted: {
				if (!moduleData)
					return

				JS.setSqlFields([textQuestion, chTrue], moduleData)
			}


			function getData() {
				moduleData = JS.getSqlFields([textQuestion, chTrue])

				return moduleData
			}
		}

	}




	Component {
		id: cmpBinding

		Column {

			QGridLayout {
				id: layout2
				watchModification: true
				onModifiedChanged: if (layout2.modified)
									   ldr.modified()

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
						{value: "left", text: qsTr("Bal oldal: mind igaz, jobb: mind hamis")},
						{value: "right", text: qsTr("Jobb oldal: mind igaz, bal: mind hamis")},
						{value: "generateLeft", text: qsTr("Készítés a bal oldaliakhoz")},
						{value: "generateRight", text: qsTr("Készítés a jobb oldaliakhoz")}
					]

					onActivated: preview.refresh()
				}



				QGridLabel {
					field: textQuestion2
					visible: textQuestion2.visible
				}

				QGridTextField {
					id: textQuestion2
					fieldName: qsTr("Kérdés")
					sqlField: "question"
					placeholderText: qsTr("Ez a kérdés fog megjelenni. \%1 az összerendelésből a kérdés oldali, \%2 a válasz oldali részre cserélődik.")

					visible: comboMode.currentValue === "generateLeft" || comboMode.currentValue === "generateRight"

					onTextModified: preview.refresh()
				}


				QGridText {
					text: qsTr("Feladatok száma:")
					field: spinCount
				}

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

			MapEditorObjectivePreview {
				id: preview

				refreshFunc: function() { return mapEditor.objectiveGeneratePreview("truefalse", getData(), storageModule, storageData) }

				Connections {
					target: ldr
					function onStorageDataChanged() {
						preview.refresh()
					}
				}
			}



			function getData() {
				moduleData = JS.getSqlFields([comboMode, textQuestion2])

				return moduleData
			}
		}
	}







	Component {
		id: cmpNumbers

		Column {

			QGridLayout {
				id: layout3
				watchModification: true
				onModifiedChanged: if (layout3.modified)
									   ldr.modified()

				QGridText {
					field: comboMode2
					text: qsTr("Kérdések készítése:")
				}

				QGridComboBox {
					id: comboMode2
					sqlField: "mode"

					valueRole: "value"
					textRole: "text"

					model: [
						{value: "generateLeft", text: qsTr("Készítés a bal oldaliakhoz")},
						{value: "generateRight", text: qsTr("Készítés a jobb oldaliakhoz")}
					]

					onActivated: preview2.refresh()
				}



				QGridLabel {
					field: textQuestion3
				}

				QGridTextField {
					id: textQuestion3
					fieldName: qsTr("Kérdés")
					sqlField: "question"
					placeholderText: qsTr("Ez a kérdés fog megjelenni. \%1 az összerendelésből a kérdés oldali, \%2 a válasz oldali részre cserélődik.")

					onTextModified: preview2.refresh()
				}


				QGridText {
					text: qsTr("Feladatok száma:")
					field: spinCount2
				}

				QGridSpinBox {
					id: spinCount2
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

					JS.setSqlFields([comboMode2, textQuestion3], moduleData)
					spinCount2.setData(storageCount)

				}

			}

			MapEditorObjectivePreview {
				id: preview2

				refreshFunc: function() { return mapEditor.objectiveGeneratePreview("truefalse", getData(), storageModule, storageData) }

				Connections {
					target: ldr
					function onStorageDataChanged() {
						preview2.refresh()
					}
				}
			}



			function getData() {
				moduleData = JS.getSqlFields([comboMode2, textQuestion3])

				return moduleData
			}
		}
	}




	Component.onCompleted: {
		if (storageModule == "binding")
			ldr.sourceComponent = cmpBinding
		else if (storageModule == "numbers")
			ldr.sourceComponent = cmpNumbers
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

