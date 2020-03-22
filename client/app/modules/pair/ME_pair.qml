import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QCollapsible {
	id: collapsible
	title: qsTr("Párosítás")

	property string moduleData: ""
	property string storageData: ""
	property string storageModule: ""
	property int storageCount: 0

	interactive: false

	QGridLayout {
		watchModification: false

		QGridLabel { field: textQuestion }

		QGridTextField {
			id: textQuestion
			fieldName: qsTr("Kérdés")
			sqlField: "question"
			placeholderText: qsTr("Ez a kérdés fog megjelenni")

			text: qsTr("Rendezd össze a párokat!")
		}


		QGridText { text: qsTr("Párok") }

		QGridDoubleTextFields {
			id: fields
			sqlField: "pairs"

		}

		QGridText { text: qsTr("Párok száma:") }

		QGridSpinBox {
			id: spinCount
			from: 2
			value: 4
			to: 99
			editable: true
			sqlField: "count"
		}


		QGridText { text: qsTr("Válaszlehetőségek száma:") }

		QGridSpinBox {
			id: spinOptions
			from: spinCount.value
			value: 5
			to: 99
			editable: true
			sqlField: "optionsCount"
		}

		QGridText {
			field: comboMode
			text: qsTr("Párok készítése:")
		}

		QGridComboBox {
			id: comboMode
			sqlField: "mode"

			valueRole: "value"
			textRole: "text"

			model: [
				{value: "first", text: qsTr("Bal oldaliakhoz")},
				{value: "second", text: qsTr("Jobb oldaliakhoz")},
				{value: "both", text: qsTr("Véletlenszerű oldaliakhoz")},
				{value: "shuffle", text: qsTr("Keverve")}
			]
		}

	}


	Component.onCompleted: {
		if (moduleData == "")
			return

		var d = JSON.parse(moduleData)

		JS.setSqlFields([textQuestion, fields, spinCount, spinOptions, comboMode], d)

	}


	function getData() {
		var d = JS.getSqlFields([textQuestion, fields, spinCount, spinOptions, comboMode])

		moduleData = JSON.stringify(d)

		console.debug("*******", moduleData)

		return moduleData
	}

}




