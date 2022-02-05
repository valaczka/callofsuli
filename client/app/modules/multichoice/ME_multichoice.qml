import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QCollapsible {
	id: collapsible
	title: qsTr("Többszörös választás")

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

			onTextModified: getData()
		}

		QGridLabel { field: areaCorrectAnswers }

		QGridTextArea {
			id: areaCorrectAnswers
			fieldName: qsTr("Helyes válaszok")

			placeholderText: qsTr("A helyes válaszok (soronként)")
			minimumHeight: CosStyle.baseHeight*2

			onTextModified: getData()
		}

		QGridLabel {
			field: areaAnswers
		}

		QGridTextArea {
			id: areaAnswers
			fieldName: qsTr("Helytelen válaszok")
			placeholderText: qsTr("Lehetséges helytelen válaszok (soronként)")
			minimumHeight: CosStyle.baseHeight*2

			onTextModified: getData()
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
		if (moduleData == "")
			return

		var d = JSON.parse(moduleData)

		JS.setSqlFields([textQuestion, spinCount, spinCourrectMin, spinCourrectMax], d)
		areaCorrectAnswers.setData(d.corrects.join("\n"))
		areaAnswers.setData(d.answers.join("\n"))
	}


	function getData() {
		var d = JS.getSqlFields([textQuestion, spinCount, spinCourrectMin, spinCourrectMax])
		d.corrects = areaCorrectAnswers.text.split("\n")
		d.answers = areaAnswers.text.split("\n")

		moduleData = JSON.stringify(d)
		return moduleData
	}

}



