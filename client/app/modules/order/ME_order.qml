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
		id: cmpSequence

		QGridLayout {
			id: layoutS

			watchModification: true
			onModifiedChanged: if (layoutS.modified)
								   ldr.modified()


			QGridLabel {
				field: areaItems
				visible: areaItems.visible
			}

			QGridTextArea {
				id: areaItems
				fieldName: qsTr("Elemek")

				placeholderText: qsTr("A sorozat elemei (soronként) növekvő sorrendben ")
				minimumHeight: CosStyle.baseHeight*2

				onTextModified: getData()
			}

			QGridText {
				text: qsTr("Elemek száma:")
				field: spinCount
			}

			QGridSpinBox {
				id: spinCount
				from: 2
				value: 5
				to: 99
				editable: true
				sqlField: "count"
			}


			QGridText {
				field: comboMode
				text: qsTr("Sorrend:")
			}

			QGridComboBox {
				id: comboMode
				sqlField: "mode"

				valueRole: "value"
				textRole: "text"

				model: [
					{value: "ascending", text: qsTr("Növekvő sorrend")},
					{value: "descending", text: qsTr("Csökkenő sorrend")},
					{value: "random", text: qsTr("Véletlenszerű sorrend")}
				]
			}

			QGridLabel {
				field: textQuestionAsc
				visible: textQuestionAsc.visible
			}

			QGridTextField {
				id: textQuestionAsc
				fieldName: qsTr("Kérdés (növekvő)")
				sqlField: "questionAsc"
				placeholderText: qsTr("Ez a kérdés fog megjelenni növekvő sorrend esetén")

				text: qsTr("Rendezd növekvő sorrendbe!")

				visible: comboMode.currentValue === "ascending" || comboMode.currentValue === "random"
			}

			QGridLabel {
				field: textQuestionDesc
				visible: textQuestionDesc.visible
			}

			QGridTextField {
				id: textQuestionDesc
				fieldName: qsTr("Kérdés (csökkenő)")
				sqlField: "questionDesc"
				placeholderText: qsTr("Ez a kérdés fog megjelenni csökkenő sorrend esetén")

				text: qsTr("Rendezd csökkenő sorrendbe!")

				visible: comboMode.currentValue === "descending" || comboMode.currentValue === "random"
			}


			QGridLabel { field: textPlaceholderMin }

			QGridTextField {
				id: textPlaceholderMin
				fieldName: qsTr("Segítő jelzés (min.)")
				sqlField: "placeholderMin"
				placeholderText: qsTr("Ezt jeleníti meg azon a helyen, ahova a legkisebb értéket kell helyezni")

				text: qsTr("legkisebb")
			}

			QGridLabel { field: textPlaceholderMax }

			QGridTextField {
				id: textPlaceholderMax
				fieldName: qsTr("Segítő jelzés (max.)")
				sqlField: "placeholderMax"
				placeholderText: qsTr("Ezt jeleníti meg azon a helyen, ahova a legnagyobb értéket kell helyezni")

				text: qsTr("legnagyobb")
			}


			QGridText {
				text: qsTr("Feladatok száma:")
				field: sCount
				visible: sCount.visible
			}

			QGridSpinBox {
				id: sCount
				from: 1
				to: 99
				editable: true

				onValueModified: {
					storageCount = value
				}
			}



			Component.onCompleted: {
				if (storageModule == "sequence" || storageModule == "numbers") {
					areaItems.visible = false
				} else {
					sCount.visible = false
				}

				if (!moduleData)
					return

				JS.setSqlFields([spinCount, comboMode, textQuestionAsc, textQuestionDesc, textPlaceholderMax, textPlaceholderMin], moduleData)

				if (storageModule == "sequence" || storageModule == "numbers") {
					sCount.setData(storageCount)
				} else {
					areaItems.setData(moduleData.items.join("\n"))
				}
			}


			function getData() {
				var d = JS.getSqlFields([spinCount, comboMode, textQuestionAsc, textQuestionDesc, textPlaceholderMax, textPlaceholderMin])

				if (storageModule == "sequence" || storageModule == "numbers") {

				} else {
					d.items = areaItems.text.split("\n")
				}

				moduleData = d

				return moduleData
			}
		}

	}


	Component.onCompleted: {
		ldr.sourceComponent = cmpSequence
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

