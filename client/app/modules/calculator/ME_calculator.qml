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
		id: cmpLabel

		QLabel {
			padding: 20
			width: parent.width
			color: CosStyle.colorErrorLighter
			wrapMode: Text.Wrap
			text: qsTr("Érvénytelen adatbank modul!")

			function getData() {
				return null
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

					onActivated: textPreview.refresh()

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

					onActivated: textPreview.refresh()

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

					onActivated: textPreview.refresh()
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

						var d = mapEditor.objectiveGeneratePreview("calculator", getData(), storageModule, storageData)
						text = d.text
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


	Component.onCompleted: {
		if (storageModule == "plusminus")
			ldr.sourceComponent = cmpPlusminus
		else
			ldr.sourceComponent = cmpLabel
	}


	function getData() {
		if (ldr.status == Loader.Ready)
			return ldr.item.getData()

		return {}
	}

}

