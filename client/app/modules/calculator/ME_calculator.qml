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
		id: cmpDefault

		QGridLayout {
			id: layoutDefault

			watchModification: true
			onModifiedChanged: if (layoutDefault.modified)
								   ldr.modified()


			QGridLabel { field: textQuestion }

			QGridTextField {
				id: textQuestion
				fieldName: qsTr("Kérdés")
				sqlField: "question"
				placeholderText: qsTr("Ez a kérdés fog megjelenni")
			}

			QGridLabel { field: textAnswer }

			QGridTextField {
				id: textAnswer
				fieldName: qsTr("Numerikus válasz")
				sqlField: "answer"
				sqlData: acceptableInput ? Number(text) : 0
				placeholderText: qsTr("Helyes válasz (szám)")

				IntValidator {
					id: validatorInt
					bottom: -999999
					top: 999999
				}

				DoubleValidator {
					id: validatorDouble
					bottom: -999999
					top: 999999
					decimals: 4
					notation: DoubleValidator.StandardNotation
					locale: "en_US"
				}

				validator: chDecimals.checked ? validatorDouble : validatorInt

				inputMethodHints: chDecimals.checked ? Qt.ImhFormattedNumbersOnly : Qt.ImhDigitsOnly
			}

			QGridLabel { field: textSuffix }

			QGridTextField {
				id: textSuffix
				fieldName: qsTr("Mértékegység")
				sqlField: "suffix"
				placeholderText: qsTr("Mértékegység (opcionális)")
			}


			QGridCheckBox {
				id: chDecimals
				text: qsTr("Tizedes számok engedélyezése")
				sqlField: "decimals"
			}

			Component.onCompleted: {
				if (!moduleData)
					return

				JS.setSqlFields([textQuestion, textAnswer, chDecimals, textSuffix], moduleData)
			}


			function getData() {
				moduleData = JS.getSqlFields([textQuestion, textAnswer, chDecimals, textSuffix])

				return moduleData
			}
		}

	}

	Component {
		id: cmpPlusminus

		Column {
			QGridLayout {
				id: layout

				watchModification: true
				onModifiedChanged: if (layout.modified)
									   ldr.modified()

				QGridText {
					text: qsTr("Művelet:")
					field: comboSubtract
				}

				QGridComboBox {
					id: comboSubtract
					sqlField: "subtract"

					valueRole: "value"
					textRole: "text"

					model: [
						{value: false, text: qsTr("Összeadás")},
						{value: true, text: qsTr("Kivonás")},
					]

					onActivated: preview.refresh()

				}



				QGridText {
					text: qsTr("Tartomány:")
					field: comboRange
				}

				QGridComboBox {
					id: comboRange
					sqlField: "range"

					valueRole: "value"
					textRole: "text"

					model: [
						{value: 1, text: qsTr("0-10 között")},
						{value: 2, text: qsTr("0-20 között")},
						{value: 3, text: qsTr("0-50 között")},
						{value: 4, text: qsTr("0-100 között")}
					]

					onActivated: preview.refresh()

				}



				QGridText {
					text: qsTr("Negatív számok:")
					field: comboNegative
				}

				QGridComboBox {
					id: comboNegative
					sqlField: "canNegative"

					valueRole: "value"
					textRole: "text"

					model: [
						{value: 0, text: qsTr("Semmi sem lehet negatív")},
						{value: 1, text: qsTr("Az eredmény lehet negatív")},
						{value: 2, text: qsTr("Az eredmény és a feladat is lehet negatív")},
					]

					onActivated: preview.refresh()
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

					onValueModified: storageCount = value
				}
			}


			MapEditorObjectivePreview {
				id: preview

				refreshFunc: function() { return mapEditor.objectiveGeneratePreview("calculator", getData(), storageModule, storageData) }

				Connections {
					target: ldr
					function onStorageDataChanged() {
						preview.refresh()
					}
				}
			}


			Component.onCompleted: {
				if (!moduleData)
					return

				JS.setSqlFields([comboSubtract, comboRange, comboNegative], moduleData)
				spinCount.setData(storageCount)

			}

			function getData() {
				moduleData = JS.getSqlFields([comboSubtract, comboRange, comboNegative])

				return moduleData
			}
		}
	}




	Component {
		id: cmpNumbers

		Column {
			QGridLayout {
				id: layoutNumbers

				watchModification: true
				onModifiedChanged: if (layoutNumbers.modified)
									   ldr.modified()


				QGridLabel { field: textQuestionN }

				QGridTextField {
					id: textQuestionN
					fieldName: qsTr("Kérdés")
					sqlField: "question"
					placeholderText: qsTr("A kérdés formátuma (%1 helyettesíti a bal oldalról választott elemet)")
					text: "%1"

					onTextModified: previewN.refresh()
				}

				QGridText {
					text: qsTr("Feladatok száma:")
					field: spinCountN
				}

				QGridSpinBox {
					id: spinCountN
					from: 1
					to: 99
					editable: true

					onValueModified: storageCount = value
				}

				QGridLabel { field: textSuffixN }

				QGridTextField {
					id: textSuffixN
					fieldName: qsTr("Mértékegység")
					sqlField: "suffix"
					placeholderText: qsTr("Mértékegység (opcionális)")
					onTextModified: previewN.refresh()
				}
			}

			MapEditorObjectivePreview {
				id: previewN

				refreshFunc: function() { return mapEditor.objectiveGeneratePreview("calculator", getData(), storageModule, storageData) }

				Connections {
					target: ldr
					function onStorageDataChanged() {
						previewN.refresh()
					}
				}
			}



			Component.onCompleted: {
				if (!moduleData)
					return

				JS.setSqlFields([textQuestionN, textSuffixN], moduleData)
				spinCountN.setData(storageCount)

			}

			function getData() {
				moduleData = JS.getSqlFields([textQuestionN, textSuffixN])

				return moduleData
			}
		}

	}




	Component.onCompleted: {
		if (storageModule == "plusminus")
			ldr.sourceComponent = cmpPlusminus
		else if (storageModule == "numbers")
			ldr.sourceComponent = cmpNumbers
		else
			ldr.sourceComponent = cmpDefault
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

