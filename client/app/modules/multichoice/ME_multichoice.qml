import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Item {
	width: parent.width
	height: layout.height

	property var moduleData: ({})
	property var storageData: ({})
	property string storageModule: ""
	property int storageCount: 0

	QGridLayout {
		id: layout

		watchModification: false

		QGridLabel { field: textQuestion }

		QGridTextField {
			id: textQuestion
			fieldName: qsTr("Kérdés")
			sqlField: "question"
			placeholderText: qsTr("Ez a kérdés fog megjelenni")
		}

		QGridLabel { field: areaCorrectAnswers }

		QGridTextArea {
			id: areaCorrectAnswers
			fieldName: qsTr("Helyes válaszok")

			placeholderText: qsTr("A helyes válaszok (soronként)")
			minimumHeight: CosStyle.baseHeight*2
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

		QGridText { text: qsTr("Min. helyes válasz:") }

		QGridSpinBox {
			id: spinCourrectMin
			from: 2
			value: 2
			to: 99
			editable: true
			sqlField: "correctMin"
		}

		QGridText { text: qsTr("Max. helyes válasz:") }

		QGridSpinBox {
			id: spinCourrectMax
			from: spinCourrectMin.value
			value: 4
			to: 99
			editable: true
			sqlField: "correctMax"
		}

		QGridText { text: qsTr("Max. lehetőség:") }

		QGridSpinBox {
			id: spinCount
			from: spinCourrectMax.value+1
			value: 5
			to: 99
			editable: true
			sqlField: "count"
		}

	}

	Component.onCompleted: {
		if (!moduleData)
			return

		JS.setSqlFields([textQuestion, spinCount, spinCourrectMin, spinCourrectMax], moduleData)
		areaCorrectAnswers.setData(moduleData.corrects.join("\n"))
		areaAnswers.setData(moduleData.answers.join("\n"))
	}


	function getData() {
		var d = JS.getSqlFields([textQuestion, spinCount, spinCourrectMin, spinCourrectMax])
		d.corrects = areaCorrectAnswers.text.split("\n")
		d.answers = areaAnswers.text.split("\n")

		moduleData = d
		return moduleData
	}

}




