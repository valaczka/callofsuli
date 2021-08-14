import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QCollapsible {
	id: collapsible
	title: qsTr("Számítási feladat")

	interactive: false

	property string moduleData: ""
	property string storageData: ""
	property string storageModule: ""
	property int storageCount: 0

	QLabel {
		padding: 20
		width: parent.width
		color: CosStyle.colorErrorLighter
		wrapMode: Text.Wrap
		text: qsTr("Érvénytelen storage modul!")
		visible: ldr.sourceComponent == undefined
	}


	Loader {
		id: ldr
		width: parent.width
	}

	Component {
		id: cmpPlusminus

		QGridLayout {
			width: parent.width

			watchModification: false

			QGridText {
				text: qsTr("Művelet:")
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

				onActivated: getData()

			}



			QGridText {
				text: qsTr("Tartomány:")
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

				onActivated: getData()

			}



			QGridText {
				text: qsTr("Negatív számok:")
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

				onActivated: getData()

			}



			QGridText { text: qsTr("Feladatok száma:") }

			QGridSpinBox {
				id: spinCount
				from: 1
				to: 99
				editable: true

				onValueModified: storageCount = value
			}


			Component.onCompleted: {
				if (moduleData == "")
					return

				JS.setSqlFields([comboSubtract, comboRange, comboNegative], JSON.parse(moduleData))
				spinCount.setData(storageCount)

			}

			function getData() {
				moduleData = JSON.stringify(JS.getSqlFields([comboSubtract, comboRange, comboNegative]))

				return moduleData
			}
		}
	}


	Component.onCompleted: {
		if (storageModule == "plusminus")
			ldr.sourceComponent = cmpPlusminus
	}


	function getData() {
		if (ldr.status == Loader.Ready)
			return ldr.item.getData()

		return ""
	}

}

