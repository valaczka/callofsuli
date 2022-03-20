import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Item {
	id: control

	width: parent.width
	height: layout.height

	property var moduleData: ({})
	property var storageData: ({})
	property string storageModule: ""
	property int storageCount: 0

	signal modified()

	QGridLayout {
		id: layout

		watchModification: true
		onModifiedChanged: if (layout.modified)
							   control.modified()

		QGridLabel { field: textQuestion }

		QGridTextField {
			id: textQuestion
			fieldName: qsTr("Kérdés")
			sqlField: "question"
			placeholderText: qsTr("Ez a kérdés fog megjelenni")

			text: qsTr("Rendezd össze a párokat!")
		}


		QGridText {
			text: qsTr("Párok")
			field: fields
			visible: fields.visible
		}

		QGridDoubleTextFields {
			id: fields
			sqlField: "pairs"
		}

		QGridText {
			text: qsTr("Párok száma:")
			field: spinCount
		}

		QGridSpinBox {
			id: spinCount
			from: 2
			value: 4
			to: 99
			editable: true
			sqlField: "count"
		}


		QGridText {
			text: qsTr("Válaszlehetőségek száma:")
			field: spinOptions
		}

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
		if (storageModule == "binding")
			fields.visible = false

		if (!moduleData)
			return

		if (storageModule == "binding")
			JS.setSqlFields([textQuestion, spinCount, spinOptions, comboMode], moduleData)
		else
			JS.setSqlFields([textQuestion, fields, spinCount, spinOptions, comboMode], moduleData)

	}


	function getData() {
		if (storageModule == "binding")
			moduleData = JS.getSqlFields([textQuestion, spinCount, spinOptions, comboMode])
		else
			moduleData = JS.getSqlFields([textQuestion, fields, spinCount, spinOptions, comboMode])

		return moduleData
	}

	function setStorageData(data) {
		storageData = data
	}


}

